#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
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

struct CoverWRange {
    int minimum{};
    int maximum{};
};

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
    std::uint64_t supply_loss_rejects{};
    std::uint64_t mixed_envelope_rejects{};
    std::uint64_t mixed_path_rejects{};
    std::uint64_t mixed_path_calls{};
    std::uint64_t mixed_path_assignments{};
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
std::unordered_set<DCKey, DCKeyHash> dc_kernel_exact_cores;
bool dc_kernel_upward_substate = false;
Counters counters;

struct MixedEnvelope {
    bool possible{};
    Deficit maximum{};
};

std::unordered_map<Key, bool, KeyHash> mixed_path_memo;
std::unordered_map<Key, std::vector<Split>, KeyHash> mixed_path_witnesses;

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

Deficit unweighted_supply(const State &state) {
    Deficit supply{};
    for (Part part : state)
        for (int coefficient = 0; coefficient < 3; ++coefficient)
            supply[coefficient] += profiles[part.profile].deficit[coefficient];
    return supply;
}

Deficit refined_supply(ProfileId profile) {
    const Deficit &supply = profiles[profile].deficit;
    return {supply[0], supply[1] + supply[0], supply[2] + supply[1]};
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

// A height-h part can end in only h singleton profiles, each containing PROFILE_ATOMS atoms.
// Cap each optimistic mixed-supply coordinate by that terminal capacity.  The bound deliberately
// ignores how those atoms must be partitioned among the h profiles, so it is cheap and safely
// optimistic.  Reaching h singleton lineages also needs h<=2^steps.
MixedEnvelope mixed_capacity_envelope(Part part, int steps) {
    if (steps < 0 || part.height > (std::uint64_t{1} << std::min(steps, 63))) return {};
    const Deficit initial = profiles[part.profile].deficit;
    const int capacity = part.height * PROFILE_ATOMS;
    return {true,
            {initial[0], std::min(capacity, initial[1] + steps * initial[0]),
             std::min(capacity, initial[2] + steps * initial[1] +
                                    steps * (steps - 1) / 2 * initial[0])}};
}

bool height_aware_mixed_possible(const State &state, int depth) {
    const Deficit required = singleton_prefix_requirement(state);
    for (int steps = 0; steps <= depth; ++steps) {
        MixedEnvelope combined{true, {}};
        for (Part part : state) {
            const MixedEnvelope local = mixed_capacity_envelope(part, steps);
            if (!local.possible) {
                combined.possible = false;
                break;
            }
            for (int coefficient = 0; coefficient < 3; ++coefficient)
                combined.maximum[coefficient] += local.maximum[coefficient];
        }
        if (!combined.possible) continue;
        Deficit difference{};
        for (int coefficient = 0; coefficient < 3; ++coefficient)
            difference[coefficient] = combined.maximum[coefficient] - required[coefficient];
        if (eventually_nonnegative(difference)) return true;
    }
    return false;
}

int mixed_supply_preserved_coordinates(const State &state, int depth) {
    const Deficit upper = mixed_supply_upper(state, depth);
    const Deficit required = singleton_prefix_requirement(state);
    int preserved = 0;
    while (preserved < 3 && upper[preserved] == required[preserved]) ++preserved;
    return preserved;
}

Deficit local_mixed_supply_loss(Part part, const Option &option) {
    const Deficit available = refined_supply(part.profile);
    const Deficit retained = unweighted_supply(option.children.state[1]);
    return {available[0] - retained[0], available[1] - retained[1],
            available[2] - retained[2]};
}

// A loss ell at the first mixed transition propagates through the remaining t-1 optimistic
// refinements by the same triangular map as supply.  Missing parts can contribute zero further
// loss but can never compensate for loss already accumulated, so this is also a sound prefix
// test while assembling a global cut.
bool mixed_supply_loss_possible(const State &state, int depth, const Deficit &loss) {
    if (depth <= 0) return loss == Deficit{};
    const int remaining = depth - 1;
    const Deficit terminal_loss{
        loss[0], loss[1] + remaining * loss[0],
        loss[2] + remaining * loss[1] + remaining * (remaining - 1) / 2 * loss[0]};
    const Deficit upper = mixed_supply_upper(state, depth);
    const Deficit required = singleton_prefix_requirement(state);
    for (int coefficient = 0; coefficient < 3; ++coefficient) {
        const int retained = upper[coefficient] - terminal_loss[coefficient];
        if (retained != required[coefficient]) return retained > required[coefficient];
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

bool dc_kernel_rejects(const DCState &state) {
    if (dc_kernel_exact_cores.empty()) return false;
    const std::uint64_t subset_count = std::uint64_t{1} << state.size();
    const std::uint64_t first_subset = dc_kernel_upward_substate ? 1 : subset_count - 1;
    for (std::uint64_t mask = first_subset; mask < subset_count; ++mask) {
        DCState subset;
        for (std::size_t index = 0; index < state.size(); ++index)
            if (mask & (std::uint64_t{1} << index)) subset.push_back(state[index]);
        if (dc_kernel_exact_cores.contains(make_dc_key(subset, 0))) return true;
        if (!dc_kernel_upward_substate) break;
    }
    return false;
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
    if (dc_kernel_rejects(state)) {
        dc_memo.emplace(key, false);
        return false;
    }
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

bool construct(const State &input, int depth, bool allow_complete_product = true);
bool construct_guided(const State &input, int depth);
bool construct_flat_root(const State &input, int depth, bool report_progress = true);
bool construct_cover_root(const State &input, int depth, bool stop_after_pure = false,
                          bool *stopped_at_pure = nullptr,
                          bool guide_mixed = false,
                          const CoverWRange *w_range = nullptr);
bool mixed_path_possible(const State &input, int depth);

bool necessary_child_possible(const State &state, int depth) {
    return d_lineage_possible(state) && mixed_supply_possible(state, depth) &&
           height_aware_mixed_possible(state, depth) && full_star_eventual(state) &&
           construct_dc(project_dc(state), depth);
}

std::array<int, 3> height_score(const LocalChildren &children) {
    std::array<int, 3> score{};
    for (int outcome = 0; outcome < 3; ++outcome)
        for (Part part : children.state[outcome]) score[outcome] += part.height;
    std::sort(score.begin(), score.end(), std::greater<>());
    return score;
}

std::vector<Option> viable_part_options(Part part, const State &whole_state, int depth) {
    std::vector<Option> out;
    for (ProfileId selected : cuts[part.profile]) {
        for (int selected_height = 0; selected_height <= part.height; ++selected_height) {
            ++counters.raw_options;
            LocalChildren children = split_part(part, selected, selected_height);
            Option candidate{selected, static_cast<std::uint8_t>(selected_height),
                             std::move(children)};
            if (!mixed_supply_loss_possible(
                    whole_state, depth, local_mixed_supply_loss(part, candidate))) {
                ++counters.supply_loss_rejects;
                continue;
            }
            bool possible = true;
            for (const State &child : candidate.children.state) {
                if (!construct(child, depth - 1)) {
                    possible = false;
                    break;
                }
            }
            if (possible)
                out.push_back(std::move(candidate));
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

// Flat/product searches do not need to prove each one-part child exactly before assembling the
// rest of the test.  Doing so repeats the same large product search for many related parent
// states.  Retain every local option whose three children pass the sound symbolic/projected
// bounds; exact recursion is still applied to each complete global child.
std::vector<Option> necessary_part_options(Part part, const State &whole_state, int depth) {
    std::vector<Option> out;
    for (ProfileId selected : cuts[part.profile]) {
        for (int selected_height = 0; selected_height <= part.height; ++selected_height) {
            ++counters.raw_options;
            LocalChildren children = split_part(part, selected, selected_height);
            Option candidate{selected, static_cast<std::uint8_t>(selected_height),
                             std::move(children)};
            if (!mixed_supply_loss_possible(
                    whole_state, depth, local_mixed_supply_loss(part, candidate))) {
                ++counters.supply_loss_rejects;
                continue;
            }
            bool possible = true;
            for (const State &child : candidate.children.state) {
                if (!necessary_child_possible(child, depth - 1)) {
                    possible = false;
                    break;
                }
            }
            if (possible) out.push_back(std::move(candidate));
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

// Exact reachability for the single adversarial transcript that answers "mixed" at every test.
// This drops both pure-outcome obligations, so YES is only permission for the full search.  A NO,
// however, is a sound bounded-depth obstruction: every genuine strategy must also handle this
// transcript.  The recursion still assembles one globally consistent cut at every level; the
// symbolic and projected tests applied to partial cuts are only sound necessary filters.
bool mixed_path_possible(const State &input, int depth) {
    ++counters.mixed_path_calls;
    State state = input;
    normalize(state);
    const Key key = make_key(state, depth);
    if (const auto found = mixed_path_memo.find(key); found != mixed_path_memo.end())
        return found->second;
    if (!necessary_child_possible(state, depth)) {
        mixed_path_memo.emplace(key, false);
        return false;
    }
    if (state.empty() || singleton(state)) {
        mixed_path_memo.emplace(key, true);
        return true;
    }
    if (depth == 0) {
        mixed_path_memo.emplace(key, false);
        return false;
    }

    struct Candidate {
        int original{};
        Part part{};
        std::vector<Option> options;
    };
    std::vector<Candidate> candidates;
    for (std::size_t part_index = 0; part_index < state.size(); ++part_index) {
        const Part part = state[part_index];
        std::vector<Option> options;
        for (ProfileId selected : cuts[part.profile]) {
            for (int selected_height = 0; selected_height <= part.height;
                 ++selected_height) {
                LocalChildren children = split_part(part, selected, selected_height);
                Option option{selected, static_cast<std::uint8_t>(selected_height),
                              std::move(children)};
                if (!mixed_supply_loss_possible(
                        state, depth, local_mixed_supply_loss(part, option)))
                    continue;
                const State &child = option.children.state[1];
                if (necessary_child_possible(child, depth - 1))
                    options.push_back(std::move(option));
            }
        }
        if (options.empty()) {
            mixed_path_memo.emplace(key, false);
            return false;
        }
        candidates.push_back(
            {static_cast<int>(part_index), part, std::move(options)});
    }
    std::stable_sort(candidates.begin(), candidates.end(), [](const Candidate &left,
                                                              const Candidate &right) {
        if (left.options.size() != right.options.size())
            return left.options.size() < right.options.size();
        const int comparison = compare_profiles(left.part.profile, right.part.profile);
        if (comparison != 0) return comparison < 0;
        return left.part.height > right.part.height;
    });

    std::vector<Split> selected_original(state.size());
    auto combine = [&](auto &&self, int index, const State &partial,
                       Deficit accumulated_loss) -> bool {
        if (index == static_cast<int>(candidates.size())) {
            if (!mixed_path_possible(partial, depth - 1)) return false;
            mixed_path_witnesses[key] = selected_original;
            return true;
        }
        const Candidate &candidate = candidates[index];
        for (const Option &option : candidate.options) {
            ++counters.mixed_path_assignments;
            const Deficit local_loss = local_mixed_supply_loss(candidate.part, option);
            Deficit next_loss{};
            for (int coefficient = 0; coefficient < 3; ++coefficient)
                next_loss[coefficient] =
                    accumulated_loss[coefficient] + local_loss[coefficient];
            if (!mixed_supply_loss_possible(state, depth, next_loss)) continue;
            const State next = add_states(partial, option.children.state[1]);
            if (!necessary_child_possible(next, depth - 1))
                continue;
            selected_original[candidate.original] =
                {option.selected_profile, option.selected_height};
            if (self(self, index + 1, next, next_loss)) return true;
        }
        return false;
    };

    const bool answer = combine(combine, 0, State{}, Deficit{});
    mixed_path_memo.emplace(key, answer);
    return answer;
}

// Exact recursion guided by the sound (D,C+D) projection.  A projected winning split fixes the
// selected D count, selected C+D count and height of every local cut; the only remaining exact
// freedom is the selected B count.  Enumerating projected splits first therefore changes search
// order only.  A positive still carries a full three-coordinate witness, and a negative still
// exhausts every exact aligned split at the requested depth.
bool construct_guided(const State &input, int depth) {
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
    if (!height_aware_mixed_possible(state, depth)) {
        ++counters.mixed_envelope_rejects;
        remember(key, false);
        return false;
    }
    if (!full_star_eventual(state)) {
        remember(key, false);
        return false;
    }
    const DCState projected = project_dc(state);
    if (!construct_dc(projected, depth)) {
        ++counters.dc_rejects;
        remember(key, false);
        return false;
    }
    if (!mixed_path_possible(state, depth)) {
        ++counters.mixed_path_rejects;
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

    struct ProjectedCandidate {
        int original{};
        Part exact_part{};
        DCPart projected_part{};
        std::vector<DCOption> options;
    };
    std::vector<DCSplit> preferred_projected(state.size());
    bool have_preferred_projected = false;
    if (const auto found = dc_witnesses.find(make_dc_key(projected, depth));
        found != dc_witnesses.end()) {
        std::vector<bool> used(projected.size(), false);
        have_preferred_projected = true;
        for (std::size_t original = 0; original < state.size(); ++original) {
            const Counts &count = profiles[state[original].profile].count;
            const DCPart wanted{
                count[3], static_cast<std::uint8_t>(count[2] + count[3]),
                state[original].height};
            bool matched = false;
            for (std::size_t index = 0; index < projected.size(); ++index) {
                if (!used[index] && projected[index] == wanted) {
                    used[index] = true;
                    preferred_projected[original] = found->second[index];
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                have_preferred_projected = false;
                break;
            }
        }
    }
    std::vector<ProjectedCandidate> projected_candidates;
    for (std::size_t original = 0; original < state.size(); ++original) {
        const Part part = state[original];
        const Counts &count = profiles[part.profile].count;
        const DCPart projected_part{
            count[3], static_cast<std::uint8_t>(count[2] + count[3]), part.height};
        std::vector<DCOption> viable;
        for (DCOption option : dc_local_options(projected_part)) {
            bool possible = true;
            for (int outcome : {1, 0, 2})
                if (!construct_dc(option.children[outcome], depth - 1)) {
                    possible = false;
                    break;
                }
            if (possible) viable.push_back(std::move(option));
        }
        if (viable.empty()) {
            remember(key, false);
            return false;
        }
        if (have_preferred_projected) {
            const DCSplit wanted = preferred_projected[original];
            std::stable_sort(viable.begin(), viable.end(),
                             [&](const DCOption &left, const DCOption &right) {
                                 const bool left_match =
                                     left.selected_d == wanted.selected_d &&
                                     left.selected_cd == wanted.selected_cd &&
                                     left.selected_height == wanted.selected_height;
                                 const bool right_match =
                                     right.selected_d == wanted.selected_d &&
                                     right.selected_cd == wanted.selected_cd &&
                                     right.selected_height == wanted.selected_height;
                                 return left_match > right_match;
                             });
        }
        projected_candidates.push_back({static_cast<int>(original), part, projected_part,
                                        std::move(viable)});
    }
    std::stable_sort(projected_candidates.begin(), projected_candidates.end(),
                     [](const ProjectedCandidate &left,
                        const ProjectedCandidate &right) {
                         if (left.options.size() != right.options.size())
                             return left.options.size() < right.options.size();
                         const int comparison =
                             compare_profiles(left.exact_part.profile,
                                              right.exact_part.profile);
                         if (comparison != 0) return comparison < 0;
                         return left.exact_part.height > right.exact_part.height;
                     });

    std::vector<DCSplit> selected_projected_original(state.size());

    auto try_exact_lift = [&]() -> bool {
        struct ExactCandidate {
            int original{};
            Part part{};
            DCSplit projected_split{};
            std::vector<Option> options;
        };
        std::vector<ExactCandidate> exact_candidates;
        for (std::size_t original = 0; original < state.size(); ++original) {
            const Part part = state[original];
            const DCSplit projected_split = selected_projected_original[original];
            std::vector<Option> options;
            for (ProfileId selected_profile : cuts[part.profile]) {
                const Counts &selected = profiles[selected_profile].count;
                if (selected[3] != projected_split.selected_d ||
                    selected[2] + selected[3] != projected_split.selected_cd)
                    continue;
                LocalChildren children =
                    split_part(part, selected_profile,
                               projected_split.selected_height);
                Option option{selected_profile, projected_split.selected_height,
                              std::move(children)};
                if (!mixed_supply_loss_possible(
                        state, depth, local_mixed_supply_loss(part, option))) {
                    ++counters.supply_loss_rejects;
                    continue;
                }
                options.push_back(std::move(option));
            }
            if (options.empty()) return false;
            std::stable_sort(options.begin(), options.end(),
                             [&](const Option &left, const Option &right) {
                                 const Deficit left_loss =
                                     local_mixed_supply_loss(part, left);
                                 const Deficit right_loss =
                                     local_mixed_supply_loss(part, right);
                                 if (left_loss != right_loss) return left_loss < right_loss;
                                 return compare_profiles(left.selected_profile,
                                                         right.selected_profile) < 0;
                             });
            exact_candidates.push_back({static_cast<int>(original), part,
                                        projected_split, std::move(options)});
        }
        std::stable_sort(exact_candidates.begin(), exact_candidates.end(),
                         [](const ExactCandidate &left, const ExactCandidate &right) {
                             if (left.options.size() != right.options.size())
                                 return left.options.size() < right.options.size();
                             const auto left_projection =
                                 std::tuple(left.projected_split.selected_d,
                                            left.projected_split.selected_cd,
                                            left.projected_split.selected_height);
                             const auto right_projection =
                                 std::tuple(right.projected_split.selected_d,
                                            right.projected_split.selected_cd,
                                            right.projected_split.selected_height);
                             if (left_projection != right_projection)
                                 return left_projection < right_projection;
                             const int comparison =
                                 compare_profiles(left.part.profile,
                                                  right.part.profile);
                             if (comparison != 0) return comparison < 0;
                             return left.part.height > right.part.height;
                         });

        std::vector<Split> selected_exact(exact_candidates.size());
        std::vector<Split> selected_exact_original(state.size());
        auto assign_exact = [&](auto &&self, int index, Deficit mixed_loss,
                                const std::array<State, 3> &partial) -> bool {
            if (index == static_cast<int>(exact_candidates.size())) {
                for (int outcome : {1, 0, 2})
                    if (!construct_guided(partial[outcome], depth - 1)) return false;
                witnesses[key] = selected_exact_original;
                return true;
            }
            const ExactCandidate &candidate = exact_candidates[index];
            for (const Option &option : candidate.options) {
                if (index > 0 &&
                    exact_candidates[index - 1].part == candidate.part &&
                    std::tuple(
                        exact_candidates[index - 1].projected_split.selected_d,
                        exact_candidates[index - 1].projected_split.selected_cd,
                        exact_candidates[index - 1].projected_split.selected_height) ==
                        std::tuple(candidate.projected_split.selected_d,
                                   candidate.projected_split.selected_cd,
                                   candidate.projected_split.selected_height)) {
                    const Split previous = selected_exact[index - 1];
                    if (std::pair(option.selected_profile,
                                  option.selected_height) <
                        std::pair(previous.selected_profile,
                                  previous.selected_height))
                        continue;
                }
                ++counters.assignments;
                const Deficit local_loss =
                    local_mixed_supply_loss(candidate.part, option);
                Deficit next_loss{};
                for (int coefficient = 0; coefficient < 3; ++coefficient)
                    next_loss[coefficient] =
                        mixed_loss[coefficient] + local_loss[coefficient];
                if (!mixed_supply_loss_possible(state, depth, next_loss)) {
                    ++counters.supply_loss_rejects;
                    continue;
                }
                std::array<State, 3> next;
                bool possible = true;
                for (int outcome : {1, 0, 2}) {
                    next[outcome] =
                        add_states(partial[outcome], option.children.state[outcome]);
                    if (!necessary_child_possible(next[outcome], depth - 1)) {
                        possible = false;
                        break;
                    }
                }
                if (!possible) {
                    ++counters.prefix_rejects;
                    continue;
                }
                selected_exact[index] =
                    {option.selected_profile, option.selected_height};
                selected_exact_original[candidate.original] = selected_exact[index];
                if (self(self, index + 1, next_loss, next)) return true;
            }
            return false;
        };
        return assign_exact(assign_exact, 0, Deficit{}, std::array<State, 3>{});
    };

    std::vector<DCSplit> selected_sorted(projected_candidates.size());
    auto assign_projected = [&](auto &&self, int index,
                                const std::array<DCState, 3> &partial) -> bool {
        if (index == static_cast<int>(projected_candidates.size()))
            return try_exact_lift();
        const ProjectedCandidate &candidate = projected_candidates[index];
        for (const DCOption &option : candidate.options) {
            const DCSplit split{option.selected_d, option.selected_cd,
                                option.selected_height};
            if (index > 0 &&
                projected_candidates[index - 1].exact_part == candidate.exact_part) {
                const DCSplit previous = selected_sorted[index - 1];
                if (std::tuple(split.selected_d, split.selected_cd,
                               split.selected_height) <
                    std::tuple(previous.selected_d, previous.selected_cd,
                               previous.selected_height))
                    continue;
            }
            std::array<DCState, 3> next;
            bool possible = true;
            for (int outcome : {1, 0, 2}) {
                next[outcome] =
                    add_dc_states(partial[outcome], option.children[outcome]);
                if (!construct_dc(next[outcome], depth - 1)) {
                    possible = false;
                    break;
                }
            }
            if (!possible) continue;
            selected_sorted[index] = split;
            selected_projected_original[candidate.original] = split;
            if (self(self, index + 1, next)) return true;
        }
        return false;
    };

    const bool answer =
        assign_projected(assign_projected, 0, std::array<DCState, 3>{});
    remember(key, answer);
    return answer;
}

bool construct(const State &input, int depth, bool allow_complete_product) {
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
    if (!height_aware_mixed_possible(state, depth)) {
        ++counters.mixed_envelope_rejects;
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
    if (!mixed_path_possible(state, depth)) {
        ++counters.mixed_path_rejects;
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

    const int preserved_mixed_coordinates =
        mixed_supply_preserved_coordinates(state, depth);
    // Two-part states, and small tight states at depths two and three, are much cheaper when
    // complete global cuts are formed before any recursive child is queried.  The ordinary prefix
    // recursion otherwise proves millions of partial states that never occur as outcomes of a
    // complete test.  This changes only search order; construct_flat_root uses the same exact
    // local cuts and child predicate.
    const bool use_complete_product =
        (depth <= 3 && state.size() == 2) ||
        (depth == 2 && state.size() >= 3 && state.size() <= 4 &&
         preserved_mixed_coordinates >= 2) ||
        (depth == 3 && state.size() >= 3 && state.size() <= 4 &&
         preserved_mixed_coordinates >= 1);
    if (allow_complete_product && use_complete_product) {
        const bool answer = construct_flat_root(state, depth, false);
        remember(key, answer);
        return answer;
    }

    struct Candidate {
        int original{};
        Part part{};
        std::vector<Option> options;
    };
    std::vector<Candidate> candidates;
    for (std::size_t i = 0; i < state.size(); ++i) {
        std::vector<Option> options =
            viable_part_options(state[i], state, depth);
        if (options.empty()) {
            remember(key, false);
            return false;
        }
        candidates.push_back({static_cast<int>(i), state[i], std::move(options)});
    }
    if (const auto preferred = mixed_path_witnesses.find(make_key(state, depth));
        preferred != mixed_path_witnesses.end()) {
        for (Candidate &candidate : candidates) {
            const Split wanted = preferred->second[candidate.original];
            std::stable_sort(candidate.options.begin(), candidate.options.end(),
                             [&](const Option &left, const Option &right) {
                                 const bool left_match =
                                     left.selected_profile == wanted.selected_profile &&
                                     left.selected_height == wanted.selected_height;
                                 const bool right_match =
                                     right.selected_profile == wanted.selected_profile &&
                                     right.selected_height == wanted.selected_height;
                                 return left_match > right_match;
                             });
        }
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
    auto dfs = [&](auto &&self, int index, Deficit mixed_supply_loss) -> bool {
        if (index == static_cast<int>(candidates.size())) return true;
        const Candidate &candidate = candidates[index];
        for (const Option &option : candidate.options) {
            if (index > 0 && candidates[index - 1].part == candidate.part) {
                const Split previous = selected[index - 1];
                if (std::pair(option.selected_profile, option.selected_height) <
                    std::pair(previous.selected_profile, previous.selected_height))
                    continue;
            }
            const Deficit local_loss = local_mixed_supply_loss(candidate.part, option);
            Deficit next_mixed_supply_loss{};
            for (int coefficient = 0; coefficient < 3; ++coefficient)
                next_mixed_supply_loss[coefficient] =
                    mixed_supply_loss[coefficient] + local_loss[coefficient];
            if (!mixed_supply_loss_possible(state, depth, next_mixed_supply_loss)) {
                ++counters.supply_loss_rejects;
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
            if (self(self, index + 1, next_mixed_supply_loss)) return true;
            partial = saved;
        }
        return false;
    };

    const bool answer = dfs(dfs, 0, Deficit{});
    remember(key, answer);
    if (answer) witnesses.emplace(key, std::move(selected_original));
    return answer;
}

// The ordinary recursion prunes after each part, which is effective for long states but can be
// counterproductive at a small root: it solves a large number of partial children that never
// occur as complete outcomes.  This exact alternative enumerates the Cartesian product of the
// already viable local options, constructs all three full children, and only then recurses.  It is
// intended as a root diagnostic; descendants still use construct().
bool construct_flat_root(const State &input, int depth, bool report_progress) {
    State state = input;
    normalize(state);
    if (depth <= 0 || state.size() < 2) return construct(state, depth);
    if (!d_lineage_possible(state)) {
        ++counters.lineage_rejects;
        return false;
    }
    if (!mixed_supply_possible(state, depth)) {
        ++counters.supply_rejects;
        return false;
    }
    if (!height_aware_mixed_possible(state, depth)) {
        ++counters.mixed_envelope_rejects;
        return false;
    }
    if (!full_star_eventual(state)) return false;
    if (!construct_dc(project_dc(state), depth)) {
        ++counters.dc_rejects;
        return false;
    }
    if (!mixed_path_possible(state, depth)) {
        ++counters.mixed_path_rejects;
        return false;
    }

    struct Candidate {
        int original{};
        Part part{};
        std::vector<Option> options;
    };
    std::vector<Candidate> candidates;
    for (std::size_t i = 0; i < state.size(); ++i) {
        std::vector<Option> options =
            necessary_part_options(state[i], state, depth);
        if (options.empty()) return false;
        candidates.push_back({static_cast<int>(i), state[i], std::move(options)});
    }
    if (report_progress) {
        for (Candidate &candidate : candidates) {
            std::stable_sort(candidate.options.begin(), candidate.options.end(),
                             [&](const Option &left, const Option &right) {
                                 const Deficit available =
                                     refined_supply(candidate.part.profile);
                                 const Deficit left_mixed =
                                     unweighted_supply(left.children.state[1]);
                                 const Deficit right_mixed =
                                     unweighted_supply(right.children.state[1]);
                                 Deficit left_loss{};
                                 Deficit right_loss{};
                                 for (int coefficient = 0; coefficient < 3; ++coefficient) {
                                     left_loss[coefficient] =
                                         available[coefficient] - left_mixed[coefficient];
                                     right_loss[coefficient] =
                                         available[coefficient] - right_mixed[coefficient];
                                 }
                                 return left_loss < right_loss;
                             });
        }
    }
    if (const auto preferred = mixed_path_witnesses.find(make_key(state, depth));
        preferred != mixed_path_witnesses.end()) {
        for (Candidate &candidate : candidates) {
            const Split wanted = preferred->second[candidate.original];
            std::stable_sort(candidate.options.begin(), candidate.options.end(),
                             [&](const Option &left, const Option &right) {
                                 const bool left_match =
                                     left.selected_profile == wanted.selected_profile &&
                                     left.selected_height == wanted.selected_height;
                                 const bool right_match =
                                     right.selected_profile == wanted.selected_profile &&
                                     right.selected_height == wanted.selected_height;
                                 return left_match > right_match;
                             });
        }
    }
    std::stable_sort(candidates.begin(), candidates.end(), [](const Candidate &left,
                                                              const Candidate &right) {
        if (left.options.size() != right.options.size())
            return left.options.size() < right.options.size();
        const int comparison = compare_profiles(left.part.profile, right.part.profile);
        if (comparison != 0) return comparison < 0;
        return left.part.height > right.part.height;
    });
    if (report_progress) {
        std::cerr << "flat_root_options";
        for (const Candidate &candidate : candidates)
            std::cerr << ' ' << candidate.options.size();
        std::cerr << '\n';
    }

    std::vector<Split> selected(candidates.size());
    std::vector<Split> selected_original(candidates.size());
    std::uint64_t flat_combinations = 0;
    std::uint64_t exact_combinations = 0;
    int slow_children_reported = 0;
    const std::array<int, 3> outcome_order =
        report_progress ? std::array<int, 3>{0, 2, 1} : std::array<int, 3>{1, 0, 2};
    auto dfs = [&](auto &&self, int index, Deficit mixed_supply_loss,
                   const std::array<State, 3> &partial) -> bool {
        if (index < static_cast<int>(candidates.size())) {
            const Candidate &candidate = candidates[index];
            for (const Option &option : candidate.options) {
                if (index > 0 && candidates[index - 1].part == candidate.part) {
                    const Split previous = selected[index - 1];
                    if (std::pair(option.selected_profile, option.selected_height) <
                        std::pair(previous.selected_profile, previous.selected_height))
                        continue;
                }
                const Deficit local_loss = local_mixed_supply_loss(candidate.part, option);
                Deficit next_mixed_supply_loss{};
                for (int coefficient = 0; coefficient < 3; ++coefficient)
                    next_mixed_supply_loss[coefficient] =
                        mixed_supply_loss[coefficient] + local_loss[coefficient];
                if (!mixed_supply_loss_possible(state, depth, next_mixed_supply_loss)) {
                    ++counters.supply_loss_rejects;
                    continue;
                }
                std::array<State, 3> next;
                bool possible = true;
                for (int outcome : {1, 0, 2}) {
                    next[outcome] =
                        add_states(partial[outcome], option.children.state[outcome]);
                    if (!necessary_child_possible(next[outcome], depth - 1)) {
                        possible = false;
                        break;
                    }
                }
                if (!possible) {
                    ++counters.prefix_rejects;
                    continue;
                }
                selected[index] = {option.selected_profile, option.selected_height};
                selected_original[candidate.original] = selected[index];
                if (self(self, index + 1, next_mixed_supply_loss, next)) return true;
            }
            return false;
        }

        ++counters.assignments;
        ++flat_combinations;
        if (report_progress && flat_combinations % 10000 == 0)
            std::cerr << "flat_root_progress combinations=" << flat_combinations
                      << " exact_memo=" << memo.size() << " dc_memo=" << dc_memo.size()
                      << '\n';
        const std::array<State, 3> &children = partial;
        for (int outcome : outcome_order) {
            if (!mixed_path_possible(children[outcome], depth - 1)) {
                ++counters.mixed_path_rejects;
                ++counters.prefix_rejects;
                return false;
            }
        }
        if (report_progress && flat_combinations == 1) {
            std::cerr << "flat_root_first_candidate split=";
            for (std::size_t i = 0; i < selected_original.size(); ++i) {
                if (i) std::cerr << ',';
                std::cerr << profile_text(selected_original[i].selected_profile) << ':'
                          << static_cast<int>(selected_original[i].selected_height);
            }
            for (int outcome = 0; outcome < 3; ++outcome) {
                std::cerr << " outcome" << outcome << '=';
                for (std::size_t i = 0; i < children[outcome].size(); ++i) {
                    if (i) std::cerr << ',';
                    std::cerr << profile_text(children[outcome][i].profile) << ':'
                              << static_cast<int>(children[outcome][i].height);
                }
            }
            std::cerr << '\n';
        }
        ++exact_combinations;
        if (report_progress && exact_combinations == 1) {
            std::cerr << "flat_root_first_exact_candidate combination=" << flat_combinations
                      << " split=";
            for (std::size_t i = 0; i < selected_original.size(); ++i) {
                if (i) std::cerr << ',';
                std::cerr << profile_text(selected_original[i].selected_profile) << ':'
                          << static_cast<int>(selected_original[i].selected_height);
            }
            for (int outcome = 0; outcome < 3; ++outcome) {
                std::cerr << " outcome" << outcome << '=';
                for (std::size_t i = 0; i < children[outcome].size(); ++i) {
                    if (i) std::cerr << ',';
                    std::cerr << profile_text(children[outcome][i].profile) << ':'
                              << static_cast<int>(children[outcome][i].height);
                }
            }
            std::cerr << '\n';
        }
        for (int outcome : outcome_order) {
            const auto child_started = std::chrono::steady_clock::now();
            const bool child_answer = construct(children[outcome], depth - 1);
            const double child_seconds = std::chrono::duration<double>(
                                             std::chrono::steady_clock::now() - child_started)
                                             .count();
            if (report_progress && child_seconds >= 1.0 && slow_children_reported < 20) {
                ++slow_children_reported;
                std::cerr << "flat_root_slow_child outcome=" << outcome
                          << " seconds=" << child_seconds
                          << " answer=" << (child_answer ? "YES" : "NO") << " state=";
                for (std::size_t i = 0; i < children[outcome].size(); ++i) {
                    if (i) std::cerr << ',';
                    std::cerr << profile_text(children[outcome][i].profile) << ':'
                              << static_cast<int>(children[outcome][i].height);
                }
                std::cerr << '\n';
            }
            if (!child_answer) {
                ++counters.prefix_rejects;
                return false;
            }
        }
        witnesses.emplace(make_key(state, depth), selected_original);
        return true;
    };
    return dfs(dfs, 0, Deficit{}, std::array<State, 3>{});
}

// A complete root product can still be expensive when many different cuts share the same hard
// child.  The flat order above follows cuts one by one and may therefore solve an uncommon child
// before discovering the common obstruction.  This alternative first materializes every root cut
// that passes the inexpensive exact bounds, interns its three children, and then solves the
// unresolved child occurring in the largest number of surviving cuts.  A negative child removes
// all of those cuts at once; a positive child is cached and reused.  The candidate set and child
// predicate are identical to construct_flat_root, so this changes search order only.
bool construct_cover_root(const State &input, int depth, bool stop_after_pure,
                          bool *stopped_at_pure, bool guide_mixed,
                          const CoverWRange *w_range) {
    State state = input;
    normalize(state);
    if (depth <= 0 || state.size() < 2) return construct(state, depth);
    if (!d_lineage_possible(state)) {
        ++counters.lineage_rejects;
        return false;
    }
    if (!mixed_supply_possible(state, depth)) {
        ++counters.supply_rejects;
        return false;
    }
    if (!height_aware_mixed_possible(state, depth)) {
        ++counters.mixed_envelope_rejects;
        return false;
    }
    if (!full_star_eventual(state)) return false;
    if (!construct_dc(project_dc(state), depth)) {
        ++counters.dc_rejects;
        return false;
    }
    if (!mixed_path_possible(state, depth)) {
        ++counters.mixed_path_rejects;
        return false;
    }

    struct Candidate {
        int original{};
        Part part{};
        std::vector<Option> options;
    };
    std::vector<Candidate> candidates;
    for (std::size_t i = 0; i < state.size(); ++i) {
        std::vector<Option> options = necessary_part_options(state[i], state, depth);
        if (options.empty()) return false;
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
    std::cerr << "cover_root_options";
    for (const Candidate &candidate : candidates)
        std::cerr << ' ' << candidate.options.size();
    std::cerr << '\n';

    struct Combination {
        std::array<int, 3> child_ids{};
        std::array<Split, MAX_PARTS> split{};
        bool active{true};
    };
    std::vector<Combination> combinations;
    std::vector<State> child_states;
    std::unordered_map<Key, int, KeyHash> child_ids;
    std::vector<Split> selected(candidates.size());
    std::vector<Split> selected_original(candidates.size());
    std::uint64_t products = 0;
    std::uint64_t cheap_rejects = 0;

    auto intern_child = [&](const State &child) {
        const Key key = make_key(child, depth - 1);
        if (const auto found = child_ids.find(key); found != child_ids.end())
            return found->second;
        const int id = static_cast<int>(child_states.size());
        child_states.push_back(child);
        child_ids.emplace(key, id);
        return id;
    };

    auto enumerate = [&](auto &&self, int index, Deficit mixed_supply_loss,
                         const std::array<State, 3> &partial) -> void {
        if (index < static_cast<int>(candidates.size())) {
            const Candidate &candidate = candidates[index];
            for (const Option &option : candidate.options) {
                if (index > 0 && candidates[index - 1].part == candidate.part) {
                    const Split previous = selected[index - 1];
                    if (std::pair(option.selected_profile, option.selected_height) <
                        std::pair(previous.selected_profile, previous.selected_height))
                        continue;
                }
                const Deficit local_loss = local_mixed_supply_loss(candidate.part, option);
                Deficit next_loss{};
                for (int coefficient = 0; coefficient < 3; ++coefficient)
                    next_loss[coefficient] =
                        mixed_supply_loss[coefficient] + local_loss[coefficient];
                if (!mixed_supply_loss_possible(state, depth, next_loss)) {
                    ++counters.supply_loss_rejects;
                    continue;
                }
                std::array<State, 3> next;
                bool possible = true;
                for (int outcome : {1, 0, 2}) {
                    next[outcome] =
                        add_states(partial[outcome], option.children.state[outcome]);
                    if (!necessary_child_possible(next[outcome], depth - 1)) {
                        possible = false;
                        break;
                    }
                }
                if (!possible) {
                    ++cheap_rejects;
                    continue;
                }
                selected[index] = {option.selected_profile, option.selected_height};
                selected_original[candidate.original] = selected[index];
                self(self, index + 1, next_loss, next);
            }
            return;
        }

        ++products;
        if (w_range != nullptr &&
            (mixed_supply_loss[0] != 0 || mixed_supply_loss[1] != 0 ||
             mixed_supply_loss[2] < w_range->minimum ||
             mixed_supply_loss[2] > w_range->maximum))
            return;
        Combination combination;
        for (int outcome = 0; outcome < 3; ++outcome)
            combination.child_ids[outcome] = intern_child(partial[outcome]);
        for (std::size_t i = 0; i < selected_original.size(); ++i)
            combination.split[i] = selected_original[i];
        combinations.push_back(combination);
    };
    enumerate(enumerate, 0, Deficit{}, std::array<State, 3>{});
    counters.assignments += products;
    counters.prefix_rejects += cheap_rejects;
    std::cerr << "cover_root_materialized products=" << products
              << " cheap_rejects=" << cheap_rejects
              << " candidates=" << combinations.size()
              << " unique_children=" << child_states.size();
    if (w_range != nullptr)
        std::cerr << " slice_loss=0,0," << w_range->minimum << ".."
                  << w_range->maximum;
    std::cerr << '\n';

    enum class Answer : std::uint8_t { UNKNOWN, NO, YES };
    std::vector<Answer> answers(child_states.size(), Answer::UNKNOWN);
    for (std::size_t id = 0; id < child_states.size(); ++id) {
        const auto found = memo.find(make_key(child_states[id], depth - 1));
        if (found != memo.end())
            answers[id] = found->second ? Answer::YES : Answer::NO;
    }

    std::size_t active_count = combinations.size();
    std::size_t solved_children = 0;
    bool pure_frontier_reported = false;
    while (active_count > 0) {
        std::vector<std::uint64_t> pure_frequency(child_states.size());
        std::vector<std::uint64_t> mixed_frequency(child_states.size());
        bool removed = false;
        for (Combination &combination : combinations) {
            if (!combination.active) continue;
            bool rejected = false;
            bool complete = true;
            for (int child_id : combination.child_ids) {
                if (answers[child_id] == Answer::NO) {
                    rejected = true;
                    break;
                }
                if (answers[child_id] != Answer::YES) complete = false;
            }
            if (rejected) {
                combination.active = false;
                --active_count;
                removed = true;
                continue;
            }
            if (complete) {
                std::vector<Split> root_split(state.size());
                for (std::size_t i = 0; i < state.size(); ++i)
                    root_split[i] = combination.split[i];
                witnesses[make_key(state, depth)] = std::move(root_split);
                std::cerr << "cover_root_result answer=YES active=" << active_count
                          << " solved_children=" << solved_children << '\n';
                return true;
            }
            for (int outcome : {0, 2}) {
                const int child_id = combination.child_ids[outcome];
                if (answers[child_id] == Answer::UNKNOWN) ++pure_frequency[child_id];
            }
            const int mixed_child = combination.child_ids[1];
            if (answers[mixed_child] == Answer::UNKNOWN)
                ++mixed_frequency[mixed_child];
        }
        if (active_count == 0) break;
        if (removed) continue;

        const bool unresolved_pure =
            std::any_of(pure_frequency.begin(), pure_frequency.end(),
                        [](std::uint64_t count) { return count != 0; });
        if (!unresolved_pure && !pure_frontier_reported) {
            pure_frontier_reported = true;
            struct LossClass {
                std::size_t candidates{};
                std::set<int> mixed_children;
            };
            std::map<Deficit, LossClass> loss_classes;
            std::set<int> all_mixed_children;
            Deficit refined_root{};
            for (Part part : state) {
                const Deficit refined = refined_supply(part.profile);
                for (int coefficient = 0; coefficient < 3; ++coefficient)
                    refined_root[coefficient] += refined[coefficient];
            }
            for (const Combination &combination : combinations) {
                if (!combination.active) continue;
                const int child_id = combination.child_ids[1];
                all_mixed_children.insert(child_id);
                const Deficit child_supply =
                    unweighted_supply(child_states[child_id]);
                Deficit loss{};
                for (int coefficient = 0; coefficient < 3; ++coefficient)
                    loss[coefficient] =
                        refined_root[coefficient] - child_supply[coefficient];
                LossClass &summary = loss_classes[loss];
                ++summary.candidates;
                summary.mixed_children.insert(child_id);
            }
            std::cerr << "cover_root_pure_frontier candidates=" << active_count
                      << " unique_mixed_children=" << all_mixed_children.size()
                      << " loss_classes=" << loss_classes.size()
                      << " pure_children_solved=" << solved_children << '\n';
            for (const auto &[loss, summary] : loss_classes)
                std::cerr << "cover_root_loss loss=" << loss[0] << ',' << loss[1]
                          << ',' << loss[2]
                          << " candidates=" << summary.candidates
                          << " unique_mixed_children="
                          << summary.mixed_children.size() << '\n';
            if (stop_after_pure) {
                if (stopped_at_pure != nullptr) *stopped_at_pure = true;
                return false;
            }
        }
        int chosen = -1;
        const std::vector<std::uint64_t> &frequency =
            unresolved_pure ? pure_frequency : mixed_frequency;
        for (std::size_t id = 0; id < frequency.size(); ++id) {
            if (frequency[id] == 0) continue;
            if (unresolved_pure) {
                const auto score = std::tuple(
                    frequency[id], total_height(child_states[id]),
                    mixed_supply_preserved_coordinates(child_states[id], depth - 1));
                const auto chosen_score =
                    chosen < 0
                        ? std::tuple<std::uint64_t, std::size_t, int>{0, 0, 0}
                        : std::tuple(
                              frequency[chosen], total_height(child_states[chosen]),
                              mixed_supply_preserved_coordinates(child_states[chosen],
                                                                 depth - 1));
                if (chosen < 0 || score > chosen_score)
                    chosen = static_cast<int>(id);
            } else {
                // Once both pure outcomes are known, try the mixed child with the most retained
                // symbolic supply first.  A positive child completes a root strategy, so this is
                // the constructive analogue of the frequency-first negative coverage order.
                const Deficit supply = unweighted_supply(child_states[id]);
                const Deficit chosen_supply =
                    chosen < 0 ? Deficit{} : unweighted_supply(child_states[chosen]);
                const auto score = std::tuple(supply[0], supply[1], supply[2],
                                              frequency[id], total_height(child_states[id]));
                const auto chosen_score =
                    std::tuple(chosen_supply[0], chosen_supply[1], chosen_supply[2],
                               chosen < 0 ? std::uint64_t{} : frequency[chosen],
                               chosen < 0 ? std::size_t{} : total_height(child_states[chosen]));
                if (chosen < 0 || score > chosen_score)
                    chosen = static_cast<int>(id);
            }
        }
        if (chosen < 0)
            throw std::logic_error("cover search has active cuts but no unresolved child");

        if (!unresolved_pure)
            std::cerr << "cover_root_mixed_start depth=" << depth << " id=" << chosen
                      << " kind=" << (unresolved_pure ? "pure" : "mixed")
                      << " frequency=" << frequency[chosen]
                      << " active=" << active_count
                      << " solved_children=" << solved_children << " state=";
        if (!unresolved_pure) {
            for (std::size_t i = 0; i < child_states[chosen].size(); ++i) {
                if (i) std::cerr << ',';
                std::cerr << profile_text(child_states[chosen][i].profile) << ':'
                          << static_cast<int>(child_states[chosen][i].height);
            }
            std::cerr << '\n';
        }
        const auto started = std::chrono::steady_clock::now();
        bool answer;
        if (guide_mixed && !unresolved_pure) {
            answer = construct_guided(child_states[chosen], depth - 1);
        } else {
            answer = construct(child_states[chosen], depth - 1);
        }
        const double seconds = std::chrono::duration<double>(
                                   std::chrono::steady_clock::now() - started)
                                   .count();
        answers[chosen] = answer ? Answer::YES : Answer::NO;
        ++solved_children;
        if (solved_children <= 20 || solved_children % 50 == 0 || seconds >= 1.0) {
            std::cerr << "cover_root_child id=" << chosen
                      << " kind=" << (unresolved_pure ? "pure" : "mixed")
                      << " frequency=" << frequency[chosen]
                      << " answer=" << (answer ? "YES" : "NO")
                      << " seconds=" << seconds << " active=" << active_count
                      << " solved_children=" << solved_children << " state=";
            for (std::size_t i = 0; i < child_states[chosen].size(); ++i) {
                if (i) std::cerr << ',';
                std::cerr << profile_text(child_states[chosen][i].profile) << ':'
                          << static_cast<int>(child_states[chosen][i].height);
            }
            std::cerr << '\n';
        }
    }

    std::cerr << "cover_root_result answer=NO active=0 solved_children="
              << solved_children << '\n';
    return false;
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
    mixed_path_memo.clear();
    mixed_path_witnesses.clear();
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
              << " supply_loss_rejects=" << counters.supply_loss_rejects
              << " mixed_envelope_rejects=" << counters.mixed_envelope_rejects
              << " mixed_path_rejects=" << counters.mixed_path_rejects
              << " mixed_path_calls=" << counters.mixed_path_calls
              << " mixed_path_assignments=" << counters.mixed_path_assignments
              << " dc_rejects=" << counters.dc_rejects << " dc_memo=" << dc_memo.size()
              << " dc_kernel_exact_cores=" << dc_kernel_exact_cores.size()
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

void load_dc_kernel_exact_cores(const char *path) {
    std::ifstream input(path);
    if (!input) throw std::invalid_argument("cannot open DC kernel certificate");
    dc_kernel_exact_cores.clear();
    dc_kernel_upward_substate = false;
    std::string line;
    bool matching_atoms = false;
    bool all_depth_verdict = false;
    while (std::getline(input, line)) {
        if (line.starts_with("dc_kernel_certificate ")) {
            matching_atoms =
                line.find("profile_atoms=" + std::to_string(PROFILE_ATOMS) + " ") !=
                std::string::npos;
            all_depth_verdict =
                line.find("verdict=ALL_DEPTH_NO") != std::string::npos;
            dc_kernel_upward_substate =
                line.find("closure=upward_substate") != std::string::npos;
            continue;
        }
        constexpr const char *PREFIX = "dc_kernel_state state=";
        if (!line.starts_with(PREFIX)) continue;
        DCState state;
        const char *cursor = line.c_str() + std::char_traits<char>::length(PREFIX);
        while (*cursor) {
            int d = -1;
            int cd = -1;
            int height = -1;
            int consumed = 0;
            if (std::sscanf(cursor, "(%d,%d):%d%n", &d, &cd, &height, &consumed) != 3 ||
                consumed <= 0 || d < 0 || d > cd || cd > PROFILE_ATOMS || height < 1 ||
                height > MAX_HEIGHT)
                throw std::invalid_argument("malformed DC kernel state");
            state.push_back({static_cast<std::uint8_t>(d), static_cast<std::uint8_t>(cd),
                             static_cast<std::uint8_t>(height)});
            cursor += consumed;
            if (*cursor == ',') ++cursor;
            else if (*cursor != '\0')
                throw std::invalid_argument("malformed DC kernel separator");
        }
        normalize_dc(state);
        dc_kernel_exact_cores.insert(make_dc_key(state, 0));
    }
    if (!matching_atoms || !all_depth_verdict || !dc_kernel_upward_substate ||
        dc_kernel_exact_cores.empty())
        throw std::invalid_argument(
            "DC kernel certificate has wrong normalization, verdict, or no cores");
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

        if (argc >= 4 &&
            (std::string(argv[1]) == "profile-state" ||
             std::string(argv[1]) == "profile-state-prefix" ||
             std::string(argv[1]) == "profile-state-prefix-dc-kernel" ||
             std::string(argv[1]) == "profile-state-guided" ||
             std::string(argv[1]) == "profile-state-guided-dc-kernel" ||
             std::string(argv[1]) == "profile-state-flat" ||
             std::string(argv[1]) == "profile-state-flat-dc-kernel" ||
             std::string(argv[1]) == "profile-state-cover" ||
             std::string(argv[1]) == "profile-state-cover-dc-kernel" ||
             std::string(argv[1]) == "profile-state-cover-guided" ||
             std::string(argv[1]) == "profile-state-cover-guided-dc-kernel" ||
             std::string(argv[1]) == "profile-state-cover-guided-w-range" ||
             std::string(argv[1]) ==
                 "profile-state-cover-guided-w-range-dc-kernel" ||
             std::string(argv[1]) == "profile-state-pure-frontier" ||
             std::string(argv[1]) == "profile-state-pure-frontier-dc-kernel")) {
            const std::string mode = argv[1];
            const bool flat_root = mode == "profile-state-flat" ||
                                   mode == "profile-state-flat-dc-kernel";
            const bool guided = mode == "profile-state-guided" ||
                                mode == "profile-state-guided-dc-kernel";
            const bool cover_root = mode == "profile-state-cover" ||
                                    mode == "profile-state-cover-dc-kernel" ||
                                    mode == "profile-state-cover-guided" ||
                                    mode == "profile-state-cover-guided-dc-kernel" ||
                                    mode == "profile-state-cover-guided-w-range" ||
                                    mode ==
                                        "profile-state-cover-guided-w-range-dc-kernel" ||
                                    mode == "profile-state-pure-frontier" ||
                                    mode == "profile-state-pure-frontier-dc-kernel";
            const bool guided_cover =
                mode == "profile-state-cover-guided" ||
                mode == "profile-state-cover-guided-dc-kernel" ||
                mode == "profile-state-cover-guided-w-range" ||
                mode == "profile-state-cover-guided-w-range-dc-kernel";
            const bool w_range_mode =
                mode == "profile-state-cover-guided-w-range" ||
                mode == "profile-state-cover-guided-w-range-dc-kernel";
            const bool pure_frontier = mode == "profile-state-pure-frontier" ||
                                       mode == "profile-state-pure-frontier-dc-kernel";
            const bool prefix_root = mode == "profile-state-prefix" ||
                                     mode == "profile-state-prefix-dc-kernel";
            const bool load_kernel = mode == "profile-state-flat-dc-kernel" ||
                                     mode == "profile-state-prefix-dc-kernel" ||
                                     mode == "profile-state-guided-dc-kernel" ||
                                     mode == "profile-state-cover-dc-kernel" ||
                                     mode == "profile-state-cover-guided-dc-kernel" ||
                                     mode ==
                                         "profile-state-cover-guided-w-range-dc-kernel" ||
                                     mode == "profile-state-pure-frontier-dc-kernel";
            if (load_kernel && argc < 5)
                throw std::invalid_argument(
                    "DC-kernel profile mode requires a certificate and a state");
            const int depth = parse_nonnegative(argv[2], "depth", 255);
            int first_part = load_kernel ? 4 : 3;
            if (load_kernel) load_dc_kernel_exact_cores(argv[3]);
            CoverWRange w_range;
            if (w_range_mode) {
                if (argc <= first_part + 2)
                    throw std::invalid_argument(
                        "guided W-range mode requires MIN_W MAX_W and a state");
                w_range.minimum =
                    parse_nonnegative(argv[first_part], "minimum W loss", 1000000);
                w_range.maximum =
                    parse_nonnegative(argv[first_part + 1], "maximum W loss", 1000000);
                if (w_range.maximum < w_range.minimum)
                    throw std::invalid_argument("maximum W loss is below minimum W loss");
                first_part += 2;
            }
            State state;
            for (int i = first_part; i < argc; ++i)
                state.push_back(parse_profile_part(argv[i]));
            normalize(state);
            if (total_height(state) > MAX_HEIGHT)
                throw std::invalid_argument("profile state exceeds height bound");
            reset_search();
            const auto started = std::chrono::steady_clock::now();
            bool stopped_at_pure = false;
            const bool answer = cover_root
                                    ? construct_cover_root(state, depth, pure_frontier,
                                                           &stopped_at_pure,
                                                           guided_cover,
                                                           w_range_mode ? &w_range : nullptr)
                                : guided ? construct_guided(state, depth)
                                : flat_root ? construct_flat_root(state, depth)
                                            : construct(state, depth, !prefix_root);
            const double seconds = std::chrono::duration<double>(
                                       std::chrono::steady_clock::now() - started)
                                       .count();
            if (pure_frontier && stopped_at_pure) {
                std::cout << "atom_profile_pure_frontier depth=" << depth
                          << " state=";
                print_state(state);
                std::cout
                          << " surviving_candidates_have_exact_pure_outcomes=YES"
                          << " mixed_outcome=UNRESOLVED"
                          << " restricted=power_of_two_atom_aligned"
                          << " profile_atoms=" << PROFILE_ATOMS
                          << " wall=" << seconds << "s\n";
                return 0;
            }
            if (w_range_mode && !answer) {
                std::cout << "atom_profile_cover_slice depth=" << depth
                          << " answer=NO loss_D=0 loss_V=0 loss_W="
                          << w_range.minimum << ".." << w_range.maximum
                          << " state=";
                print_state(state);
                std::cout << " scope=declared_root_loss_slice_only"
                          << " eventual_model=YES"
                          << " restricted=power_of_two_atom_aligned"
                          << " profile_atoms=" << PROFILE_ATOMS
                          << " wall=" << seconds << "s calls=" << counters.calls
                          << " memo_hits=" << counters.memo_hits
                          << " memo=" << memo.size()
                          << " assignments=" << counters.assignments << '\n';
                return 1;
            }
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
                  << "       " << argv[0]
                  << " profile-state-prefix depth WORD:height [...]\n"
                  << "       " << argv[0]
                  << " profile-state-prefix-dc-kernel depth CERT WORD:height [...]\n"
                  << "       " << argv[0]
                  << " profile-state-guided depth WORD:height [...]\n"
                  << "       " << argv[0]
                  << " profile-state-guided-dc-kernel depth CERT WORD:height [...]\n"
                  << "       " << argv[0]
                  << " profile-state-flat depth WORD:height [...]\n"
                  << "       " << argv[0]
                  << " profile-state-flat-dc-kernel depth CERT WORD:height [...]\n"
                  << "       " << argv[0]
                  << " profile-state-cover depth WORD:height [...]\n"
                  << "       " << argv[0]
                  << " profile-state-cover-dc-kernel depth CERT WORD:height [...]\n"
                  << "       " << argv[0]
                  << " profile-state-cover-guided depth WORD:height [...]\n"
                  << "       " << argv[0]
                  << " profile-state-cover-guided-dc-kernel depth CERT WORD:height [...]\n"
                  << "       " << argv[0]
                  << " profile-state-cover-guided-w-range depth MIN_W MAX_W"
                     " WORD:height [...]\n"
                  << "       " << argv[0]
                  << " profile-state-cover-guided-w-range-dc-kernel depth CERT MIN_W MAX_W"
                     " WORD:height [...]\n"
                  << "       " << argv[0]
                  << " profile-state-pure-frontier depth WORD:height [...]\n"
                  << "       " << argv[0]
                  << " profile-state-pure-frontier-dc-kernel depth CERT WORD:height [...]\n"
                  << "       " << argv[0] << " height6-dc-kernel-certificate\n"
                  << "       " << argv[0] << " height6-lineage-certificate\n";
        return 2;
    } catch (const std::exception &error) {
        std::cerr << "ABORT: " << error.what() << " (not a negative verdict)\n";
        return 3;
    }
}
