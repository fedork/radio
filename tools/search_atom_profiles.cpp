#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

// Symbolic search for the excessive-q, power-of-two-atom height-6 construction.
//
// A profile (a,b,c,d) denotes a*A_r+b*B_r+c*C_r+d*D_r, with
// a+b+c+d=PROFILE_ATOMS (8 by default, optionally 16 or 32 at compile time).
// Refinement from G_r to G_(r-1) is
//
//   A -> AA,  B -> AB,  C -> BC,  D -> CD.
//
// A synchronized cut selects exactly half of the refined atoms.  Consequently every
// descendant width has the configured profile size and, after any fixed number of synchronized
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

#ifndef ATOM_PROFILE_ATOMS
#define ATOM_PROFILE_ATOMS 8
#endif
constexpr int PROFILE_ATOMS = ATOM_PROFILE_ATOMS;
constexpr int TYPES = 4;
constexpr int MAX_HEIGHT = 6;
constexpr int MAX_PARTS = MAX_HEIGHT;
static_assert(PROFILE_ATOMS >= 8 && PROFILE_ATOMS <= 32,
              "supported normalizations contain 8, 16 or 32 atoms");
static_assert((PROFILE_ATOMS & (PROFILE_ATOMS - 1)) == 0,
              "profile atom count must be a power of two");
#ifndef ATOM_PROFILE_MAX_MEMO
#define ATOM_PROFILE_MAX_MEMO 2000000
#endif
constexpr std::size_t MAX_MEMO = ATOM_PROFILE_MAX_MEMO;

using Counts = std::array<std::uint8_t, TYPES>;
using Deficit = std::array<int, 3>;  // coefficients of C(r,2), r, 1
using ProfileId = std::uint16_t;

struct Profile {
    Counts count{};
    Deficit deficit{};
};

struct Part {
    ProfileId profile{};
    std::uint8_t height{};

    friend bool operator==(const Part &, const Part &) = default;
};

using State = std::vector<Part>;

struct Key {
    std::array<std::uint32_t, MAX_PARTS> parts{};
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

struct DCPart {
    std::uint8_t d{};
    std::uint8_t cd{};
    std::uint8_t height{};

    friend bool operator==(const DCPart &, const DCPart &) = default;
};

using DCState = std::vector<DCPart>;

struct DCKey {
    std::array<std::uint32_t, MAX_PARTS> parts{};
    std::uint8_t size{};
    std::uint8_t depth{};

