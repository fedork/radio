#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// Exact bounded-depth search for a strategy whose frontier consists entirely of
// singleton rectangles.  With depth == k it is exact solvability: singleton
// terminals are decided by majorization, and an R_0 state at k == 0 has <= 1 edge.
// It is specialized to the fixed-small-m, near-2^k regime: full-star
// majorization bounds every wide-side cut to a short interval.

namespace {

constexpr int MAX_PARTS = 12;
constexpr std::size_t MAX_MEMO = 8'000'000;
constexpr std::array<int, 3> SEARCH_OUTCOMES{2, 1, 0};

struct Part {
    int n;
    int m;

    friend bool operator==(const Part &, const Part &) = default;
};

using State = std::vector<Part>;

struct Children {
    // Printed/checker order: both defectives selected, mixed, neither selected.
    std::array<State, 3> state;
};

struct Option {
    int a;
    int b;
    Children children;
};

struct Key {
    std::array<std::uint16_t, MAX_PARTS> parts{};
    std::uint8_t size{};
    std::uint8_t k{};
    std::uint8_t depth{};

    friend bool operator==(const Key &, const Key &) = default;
};

struct KeyHash {
    std::size_t operator()(const Key &key) const noexcept {
        std::size_t h = 1469598103934665603ULL;
        auto mix = [&](std::size_t value) {
            h ^= value;
            h *= 1099511628211ULL;
        };
        mix(key.size);
        mix(key.k);
        mix(key.depth);
        for (int i = 0; i < key.size; ++i) mix(key.parts[i]);
        return h;
    }
};

struct Split {
    int a;
    int b;
};

struct Counters {
    std::uint64_t calls{};
    std::uint64_t memo_hits{};
    std::uint64_t options{};
    std::uint64_t assignments{};
    std::uint64_t prefix_rejects{};
};

std::vector<std::vector<int>> bases;
std::unordered_map<Key, bool, KeyHash> memo;
std::unordered_map<Key, std::vector<Split>, KeyHash> witnesses;
Counters counters;

void normalize(State &state) {
    State out;
    out.reserve(state.size());
    for (Part part : state) {
        if (part.n <= 0 || part.m <= 0) continue;
        if (part.n < part.m) std::swap(part.n, part.m);
        out.push_back(part);
    }
    std::sort(out.begin(), out.end(), [](Part left, Part right) {
        if (left.n != right.n) return left.n > right.n;
        return left.m > right.m;
    });
    if (out.size() > MAX_PARTS) throw std::runtime_error("state exceeds MAX_PARTS");
    state = std::move(out);
}

Key make_key(const State &state, int k, int depth) {
    Key key;
    key.size = static_cast<std::uint8_t>(state.size());
    key.k = static_cast<std::uint8_t>(k);
    key.depth = static_cast<std::uint8_t>(depth);
    for (std::size_t i = 0; i < state.size(); ++i) {
        if (state[i].n >= 4096 || state[i].m >= 16)
            throw std::runtime_error("part does not fit key encoding");
        key.parts[i] = static_cast<std::uint16_t>((state[i].n << 4) | state[i].m);
    }
    return key;
}

std::int64_t power_int(int base, int exponent) {
    std::int64_t value = 1;
    while (exponent-- > 0) value *= base;
    return value;
}

void make_bases(int max_k) {
    bases.assign(max_k + 1, {});
    bases[0] = {1};
    for (int k = 1; k <= max_k; ++k) {
        const auto &old = bases[k - 1];
        std::vector<int> next(2 * old.size());
        for (std::size_t i = 0; i < old.size(); ++i) {
            next[i] += old[i];
            next[2 * i] += old[i];
            next[2 * i + 1] += old[i];
        }
        std::sort(next.begin(), next.end(), std::greater<>());
        bases[k] = std::move(next);
    }
}

bool r0(const State &state, int k) {
    std::int64_t mass = 0;
    std::vector<int> profile;
    for (Part part : state) {
        mass += static_cast<std::int64_t>(part.n) * part.m;
        for (int j = 0; j < part.m; ++j) profile.push_back(part.n);
    }
    if (mass > power_int(3, k)) return false;
    std::sort(profile.begin(), profile.end(), std::greater<>());
    const auto &g = bases[k];
    std::int64_t left = 0;
    std::int64_t right = 0;
    const std::size_t compared = std::min(profile.size(), g.size());
    for (std::size_t i = 0; i < compared; ++i) {
        left += profile[i];
        right += g[i];
        if (left > right) return false;
    }
    return true;
}

bool singleton(const State &state) {
    return std::all_of(state.begin(), state.end(), [](Part part) { return part.m == 1; });
}

Children split_part(Part part, int a, int b) {
    Children out;
    out.state[0] = {{a, b}};
    out.state[1] = {{a, part.m - b}, {part.n - a, b}};
    out.state[2] = {{part.n - a, part.m - b}};
    for (State &child : out.state) normalize(child);
    return out;
}

State add_states(const State &left, const State &right) {
    State out = left;
    out.insert(out.end(), right.begin(), right.end());
    normalize(out);
    return out;
}

bool construct(const State &state, int k, int depth);

auto option_score(Part part, const Option &option) {
    std::array<std::int64_t, 3> masses{};
    for (int outcome = 0; outcome < 3; ++outcome)
        for (Part child : option.children.state[outcome])
            masses[outcome] += static_cast<std::int64_t>(child.n) * child.m;
    auto [minimum, maximum] = std::minmax_element(masses.begin(), masses.end());
    return std::array<std::int64_t, 6>{
        *maximum, *maximum - *minimum,
        masses[0] * masses[0] + masses[1] * masses[1] + masses[2] * masses[2],
        std::llabs(2LL * option.a - part.n), option.b, option.a};
}

std::vector<Option> raw_options(Part part, int k) {
    std::vector<Option> out;
    const int child_max = 1 << (k - 1);
    // Every a-side piece occurs opposite a positive narrow-side piece in at least
    // one outcome (and likewise n-a).  R_0 therefore forces both <= max(G_(k-1)).
    const int low = std::max(0, part.n - child_max);
    const int high = std::min(part.n, child_max);
    for (int b = 0; b <= part.m; ++b) {
        for (int a = low; a <= high; ++a) {
            ++counters.options;
            Children children = split_part(part, a, b);
            out.push_back({a, b, std::move(children)});
        }
    }
    std::sort(out.begin(), out.end(), [part](const Option &left, const Option &right) {
        return option_score(part, left) < option_score(part, right);
    });
    return out;
}

std::vector<Option> part_options(Part part, int k, int depth) {
    std::vector<Option> out;
    for (Option option : raw_options(part, k)) {
        bool possible = true;
        for (int outcome : SEARCH_OUTCOMES) {
            const State &child = option.children.state[outcome];
            if (!construct(child, k - 1, depth - 1)) {
                possible = false;
                break;
            }
        }
        if (possible) out.push_back(std::move(option));
    }
    return out;
}

bool construct(const State &state, int k, int depth) {
    ++counters.calls;
    const Key key = make_key(state, k, depth);
    if (auto found = memo.find(key); found != memo.end()) {
        ++counters.memo_hits;
        return found->second;
    }
    if (memo.size() >= MAX_MEMO) throw std::runtime_error("memo limit reached (abort, not NO)");
    if (!r0(state, k)) {
        memo.emplace(key, false);
        return false;
    }
    if (state.empty() || singleton(state)) {
        memo.emplace(key, true);
        return true;
    }
    if (depth == 0 || k == 0) {
        memo.emplace(key, false);
        return false;
    }

    // A one-part state needs no Cartesian-product setup.  Test balanced cuts
    // lazily so a positive frontier query returns as soon as it has a witness.
    if (state.size() == 1) {
        for (const Option &option : raw_options(state.front(), k)) {
            ++counters.assignments;
            bool possible = true;
            for (int outcome : SEARCH_OUTCOMES) {
                const State &child = option.children.state[outcome];
                if (!construct(child, k - 1, depth - 1)) {
                    possible = false;
                    break;
                }
            }
            if (possible) {
                memo.emplace(key, true);
                witnesses.emplace(key, std::vector<Split>{{option.a, option.b}});
                return true;
            }
            ++counters.prefix_rejects;
        }
        memo.emplace(key, false);
        return false;
    }

    struct Candidate {
        int original;
        Part part;
        std::vector<Option> options;
    };
    std::vector<Candidate> candidates;
    candidates.reserve(state.size());
    for (std::size_t i = 0; i < state.size(); ++i) {
        auto options = part_options(state[i], k, depth);
        if (options.empty()) {
            memo.emplace(key, false);
            return false;
        }
        candidates.push_back({static_cast<int>(i), state[i], std::move(options)});
    }
    std::stable_sort(candidates.begin(), candidates.end(), [](const Candidate &left,
                                                              const Candidate &right) {
        if (left.options.size() != right.options.size())
            return left.options.size() < right.options.size();
        if (left.part.n != right.part.n) return left.part.n > right.part.n;
        return left.part.m > right.part.m;
    });

    std::array<State, 3> partial;
    std::vector<Split> selected(candidates.size());
    std::vector<Split> selected_original(candidates.size());

    auto dfs = [&](auto &&self, int index) -> bool {
        if (index == static_cast<int>(candidates.size())) return true;
        const Candidate &candidate = candidates[index];
        for (const Option &option : candidate.options) {
            if (index > 0 && candidates[index - 1].part == candidate.part) {
                const Split previous = selected[index - 1];
                if (std::pair(option.a, option.b) < std::pair(previous.a, previous.b)) continue;
            }
            ++counters.assignments;
            std::array<State, 3> next;
            bool possible = true;
            for (int outcome : SEARCH_OUTCOMES) {
                // A complete strategy for the eventual child would restrict to a
                // strategy for this partial subgraph, so failure is a sound prefix cut.
                next[outcome] = add_states(partial[outcome], option.children.state[outcome]);
                if (!construct(next[outcome], k - 1, depth - 1)) {
                    possible = false;
                    break;
                }
            }
            if (!possible) {
                ++counters.prefix_rejects;
                continue;
            }
            const auto saved = partial;
            partial = std::move(next);
            selected[index] = {option.a, option.b};
            selected_original[candidate.original] = {option.a, option.b};
            if (self(self, index + 1)) return true;
            partial = saved;
        }
        return false;
    };

    const bool answer = dfs(dfs, 0);
    memo.emplace(key, answer);
    if (answer) witnesses.emplace(key, std::move(selected_original));
    return answer;
}

void print_state(const State &state) {
    if (state.empty()) {
        std::cout << "0:0";
        return;
    }
    for (std::size_t i = 0; i < state.size(); ++i) {
        if (i) std::cout << ',';
        std::cout << state[i].n << ':' << state[i].m;
    }
}

void print_tree(const State &state, int k, int depth, int indent = 0) {
    std::cout << std::string(indent, ' ');
    print_state(state);
    std::cout << " @" << k;
    if (state.empty() || singleton(state)) {
        std::cout << " [majorized G_" << k << "]\n";
        return;
    }
    const Key key = make_key(state, k, depth);
    const auto found = witnesses.find(key);
    if (found == witnesses.end()) {
        std::cout << " [missing witness]\n";
        return;
    }
    std::cout << " --[";
    for (std::size_t i = 0; i < found->second.size(); ++i) {
        if (i) std::cout << ',';
        std::cout << found->second[i].a << ':' << found->second[i].b;
    }
    std::cout << "]-->\n";
    std::array<State, 3> children;
    for (std::size_t i = 0; i < state.size(); ++i) {
        Children local = split_part(state[i], found->second[i].a, found->second[i].b);
        for (int outcome = 0; outcome < 3; ++outcome)
            children[outcome] = add_states(children[outcome], local.state[outcome]);
    }
    for (const State &child : children) print_tree(child, k - 1, depth - 1, indent + 2);
}

}  // namespace

