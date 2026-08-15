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

// Symbolic search for the excessive-q, eight-atom height-6 construction.
//
// A profile (a,b,c,d) denotes a*A_r+b*B_r+c*C_r+d*D_r, with a+b+c+d=8.
// Refinement from G_r to G_(r-1) is
//
//   A -> AA,  B -> AB,  C -> BC,  D -> CD.
//
// A synchronized cut selects exactly eight of the sixteen refined atoms.  Consequently every
// descendant width is again an eight-atom profile and, after any fixed number of synchronized
// levels, a singleton leaf can be compared with G_(r+3) independently of scale.  Since
//
//   A_r = 2^r,
//   B_r = 2^r - 1,
//   C_r = 2^r - 1 - r,
//   D_r = 2^r - 1 - r - C(r,2),
//
// eventual width comparisons are lexicographic comparisons of the coefficients of
// (C(r,2), r, 1).  This tool searches exactly inside that explicitly restricted profile model;
// a NO is not a claim about unrestricted group-testing solvability.

namespace {

constexpr int PROFILE_ATOMS = 8;
constexpr int TYPES = 4;
constexpr int MAX_HEIGHT = 6;
constexpr int MAX_PARTS = MAX_HEIGHT;
#ifndef ATOM_PROFILE_MAX_MEMO
#define ATOM_PROFILE_MAX_MEMO 2000000
#endif
constexpr std::size_t MAX_MEMO = ATOM_PROFILE_MAX_MEMO;

using Counts = std::array<std::uint8_t, TYPES>;
using Deficit = std::array<int, 3>;  // coefficients of C(r,2), r, 1

struct Profile {
    Counts count{};
    Deficit deficit{};
};

struct Part {
    std::uint8_t profile{};
    std::uint8_t height{};

    friend bool operator==(const Part &, const Part &) = default;
};

using State = std::vector<Part>;

struct Key {
    std::array<std::uint16_t, MAX_PARTS> parts{};
    std::uint8_t size{};
    std::uint8_t depth{};

