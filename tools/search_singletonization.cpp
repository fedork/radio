#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
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
#ifndef SINGLETONIZATION_MAX_MEMO
#define SINGLETONIZATION_MAX_MEMO 8000000
#endif
constexpr std::size_t MAX_MEMO = SINGLETONIZATION_MAX_MEMO;
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

struct ParetoPoint {
    int k;
    int m;
    int n;
    std::string status;
    std::string source;
};

struct AssemblyTriple {
    ParetoPoint a;
    ParetoPoint b;
    ParetoPoint c;
    State fixed;
    int d_upper;
    int width_upper;
};

struct AssemblyWinner {
    AssemblyTriple triple;
    int d;
    State branch;
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

std::vector<std::string> parse_csv_line(const std::string &line) {
    std::vector<std::string> fields;
    std::string field;
    bool quoted = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char ch = line[i];
        if (quoted) {
            if (ch == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    field.push_back('"');
                    ++i;
                } else {
                    quoted = false;
                }
            } else {
                field.push_back(ch);
            }
        } else if (ch == ',') {
            fields.push_back(std::move(field));
            field.clear();
        } else if (ch == '"' && field.empty()) {
            quoted = true;
        } else {
            field.push_back(ch);
        }
    }
    if (quoted) throw std::runtime_error("unterminated quoted CSV field");
    fields.push_back(std::move(field));
    return fields;
}

int parse_nonnegative_int(const std::string &text, const std::string &field, int line_number) {
    if (text.empty())
        throw std::runtime_error("empty " + field + " at CSV line " +
                                 std::to_string(line_number));
    std::size_t consumed = 0;
    const long value = std::stol(text, &consumed);
    if (consumed != text.size() || value < 0 || value > 2147483647L)
        throw std::runtime_error("invalid " + field + " at CSV line " +
                                 std::to_string(line_number));
    return static_cast<int>(value);
}

std::map<int, std::vector<ParetoPoint>> load_proven_pareto(
    const std::string &path, std::map<int, bool> &incomplete_levels) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("cannot open Pareto CSV: " + path);

    std::string line;
    if (!std::getline(input, line)) throw std::runtime_error("empty Pareto CSV: " + path);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    const auto header = parse_csv_line(line);
    std::map<std::string, std::size_t> column;
    for (std::size_t i = 0; i < header.size(); ++i) {
        if (!column.emplace(header[i], i).second)
            throw std::runtime_error("duplicate Pareto CSV column: " + header[i]);
    }
    for (const std::string required : {"k", "m", "n1", "bound", "status", "source"})
        if (!column.contains(required))
            throw std::runtime_error("missing Pareto CSV column: " + required);

    std::map<int, std::vector<ParetoPoint>> table;
    int line_number = 1;
    while (std::getline(input, line)) {
        ++line_number;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        const auto fields = parse_csv_line(line);
        if (fields.size() != header.size())
            throw std::runtime_error("wrong field count at CSV line " +
                                     std::to_string(line_number));
        auto get = [&](const std::string &name) -> const std::string & {
            return fields[column.at(name)];
        };
        const int row_k = parse_nonnegative_int(get("k"), "k", line_number);
        const bool proven_status =
            get("status") == "proven-exhaustive" || get("status") == "proven-theorem";
        if (get("bound") != "max" || !proven_status) {
            incomplete_levels[row_k] = true;
            continue;
        }
        if (get("source").empty())
            throw std::runtime_error("proven maximum lacks source at CSV line " +
                                     std::to_string(line_number));
        ParetoPoint point{row_k,
                          parse_nonnegative_int(get("m"), "m", line_number),
                          parse_nonnegative_int(get("n1"), "n1", line_number),
                          get("status"), get("source")};
        if (point.k == 0 || point.m == 0 || point.n == 0)
            throw std::runtime_error("zero dimension at CSV line " +
                                     std::to_string(line_number));
        table[point.k].push_back(std::move(point));
    }
    return table;
}