int main(int argc, char **argv) {
    if (argc == 8 && std::string(argv[1]) == "forced") {
        const int k = std::atoi(argv[2]);
        const int depth = std::atoi(argv[3]);
        State state{{std::atoi(argv[4]), std::atoi(argv[5])}};
        const Split split{std::atoi(argv[6]), std::atoi(argv[7])};
        normalize(state);
        if (k <= 0 || depth <= 0 || depth > k || state.size() != 1 ||
            split.a < 0 || split.a > state[0].n || split.b < 0 || split.b > state[0].m) {
            std::cerr << "usage: " << argv[0] << " forced k depth n m a b\n";
            return 2;
        }
        make_bases(k);
        const auto started = std::chrono::steady_clock::now();
        try {
            const Children children = split_part(state[0], split.a, split.b);
            bool answer = r0(state, k);
            for (int outcome : SEARCH_OUTCOMES)
                answer = answer && construct(children.state[outcome], k - 1, depth - 1);
            const double seconds = std::chrono::duration<double>(
                                       std::chrono::steady_clock::now() - started)
                                       .count();
            std::cout << "forced=" << (answer ? "YES" : "NO") << " wall=" << seconds
                      << "s calls=" << counters.calls << " assignments=" << counters.assignments
                      << " memo=" << memo.size() << '\n';
            if (answer) {
                const Key key = make_key(state, k, depth);
                memo[key] = true;
                witnesses[key] = {split};
                print_tree(state, k, depth);
            }
            return answer ? 0 : 1;
        } catch (const std::exception &error) {
            std::cerr << "ABORT: " << error.what() << " (not a negative verdict)\n";
            return 3;
        }
    }
    if (argc == 6 && std::string(argv[1]) == "frontier") {
        const int k = std::atoi(argv[2]);
        const int m = std::atoi(argv[3]);
        const int start = std::atoi(argv[4]);
        const int minimum = std::atoi(argv[5]);
        if (k < 0 || m <= 0 || minimum < 0 || start < minimum) {
            std::cerr << "usage: " << argv[0] << " frontier k m start_n minimum_n\n";
            return 2;
        }
        make_bases(k);
        try {
            for (int n = start; n >= minimum; --n) {
                State state{{n, m}};
                normalize(state);
                const Counters before = counters;
                const auto started = std::chrono::steady_clock::now();
                const bool answer = construct(state, k, k);
                const double seconds = std::chrono::duration<double>(
                                           std::chrono::steady_clock::now() - started)
                                           .count();
                std::cout << "n=" << n << ' ' << (answer ? "YES" : "NO")
                          << " wall=" << seconds << "s calls=" << counters.calls - before.calls
                          << " assignments=" << counters.assignments - before.assignments
                          << " memo=" << memo.size() << '\n'
                          << std::flush;
                if (answer) {
                    print_tree(state, k, k);
                    return 0;
                }
            }
            return 1;
        } catch (const std::exception &error) {
            std::cerr << "ABORT: " << error.what() << " (not a negative verdict)\n";
            return 3;
        }
    }
    if (argc < 5 || ((argc - 3) % 2) != 0) {
        std::cerr << "usage: " << argv[0] << " k depth n1 m1 [n2 m2 ...]\n"
                  << "       " << argv[0] << " forced k depth n m a b\n"
                  << "       " << argv[0] << " frontier k m start_n minimum_n\n";
        return 2;
    }
    const int k = std::atoi(argv[1]);
    const int depth = std::atoi(argv[2]);
    if (k < 0 || depth < 0 || depth > k) {
        std::cerr << "require 0 <= depth <= k\n";
        return 2;
    }
    State state;
    for (int i = 3; i < argc; i += 2) state.push_back({std::atoi(argv[i]), std::atoi(argv[i + 1])});
    normalize(state);
    make_bases(k);

    const auto started = std::chrono::steady_clock::now();
    try {
        const bool answer = construct(state, k, depth);
        const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
        std::cout << "singletonize_" << depth << '=' << (answer ? "YES" : "NO") << " k=" << k
                  << " state=";
        print_state(state);
        std::cout << " wall=" << seconds << "s calls=" << counters.calls
                  << " memo_hits=" << counters.memo_hits << " memo=" << memo.size()
                  << " options=" << counters.options << " assignments=" << counters.assignments
                  << " prefix_rejects=" << counters.prefix_rejects << '\n';
        if (answer) print_tree(state, k, depth);
        return answer ? 0 : 1;
    } catch (const std::exception &error) {
        std::cerr << "ABORT: " << error.what() << " (not a negative verdict)\n";
        return 3;
    }
}