    friend bool operator==(const DCKey &, const DCKey &) = default;
};

struct DCKeyHash {
    std::size_t operator()(const DCKey &key) const noexcept {
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

struct DCSplit {
    std::uint8_t selected_d{};
    std::uint8_t selected_cd{};
    std::uint8_t selected_height{};
};

struct LocalChildren {
    std::array<State, 3> state;  // both, mixed, neither
};

struct Option {
    ProfileId selected_profile{};
    std::uint8_t selected_height{};
    LocalChildren children;
};

struct Split {
    ProfileId selected_profile{};
    std::uint8_t selected_height{};
};

struct Counters {
    std::uint64_t calls{};
    std::uint64_t memo_hits{};
    std::uint64_t lineage_rejects{};
    std::uint64_t supply_rejects{};
    std::uint64_t dc_rejects{};
    std::uint64_t raw_options{};
    std::uint64_t assignments{};
    std::uint64_t prefix_rejects{};
};

std::vector<Profile> profiles;
std::array<std::array<std::array<std::array<int, PROFILE_ATOMS + 1>, PROFILE_ATOMS + 1>,
                      PROFILE_ATOMS + 1>,
           PROFILE_ATOMS + 1>
    profile_id{};
std::vector<std::vector<ProfileId>> cuts;
std::array<ProfileId, MAX_HEIGHT> reference_profiles{};

std::unordered_map<Key, bool, KeyHash> memo;
std::unordered_map<Key, std::vector<Split>, KeyHash> witnesses;
std::unordered_map<DCKey, bool, DCKeyHash> dc_memo;
std::unordered_map<DCKey, std::vector<DCSplit>, DCKeyHash> dc_witnesses;
Counters counters;

Deficit make_deficit(const Counts &count) {
    return {count[3], count[2] + count[3], count[1] + count[2] + count[3]};
}

// Negative means left is eventually wider.  Equal deficits imply equal profiles because both
// profiles contain exactly PROFILE_ATOMS atoms.
constexpr int normalization_levels() {
    int atoms = PROFILE_ATOMS;
    int levels = 0;
    while (atoms > 1) {
        atoms /= 2;
        ++levels;
    }
    return levels;
}

Counts refine_counts_raw(const Counts &old) {
    return {static_cast<std::uint8_t>(2 * old[0] + old[1]),
            static_cast<std::uint8_t>(old[1] + old[2]),
            static_cast<std::uint8_t>(old[2] + old[3]), old[3]};
}

Counts refine_to_profile_atoms(Counts count) {
    int atoms = 0;
    for (int value : count) atoms += value;
    while (atoms < PROFILE_ATOMS) {
        count = refine_counts_raw(count);
        atoms *= 2;
    }
    if (atoms != PROFILE_ATOMS)
        throw std::logic_error("profile cannot be refined to configured atom count");
    return count;
}

int compare_profiles(ProfileId left, ProfileId right) {
    for (int i = 0; i < 3; ++i) {
        if (profiles[left].deficit[i] != profiles[right].deficit[i])
            return profiles[left].deficit[i] - profiles[right].deficit[i];
    }
    return 0;
}

std::string profile_text(ProfileId id) {
    static constexpr std::array<char, TYPES> LETTERS{'A', 'B', 'C', 'D'};
    std::string out;
    for (int i = 0; i < TYPES; ++i)
        out.append(profiles[id].count[i], LETTERS[i]);
    return out.empty() ? "-" : out;
}

ProfileId lookup_profile(const Counts &count) {
    const int id = profile_id[count[0]][count[1]][count[2]][count[3]];
    if (id < 0) throw std::logic_error("profile lookup failed");
    return static_cast<ProfileId>(id);
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
        const Counts refined = refine_counts_raw(old);
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
        std::sort(cuts[id].begin(), cuts[id].end(), [](ProfileId left, ProfileId right) {
            return compare_profiles(left, right) < 0;
        });
    }

    // The first six atoms of G_(r+s), each refined s=log2(PROFILE_ATOMS) times to G_r.
    const std::array<Counts, MAX_HEIGHT> units{
        Counts{1, 0, 0, 0}, Counts{0, 1, 0, 0}, Counts{0, 0, 1, 0},
        Counts{0, 0, 1, 0}, Counts{0, 0, 0, 1}, Counts{0, 0, 0, 1}};
    for (int i = 0; i < MAX_HEIGHT; ++i)
        reference_profiles[i] = lookup_profile(refine_to_profile_atoms(units[i]));
}

Counts refined_counts(ProfileId id) {
    return refine_counts_raw(profiles[id].count);
}

ProfileId complement_profile(ProfileId parent, ProfileId selected) {
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
        key.parts[i] = static_cast<std::uint32_t>(state[i].profile * (MAX_HEIGHT + 1) +
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

std::vector<ProfileId> expanded_profiles(const State &state) {
    std::vector<ProfileId> expanded;
    for (Part part : state)
        for (int i = 0; i < part.height; ++i) expanded.push_back(part.profile);
    if (expanded.size() > MAX_HEIGHT) return {};
    std::sort(expanded.begin(), expanded.end(), [](ProfileId left, ProfileId right) {
        return compare_profiles(left, right) < 0;
    });
    return expanded;
}

std::vector<Deficit> full_star_differences(const State &state) {
    const std::vector<ProfileId> expanded = expanded_profiles(state);
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
    const std::vector<ProfileId> expanded = expanded_profiles(state);
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

// D is a non-branching lineage under refinement: D -> CD contains exactly one D.  In the
// mixed child, the selected and complementary D atoms therefore partition the parent's D
// atoms, while total height is preserved.  Following mixed outcomes forever shows that a
// state can reach an eventual singleton leaf only if it already has at least h-4 distinct
// D lineages: the first h profiles of the terminal reference contain max(0,h-4) D atoms.
// Unlike full-star majorization, the lineage count is not multiplied by part height.
int d_lineages(const State &state) {
    int lineages = 0;
    for (Part part : state) lineages += profiles[part.profile].count[3];
    return lineages;
}

int required_d_lineages(const State &state) {
    return std::max(0, static_cast<int>(total_height(state)) - 4);
}

bool d_lineage_possible(const State &state) {
    return d_lineages(state) >= required_d_lineages(state);
}

// Follow the mixed outcome for every remaining test.  If a state has unweighted deficit
// supply (D,V,W), where V=C+D and W=B+C+D, one refinement can supply at most
//
//     (D, V+D, W+V).
//
// Selected and complementary profiles attain that sum only when neither zero-height piece is
// discarded.  After t levels the componentwise upper bound is therefore
//
//     (D, V+tD, W+tV+C(t,2)D).
//
// A singleton leaf of height h must dominate the complete prefix of the first h reference
// profiles.  If even this optimistic supply is lexicographically smaller, nature can keep taking
// the mixed outcome and the state cannot finish within t levels.  Heights do not multiply supply:
// each state part denotes one profile lineage, exactly as in the all-depth D-lineage lemma.
Deficit mixed_supply_upper(const State &state, int depth) {
    Deficit supply{};
    for (Part part : state)
        for (int coefficient = 0; coefficient < 3; ++coefficient)
            supply[coefficient] += profiles[part.profile].deficit[coefficient];

    const int initial_d = supply[0];
    const int initial_v = supply[1];
    supply[1] += depth * initial_d;
    supply[2] += depth * initial_v + depth * (depth - 1) / 2 * initial_d;
    return supply;
}

Deficit singleton_prefix_requirement(const State &state) {
    Deficit required{};
    const std::size_t height = total_height(state);
    if (height > MAX_HEIGHT) throw std::logic_error("state exceeds terminal height bound");
    for (std::size_t index = 0; index < height; ++index)
        for (int coefficient = 0; coefficient < 3; ++coefficient)
            required[coefficient] += profiles[reference_profiles[index]].deficit[coefficient];
    return required;
}

bool mixed_supply_possible(const State &state, int depth) {
    const Deficit supply = mixed_supply_upper(state, depth);
    const Deficit required = singleton_prefix_requirement(state);
    for (int coefficient = 0; coefficient < 3; ++coefficient) {
        if (supply[coefficient] != required[coefficient])
            return supply[coefficient] > required[coefficient];
    }
    return true;
}

// Sound two-coefficient over-approximation.  A projected profile keeps only
// (p_D,p_C+p_D).  Its refinement and every possible projected N-of-2N cut depend only on
// those two coordinates.  Terminal comparison deliberately ignores the final deficit
// coefficient, so construct_dc can prove bounded-depth NO; a DC YES is only permission to
// continue the full search, never a full verdict.
void normalize_dc(DCState &state) {
    state.erase(std::remove_if(state.begin(), state.end(),
                               [](DCPart part) { return part.height == 0; }),
                state.end());
    std::sort(state.begin(), state.end(), [](DCPart left, DCPart right) {
        if (left.d != right.d) return left.d < right.d;
        if (left.cd != right.cd) return left.cd < right.cd;
        return left.height > right.height;
    });
    if (state.size() > MAX_PARTS) throw std::logic_error("DC state exceeds height bound");
}

DCState project_dc(const State &state) {
    DCState out;
    for (Part part : state) {
        const Counts &count = profiles[part.profile].count;
        out.push_back({count[3], static_cast<std::uint8_t>(count[2] + count[3]),
                       part.height});
    }
    normalize_dc(out);
    return out;
}

DCState add_dc_states(const DCState &left, const DCState &right) {
    DCState out = left;
    out.insert(out.end(), right.begin(), right.end());
    normalize_dc(out);
    return out;
}

DCKey make_dc_key(const DCState &state, int depth) {
    DCKey key;
    key.size = static_cast<std::uint8_t>(state.size());
    key.depth = static_cast<std::uint8_t>(depth);
    for (std::size_t i = 0; i < state.size(); ++i) {
        const int profile = state[i].d * (PROFILE_ATOMS + 1) + state[i].cd;
        key.parts[i] = static_cast<std::uint32_t>(profile * (MAX_HEIGHT + 1) +
                                                  state[i].height);
    }
    return key;
}

std::size_t dc_total_height(const DCState &state) {
    std::size_t height = 0;
    for (DCPart part : state) height += part.height;
    return height;
}

bool dc_lineage_possible(const DCState &state) {
    int lineages = 0;
    for (DCPart part : state) lineages += part.d;
    return lineages >= std::max(0, static_cast<int>(dc_total_height(state)) - 4);
}

bool dc_full_star(const DCState &state) {
    std::vector<std::pair<int, int>> expanded;
    for (DCPart part : state)
        for (int i = 0; i < part.height; ++i) expanded.emplace_back(part.d, part.cd);
    if (expanded.size() > MAX_HEIGHT) return false;
    std::sort(expanded.begin(), expanded.end());

    int left_d = 0;
    int left_cd = 0;
    int right_d = 0;
    int right_cd = 0;
    for (std::size_t i = 0; i < expanded.size(); ++i) {
        left_d += expanded[i].first;
        left_cd += expanded[i].second;
        const Deficit &reference = profiles[reference_profiles[i]].deficit;
        right_d += reference[0];
        right_cd += reference[1];
        if (left_d < right_d || (left_d == right_d && left_cd < right_cd)) return false;
    }
    return true;
}

bool dc_singleton(const DCState &state) {
    return std::all_of(state.begin(), state.end(),
                       [](DCPart part) { return part.height == 1; });
}

struct DCOption {
    std::uint8_t selected_d{};
    std::uint8_t selected_cd{};
    std::uint8_t selected_height{};
    std::array<DCState, 3> children;
};

std::vector<DCOption> dc_local_options(DCPart part) {
    std::vector<DCOption> out;
    const int refined_d = part.d;
    const int refined_cd = part.d + part.cd;
    const int refined_non_cd = 2 * PROFILE_ATOMS - refined_cd;
    for (int selected_d = 0; selected_d <= part.d; ++selected_d) {
        for (int selected_cd = selected_d; selected_cd <= PROFILE_ATOMS; ++selected_cd) {
            const int selected_c = selected_cd - selected_d;
            if (selected_c > part.cd) continue;
            if (PROFILE_ATOMS - selected_cd > refined_non_cd) continue;
            const int complement_d = refined_d - selected_d;
            const int complement_cd = refined_cd - selected_cd;
            if (complement_cd < complement_d || complement_cd > PROFILE_ATOMS) continue;
            for (int selected_height = 0; selected_height <= part.height; ++selected_height) {
                std::array<DCState, 3> children;
                children[0] = {{static_cast<std::uint8_t>(selected_d),
                                static_cast<std::uint8_t>(selected_cd),
                                static_cast<std::uint8_t>(selected_height)}};
                children[1] = {
                    {static_cast<std::uint8_t>(selected_d),
                     static_cast<std::uint8_t>(selected_cd),
                     static_cast<std::uint8_t>(part.height - selected_height)},
                    {static_cast<std::uint8_t>(complement_d),
                     static_cast<std::uint8_t>(complement_cd),
                     static_cast<std::uint8_t>(selected_height)}};
                children[2] = {{static_cast<std::uint8_t>(complement_d),
                                static_cast<std::uint8_t>(complement_cd),
                                static_cast<std::uint8_t>(part.height - selected_height)}};
                for (DCState &child : children) normalize_dc(child);
                out.push_back({static_cast<std::uint8_t>(selected_d),
                               static_cast<std::uint8_t>(selected_cd),
                               static_cast<std::uint8_t>(selected_height),
                               std::move(children)});
            }
        }
    }
    return out;
}

bool construct_dc(const DCState &input, int depth) {
    DCState state = input;
    normalize_dc(state);
    const DCKey key = make_dc_key(state, depth);
    if (const auto found = dc_memo.find(key); found != dc_memo.end()) return found->second;
    if (!dc_lineage_possible(state) || !dc_full_star(state)) {
        dc_memo.emplace(key, false);
        return false;
    }
    if (state.empty() || dc_singleton(state)) {
        dc_memo.emplace(key, true);
        return true;
    }
    if (depth == 0) {
        dc_memo.emplace(key, false);
        return false;
    }

    struct Candidate {
        int original{};
        DCPart part{};
        std::vector<DCOption> options;
    };
    std::vector<Candidate> candidates;
    for (std::size_t part_index = 0; part_index < state.size(); ++part_index) {
        const DCPart part = state[part_index];
        std::vector<DCOption> viable;
        for (DCOption option : dc_local_options(part)) {
            bool possible = true;
            for (int outcome : {1, 0, 2}) {
                if (!construct_dc(option.children[outcome], depth - 1)) {
                    possible = false;
                    break;
                }
            }
            if (possible) viable.push_back(std::move(option));
        }
        if (viable.empty()) {
            dc_memo.emplace(key, false);
            return false;
        }
        candidates.push_back(
            {static_cast<int>(part_index), part, std::move(viable)});
    }
    std::stable_sort(candidates.begin(), candidates.end(), [](const Candidate &left,
                                                              const Candidate &right) {
        return left.options.size() < right.options.size();
    });

    std::array<DCState, 3> partial;
    std::vector<DCSplit> selected_original(candidates.size());
    auto dfs = [&](auto &&self, int index) -> bool {
        if (index == static_cast<int>(candidates.size())) return true;
        for (const DCOption &option : candidates[index].options) {
            std::array<DCState, 3> next;
            bool possible = true;
            for (int outcome : {1, 0, 2}) {
                next[outcome] = add_dc_states(partial[outcome], option.children[outcome]);
                if (!construct_dc(next[outcome], depth - 1)) {
                    possible = false;
                    break;
                }
            }
            if (!possible) continue;
            const auto saved = partial;
            partial = std::move(next);
            selected_original[candidates[index].original] =
                {option.selected_d, option.selected_cd, option.selected_height};
            if (self(self, index + 1)) return true;
            partial = saved;
        }
        return false;
    };

    const bool answer = dfs(dfs, 0);
    dc_memo.emplace(key, answer);
    if (answer) dc_witnesses.emplace(key, std::move(selected_original));
    return answer;
}

LocalChildren split_part(Part part, ProfileId selected, int selected_height) {
    const ProfileId complement = complement_profile(part.profile, selected);
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
    for (ProfileId selected : cuts[part.profile]) {
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
    if (!d_lineage_possible(state)) {
        ++counters.lineage_rejects;
        remember(key, false);
        return false;
    }
    if (!mixed_supply_possible(state, depth)) {
        ++counters.supply_rejects;
        remember(key, false);
        return false;
    }
    const DCState projected = project_dc(state);
    if (!construct_dc(projected, depth)) {
        ++counters.dc_rejects;
        remember(key, false);
        return false;
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

void print_dc_state(const DCState &state) {
    for (std::size_t i = 0; i < state.size(); ++i) {
        if (i) std::cout << ',';
        std::cout << '(' << static_cast<int>(state[i].d) << ','
                  << static_cast<int>(state[i].cd) << "):"
                  << static_cast<int>(state[i].height);
    }
    if (state.empty()) std::cout << '-';
}

std::array<DCState, 3> dc_children_of(const DCState &state,
                                      const std::vector<DCSplit> &split) {
    std::array<DCState, 3> children;
    for (std::size_t i = 0; i < state.size(); ++i) {
        const DCPart part = state[i];
        const DCSplit cut = split[i];
        const int complement_d = part.d - cut.selected_d;
        const int complement_cd = part.d + part.cd - cut.selected_cd;
        std::array<DCState, 3> local{
            DCState{{cut.selected_d, cut.selected_cd, cut.selected_height}},
            DCState{{cut.selected_d, cut.selected_cd,
                     static_cast<std::uint8_t>(part.height - cut.selected_height)},
                    {static_cast<std::uint8_t>(complement_d),
                     static_cast<std::uint8_t>(complement_cd), cut.selected_height}},
            DCState{{static_cast<std::uint8_t>(complement_d),
                     static_cast<std::uint8_t>(complement_cd),
                     static_cast<std::uint8_t>(part.height - cut.selected_height)}}};
        for (int outcome = 0; outcome < 3; ++outcome) {
            normalize_dc(local[outcome]);
            children[outcome] = add_dc_states(children[outcome], local[outcome]);
        }
    }
    return children;
}

void print_dc_tree(const DCState &input, int depth, int parent = -1, int outcome = -1,
                   int level = 0, int *next_id_pointer = nullptr) {
    int local_next_id = 0;
    int &next_id = next_id_pointer ? *next_id_pointer : local_next_id;
    DCState state = input;
    normalize_dc(state);
    const int id = next_id++;
    const DCKey key = make_dc_key(state, depth);
    const auto found = dc_witnesses.find(key);
    std::cout << "dc_tree_node id=" << id << " parent=" << parent
              << " outcome=" << outcome << " level=" << level << " state=";
    print_dc_state(state);
    if (found == dc_witnesses.end()) {
        std::cout << " leaf=YES\n";
        return;
    }
    std::cout << " split=";
    for (std::size_t i = 0; i < found->second.size(); ++i) {
        if (i) std::cout << ',';
        const DCSplit split = found->second[i];
        std::cout << '(' << static_cast<int>(split.selected_d) << ','
                  << static_cast<int>(split.selected_cd) << "):"
                  << static_cast<int>(split.selected_height);
    }
    std::cout << '\n';
    const std::array<DCState, 3> children = dc_children_of(state, found->second);
    for (int child_outcome = 0; child_outcome < 3; ++child_outcome)
        print_dc_tree(children[child_outcome], depth - 1, id, child_outcome, level + 1,
                      &next_id);
    if (!next_id_pointer)
        std::cout << "dc_tree_certificate version=1 root=0 nodes=" << next_id
                  << " profile_atoms=" << PROFILE_ATOMS << '\n';
}

DCState dc_state_from_key(const DCKey &key) {
    DCState state;
    for (int i = 0; i < key.size; ++i) {
        const int encoded_profile = key.parts[i] / (MAX_HEIGHT + 1);
        const int height = key.parts[i] % (MAX_HEIGHT + 1);
        const int d = encoded_profile / (PROFILE_ATOMS + 1);
        const int cd = encoded_profile % (PROFILE_ATOMS + 1);
        state.push_back({static_cast<std::uint8_t>(d), static_cast<std::uint8_t>(cd),
                         static_cast<std::uint8_t>(height)});
    }
    normalize_dc(state);
    return state;
}

bool dc_is_substate(const DCState &small, const DCState &large) {
    if (small.size() > large.size()) return false;
    std::vector<bool> used(large.size(), false);
    for (DCPart wanted : small) {
        bool found = false;
        for (std::size_t i = 0; i < large.size(); ++i) {
            if (!used[i] && large[i] == wanted) {
                used[i] = true;
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

void reset_search();

// Synthesize a candidate coinductive kernel from two repeated bounded-search layers.  Equality of
// the layers is only a discovery device, not the all-depth proof: the independent Python checker
// exhausts every cut from the emitted minimal cores and verifies upward-substate closure directly.
void print_dc_kernel_certificate(const DCState &root) {
    constexpr int SEARCH_DEPTH = 20;
    reset_search();
    if (construct_dc(root, SEARCH_DEPTH))
        throw std::logic_error("kernel target unexpectedly became constructible");

    std::unordered_map<DCKey, bool, DCKeyHash> layer3;
    std::unordered_map<DCKey, bool, DCKeyHash> layer4;
    for (const auto &[stored_key, answer] : dc_memo) {
        if (answer || (stored_key.depth != 3 && stored_key.depth != 4)) continue;
        DCKey key = stored_key;
        key.depth = 0;
        (stored_key.depth == 3 ? layer3 : layer4).emplace(key, false);
    }
    if (layer3.size() != layer4.size())
        throw std::logic_error("candidate DC kernel layers differ in size");
    for (const auto &[key, answer] : layer3) {
        (void)answer;
        if (!layer4.contains(key))
            throw std::logic_error("candidate DC kernel layers differ in membership");
    }
    DCKey root_key = make_dc_key(root, 0);
    if (!layer4.contains(root_key))
        throw std::logic_error("DC kernel does not contain its requested root");

    std::vector<DCState> states;
    states.reserve(layer4.size());
    for (const auto &[key, answer] : layer4) {
        (void)answer;
        DCState state = dc_state_from_key(key);
        if (dc_lineage_possible(state) && dc_full_star(state))
            states.push_back(std::move(state));
    }
    std::vector<DCState> minimal_states;
    for (std::size_t i = 0; i < states.size(); ++i) {
        bool redundant = false;
        for (std::size_t j = 0; j < states.size(); ++j) {
            if (i != j && dc_is_substate(states[j], states[i])) {
                redundant = true;
                break;
            }
        }
        if (!redundant) minimal_states.push_back(states[i]);
    }
    states = std::move(minimal_states);
    std::sort(states.begin(), states.end(), [](const DCState &left, const DCState &right) {
        if (left.size() != right.size()) return left.size() < right.size();
        for (std::size_t i = 0; i < left.size(); ++i) {
            const auto left_tuple =
                std::tuple(left[i].d, left[i].cd, left[i].height);
            const auto right_tuple =
                std::tuple(right[i].d, right[i].cd, right[i].height);
            if (left_tuple != right_tuple) return left_tuple < right_tuple;
        }
        return false;
    });

    std::cout << "dc_kernel_certificate version=1 model=power_of_two_atom_aligned"
              << " profile_atoms=" << PROFILE_ATOMS << " search_depth=" << SEARCH_DEPTH
              << " fixed_layers=3,4 core_states=" << states.size()
              << " candidate_rank_first=290 candidate_rank_last=304 next_rank=305"
              << " implicit_axioms=d_lineage,full_star minimized=multiset"
              << " closure=upward_substate root=";
    print_dc_state(root);
    std::cout << " verdict=ALL_DEPTH_NO\n";
    for (const DCState &state : states) {
        std::cout << "dc_kernel_state state=";
        print_dc_state(state);
        std::cout << '\n';
    }
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

int print_machine_tree(const State &input, int depth, int parent, int outcome, int level,
                       int &next_id, int &root_threshold) {
    State state = input;
    normalize(state);
    const int id = next_id++;
    const Key key = make_key(state, depth);
    const auto found = witnesses.find(key);

    std::cout << "atom_profile_tree_node id=" << id << " parent=" << parent
              << " outcome=" << outcome << " level=" << level << " state=";
    print_state(state);
    if (found == witnesses.end()) {
        const int threshold = singleton_threshold(state);
        root_threshold = std::max(root_threshold, level + threshold);
        std::cout << " leaf_threshold=" << threshold << '\n';
        return id;
    }

    std::cout << " split=";
    for (std::size_t i = 0; i < found->second.size(); ++i) {
        if (i) std::cout << ',';
        std::cout << profile_text(found->second[i].selected_profile) << ':'
                  << static_cast<int>(found->second[i].selected_height);
    }
    std::cout << '\n';
    const std::array<State, 3> children = children_of(state, found->second);
    for (int child_outcome = 0; child_outcome < 3; ++child_outcome)
        print_machine_tree(children[child_outcome], depth - 1, id, child_outcome, level + 1,
                           next_id, root_threshold);
    return id;
}

void print_machine_tree_certificate(const State &state, int depth, int expected_threshold) {
    int next_id = 0;
    int root_threshold = 3;
    const int root = print_machine_tree(state, depth, -1, -1, 0, next_id, root_threshold);
    if (root != 0 || root_threshold != expected_threshold)
        throw std::logic_error("machine tree threshold disagrees with human tree");
    std::cout << "atom_profile_tree_certificate version=1 root=0 nodes=" << next_id
              << " root_base_threshold=" << root_threshold
              << " profile_atoms=" << PROFILE_ATOMS
              << " normalization_levels=" << normalization_levels() << '\n';
}

ProfileId lifted_profile(const Counts &eight_atom_profile) {
    return lookup_profile(refine_to_profile_atoms(eight_atom_profile));
}

State height6_state(ProfileId d_profile) {
    State state{{lifted_profile({5, 2, 1, 0}), 1},
                {lifted_profile({3, 3, 2, 0}), 2},
                {d_profile, 3}};
    normalize(state);
    return state;
}

State height6_literal_residual_state() {
    // The mixed child of the unique aligned refinement of the k=10 witness split,
    // lifted further when the configured normalization has sixteen atoms:
    // P=A^4B^3C, Q=A^2B^4C^2, R=A^3B^3CD.
    State state{{lifted_profile({4, 3, 1, 0}), 1},
                {lifted_profile({4, 3, 1, 0}), 1},
                {lifted_profile({2, 4, 2, 0}), 2},
                {lifted_profile({3, 3, 1, 1}), 2}};
    normalize(state);
    return state;
}

State height6_literal_core_state() {
    // The mixed child after the literal residual split.  This is the symbolic version of
    // Sb(57:1,57:1,57:1,56:1,46:2)@6 in the stored k=10 witness.
    State state{{lifted_profile({4, 3, 1, 0}), 1},
                {lifted_profile({4, 3, 1, 0}), 1},
                {lifted_profile({4, 3, 1, 0}), 1},
                {lifted_profile({3, 4, 1, 0}), 1},
                {lifted_profile({2, 3, 2, 1}), 2}};
    normalize(state);
    return state;
}

State height5_control_state() {
    // Refine the four-atom profiles ABCD, AAAB, AABC to the configured normalization.
    State state{{lifted_profile({3, 2, 2, 1}), 2},
                {lifted_profile({7, 1, 0, 0}), 1},
                {lifted_profile({5, 2, 1, 0}), 2}};
    normalize(state);
    return state;
}

State height4_control_state() {
    // Refine CC, AA, AB twice to the same normalization.
    State state{{lifted_profile({2, 4, 2, 0}), 2},
                {lifted_profile({8, 0, 0, 0}), 1},
                {lifted_profile({7, 1, 0, 0}), 1}};
    normalize(state);
    return state;
}

void reset_search() {
    memo.clear();
    witnesses.clear();
    dc_memo.clear();
    dc_witnesses.clear();
    counters = {};
}

void print_result(const State &state, int depth, bool answer, double seconds) {
    std::cout << "atom_profile depth=" << depth << " answer=" << (answer ? "YES" : "NO")
              << " state=";
    print_state(state);
    std::cout << " eventual_model=YES restricted=power_of_two_atom_aligned"
              << " profile_atoms=" << PROFILE_ATOMS
              << " wall=" << seconds << "s calls=" << counters.calls
              << " memo_hits=" << counters.memo_hits << " memo=" << memo.size()
              << " lineage_rejects=" << counters.lineage_rejects
              << " supply_rejects=" << counters.supply_rejects
              << " dc_rejects=" << counters.dc_rejects << " dc_memo=" << dc_memo.size()
              << " raw_options=" << counters.raw_options
              << " assignments=" << counters.assignments
              << " prefix_rejects=" << counters.prefix_rejects << '\n';
    if (!d_lineage_possible(state))
        std::cout << "atom_profile_all_depth_obstruction=d_lineage"
                  << " d_lineages=" << d_lineages(state)
                  << " required=" << required_d_lineages(state)
                  << " preserving_outcome=mixed\n";
    if (!mixed_supply_possible(state, depth)) {
        const Deficit supply = mixed_supply_upper(state, depth);
        const Deficit required = singleton_prefix_requirement(state);
        std::cout << "atom_profile_depth_obstruction=mixed_supply depth=" << depth
                  << " supply_upper=" << supply[0] << ',' << supply[1] << ',' << supply[2]
                  << " required=" << required[0] << ',' << required[1] << ',' << required[2]
                  << " preserving_outcome=mixed verdict=NO_WITHIN_DEPTH\n";
    }
    if (answer) {
        const int threshold = print_tree(state, depth);
        std::cout << "atom_profile_proof root_base_threshold=" << threshold
                  << " meaning=valid_for_every_integer_base_at_or_above_threshold\n";
        print_machine_tree_certificate(state, depth, threshold);
    }
}

int parse_nonnegative(const char *text, const char *name, int maximum = 1000000) {
    char *end = nullptr;
    const long value = std::strtol(text, &end, 10);
    if (end == text || *end != '\0' || value < 0 || value > maximum)
        throw std::invalid_argument(std::string("invalid ") + name);
    return static_cast<int>(value);
}

Part parse_profile_part(const char *text) {
    const std::string encoded(text);
    const std::size_t separator = encoded.rfind(':');
    if (separator == std::string::npos || separator == 0 || separator + 1 == encoded.size())
        throw std::invalid_argument("invalid profile part (expected canonical WORD:height)");
    const std::string word = encoded.substr(0, separator);
    if (word.size() != static_cast<std::size_t>(PROFILE_ATOMS))
        throw std::invalid_argument("profile word has the wrong normalization size");

    static constexpr std::array<char, TYPES> LETTERS{'A', 'B', 'C', 'D'};
    Counts count{};
    int previous = 0;
    for (char letter : word) {
        const auto found = std::find(LETTERS.begin(), LETTERS.end(), letter);
        if (found == LETTERS.end())
            throw std::invalid_argument("profile word contains an unknown atom letter");
        const int index = static_cast<int>(found - LETTERS.begin());
        if (index < previous)
            throw std::invalid_argument("profile word is not in canonical A/B/C/D order");
        previous = index;
        ++count[index];
    }
    const int height = parse_nonnegative(encoded.c_str() + separator + 1, "profile height",
                                         MAX_HEIGHT);
    if (height == 0) throw std::invalid_argument("profile height must be positive");
    return {lookup_profile(count), static_cast<std::uint8_t>(height)};
}

DCPart parse_dc_part(const char *text) {
    int d = -1;
    int cd = -1;
    int height = -1;
    char trailing = '\0';
    if (std::sscanf(text, "%d,%d,%d%c", &d, &cd, &height, &trailing) != 3 || d < 0 ||
        d > cd || cd > PROFILE_ATOMS || height < 1 || height > MAX_HEIGHT)
        throw std::invalid_argument("invalid DC part (expected d,cd,height)");
    return {static_cast<std::uint8_t>(d), static_cast<std::uint8_t>(cd),
            static_cast<std::uint8_t>(height)};
}

std::vector<ProfileId> ordered_profiles() {
    std::vector<ProfileId> ordered;
    for (std::size_t id = 0; id < profiles.size(); ++id)
        ordered.push_back(static_cast<ProfileId>(id));
    std::sort(ordered.begin(), ordered.end(), [](ProfileId left, ProfileId right) {
        return compare_profiles(left, right) < 0;
    });
    return ordered;
}

int profile_rank(const std::vector<ProfileId> &ordered, ProfileId profile) {
    const auto found = std::find(ordered.begin(), ordered.end(), profile);
    if (found == ordered.end()) throw std::logic_error("rank lookup failed");
    return static_cast<int>(found - ordered.begin()) + 1;
}

void print_height6_lineage_certificate() {
    const std::vector<ProfileId> ordered = ordered_profiles();
    int last_excluded = 0;
    while (last_excluded < static_cast<int>(ordered.size()) &&
           profiles[ordered[last_excluded]].count[3] < 2)
        ++last_excluded;
    if (last_excluded == 0 || last_excluded == static_cast<int>(ordered.size()))
        throw std::logic_error("unexpected D-lineage rank boundary");

    const ProfileId target = lifted_profile({1, 5, 1, 1});
    const ProfileId fixed_b = lifted_profile({5, 2, 1, 0});
    const ProfileId fixed_ac = lifted_profile({3, 3, 2, 0});
    std::cout
        << "atom_lineage_certificate version=1 model=power_of_two_atom_aligned"
        << " profile_atoms=" << PROFILE_ATOMS
        << " normalization_levels=" << normalization_levels()
        << " fixed_parts=" << profile_text(fixed_b) << ":1," << profile_text(fixed_ac)
        << ":2 candidate_height=3"
        << " candidate_rank_first=1 candidate_rank_last=" << last_excluded
        << " candidate_d_max=1 root_height=6 root_d_lineages_max=1"
        << " terminal_reference_d=0,0,0,0,1,1 required_d_lineages=2"
        << " target_rank=" << profile_rank(ordered, target)
        << " target=" << profile_text(target)
        << " next_rank=" << last_excluded + 1
        << " next=" << profile_text(ordered[last_excluded])
        << " closure_outcome=mixed verdict=ALL_DEPTH_NO\n";
}

}  // namespace

int main(int argc, char **argv) {
    try {
        initialize_profiles();
        if (argc == 2 && std::string(argv[1]) == "height6-lineage-certificate") {
            print_height6_lineage_certificate();
            return 0;
        }
        if (argc == 2 && std::string(argv[1]) == "height6-dc-kernel-certificate") {
            const std::vector<ProfileId> ordered = ordered_profiles();
            if (ordered.size() < 290)
                throw std::invalid_argument(
                    "height6 DC kernel certificate requires the 16-atom build");
            print_dc_kernel_certificate(project_dc(height6_state(ordered[289])));
            return 0;
        }
        if (argc == 3 && (std::string(argv[1]) == "height4-control" ||
                          std::string(argv[1]) == "height5-control" ||
                          std::string(argv[1]) == "height6" ||
                          std::string(argv[1]) == "height6-literal-residual" ||
                          std::string(argv[1]) == "height6-literal-core")) {
            const int maximum_depth = parse_nonnegative(argv[2], "maximum depth", 255);
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
                state = height6_state(lifted_profile({1, 5, 1, 1}));  // ABBBBBCD class
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
            const int depth = parse_nonnegative(argv[2], "depth", 255);
            const std::vector<ProfileId> ordered = ordered_profiles();
            const int first_rank = argc >= 4 ? parse_nonnegative(argv[3], "first rank") : 1;
            const int last_rank =
                argc >= 5 ? parse_nonnegative(argv[4], "last rank")
                          : static_cast<int>(ordered.size());
            if (first_rank < 1 || last_rank < first_rank ||
                last_rank > static_cast<int>(ordered.size()))
                throw std::invalid_argument("rank interval lies outside configured profile list");

            reset_search();
            bool range_all_depth_excluded = true;
            for (int rank = first_rank - 1; rank < last_rank; ++rank) {
                const State state = height6_state(ordered[rank]);
                const bool root_full_star = full_star_eventual(state);
                const bool lineage_possible = d_lineage_possible(state);
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
                          << " d_lineages=" << d_lineages(state)
                          << " required_d_lineages=" << required_d_lineages(state)
                          << " all_depth_obstruction="
                          << (lineage_possible ? "NO" : "d_lineage")
                          << " wall=" << seconds << "s calls=" << counters.calls - before.calls
                          << " assignments=" << counters.assignments - before.assignments
                          << " dc_rejects=" << counters.dc_rejects - before.dc_rejects
                          << " shared_memo=" << memo.size() << " dc_memo=" << dc_memo.size()
                          << '\n'
                          << std::flush;
                if (answer) {
                    const int threshold = print_tree(state, depth);
                    std::cout << "height6_max_proof root_base_threshold=" << threshold
                              << " maximal_at_requested_depth="
                              << (first_rank == 1 ? "YES" : "NO")
                              << " maximal_all_depth="
                              << (first_rank == 1 && range_all_depth_excluded ? "YES" : "NO")
                              << '\n';
                    print_machine_tree_certificate(state, depth, threshold);
                    return 0;
                }
                if (lineage_possible) range_all_depth_excluded = false;
            }
            std::cout << "height6_max_result feasible=NONE depth=" << depth
                      << " first_rank=" << first_rank << " last_rank=" << last_rank
                      << " range_complete=YES restricted=power_of_two_atom_aligned"
                         " eventual_model=YES profile_atoms="
                      << PROFILE_ATOMS << " all_depth_excluded="
                      << (range_all_depth_excluded ? "YES" : "NO") << '\n';
            return 1;
        }

        if (argc == 4 && std::string(argv[1]) == "height6-dc") {
            const int depth = parse_nonnegative(argv[2], "depth", 255);
            const std::vector<ProfileId> ordered = ordered_profiles();
            const int rank = parse_nonnegative(argv[3], "rank");
            if (rank < 1 || rank > static_cast<int>(ordered.size()))
                throw std::invalid_argument("rank lies outside configured profile list");

            const State state = height6_state(ordered[rank - 1]);
            const DCState projected = project_dc(state);
            reset_search();
            const auto started = std::chrono::steady_clock::now();
            const bool answer = construct_dc(projected, depth);
            const double seconds = std::chrono::duration<double>(
                                       std::chrono::steady_clock::now() - started)
                                       .count();
            std::cout << "height6_dc rank=" << rank
                      << " D=" << profile_text(ordered[rank - 1])
                      << " depth=" << depth << " answer=" << (answer ? "YES" : "NO")
                      << " state=";
            print_dc_state(projected);
            std::cout << " wall=" << seconds << "s dc_memo=" << dc_memo.size() << '\n';
            if (answer) print_dc_tree(projected, depth);
            return answer ? 0 : 1;
        }

        if (argc >= 4 && std::string(argv[1]) == "dc-state") {
            const int depth = parse_nonnegative(argv[2], "depth", 255);
            DCState state;
            for (int i = 3; i < argc; ++i) state.push_back(parse_dc_part(argv[i]));
            normalize_dc(state);
            if (dc_total_height(state) > MAX_HEIGHT)
                throw std::invalid_argument("DC state exceeds height bound");
            reset_search();
            const auto started = std::chrono::steady_clock::now();
            const bool answer = construct_dc(state, depth);
            const double seconds = std::chrono::duration<double>(
                                       std::chrono::steady_clock::now() - started)
                                       .count();
            std::cout << "dc_state depth=" << depth << " answer="
                      << (answer ? "YES" : "NO") << " state=";
            print_dc_state(state);
            std::cout << " wall=" << seconds << "s dc_memo=" << dc_memo.size() << '\n';
            if (answer) print_dc_tree(state, depth);
            return answer ? 0 : 1;
        }

        if (argc >= 4 && std::string(argv[1]) == "profile-state") {
            const int depth = parse_nonnegative(argv[2], "depth", 255);
            State state;
            for (int i = 3; i < argc; ++i) state.push_back(parse_profile_part(argv[i]));
            normalize(state);
            if (total_height(state) > MAX_HEIGHT)
                throw std::invalid_argument("profile state exceeds height bound");
            reset_search();
            const auto started = std::chrono::steady_clock::now();
            const bool answer = construct(state, depth);
            const double seconds = std::chrono::duration<double>(
                                       std::chrono::steady_clock::now() - started)
                                       .count();
            print_result(state, depth, answer, seconds);
            return answer ? 0 : 1;
        }

        std::cerr << "usage: " << argv[0]
                  << " height4-control|height5-control|height6|height6-literal-residual"
                     "|height6-literal-core maximum_depth\n"
                  << "       " << argv[0] << " height6-max depth [first_rank [last_rank]]\n"
                  << "       " << argv[0] << " height6-dc depth rank\n"
                  << "       " << argv[0] << " dc-state depth d,cd,height [...]\n"
                  << "       " << argv[0] << " profile-state depth WORD:height [...]\n"
                  << "       " << argv[0] << " height6-dc-kernel-certificate\n"
                  << "       " << argv[0] << " height6-lineage-certificate\n";
        return 2;
    } catch (const std::exception &error) {
        std::cerr << "ABORT: " << error.what() << " (not a negative verdict)\n";
        return 3;
    }
}