const std::vector<ParetoPoint> &complete_pareto_level(
    std::map<int, std::vector<ParetoPoint>> &table,
    const std::map<int, bool> &incomplete_levels, int k) {
    if (incomplete_levels.contains(k))
        throw std::runtime_error("Pareto level " + std::to_string(k) +
                                 " contains non-max or non-proven rows");
    auto found = table.find(k);
    if (found == table.end())
        throw std::runtime_error("no proven Pareto maxima for level " + std::to_string(k));
    auto &points = found->second;
    std::sort(points.begin(), points.end(), [](const ParetoPoint &left, const ParetoPoint &right) {
        return left.m < right.m;
    });
    for (std::size_t i = 0; i < points.size(); ++i) {
        const int expected_m = static_cast<int>(i) + 1;
        if (points[i].m != expected_m)
            throw std::runtime_error("proven Pareto level " + std::to_string(k) +
                                     " is not contiguous through the normalized endpoint");
        if (points[i].n < points[i].m)
            throw std::runtime_error("unnormalized Pareto row at level " + std::to_string(k) +
                                     ", height " + std::to_string(points[i].m));
        if (points[i].n > power_int(2, k) ||
            static_cast<std::int64_t>(points[i].n) * points[i].m > power_int(3, k))
            throw std::runtime_error("Pareto row exceeds a theorem capacity at level " +
                                     std::to_string(k) + ", height " +
                                     std::to_string(points[i].m));
        if (i > 0 && points[i].n > points[i - 1].n)
            throw std::runtime_error("Pareto widths increase with height at level " +
                                     std::to_string(k));
    }
    const ParetoPoint &last = points.back();
    // The CSV stores the unordered Pareto antichain only through its normalized
    // near-diagonal endpoint.  This schema check rejects an obvious truncated level;
    // the max status and source on each row remain the evidence for the frontier.
    if (last.n > last.m + 1)
        throw std::runtime_error("proven Pareto level " + std::to_string(k) +
                                 " stops before the normalized endpoint");
    return points;
}

State assembly_branch(const State &fixed, int d, int beta) {
    State branch = fixed;
    branch.push_back({d, beta});
    normalize(branch);
    return branch;
}

int assembly_r0_upper(const State &fixed, int beta, int residual_k) {
    const int legal_upper = static_cast<int>(power_int(2, residual_k));
    if (!r0(assembly_branch(fixed, 0, beta), residual_k)) return -1;
    if (r0(assembly_branch(fixed, legal_upper, beta), residual_k)) return legal_upper;
    int low = 0;
    int high = legal_upper;
    while (low + 1 < high) {
        const int middle = low + (high - low) / 2;
        if (r0(assembly_branch(fixed, middle, beta), residual_k))
            low = middle;
        else
            high = middle;
    }
    return low;
}