    friend bool operator==(const Key &, const Key &) = default;
};

struct KeyHash {
    std::size_t operator()(const Key &key) const noexcept {
        std::size_t value = 1469598103934665603ULL;
        auto mix = [&](std::size_t item) {
            value ^= item;
            value *= 1099511628211ULL;
        };
        mix(key.size);
        mix(key.depth);
        for (int i = 0; i < key.size; ++i) mix(key.parts[i]);
        return value;
    }
};

struct LocalChildren {
    std::array<State, 3> state;  // both, mixed, neither
};

struct Option {
    std::uint8_t selected_profile{};
    std::uint8_t selected_height{};
    LocalChildren children;
};

struct Split {
    std::uint8_t selected_profile{};
    std::uint8_t selected_height{};
};

struct Counters {
    std::uint64_t calls{};
    std::uint64_t memo_hits{};
    std::uint64_t raw_options{};
    std::uint64_t assignments{};
    std::uint64_t prefix_rejects{};
};

std::vector<Profile> profiles;
std::array<std::array<std::array<std::array<int, PROFILE_ATOMS + 1>, PROFILE_ATOMS + 1>,
                      PROFILE_ATOMS + 1>,
           PROFILE_ATOMS + 1>
    profile_id{};
std::vector<std::vector<std::uint8_t>> cuts;
std::array<std::uint8_t, MAX_HEIGHT> reference_profiles{};

std::unordered_map<Key, bool, KeyHash> memo;
std::unordered_map<Key, std::vector<Split>, KeyHash> witnesses;
Counters counters;

Deficit make_deficit(const Counts &count) {
    return {count[3], count[2] + count[3], count[1] + count[2] + count[3]};
}

// Negative means left is eventually wider.  Equal deficits imply equal profiles because both
// profiles contain exactly eight atoms.
int compare_profiles(std::uint8_t left, std::uint8_t right) {
    for (int i = 0; i < 3; ++i) {
        if (profiles[left].deficit[i] != profiles[right].deficit[i])
            return profiles[left].deficit[i] - profiles[right].deficit[i];
    }
    return 0;
}

std::string profile_text(std::uint8_t id) {
    static constexpr std::array<char, TYPES> LETTERS{'A', 'B', 'C', 'D'};
    std::string out;
    for (int i = 0; i < TYPES; ++i)
        out.append(profiles[id].count[i], LETTERS[i]);
    return out.empty() ? "-" : out;
}

std::uint8_t lookup_profile(const Counts &count) {
    const int id = profile_id[count[0]][count[1]][count[2]][count[3]];
    if (id < 0) throw std::logic_error("profile lookup failed");
    return static_cast<std::uint8_t>(id);
}

void initialize_profiles() {
    for (auto &a : profile_id)
        for (auto &b : a)
            for (auto &c : b) c.fill(-1);

    for (int a = 0; a <= PROFILE_ATOMS; ++a)
        for (int b = 0; b <= PROFILE_ATOMS - a; ++b)
            for (int c = 0; c <= PROFILE_ATOMS - a - b; ++c) {
                const int d = PROFILE_ATOMS - a - b - c;
                Counts count{static_cast<std::uint8_t>(a), static_cast<std::uint8_t>(b),
                             static_cast<std::uint8_t>(c), static_cast<std::uint8_t>(d)};
                const int id = static_cast<int>(profiles.size());
                profile_id[a][b][c][d] = id;
                profiles.push_back({count, make_deficit(count)});
            }

    cuts.resize(profiles.size());
    for (std::size_t id = 0; id < profiles.size(); ++id) {
        const Counts &old = profiles[id].count;
        const Counts refined{
            static_cast<std::uint8_t>(2 * old[0] + old[1]),
            static_cast<std::uint8_t>(old[1] + old[2]),
            static_cast<std::uint8_t>(old[2] + old[3]), old[3]};
        for (int a = 0; a <= std::min(PROFILE_ATOMS, static_cast<int>(refined[0])); ++a)
            for (int b = 0;
                 b <= std::min(PROFILE_ATOMS - a, static_cast<int>(refined[1])); ++b)
                for (int c = 0;
                     c <= std::min(PROFILE_ATOMS - a - b, static_cast<int>(refined[2])); ++c) {
                    const int d = PROFILE_ATOMS - a - b - c;
                    if (d <= refined[3]) {
                        Counts selected{static_cast<std::uint8_t>(a),
                                        static_cast<std::uint8_t>(b),
                                        static_cast<std::uint8_t>(c),
                                        static_cast<std::uint8_t>(d)};
                        cuts[id].push_back(lookup_profile(selected));
                    }
                }
        std::sort(cuts[id].begin(), cuts[id].end(), [](std::uint8_t left, std::uint8_t right) {
            return compare_profiles(left, right) < 0;
        });
    }

    // The first six atoms of G_(r+3), each refined three times to G_r.
    reference_profiles = {
        lookup_profile({8, 0, 0, 0}), lookup_profile({7, 1, 0, 0}),
        lookup_profile({4, 3, 1, 0}), lookup_profile({4, 3, 1, 0}),
        lookup_profile({1, 3, 3, 1}), lookup_profile({1, 3, 3, 1})};
}

Counts refined_counts(std::uint8_t id) {
    const Counts &old = profiles[id].count;
    return {static_cast<std::uint8_t>(2 * old[0] + old[1]),
            static_cast<std::uint8_t>(old[1] + old[2]),
            static_cast<std::uint8_t>(old[2] + old[3]), old[3]};
}

std::uint8_t complement_profile(std::uint8_t parent, std::uint8_t selected) {
    const Counts total = refined_counts(parent);
    const Counts &take = profiles[selected].count;
    Counts complement{};
    for (int i = 0; i < TYPES; ++i) complement[i] = total[i] - take[i];
    return lookup_profile(complement);
}

void normalize(State &state) {
    state.erase(std::remove_if(state.begin(), state.end(),
                               [](Part part) { return part.height == 0; }),
                state.end());
    std::sort(state.begin(), state.end(), [](Part left, Part right) {
        const int comparison = compare_profiles(left.profile, right.profile);
        if (comparison != 0) return comparison < 0;
        return left.height > right.height;
    });
    if (state.size() > MAX_PARTS) throw std::logic_error("state exceeds height bound");
}

State add_states(const State &left, const State &right) {
    State out = left;
    out.insert(out.end(), right.begin(), right.end());
    normalize(out);
    return out;
}

Key make_key(const State &state, int depth) {
    Key key;
    key.size = static_cast<std::uint8_t>(state.size());
    key.depth = static_cast<std::uint8_t>(depth);
    for (std::size_t i = 0; i < state.size(); ++i)
        key.parts[i] = static_cast<std::uint16_t>(state[i].profile * (MAX_HEIGHT + 1) +
                                                  state[i].height);
    return key;
}

bool eventually_nonnegative(const Deficit &value) {
    for (int coefficient : value) {
        if (coefficient != 0) return coefficient > 0;
    }
    return true;
}

std::int64_t evaluate_deficit_difference(const Deficit &value, int r) {
    return static_cast<std::int64_t>(value[0]) * r * (r - 1) / 2 +
           static_cast<std::int64_t>(value[1]) * r + value[2];
}

int eventual_threshold(const Deficit &value) {
    if (!eventually_nonnegative(value))
        throw std::logic_error("threshold requested for eventually negative polynomial");
    for (int r = 3; r < 1000000; ++r) {
        const std::int64_t forward_difference =
            static_cast<std::int64_t>(value[0]) * r + value[1];
        if (evaluate_deficit_difference(value, r) >= 0 && forward_difference >= 0) return r;
    }
    throw std::logic_error("eventual threshold search exceeded bound");
}

std::size_t total_height(const State &state);

std::vector<std::uint8_t> expanded_profiles(const State &state) {
    std::vector<std::uint8_t> expanded;
    for (Part part : state)
        for (int i = 0; i < part.height; ++i) expanded.push_back(part.profile);
    if (expanded.size() > MAX_HEIGHT) return {};
    std::sort(expanded.begin(), expanded.end(), [](std::uint8_t left, std::uint8_t right) {
        return compare_profiles(left, right) < 0;
    });
    return expanded;
}

std::vector<Deficit> full_star_differences(const State &state) {
    const std::vector<std::uint8_t> expanded = expanded_profiles(state);
    if (expanded.size() != total_height(state)) return {};

    std::vector<Deficit> differences;
    Deficit left{};
    Deficit right{};
    for (std::size_t i = 0; i < expanded.size(); ++i) {
        for (int coefficient = 0; coefficient < 3; ++coefficient) {
            left[coefficient] += profiles[expanded[i]].deficit[coefficient];
            right[coefficient] += profiles[reference_profiles[i]].deficit[coefficient];
        }
        Deficit difference{};
        for (int coefficient = 0; coefficient < 3; ++coefficient)
            difference[coefficient] = left[coefficient] - right[coefficient];
        differences.push_back(difference);
    }
    return differences;
}

bool full_star_eventual(const State &state) {
    const std::vector<Deficit> differences = full_star_differences(state);
    if (differences.size() != total_height(state)) return false;
    for (const Deficit &difference : differences)
        if (!eventually_nonnegative(difference)) return false;
    return true;
}

int singleton_threshold(const State &state) {
    int threshold = 3;
    const std::vector<std::uint8_t> expanded = expanded_profiles(state);
    for (std::size_t i = 1; i < expanded.size(); ++i) {
        Deficit order_difference{};
        for (int coefficient = 0; coefficient < 3; ++coefficient)
            order_difference[coefficient] =
                profiles[expanded[i]].deficit[coefficient] -
                profiles[expanded[i - 1]].deficit[coefficient];
        threshold = std::max(threshold, eventual_threshold(order_difference));
    }
    for (const Deficit &difference : full_star_differences(state))
        threshold = std::max(threshold, eventual_threshold(difference));
    return threshold;
}

void remember(const Key &key, bool answer) {
    if (memo.size() >= MAX_MEMO)
        throw std::runtime_error("memo limit reached (abort, not NO)");
    memo.emplace(key, answer);
}

bool singleton(const State &state) {
    return std::all_of(state.begin(), state.end(),
                       [](Part part) { return part.height == 1; });
}

std::size_t total_height(const State &state) {
    std::size_t height = 0;
    for (Part part : state) height += part.height;
    return height;
}

LocalChildren split_part(Part part, std::uint8_t selected, int selected_height) {
    const std::uint8_t complement = complement_profile(part.profile, selected);
    LocalChildren children;
    children.state[0] = {{selected, static_cast<std::uint8_t>(selected_height)}};
    children.state[1] = {
        {selected, static_cast<std::uint8_t>(part.height - selected_height)},
        {complement, static_cast<std::uint8_t>(selected_height)}};
    children.state[2] = {
        {complement, static_cast<std::uint8_t>(part.height - selected_height)}};
    for (State &state : children.state) normalize(state);
    return children;
}

bool construct(const State &input, int depth);

std::array<int, 3> height_score(const LocalChildren &children) {
    std::array<int, 3> score{};
    for (int outcome = 0; outcome < 3; ++outcome)
        for (Part part : children.state[outcome]) score[outcome] += part.height;
    std::sort(score.begin(), score.end(), std::greater<>());
    return score;
}

std::vector<Option> viable_part_options(Part part, int depth) {
    std::vector<Option> out;
    for (std::uint8_t selected : cuts[part.profile]) {
        for (int selected_height = 0; selected_height <= part.height; ++selected_height) {
            ++counters.raw_options;
            LocalChildren children = split_part(part, selected, selected_height);
            bool possible = true;
            for (const State &child : children.state) {
                if (!construct(child, depth - 1)) {
                    possible = false;
                    break;
                }
            }
            if (possible)
                out.push_back({selected, static_cast<std::uint8_t>(selected_height),
                               std::move(children)});
        }
    }
    std::sort(out.begin(), out.end(), [](const Option &left, const Option &right) {
        const auto left_score = height_score(left.children);
        const auto right_score = height_score(right.children);
        if (left_score != right_score) return left_score < right_score;
        const int comparison = compare_profiles(left.selected_profile, right.selected_profile);
        if (comparison != 0) return comparison < 0;
        return left.selected_height < right.selected_height;
    });
    return out;
}

bool construct(const State &input, int depth) {
    ++counters.calls;
    State state = input;
    normalize(state);
    const Key key = make_key(state, depth);
    if (const auto found = memo.find(key); found != memo.end()) {
        ++counters.memo_hits;
        return found->second;
    }
    if (!full_star_eventual(state)) {
        remember(key, false);
        return false;
    }
    if (state.empty() || singleton(state)) {
        remember(key, true);
        return true;
    }
    if (depth == 0) {
        remember(key, false);
        return false;
    }

    struct Candidate {
        int original{};
        Part part{};
        std::vector<Option> options;
    };
    std::vector<Candidate> candidates;
    for (std::size_t i = 0; i < state.size(); ++i) {
        std::vector<Option> options = viable_part_options(state[i], depth);
        if (options.empty()) {
            remember(key, false);
            return false;
        }
        candidates.push_back({static_cast<int>(i), state[i], std::move(options)});
    }
    std::stable_sort(candidates.begin(), candidates.end(), [](const Candidate &left,
                                                              const Candidate &right) {
        if (left.options.size() != right.options.size())
            return left.options.size() < right.options.size();
        const int comparison = compare_profiles(left.part.profile, right.part.profile);
        if (comparison != 0) return comparison < 0;
        return left.part.height > right.part.height;
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
                if (std::pair(option.selected_profile, option.selected_height) <
                    std::pair(previous.selected_profile, previous.selected_height))
                    continue;
            }
            ++counters.assignments;
            std::array<State, 3> next;
            bool possible = true;
            for (int outcome = 0; outcome < 3; ++outcome) {
                next[outcome] = add_states(partial[outcome], option.children.state[outcome]);
                if (!construct(next[outcome], depth - 1)) {
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
            selected[index] = {option.selected_profile, option.selected_height};
            selected_original[candidate.original] = selected[index];
            if (self(self, index + 1)) return true;
            partial = saved;
        }
        return false;
    };

    const bool answer = dfs(dfs, 0);
    remember(key, answer);
    if (answer) witnesses.emplace(key, std::move(selected_original));
    return answer;
}

void print_state(const State &state) {
    for (std::size_t i = 0; i < state.size(); ++i) {
        if (i) std::cout << ',';
        std::cout << profile_text(state[i].profile) << ':' << static_cast<int>(state[i].height);
    }
    if (state.empty()) std::cout << '-';
}

std::array<State, 3> children_of(const State &state, const std::vector<Split> &split) {
    std::array<State, 3> children;
    for (std::size_t i = 0; i < state.size(); ++i) {
        const LocalChildren local =
            split_part(state[i], split[i].selected_profile, split[i].selected_height);
        for (int outcome = 0; outcome < 3; ++outcome)
            children[outcome] = add_states(children[outcome], local.state[outcome]);
    }
    return children;
}

int print_tree(const State &input, int depth, int level = 0, int indent = 0) {
    State state = input;
    normalize(state);
    std::cout << std::string(indent, ' ');
    print_state(state);
    const Key key = make_key(state, depth);
    const auto found = witnesses.find(key);
    if (found == witnesses.end()) {
        const int threshold = singleton_threshold(state);
        std::cout << " [majorized G_(r+3) for base r>=" << threshold << "]\n";
        return level + threshold;
    }
    std::cout << " --[";
    for (std::size_t i = 0; i < found->second.size(); ++i) {
        if (i) std::cout << ',';
        std::cout << profile_text(found->second[i].selected_profile) << ':'
                  << static_cast<int>(found->second[i].selected_height);
    }
    std::cout << "]-->\n";
    int root_threshold = level + 3;
    for (const State &child : children_of(state, found->second))
        root_threshold =
            std::max(root_threshold, print_tree(child, depth - 1, level + 1, indent + 2));
    return root_threshold;
}

State height6_state(std::uint8_t d_profile) {
    State state{{lookup_profile({5, 2, 1, 0}), 1},
                {lookup_profile({3, 3, 2, 0}), 2},
                {d_profile, 3}};
    normalize(state);
    return state;
}

State height6_literal_residual_state() {
    // The mixed child of the unique aligned refinement of the k=10 witness split:
    // P=A^4B^3C, Q=A^2B^4C^2, R=A^3B^3CD.
    State state{{lookup_profile({4, 3, 1, 0}), 1},
                {lookup_profile({4, 3, 1, 0}), 1},
                {lookup_profile({2, 4, 2, 0}), 2},
                {lookup_profile({3, 3, 1, 1}), 2}};
    normalize(state);
    return state;
}

State height6_literal_core_state() {
    // The mixed child after the literal residual split.  This is the symbolic version of
    // Sb(57:1,57:1,57:1,56:1,46:2)@6 in the stored k=10 witness.
    State state{{lookup_profile({4, 3, 1, 0}), 1},
                {lookup_profile({4, 3, 1, 0}), 1},
                {lookup_profile({4, 3, 1, 0}), 1},
                {lookup_profile({3, 4, 1, 0}), 1},
                {lookup_profile({2, 3, 2, 1}), 2}};
    normalize(state);
    return state;
}

State height5_control_state() {
    // Refine the four-atom profiles ABCD, AAAB, AABC once so they use the common
    // eight-atom / three-level normalization of this tool.
    State state{{lookup_profile({3, 2, 2, 1}), 2},
                {lookup_profile({7, 1, 0, 0}), 1},
                {lookup_profile({5, 2, 1, 0}), 2}};
    normalize(state);
    return state;
}

State height4_control_state() {
    // Refine CC, AA, AB twice to the same normalization.
    State state{{lookup_profile({2, 4, 2, 0}), 2},
                {lookup_profile({8, 0, 0, 0}), 1},
                {lookup_profile({7, 1, 0, 0}), 1}};
    normalize(state);
    return state;
}

void reset_search() {
    memo.clear();
    witnesses.clear();
    counters = {};
}

void print_result(const State &state, int depth, bool answer, double seconds) {
    std::cout << "atom_profile depth=" << depth << " answer=" << (answer ? "YES" : "NO")
              << " state=";
    print_state(state);
    std::cout << " eventual_model=YES restricted=eight_atom_aligned"
              << " wall=" << seconds << "s calls=" << counters.calls
              << " memo_hits=" << counters.memo_hits << " memo=" << memo.size()
              << " raw_options=" << counters.raw_options
              << " assignments=" << counters.assignments
              << " prefix_rejects=" << counters.prefix_rejects << '\n';
    if (answer) {
        const int threshold = print_tree(state, depth);
        std::cout << "atom_profile_proof root_base_threshold=" << threshold
                  << " meaning=valid_for_every_integer_base_at_or_above_threshold\n";
    }
}

int parse_nonnegative(const char *text, const char *name) {
    char *end = nullptr;
    const long value = std::strtol(text, &end, 10);
    if (end == text || *end != '\0' || value < 0 || value > 255)
        throw std::invalid_argument(std::string("invalid ") + name);
    return static_cast<int>(value);
}

}  // namespace

int main(int argc, char **argv) {
    try {
        initialize_profiles();
        if (argc == 3 && (std::string(argv[1]) == "height4-control" ||
                          std::string(argv[1]) == "height5-control" ||
                          std::string(argv[1]) == "height6" ||
                          std::string(argv[1]) == "height6-literal-residual" ||
                          std::string(argv[1]) == "height6-literal-core")) {
            const int maximum_depth = parse_nonnegative(argv[2], "maximum depth");
            State state;
            if (std::string(argv[1]) == "height4-control")
                state = height4_control_state();
            else if (std::string(argv[1]) == "height5-control")
                state = height5_control_state();
            else if (std::string(argv[1]) == "height6-literal-residual")
                state = height6_literal_residual_state();
            else if (std::string(argv[1]) == "height6-literal-core")
                state = height6_literal_core_state();
            else
                state = height6_state(lookup_profile({1, 5, 1, 1}));  // ABBBBBCD
            for (int depth = 0; depth <= maximum_depth; ++depth) {
                reset_search();
                const auto started = std::chrono::steady_clock::now();
                const bool answer = construct(state, depth);
                const double seconds = std::chrono::duration<double>(
                                           std::chrono::steady_clock::now() - started)
                                           .count();
                print_result(state, depth, answer, seconds);
                if (answer) return 0;
            }
            return 1;
        }

        if (argc >= 3 && argc <= 5 && std::string(argv[1]) == "height6-max") {
            const int depth = parse_nonnegative(argv[2], "depth");
            std::vector<std::uint8_t> ordered;
            for (std::size_t id = 0; id < profiles.size(); ++id)
                ordered.push_back(static_cast<std::uint8_t>(id));
            std::sort(ordered.begin(), ordered.end(), [](std::uint8_t left, std::uint8_t right) {
                return compare_profiles(left, right) < 0;
            });
            const int first_rank = argc >= 4 ? parse_nonnegative(argv[3], "first rank") : 1;
            const int last_rank =
                argc >= 5 ? parse_nonnegative(argv[4], "last rank")
                          : static_cast<int>(ordered.size());
            if (first_rank < 1 || last_rank < first_rank ||
                last_rank > static_cast<int>(ordered.size()))
                throw std::invalid_argument("require 1 <= first rank <= last rank <= 165");

            reset_search();
            for (int rank = first_rank - 1; rank < last_rank; ++rank) {
                const State state = height6_state(ordered[rank]);
                const bool root_full_star = full_star_eventual(state);
                const Counters before = counters;
                const auto started = std::chrono::steady_clock::now();
                const bool answer = construct(state, depth);
                const double seconds = std::chrono::duration<double>(
                                           std::chrono::steady_clock::now() - started)
                                           .count();
                std::cout << "height6_candidate rank=" << rank + 1
                          << " D=" << profile_text(ordered[rank])
                          << " deficit_tuple=" << profiles[ordered[rank]].deficit[0] << ','
                          << profiles[ordered[rank]].deficit[1] << ','
                          << profiles[ordered[rank]].deficit[2]
                          << " answer=" << (answer ? "YES" : "NO") << " depth=" << depth
                          << " full_star=" << (root_full_star ? "YES" : "NO")
                          << " wall=" << seconds << "s calls=" << counters.calls - before.calls
                          << " assignments=" << counters.assignments - before.assignments
                          << " shared_memo=" << memo.size() << '\n'
                          << std::flush;
                if (answer) {
                    const int threshold = print_tree(state, depth);
                    std::cout << "height6_max_proof root_base_threshold=" << threshold
                              << " maximal_at_requested_depth="
                              << (first_rank == 1 ? "YES" : "NO") << '\n';
                    return 0;
                }
            }
            std::cout << "height6_max_result feasible=NONE depth=" << depth
                      << " first_rank=" << first_rank << " last_rank=" << last_rank
                      << " range_complete=YES restricted=eight_atom_aligned"
                         " eventual_model=YES\n";
            return 1;
        }

        std::cerr << "usage: " << argv[0]
                  << " height4-control|height5-control|height6|height6-literal-residual"
                     "|height6-literal-core maximum_depth\n"
                  << "       " << argv[0] << " height6-max depth [first_rank [last_rank]]\n";
        return 2;
    } catch (const std::exception &error) {
        std::cerr << "ABORT: " << error.what() << " (not a negative verdict)\n";
        return 3;
    }
}