void print_pareto_point(const char *role, const ParetoPoint &point) {
    std::cout << "assembly_pareto role=" << role << " k=" << point.k << " state=" << point.n
              << ':' << point.m << " status=" << point.status << " source=" << point.source
              << '\n';
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
    if (argc >= 2 && (std::string(argv[1]) == "assembly-enumerate" ||
                      std::string(argv[1]) == "assembly-rank")) {
        const bool rank_only = std::string(argv[1]) == "assembly-rank";
        if (argc != 5) {
            std::cerr << "usage: " << argv[0]
                      << " " << argv[1] << " parent_k total_m pareto_csv\n";
            return 2;
        }
        const int parent_k = std::atoi(argv[2]);
        const int total_m = std::atoi(argv[3]);
        const int residual_k = parent_k - 2;
        if (parent_k < 3 || parent_k > 12 || total_m <= 0 ||
            (!rank_only && total_m >= 16)) {
            std::cerr << "usage: " << argv[0]
                      << " " << argv[1] << " parent_k total_m pareto_csv\n"
                      << "require 3<=parent_k<=12 and total_m>0; exact enumeration also"
                         " requires total_m<16 because of the exact-state key\n";
            return 2;
        }

        try {
            std::map<int, bool> incomplete_levels;
            auto table = load_proven_pareto(argv[4], incomplete_levels);
            const auto &a_points =
                complete_pareto_level(table, incomplete_levels, parent_k - 1);
            const auto &bc_points =
                complete_pareto_level(table, incomplete_levels, parent_k - 2);
            make_bases(residual_k);
            const ParetoPoint *known_parent = nullptr;
            if (auto found = table.find(parent_k); found != table.end()) {
                for (const ParetoPoint &point : found->second)
                    if (point.m == total_m) known_parent = &point;
            }

            std::cout << (rank_only ? "assembly_ranking" : "assembly_enumeration")
                      << " parent_k=" << parent_k
                      << " residual_k=" << residual_k << " total_m=" << total_m
                      << " pareto_csv=" << argv[4]
                      << " pareto_inputs=PROVEN_MAX_COMPLETE"
                         " working_m_le_2a=ENFORCED_UNPROVEN\n";
            for (const ParetoPoint &point : a_points) print_pareto_point("A", point);
            for (const ParetoPoint &point : bc_points) print_pareto_point("BC", point);

            const std::uint64_t cartesian = static_cast<std::uint64_t>(a_points.size()) *
                                            bc_points.size() * bc_points.size();
            std::uint64_t beta_failures = 0;
            std::uint64_t height_failures = 0;
            std::uint64_t c_width_failures = 0;
            std::uint64_t working_height_failures = 0;
            std::uint64_t admissible = 0;
            std::uint64_t r0_infeasible = 0;
            std::vector<AssemblyTriple> triples;

            for (const ParetoPoint &a : a_points) {
                for (const ParetoPoint &b : bc_points) {
                    for (const ParetoPoint &c : bc_points) {
                        const bool beta_ok = b.m <= a.m;
                        const bool height_ok = a.m + c.m <= total_m;
                        const bool c_width_ok = c.n <= a.n;
                        const bool working_height_ok =
                            static_cast<std::int64_t>(total_m) <= 2LL * a.n;
                        beta_failures += !beta_ok;
                        height_failures += !height_ok;
                        c_width_failures += !c_width_ok;
                        working_height_failures += !working_height_ok;
                        if (!beta_ok || !height_ok || !c_width_ok || !working_height_ok) continue;
                        ++admissible;

                        State fixed{{b.n, a.m - b.m},
                                    {c.n, total_m - a.m - c.m},
                                    {a.n - c.n, c.m}};
                        normalize(fixed);
                        const int d_upper = assembly_r0_upper(fixed, b.m, residual_k);
                        if (d_upper < 0) {
                            ++r0_infeasible;
                            std::cout << "assembly_triple_static decision=R0_INFEASIBLE"
                                      << " A=" << a.n << ':' << a.m << " B=" << b.n << ':'
                                      << b.m << " C=" << c.n << ':' << c.m << " fixed=";
                            print_state(fixed);
                            std::cout << '\n';
                            continue;
                        }
                        triples.push_back(
                            {a, b, c, std::move(fixed), d_upper, a.n + b.n + d_upper});
                    }
                }
            }

            std::sort(triples.begin(), triples.end(), [](const AssemblyTriple &left,
                                                         const AssemblyTriple &right) {
                if (left.width_upper != right.width_upper)
                    return left.width_upper > right.width_upper;
                if (left.d_upper != right.d_upper) return left.d_upper > right.d_upper;
                return std::tie(left.a.m, left.b.m, left.c.m, left.a.n, left.b.n, left.c.n) <
                       std::tie(right.a.m, right.b.m, right.c.m, right.a.n, right.b.n,
                                right.c.n);
            });

            std::cout << "assembly_inventory cartesian=" << cartesian
                      << " admissible=" << admissible
                      << " geometry_rejected=" << cartesian - admissible
                      << " beta_failures=" << beta_failures
                      << " height_failures=" << height_failures
                      << " c_width_failures=" << c_width_failures
                      << " working_m_le_2a_failures=" << working_height_failures
                      << " r0_infeasible=" << r0_infeasible
                      << " r0_ranked=" << triples.size() << '\n'
                      << std::flush;
            for (std::size_t rank = 0; rank < triples.size(); ++rank) {
                const AssemblyTriple &triple = triples[rank];
                std::cout << "assembly_rank rank=" << rank + 1
                          << " width_upper=" << triple.width_upper
                          << " d_upper=" << triple.d_upper << " A=" << triple.a.n << ':'
                          << triple.a.m << " B=" << triple.b.n << ':' << triple.b.m << " C="
                          << triple.c.n << ':' << triple.c.m << " fixed=";
                print_state(triple.fixed);
                std::cout << '\n';
            }
            std::cout << "assembly_ranking_result ranked=" << triples.size()
                      << " complete=YES bound=R0_NECESSARY";
            if (known_parent != nullptr)
                std::cout << " known_parent_max=" << known_parent->n
                          << " known_parent_status=" << known_parent->status
                          << " known_parent_source=" << known_parent->source;
            std::cout << '\n' << std::flush;
            if (rank_only) return 0;

            int best_width = -1;
            std::vector<AssemblyWinner> winners;
            std::uint64_t exact_triples = 0;
            std::uint64_t exact_probes = 0;
            std::uint64_t upper_pruned = 0;
            std::uint64_t competitive_prefix_rejected = 0;
            bool all_dmax_exact = true;

            for (std::size_t rank = 0; rank < triples.size(); ++rank) {
                const AssemblyTriple &triple = triples[rank];
                if (best_width >= 0 && triple.width_upper < best_width) {
                    ++upper_pruned;
                    all_dmax_exact = false;
                    std::cout << "assembly_triple rank=" << rank + 1
                              << " decision=UPPER_PRUNED width_upper=" << triple.width_upper
                              << " incumbent=" << best_width << " d_upper=" << triple.d_upper
                              << " A=" << triple.a.n << ':' << triple.a.m << " B=" << triple.b.n
                              << ':' << triple.b.m << " C=" << triple.c.n << ':' << triple.c.m
                              << '\n';
                    continue;
                }

                ++exact_triples;
                const int base_width = triple.a.n + triple.b.n;
                const int minimum_competitive_d =
                    best_width < 0 ? 0 : std::max(0, best_width - base_width);
                std::cout << "assembly_triple rank=" << rank + 1
                          << " decision=EXACT_SEARCH width_upper=" << triple.width_upper
                          << " d_range=" << triple.d_upper << ':' << minimum_competitive_d
                          << " A=" << triple.a.n << ':' << triple.a.m << " B=" << triple.b.n
                          << ':' << triple.b.m << " C=" << triple.c.n << ':' << triple.c.m
                          << " fixed=";
                print_state(triple.fixed);
                std::cout << '\n' << std::flush;

                bool found = false;
                for (int d = triple.d_upper; d >= minimum_competitive_d; --d) {
                    State branch = assembly_branch(triple.fixed, d, triple.b.m);
                    const Counters before = counters;
                    const auto started = std::chrono::steady_clock::now();
                    const bool answer = construct(branch, residual_k, residual_k);
                    const double seconds = std::chrono::duration<double>(
                                               std::chrono::steady_clock::now() - started)
                                               .count();
                    ++exact_probes;
                    std::cout << "assembly_probe rank=" << rank + 1 << " d=" << d << ' '
                              << (answer ? "YES" : "NO") << " candidate_width="
                              << base_width + d << " branch=";
                    print_state(branch);
                    std::cout << " wall=" << seconds << "s calls="
                              << counters.calls - before.calls << " assignments="
                              << counters.assignments - before.assignments
                              << " memo=" << memo.size() << '\n'
                              << std::flush;
                    if (!answer) continue;

                    found = true;
                    const int candidate_width = base_width + d;
                    std::cout << "assembly_triple_result rank=" << rank + 1 << " d_max=" << d
                              << " candidate_width=" << candidate_width
                              << " d_max_exact=YES r0_excludes_larger=YES\n";
                    if (candidate_width > best_width) {
                        best_width = candidate_width;
                        winners.clear();
                    }
                    if (candidate_width == best_width)
                        winners.push_back({triple, d, std::move(branch)});
                    break;
                }
                if (found) continue;

                if (minimum_competitive_d == 0) {
                    std::cout << "assembly_triple_result rank=" << rank + 1
                              << " feasible=NONE d_max_exact=YES\n";
                } else {
                    ++competitive_prefix_rejected;
                    all_dmax_exact = false;
                    std::cout << "assembly_triple_result rank=" << rank + 1
                              << " decision=NONCOMPETITIVE_BELOW_D d_below="
                              << minimum_competitive_d
                              << " incumbent=" << best_width << " d_max_exact=NO\n";
                }
            }

            if (known_parent != nullptr && best_width > known_parent->n)
                throw std::logic_error("assembly construction exceeds the proven parent maximum");
            std::cout << "assembly_enumeration_result best_width=";
            if (best_width < 0)
                std::cout << "NONE";
            else
                std::cout << best_width;
            std::cout << " winners=" << winners.size()
                      << " optimization_complete=YES best_exact=YES"
                      << " all_triple_dmax_exact=" << (all_dmax_exact ? "YES" : "NO")
                      << " admissible=" << admissible << " r0_infeasible=" << r0_infeasible
                      << " exact_triples=" << exact_triples << " exact_probes=" << exact_probes
                      << " upper_pruned=" << upper_pruned
                      << " competitive_prefix_rejected=" << competitive_prefix_rejected;
            if (known_parent != nullptr) {
                const char *relation = best_width < 0 || best_width < known_parent->n
                                           ? "BELOW"
                                           : (best_width == known_parent->n ? "EQUAL" : "ABOVE");
                std::cout << " known_parent_max=" << known_parent->n
                          << " known_parent_relation=" << relation
                          << " known_parent_status=" << known_parent->status
                          << " known_parent_source=" << known_parent->source;
            }
            std::cout << '\n';

            for (std::size_t i = 0; i < winners.size(); ++i) {
                const AssemblyWinner &winner = winners[i];
                std::cout << "assembly_winner index=" << i + 1 << " candidate_width="
                          << best_width << " d=" << winner.d << " A=" << winner.triple.a.n << ':'
                          << winner.triple.a.m << " B=" << winner.triple.b.n << ':'
                          << winner.triple.b.m << " C=" << winner.triple.c.n << ':'
                          << winner.triple.c.m << " branch=";
                print_state(winner.branch);
                std::cout << '\n';
                print_tree(winner.branch, residual_k, residual_k);
            }
            return best_width < 0 ? 1 : 0;
        } catch (const std::exception &error) {
            std::cerr << "ABORT: " << error.what() << " (not an enumeration result)\n";
            return 3;
        }
    }
    if (argc >= 2 && std::string(argv[1]) == "assembly") {
        if (argc != 13) {
            std::cerr << "usage: " << argv[0]
                      << " assembly parent_k depth total_m a alpha b beta c gamma"
                         " start_d minimum_d\n";
            return 2;
        }
        const int parent_k = std::atoi(argv[2]);
        const int depth = std::atoi(argv[3]);
        const int total_m = std::atoi(argv[4]);
        const int a = std::atoi(argv[5]);
        const int alpha = std::atoi(argv[6]);
        const int b = std::atoi(argv[7]);
        const int beta = std::atoi(argv[8]);
        const int c = std::atoi(argv[9]);
        const int gamma = std::atoi(argv[10]);
        const int start_d = std::atoi(argv[11]);
        const int minimum_d = std::atoi(argv[12]);
        const int residual_k = parent_k - 2;
        const std::int64_t full_width =
            residual_k >= 0 && residual_k < 31 ? power_int(2, residual_k) : 0;
        if (parent_k < 2 || parent_k >= 33 || depth < 0 || depth > residual_k ||
            total_m <= 0 || a <= 0 || alpha <= 0 || b <= 0 || beta <= 0 || c <= 0 ||
            gamma <= 0 || beta > alpha || alpha + gamma > total_m || c > a ||
            minimum_d < 0 || start_d < minimum_d || start_d > full_width) {
            std::cerr << "usage: " << argv[0]
                      << " assembly parent_k depth total_m a alpha b beta c gamma"
                         " start_d minimum_d\n"
                      << "for A=(a:alpha), B=(b:beta), C=(c:gamma), require parent_k>=2,"
                         " 0<=depth<=parent_k-2, beta<=alpha, alpha+gamma<=total_m,"
                         " c<=a, and 0<=minimum_d<=start_d<=2^(parent_k-2)\n";
            return 2;
        }

        State fixed{{b, alpha - beta}, {c, total_m - alpha - gamma}, {a - c, gamma}};
        normalize(fixed);
        make_bases(residual_k);
        bool saw_exact_negative = false;
        try {
            for (int d = start_d; d >= minimum_d; --d) {
                State state = fixed;
                state.push_back({d, beta});
                normalize(state);
                const Counters before = counters;
                const auto started = std::chrono::steady_clock::now();
                const bool answer = construct(state, residual_k, depth);
                const double seconds = std::chrono::duration<double>(
                                           std::chrono::steady_clock::now() - started)
                                           .count();
                std::cout << "assembly d=" << d << ' ' << (answer ? "YES" : "NO")
                          << " parent_width=" << a + b + d << " parent_k=" << parent_k
                          << " residual_k=" << residual_k << " depth=" << depth
                          << " total_m=" << total_m << " A=" << a << ':' << alpha
                          << " B=" << b << ':' << beta << " C=" << c << ':' << gamma
                          << " branch=";
                print_state(state);
                std::cout << " wall=" << seconds << "s calls=" << counters.calls - before.calls
                          << " assignments=" << counters.assignments - before.assignments
                          << " memo=" << memo.size() << '\n'
                          << std::flush;
                if (answer) {
                    const bool global_maximum =
                        depth == residual_k && (d == full_width || saw_exact_negative);
                    std::cout << "assembly_result d=" << d << " parent_width=" << a + b + d
                              << " global_maximum=" << (global_maximum ? "YES" : "NO")
                              << " exact=" << (depth == residual_k ? "YES" : "NO") << '\n';
                    print_tree(state, residual_k, depth);
                    return 0;
                }
                if (depth == residual_k) saw_exact_negative = true;
            }
            const bool complete = depth == residual_k && minimum_d == 0;
            std::cout << "assembly_result feasible=NONE complete="
                      << (complete ? "YES" : "NO")
                      << " exact=" << (depth == residual_k ? "YES" : "NO") << '\n';
            return 1;
        } catch (const std::exception &error) {
            std::cerr << "ABORT: " << error.what() << " (not a negative verdict)\n";
            return 3;
        }
    }
    if (argc >= 2 && std::string(argv[1]) == "slice") {
        if (argc < 7 || ((argc - 7) % 2) != 0) {
            std::cerr << "usage: " << argv[0]
                      << " slice k depth variable_m start_delta maximum_delta"
                         " [fixed_n1 fixed_m1 ...]\n";
            return 2;
        }
        const int k = std::atoi(argv[2]);
        const int depth = std::atoi(argv[3]);
        const int variable_m = std::atoi(argv[4]);
        const int start_delta = std::atoi(argv[5]);
        const int maximum_delta = std::atoi(argv[6]);
        const std::int64_t full_width = k >= 0 && k < 31 ? power_int(2, k) : 0;
        State fixed;
        bool fixed_dimensions_valid = true;
        for (int i = 7; i < argc; i += 2) {
            const Part part{std::atoi(argv[i]), std::atoi(argv[i + 1])};
            fixed_dimensions_valid = fixed_dimensions_valid && part.n > 0 && part.m > 0;
            fixed.push_back(part);
        }
        if (fixed.size() >= MAX_PARTS) {
            std::cerr << "slice leaves room for at most " << MAX_PARTS - 1
                      << " fixed parts plus the variable part\n";
            return 2;
        }
        normalize(fixed);
        if (k < 0 || k >= 31 || depth < 0 || depth > k || variable_m <= 0 ||
            start_delta < 0 || maximum_delta < start_delta || maximum_delta >= full_width ||
            full_width - maximum_delta < variable_m || !fixed_dimensions_valid) {
            std::cerr << "usage: " << argv[0]
                      << " slice k depth variable_m start_delta maximum_delta"
                         " [fixed_n1 fixed_m1 ...]\n"
                      << "require 0 <= depth <= k < 31, 0 <= start_delta <= maximum_delta,"
                         " 0 < variable_m <= 2^k-maximum_delta, and positive fixed parts\n";
            return 2;
        }
        make_bases(k);
        try {
            for (int delta = start_delta; delta <= maximum_delta; ++delta) {
                const int variable_width = static_cast<int>(full_width - delta);
                State state = fixed;
                state.push_back({variable_width, variable_m});
                normalize(state);
                const Counters before = counters;
                const auto started = std::chrono::steady_clock::now();
                const bool answer = construct(state, k, depth);
                const double seconds = std::chrono::duration<double>(
                                           std::chrono::steady_clock::now() - started)
                                           .count();
                std::cout << "slice delta=" << delta << " variable_width=" << variable_width
                          << ' ' << (answer ? "YES" : "NO") << " k=" << k
                          << " depth=" << depth << " state=";
                print_state(state);
                std::cout << " wall=" << seconds << "s calls=" << counters.calls - before.calls
                          << " assignments=" << counters.assignments - before.assignments
                          << " memo=" << memo.size() << '\n'
                          << std::flush;
                if (answer) {
                    print_tree(state, k, depth);
                    return 0;
                }
            }
            return 1;
        } catch (const std::exception &error) {
            std::cerr << "ABORT: " << error.what() << " (not a negative verdict)\n";
            return 3;
        }
    }
    if (argc >= 2 && std::string(argv[1]) == "mixed-frontier") {
        if (argc < 8 || ((argc - 8) % 2) != 0) {
            std::cerr << "usage: " << argv[0]
                      << " mixed-frontier k depth left_m right_m maximum_u maximum_v"
                         " [fixed_n1 fixed_m1 ...]\n";
            return 2;
        }
        const int k = std::atoi(argv[2]);
        const int depth = std::atoi(argv[3]);
        const int left_m = std::atoi(argv[4]);
        const int right_m = std::atoi(argv[5]);
        const int maximum_u = std::atoi(argv[6]);
        const int maximum_v = std::atoi(argv[7]);
        const std::int64_t full_width = k >= 0 && k < 31 ? power_int(2, k) : 0;
        State fixed;
        bool fixed_dimensions_valid = true;
        for (int i = 8; i < argc; i += 2) {
            const Part part{std::atoi(argv[i]), std::atoi(argv[i + 1])};
            fixed_dimensions_valid = fixed_dimensions_valid && part.n > 0 && part.m > 0;
            fixed.push_back(part);
        }
        if (fixed.size() > MAX_PARTS - 2) {
            std::cerr << "mixed-frontier leaves room for at most " << MAX_PARTS - 2
                      << " fixed parts plus the two variable parts\n";
            return 2;
        }
        normalize(fixed);
        if (k < 0 || k >= 31 || depth < 0 || depth > k || left_m <= 0 || right_m <= 0 ||
            maximum_u < 0 || maximum_v < 0 || maximum_u > full_width ||
            maximum_v > full_width || !fixed_dimensions_valid) {
            std::cerr << "usage: " << argv[0]
                      << " mixed-frontier k depth left_m right_m maximum_u maximum_v"
                         " [fixed_n1 fixed_m1 ...]\n"
                      << "require 0 <= depth <= k < 31, positive heights and fixed parts,"
                         " and 0 <= u,v <= 2^k\n";
            return 2;
        }
        const int legal_maximum_u = static_cast<int>(full_width);
        const int legal_maximum_v = static_cast<int>(full_width);
        make_bases(k);
        try {
            std::vector<std::pair<int, int>> frontier;
            int previous_threshold = -1;
            bool vertical_complete = maximum_v == legal_maximum_v;
            bool reached_zero = false;

            for (int u = 0; u <= maximum_u; ++u) {
                const Counters before = counters;
                const auto started = std::chrono::steady_clock::now();
                auto make_state = [&](int v) {
                    State state = fixed;
                    state.push_back({static_cast<int>(full_width - u), left_m});
                    state.push_back({static_cast<int>(full_width - v), right_m});
                    normalize(state);
                    return state;
                };
                auto evaluate = [&](int v) { return construct(make_state(v), k, depth); };

                int high = previous_threshold >= 0 ? previous_threshold : maximum_v;
                if (!evaluate(high)) {
                    if (previous_threshold >= 0)
                        throw std::logic_error("mixed frontier violated subgraph monotonicity");
                    const double seconds = std::chrono::duration<double>(
                                               std::chrono::steady_clock::now() - started)
                                               .count();
                    std::cout << "mixed_threshold u=" << u << " minimum_v=NONE"
                              << " wall=" << seconds << "s calls=" << counters.calls - before.calls
                              << " assignments=" << counters.assignments - before.assignments
                              << " memo=" << memo.size() << '\n'
                              << std::flush;
                    continue;
                }
                if (u == 0) vertical_complete = true;

                int low = 0;
                while (low < high) {
                    const int middle = low + (high - low) / 2;
                    if (evaluate(middle))
                        high = middle;
                    else
                        low = middle + 1;
                }
                const int threshold = low;
                if (!evaluate(threshold) || (threshold > 0 && evaluate(threshold - 1)))
                    throw std::logic_error("mixed frontier threshold is not sharp");

                const bool new_point = previous_threshold < 0 || threshold < previous_threshold;
                const double seconds = std::chrono::duration<double>(
                                           std::chrono::steady_clock::now() - started)
                                           .count();
                std::cout << "mixed_threshold u=" << u << " minimum_v=" << threshold << ' '
                          << (new_point ? "POINT" : "DOMINATED") << " wall=" << seconds
                          << "s calls=" << counters.calls - before.calls
                          << " assignments=" << counters.assignments - before.assignments
                          << " memo=" << memo.size() << '\n'
                          << std::flush;
                if (new_point) frontier.emplace_back(u, threshold);
                previous_threshold = threshold;
                if (threshold == 0) {
                    reached_zero = true;
                    break;
                }
            }

            const bool horizontal_complete = reached_zero || maximum_u == legal_maximum_u;
            const bool complete = vertical_complete && horizontal_complete;
            std::cout << "mixed_frontier points=" << frontier.size()
                      << " complete=" << (complete ? "YES" : "NO")
                      << " exact=" << (depth == k ? "YES" : "NO") << " u_box=0:" << maximum_u
                      << " v_box=0:" << maximum_v << " k=" << k << " depth=" << depth << '\n';
            if (!frontier.empty()) {
                int piece_start = frontier.front().first;
                int piece_end = piece_start;
                int piece_sum = frontier.front().first + frontier.front().second;
                auto print_piece = [&] {
                    std::cout << "mixed_piece u=" << piece_start << ':' << piece_end
                              << " sum=" << piece_sum << " formula=v=" << piece_sum << "-u\n";
                };
                for (std::size_t i = 1; i < frontier.size(); ++i) {
                    const int u = frontier[i].first;
                    const int sum = u + frontier[i].second;
                    if (u == piece_end + 1 && sum == piece_sum) {
                        piece_end = u;
                    } else {
                        print_piece();
                        piece_start = piece_end = u;
                        piece_sum = sum;
                    }
                }
                print_piece();
            }
            for (auto [u, v] : frontier) {
                State state = fixed;
                state.push_back({static_cast<int>(full_width - u), left_m});
                state.push_back({static_cast<int>(full_width - v), right_m});
                normalize(state);
                std::cout << "mixed_point u=" << u << " v=" << v << " state=";
                print_state(state);
                std::cout << '\n';
                print_tree(state, k, depth);
            }
            return frontier.empty() ? 1 : 0;
        } catch (const std::exception &error) {
            std::cerr << "ABORT: " << error.what() << " (not a negative verdict)\n";
            return 3;
        }
    }
    if (argc < 5 || ((argc - 3) % 2) != 0) {
        std::cerr << "usage: " << argv[0] << " k depth n1 m1 [n2 m2 ...]\n"
                  << "       " << argv[0] << " forced k depth n m a b\n"
                  << "       " << argv[0] << " frontier k m start_n minimum_n\n"
                  << "       " << argv[0]
                  << " assembly-rank parent_k total_m pareto_csv\n"
                  << "       " << argv[0]
                  << " assembly-enumerate parent_k total_m pareto_csv\n"
                  << "       " << argv[0]
                  << " assembly parent_k depth total_m a alpha b beta c gamma"
                     " start_d minimum_d\n"
                  << "       " << argv[0]
                  << " slice k depth variable_m start_delta maximum_delta"
                     " [fixed_n1 fixed_m1 ...]\n"
                  << "       " << argv[0]
                  << " mixed-frontier k depth left_m right_m maximum_u maximum_v"
                     " [fixed_n1 fixed_m1 ...]\n";
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
