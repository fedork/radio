#include <algorithm>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

// Diagnostics for repeated Pascal halving in the balanced-column model.
//
// For every exact-support full-mass parent a <= G_K, construct the canonical
// bipartite Havel--Hakimi realization: process the conjugate columns of G_K in
// nonincreasing order and connect each column to the rows of largest current
// residual degree, breaking ties by original row index.  Then ask whether one
// row bisection meets the exact Pascal quota of balanced columns at every
// equal-degree rank.  The canonical realization alone is false at K=5.  The
// joint diagnostic additionally permits balanced row swaps and degree-
// preserving 2x2 incidence switches, taking a strict decrease in the quota
// energy or one neutral setup move followed by a strict decrease.  Failure of
// any restricted rule here would not refute the Singleton Majorization Lemma.

namespace {

using Sequence = std::vector<int>;

int choose(int n, int k) {
    if (k < 0 || k > n) return 0;
    k = std::min(k, n - k);
    int result = 1;
    for (int i = 1; i <= k; ++i) result = result * (n - k + i) / i;
    return result;
}

Sequence singleton_base(int k) {
    Sequence current{1};
    for (int level = 0; level < k; ++level) {
        Sequence next(2 * current.size(), 0);
        for (std::size_t index = 0; index < current.size(); ++index) {
            next[index] += current[index];
            next[2 * index] += current[index];
            next[2 * index + 1] += current[index];
        }
        std::sort(next.begin(), next.end(), std::greater<int>());
        current = std::move(next);
    }
    return current;
}

std::string show(const Sequence &values) {
    std::string result = "(";
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index) result += ',';
        result += std::to_string(values[index]);
    }
    return result + ')';
}

std::string show_mask(std::uint64_t mask, int rows) {
    std::string result = "{";
    bool first = true;
    for (int row = 0; row < rows; ++row) {
        if ((mask & (std::uint64_t{1} << row)) == 0) continue;
        if (!first) result += ',';
        first = false;
        result += std::to_string(row + 1);
    }
    return result + '}';
}

struct GroupedColorSearch {
    struct Group {
        std::uint64_t signature = 0;
        std::vector<int> rows;
        int alternating_count = 0;
        bool contains_first = false;
    };

    int k;
    int row_count;
    int half;
    const std::vector<std::uint64_t> &supports;
    const std::vector<int> &ranks;
    std::vector<Group> groups;
    std::vector<int> assigned;
    std::vector<int> remaining;
    std::vector<int> choices;
    std::vector<int> solution;
    std::uint64_t nodes = 0;
    std::uint64_t node_cap;
    bool aborted = false;
    bool found = false;

    GroupedColorSearch(int level, int rows,
                       const std::vector<std::uint64_t> &column_supports,
                       const std::vector<int> &column_ranks,
                       std::uint64_t cap)
        : k(level), row_count(rows), half(rows / 2), supports(column_supports),
          ranks(column_ranks), assigned(supports.size(), 0),
          remaining(supports.size(), 0), node_cap(cap) {
        std::map<std::uint64_t, std::vector<int>> by_signature;
        for (int row = 0; row < row_count; ++row) {
            std::uint64_t signature = 0;
            for (std::size_t column = 0; column < supports.size(); ++column)
                if ((supports[column] & (std::uint64_t{1} << row)) != 0)
                    signature |= std::uint64_t{1} << column;
            by_signature[signature].push_back(row);
        }
        for (auto &[signature, members] : by_signature) {
            Group group;
            group.signature = signature;
            group.rows = std::move(members);
            group.contains_first = group.rows.front() == 0;
            for (int row : group.rows)
                if (row % 2 == 0) ++group.alternating_count;
            groups.push_back(std::move(group));
        }
        std::stable_sort(groups.begin(), groups.end(),
                         [](const Group &first, const Group &second) {
            return std::tuple{!first.contains_first,
                              -std::popcount(first.signature),
                              -static_cast<int>(first.rows.size()),
                              first.rows.front()} <
                   std::tuple{!second.contains_first,
                              -std::popcount(second.signature),
                              -static_cast<int>(second.rows.size()),
                              second.rows.front()};
        });
        for (std::size_t column = 0; column < supports.size(); ++column)
            remaining[column] = std::popcount(supports[column]);
        choices.resize(groups.size(), 0);
    }

    bool possible(int assigned_rows, int unassigned_rows) const {
        if (assigned_rows > half || assigned_rows + unassigned_rows < half)
            return false;
        for (int rank = 0; rank < k; ++rank) {
            int possible_columns = 0;
            for (std::size_t column = 0; column < supports.size(); ++column) {
                if (ranks[column] != rank) continue;
                const int target = (1 << (k - rank)) / 2;
                if (assigned[column] <= target &&
                    assigned[column] + remaining[column] >= target)
                    ++possible_columns;
            }
            if (possible_columns < choose(k - 1, rank)) return false;
        }
        return true;
    }

    void dfs(std::size_t group_index, int assigned_rows,
             int unassigned_rows) {
        if (found || aborted) return;
        if (++nodes > node_cap) {
            aborted = true;
            return;
        }
        if (!possible(assigned_rows, unassigned_rows)) return;
        if (group_index == groups.size()) {
            found = assigned_rows == half;
            if (found) solution = choices;
            return;
        }

        const Group &group = groups[group_index];
        const int size = static_cast<int>(group.rows.size());
        const int minimum = group.contains_first ? 1 : 0;
        std::vector<int> values;
        for (int count = minimum; count <= size; ++count)
            values.push_back(count);
        std::stable_sort(values.begin(), values.end(),
                         [&group](int first, int second) {
            return std::tuple{std::abs(first - group.alternating_count), first} <
                   std::tuple{std::abs(second - group.alternating_count), second};
        });
        for (int count : values) {
            choices[group_index] = count;
            for (std::size_t column = 0; column < supports.size(); ++column) {
                if ((group.signature & (std::uint64_t{1} << column)) == 0)
                    continue;
                assigned[column] += count;
                remaining[column] -= size;
            }
            dfs(group_index + 1, assigned_rows + count,
                unassigned_rows - size);
            for (std::size_t column = 0; column < supports.size(); ++column) {
                if ((group.signature & (std::uint64_t{1} << column)) == 0)
                    continue;
                remaining[column] += size;
                assigned[column] -= count;
            }
            if (found || aborted) return;
        }
    }

    void run() { dfs(0, 0, row_count); }
};

struct Census {
    int k;
    int rows;
    Sequence parent;
    Sequence parent_prefix{0};
    int mass = 0;
    std::vector<std::uint64_t> colorings;
    std::uint64_t limit = 0;
    std::uint64_t skip = 0;
    std::uint64_t seen = 0;
    std::uint64_t tested = 0;
    std::uint64_t coloring_nodes = 0;
    std::uint64_t maximum_nodes = 0;
    std::uint64_t greedy_swaps_total = 0;
    int maximum_greedy_swaps = 0;
    std::uint64_t plateau_escapes = 0;
    std::uint64_t plateau_states_total = 0;
    std::uint64_t maximum_plateau_states = 0;
    std::uint64_t grouped_fallbacks = 0;
    std::uint64_t grouped_nodes_total = 0;
    std::uint64_t maximum_grouped_nodes = 0;
    std::uint64_t joint_steps_total = 0;
    std::uint64_t joint_row_swaps_total = 0;
    std::uint64_t joint_incidence_switches_total = 0;
    std::uint64_t joint_neutral_moves_total = 0;
    int maximum_joint_steps = 0;
    int maximum_initial_joint_energy = 0;
    int maximum_initial_column_imbalance = 0;
    Sequence first_maximum_initial_energy;
    Sequence first_maximum_column_imbalance;
    Sequence first_neutral_state;
    std::string first_neutral_trace;
    Sequence first_incidence_state;
    std::string first_incidence_trace;
    std::vector<std::uint64_t> distance_histogram;
    std::uint64_t alternating_failures = 0;
    std::uint64_t top_rank_first_failures = 0;
    std::uint64_t greedy_swap_failures = 0;
    bool fixed_prefix_failed = false;
    bool all_even_failed = false;
    Sequence worst;
    Sequence first_alternating_failure;
    Sequence first_top_rank_first_failure;
    Sequence first_greedy_swap_failure;
    Sequence first_fixed_prefix_failure;
    Sequence first_all_even_failure;
    std::chrono::steady_clock::time_point started =
        std::chrono::steady_clock::now();

    Census(int level, std::uint64_t state_limit, std::uint64_t state_skip)
        : k(level), rows(1 << k), parent(singleton_base(k)),
          limit(state_limit), skip(state_skip) {
        for (int value : parent) {
            mass += value;
            parent_prefix.push_back(mass);
        }
        const int half = rows / 2;
        if (k <= 4) {
            const std::uint64_t end = std::uint64_t{1} << rows;
            for (std::uint64_t mask = 1; mask < end; mask += 2) {
                if (std::popcount(mask) == half) colorings.push_back(mask);
            }
        }
        const std::uint64_t alternating = [] (int count) {
            std::uint64_t mask = 0;
            for (int index = 0; index < count; index += 2)
                mask |= std::uint64_t{1} << index;
            return mask;
        }(rows);
        if (!colorings.empty())
            std::stable_sort(colorings.begin(), colorings.end(),
                             [alternating](std::uint64_t first,
                                           std::uint64_t second) {
                return std::tuple{std::popcount(first ^ alternating), first} <
                       std::tuple{std::popcount(second ^ alternating), second};
            });
        distance_histogram.resize(rows / 2 + 1, 0);
    }

    bool done() const { return limit != 0 && tested >= limit; }

    std::pair<std::vector<std::uint64_t>, std::vector<int>>
    canonical_havel_hakimi(const Sequence &state) const {
        Sequence residual = state;
        std::vector<std::uint64_t> supports;
        std::vector<int> ranks;
        for (int rank = 0; rank <= k; ++rank) {
            const int degree = 1 << (k - rank);
            for (int copy = 0; copy < choose(k, rank); ++copy) {
                std::vector<int> order(rows);
                std::iota(order.begin(), order.end(), 0);
                std::stable_sort(order.begin(), order.end(),
                                 [&residual](int first, int second) {
                    return std::tuple{-residual[first], first} <
                           std::tuple{-residual[second], second};
                });
                std::uint64_t support = 0;
                for (int index = 0; index < degree; ++index) {
                    const int row = order[index];
                    if (residual[row] <= 0) {
                        std::cerr << "CANONICAL_HH_NEGATIVE_RESIDUAL\n";
                        std::exit(2);
                    }
                    --residual[row];
                    support |= std::uint64_t{1} << row;
                }
                supports.push_back(support);
                ranks.push_back(rank);
            }
        }
        if (std::any_of(residual.begin(), residual.end(),
                        [](int value) { return value != 0; })) {
            std::cerr << "CANONICAL_HH_NONZERO_RESIDUAL parent=" << show(state)
                      << " residual=" << show(residual) << '\n';
            std::exit(2);
        }
        return {std::move(supports), std::move(ranks)};
    }

    bool quota_ok(std::uint64_t coloring,
                  const std::vector<std::uint64_t> &supports,
                  const std::vector<int> &ranks) const {
        for (int rank = 0; rank <= k; ++rank) {
            const int quota = choose(k - 1, rank);
            const int degree = 1 << (k - rank);
            int balanced = 0;
            for (std::size_t column = 0; column < supports.size(); ++column) {
                if (ranks[column] != rank) continue;
                if (2 * std::popcount(supports[column] & coloring) == degree)
                    ++balanced;
            }
            if (balanced < quota) return false;
        }
        return true;
    }

    bool top_rank_ok(std::uint64_t coloring,
                     const std::vector<std::uint64_t> &supports,
                     const std::vector<int> &ranks) const {
        const int degree = rows / 2;
        int balanced = 0;
        for (std::size_t column = 0; column < supports.size(); ++column)
            if (ranks[column] == 1 &&
                2 * std::popcount(supports[column] & coloring) == degree)
                ++balanced;
        return balanced >= k - 1;
    }

    int quota_deficit(std::uint64_t coloring,
                      const std::vector<std::uint64_t> &supports,
                      const std::vector<int> &ranks) const {
        int deficit = 0;
        for (int rank = 1; rank < k; ++rank) {
            const int degree = 1 << (k - rank);
            int balanced = 0;
            for (std::size_t column = 0; column < supports.size(); ++column)
                if (ranks[column] == rank &&
                    2 * std::popcount(supports[column] & coloring) == degree)
                    ++balanced;
            deficit += std::max(0, choose(k - 1, rank) - balanced);
        }
        return deficit;
    }

    int quota_energy(std::uint64_t coloring,
                     const std::vector<std::uint64_t> &supports,
                     const std::vector<int> &ranks) const {
        int energy = 0;
        for (int rank = 1; rank < k; ++rank) {
            const int degree = 1 << (k - rank);
            std::vector<int> deviations;
            for (std::size_t column = 0; column < supports.size(); ++column) {
                if (ranks[column] != rank) continue;
                const int imbalance =
                    std::popcount(supports[column] & coloring) - degree / 2;
                deviations.push_back(imbalance * imbalance);
            }
            std::sort(deviations.begin(), deviations.end());
            const int quota = choose(k - 1, rank);
            energy += std::accumulate(deviations.begin(),
                                      deviations.begin() + quota, 0);
        }
        return energy;
    }

    std::string energy_profile(
        std::uint64_t coloring,
        const std::vector<std::uint64_t> &supports,
        const std::vector<int> &ranks) const {
        Sequence profile;
        for (int rank = 1; rank < k; ++rank) {
            const int degree = 1 << (k - rank);
            std::vector<int> deviations;
            for (std::size_t column = 0; column < supports.size(); ++column) {
                if (ranks[column] != rank) continue;
                const int imbalance =
                    std::popcount(supports[column] & coloring) - degree / 2;
                deviations.push_back(imbalance * imbalance);
            }
            std::sort(deviations.begin(), deviations.end());
            const int quota = choose(k - 1, rank);
            profile.push_back(std::accumulate(
                deviations.begin(), deviations.begin() + quota, 0));
        }
        return show(profile);
    }

    bool balanced_descent(std::uint64_t &coloring,
                          const std::vector<std::uint64_t> &supports,
                          const std::vector<int> &ranks,
                          int &swaps, std::uint64_t &plateau_states) const {
        constexpr std::uint64_t plateau_cap = 10000;
        int energy = quota_energy(coloring, supports, ranks);
        while (energy != 0) {
            int best = energy;
            std::uint64_t best_coloring = coloring;
            for (int first = 0; first < rows; ++first) {
                if ((coloring & (std::uint64_t{1} << first)) == 0) continue;
                for (int second = 0; second < rows; ++second) {
                    if ((coloring & (std::uint64_t{1} << second)) != 0) continue;
                    const std::uint64_t candidate =
                        coloring ^ (std::uint64_t{1} << first) ^
                        (std::uint64_t{1} << second);
                    const int candidate_energy =
                        quota_energy(candidate, supports, ranks);
                    if (candidate_energy < best ||
                        (candidate_energy == best && candidate < best_coloring)) {
                        best = candidate_energy;
                        best_coloring = candidate;
                    }
                }
            }
            if (best < energy) {
                coloring = best_coloring;
                energy = best;
                ++swaps;
                continue;
            }

            std::deque<std::uint64_t> queue{coloring};
            std::unordered_set<std::uint64_t> visited{coloring};
            bool escaped = false;
            while (!queue.empty() && visited.size() <= plateau_cap && !escaped) {
                const std::uint64_t current = queue.front();
                queue.pop_front();
                for (int first = 0; first < rows && !escaped; ++first) {
                    if ((current & (std::uint64_t{1} << first)) == 0) continue;
                    for (int second = 0; second < rows; ++second) {
                        if ((current & (std::uint64_t{1} << second)) != 0)
                            continue;
                        const std::uint64_t candidate =
                            current ^ (std::uint64_t{1} << first) ^
                            (std::uint64_t{1} << second);
                        if (visited.contains(candidate)) continue;
                        const int candidate_energy =
                            quota_energy(candidate, supports, ranks);
                        if (candidate_energy < energy) {
                            coloring = candidate;
                            energy = candidate_energy;
                            escaped = true;
                            break;
                        }
                        if (candidate_energy == energy) {
                            visited.insert(candidate);
                            queue.push_back(candidate);
                        }
                    }
                }
            }
            plateau_states += visited.size();
            if (!escaped) return false;
            ++swaps;
        }
        return true;
    }

    bool joint_descent(std::uint64_t &coloring,
                       std::vector<std::uint64_t> &supports,
                       const std::vector<int> &ranks,
                       int &steps, int &row_swaps,
                       int &incidence_switches,
                       int &neutral_moves,
                       std::string *trace = nullptr) const {
        const std::uint64_t row_mask =
            rows == 64 ? ~std::uint64_t{0}
                       : (std::uint64_t{1} << rows) - 1;
        enum class MoveType { none, row_swap, incidence_switch };
        struct MoveData {
            MoveType type = MoveType::none;
            int energy = 0;
            int first = -1;
            int second = -1;
            std::uint64_t first_bit = 0;
            std::uint64_t second_bit = 0;
        };
        const auto find_strict = [&](std::uint64_t current_coloring,
                                     std::vector<std::uint64_t> &current_supports,
                                     int current_energy) {
            MoveData best;
            best.energy = current_energy;
            for (int first = 0; first < rows; ++first) {
                if ((current_coloring & (std::uint64_t{1} << first)) == 0)
                    continue;
                for (int second = 0; second < rows; ++second) {
                    if ((current_coloring & (std::uint64_t{1} << second)) != 0)
                        continue;
                    const std::uint64_t candidate =
                        current_coloring ^ (std::uint64_t{1} << first) ^
                        (std::uint64_t{1} << second);
                    const int candidate_energy =
                        quota_energy(candidate, current_supports, ranks);
                    if (candidate_energy < best.energy)
                        best = {MoveType::row_swap, candidate_energy,
                                first, second, 0, 0};
                }
            }
            for (int first = 0;
                 first < static_cast<int>(current_supports.size()); ++first) {
                for (int second = first + 1;
                     second < static_cast<int>(current_supports.size()); ++second) {
                    const std::uint64_t first_only =
                        current_supports[first] & ~current_supports[second] &
                        row_mask;
                    const std::uint64_t second_only =
                        current_supports[second] & ~current_supports[first] &
                        row_mask;
                    const std::pair<std::uint64_t, std::uint64_t> directions[] = {
                        {first_only & current_coloring,
                         second_only & ~current_coloring & row_mask},
                        {first_only & ~current_coloring & row_mask,
                         second_only & current_coloring}};
                    for (const auto &[first_rows, second_rows] : directions) {
                        if (first_rows == 0 || second_rows == 0) continue;
                        const std::uint64_t u = first_rows & -first_rows;
                        const std::uint64_t v = second_rows & -second_rows;
                        current_supports[first] ^= u | v;
                        current_supports[second] ^= u | v;
                        const int candidate_energy = quota_energy(
                            current_coloring, current_supports, ranks);
                        current_supports[first] ^= u | v;
                        current_supports[second] ^= u | v;
                        if (candidate_energy < best.energy)
                            best = {MoveType::incidence_switch,
                                    candidate_energy, first, second, u, v};
                    }
                }
            }
            return best;
        };
        const auto apply_move = [&](const MoveData &move,
                                    std::uint64_t &current_coloring,
                                    std::vector<std::uint64_t> &current_supports,
                                    bool neutral) {
            if (trace != nullptr) {
                *trace += neutral ? " neutral=" : " strict=";
                if (move.type == MoveType::row_swap) {
                    *trace += "R(" + std::to_string(move.first + 1) + "," +
                              std::to_string(move.second + 1) + ")";
                } else {
                    *trace += "I(c" + std::to_string(move.first + 1) +
                              "@r" + std::to_string(ranks[move.first]) +
                              ",c" + std::to_string(move.second + 1) +
                              "@r" + std::to_string(ranks[move.second]) +
                              ";row" +
                              std::to_string(std::countr_zero(
                                                 move.first_bit) + 1) +
                              ",row" +
                              std::to_string(std::countr_zero(
                                                 move.second_bit) + 1) +
                              ")";
                }
                *trace += "->" + std::to_string(move.energy);
            }
            if (move.type == MoveType::row_swap) {
                current_coloring ^= std::uint64_t{1} << move.first;
                current_coloring ^= std::uint64_t{1} << move.second;
                ++row_swaps;
            } else {
                current_supports[move.first] ^=
                    move.first_bit | move.second_bit;
                current_supports[move.second] ^=
                    move.first_bit | move.second_bit;
                ++incidence_switches;
            }
            ++steps;
            if (neutral) ++neutral_moves;
        };

        int energy = quota_energy(coloring, supports, ranks);
        if (trace != nullptr)
            *trace = "start=" + std::to_string(energy) +
                     " profile=" + energy_profile(coloring, supports, ranks);
        while (energy != 0) {
            const MoveData strict = find_strict(coloring, supports, energy);
            if (strict.type != MoveType::none) {
                apply_move(strict, coloring, supports, false);
                energy = strict.energy;
                continue;
            }

            bool escaped = false;
            for (int first = 0; first < rows && !escaped; ++first) {
                if ((coloring & (std::uint64_t{1} << first)) == 0) continue;
                for (int second = 0; second < rows; ++second) {
                    if ((coloring & (std::uint64_t{1} << second)) != 0) continue;
                    std::uint64_t candidate_coloring =
                        coloring ^ (std::uint64_t{1} << first) ^
                        (std::uint64_t{1} << second);
                    if (quota_energy(candidate_coloring, supports, ranks) != energy)
                        continue;
                    std::vector<std::uint64_t> candidate_supports = supports;
                    const MoveData next = find_strict(
                        candidate_coloring, candidate_supports, energy);
                    if (next.type == MoveType::none) continue;
                    const MoveData neutral{MoveType::row_swap, energy,
                                           first, second, 0, 0};
                    apply_move(neutral, coloring, supports, true);
                    apply_move(next, coloring, supports, false);
                    energy = next.energy;
                    escaped = true;
                    break;
                }
            }
            for (int first = 0;
                 first < static_cast<int>(supports.size()) && !escaped; ++first) {
                for (int second = first + 1;
                     second < static_cast<int>(supports.size()) && !escaped;
                     ++second) {
                    const std::uint64_t first_only =
                        supports[first] & ~supports[second] & row_mask;
                    const std::uint64_t second_only =
                        supports[second] & ~supports[first] & row_mask;
                    for (std::uint64_t first_rows = first_only;
                         first_rows != 0 && !escaped;
                         first_rows &= first_rows - 1) {
                        const std::uint64_t u = first_rows & -first_rows;
                        for (std::uint64_t second_rows = second_only;
                             second_rows != 0;
                             second_rows &= second_rows - 1) {
                            const std::uint64_t v =
                                second_rows & -second_rows;
                            std::vector<std::uint64_t> candidate_supports =
                                supports;
                            candidate_supports[first] ^= u | v;
                            candidate_supports[second] ^= u | v;
                            if (quota_energy(coloring, candidate_supports,
                                             ranks) != energy)
                                continue;
                            const MoveData next = find_strict(
                                coloring, candidate_supports, energy);
                            if (next.type == MoveType::none) continue;
                            const MoveData neutral{
                                MoveType::incidence_switch, energy,
                                first, second, u, v};
                            apply_move(neutral, coloring, supports, true);
                            apply_move(next, coloring, supports, false);
                            energy = next.energy;
                            escaped = true;
                            break;
                        }
                    }
                }
            }
            if (!escaped) return false;
        }
        return true;
    }

    bool fixed_prefix_ok(std::uint64_t coloring,
                         const std::vector<std::uint64_t> &supports,
                         const std::vector<int> &ranks) const {
        for (int rank = 0; rank < k; ++rank) {
            const int degree = 1 << (k - rank);
            int required = choose(k - 1, rank);
            for (std::size_t column = 0; column < supports.size(); ++column) {
                if (ranks[column] != rank || required == 0) continue;
                if (2 * std::popcount(supports[column] & coloring) != degree)
                    return false;
                --required;
            }
        }
        return true;
    }

    bool all_even_ok(std::uint64_t coloring,
                     const std::vector<std::uint64_t> &supports,
                     const std::vector<int> &ranks) const {
        for (std::size_t column = 0; column < supports.size(); ++column) {
            const int degree = 1 << (k - ranks[column]);
            if (degree > 1 &&
                2 * std::popcount(supports[column] & coloring) != degree)
                return false;
        }
        return true;
    }

    void inspect(const Sequence &state) {
        auto [supports, ranks] = canonical_havel_hakimi(state);
        std::uint64_t alternating = 0;
        for (int row = 0; row < rows; row += 2)
            alternating |= std::uint64_t{1} << row;
        const int initial_joint_energy =
            quota_energy(alternating, supports, ranks);
        if (initial_joint_energy > maximum_initial_joint_energy) {
            maximum_initial_joint_energy = initial_joint_energy;
            first_maximum_initial_energy = state;
        }
        for (std::size_t column = 0; column < supports.size(); ++column) {
            if (ranks[column] == k) continue;
            const int half_degree = 1 << (k - ranks[column] - 1);
            const int imbalance = std::abs(
                std::popcount(supports[column] & alternating) - half_degree);
            if (imbalance > maximum_initial_column_imbalance) {
                maximum_initial_column_imbalance = imbalance;
                first_maximum_column_imbalance = state;
            }
        }
        std::vector<std::uint64_t> joint_supports = supports;
        std::uint64_t joint_coloring = alternating;
        int joint_steps = 0;
        int joint_row_swaps = 0;
        int joint_incidence_switches = 0;
        int joint_neutral_moves = 0;
        const bool joint_success = joint_descent(
            joint_coloring, joint_supports, ranks, joint_steps,
            joint_row_swaps, joint_incidence_switches, joint_neutral_moves);
        joint_steps_total += joint_steps;
        joint_row_swaps_total += joint_row_swaps;
        joint_incidence_switches_total += joint_incidence_switches;
        joint_neutral_moves_total += joint_neutral_moves;
        maximum_joint_steps = std::max(maximum_joint_steps, joint_steps);
        if (!joint_success) {
            std::cerr << "JOINT_BALANCED_HH_LOCAL_MINIMUM K=" << k
                      << " parent=" << show(state)
                      << " energy="
                      << quota_energy(joint_coloring, joint_supports, ranks)
                      << " steps=" << joint_steps
                      << " row_swaps=" << joint_row_swaps
                      << " incidence_switches=" << joint_incidence_switches
                      << " neutral_moves=" << joint_neutral_moves
                      << '\n';
            std::exit(1);
        }
        if (joint_neutral_moves != 0 && first_neutral_state.empty()) {
            first_neutral_state = state;
            std::uint64_t trace_coloring = alternating;
            std::vector<std::uint64_t> trace_supports = supports;
            int trace_steps = 0;
            int trace_row_swaps = 0;
            int trace_incidence_switches = 0;
            int trace_neutral_moves = 0;
            if (!joint_descent(trace_coloring, trace_supports, ranks,
                               trace_steps, trace_row_swaps,
                               trace_incidence_switches,
                               trace_neutral_moves, &first_neutral_trace)) {
                std::cerr << "JOINT_BALANCED_HH_TRACE_DIVERGED\n";
                std::exit(2);
            }
        }
        if (joint_incidence_switches != 0 && first_incidence_state.empty()) {
            first_incidence_state = state;
            std::uint64_t trace_coloring = alternating;
            std::vector<std::uint64_t> trace_supports = supports;
            int trace_steps = 0;
            int trace_row_swaps = 0;
            int trace_incidence_switches = 0;
            int trace_neutral_moves = 0;
            if (!joint_descent(trace_coloring, trace_supports, ranks,
                               trace_steps, trace_row_swaps,
                               trace_incidence_switches,
                               trace_neutral_moves,
                               &first_incidence_trace)) {
                std::cerr << "JOINT_BALANCED_HH_TRACE_DIVERGED\n";
                std::exit(2);
            }
        }
        if (colorings.empty()) {
            ++tested;
            return;
        }
        std::uint64_t greedy_coloring = alternating;
        int greedy_steps = 0;
        std::uint64_t plateau_states = 0;
        const bool greedy_success = balanced_descent(
            greedy_coloring, supports, ranks, greedy_steps, plateau_states);
        ++tested;
        std::uint64_t grouped_nodes = 0;
        bool grouped_aborted = false;
        bool canonical_solution = greedy_success;
        if (!canonical_solution) {
            GroupedColorSearch exact(k, rows, supports, ranks, 10000000);
            exact.run();
            ++grouped_fallbacks;
            grouped_nodes = exact.nodes;
            grouped_aborted = exact.aborted;
            canonical_solution = exact.found;
            grouped_nodes_total += exact.nodes;
            maximum_grouped_nodes = std::max(maximum_grouped_nodes, exact.nodes);
        }
        if (!canonical_solution) {
            std::cerr << "GREEDY_BALANCED_HH_COUNTEREXAMPLE K=" << k
                      << " parent=" << show(state)
                      << " initial_deficit="
                      << quota_deficit(alternating, supports, ranks)
                      << " initial_energy="
                      << quota_energy(alternating, supports, ranks)
                      << " plateau_states=" << plateau_states
                      << " grouped_nodes=" << grouped_nodes
                      << " grouped_aborted=" << (grouped_aborted ? "YES" : "NO")
                      << '\n';
            for (int rank = 0; rank <= k; ++rank) {
                std::cerr << "rank=" << rank
                          << " degree=" << (1 << (k - rank))
                          << " quota=" << choose(k - 1, rank)
                          << " supports=";
                for (std::size_t column = 0; column < supports.size(); ++column)
                    if (ranks[column] == rank)
                        std::cerr << show_mask(supports[column], rows);
                std::cerr << '\n';
            }
            std::exit(grouped_aborted ? 2 : 1);
        }
        greedy_swaps_total += greedy_steps;
        maximum_greedy_swaps = std::max(maximum_greedy_swaps, greedy_steps);
        if (plateau_states != 0) ++plateau_escapes;
        plateau_states_total += plateau_states;
        maximum_plateau_states = std::max(maximum_plateau_states, plateau_states);
        std::uint64_t nodes = 0;
        std::uint64_t solution = 0;
        std::uint64_t first_top_rank = 0;
        bool fixed_prefix_solution = false;
        bool all_even_solution = false;
        for (std::uint64_t coloring : colorings) {
            ++nodes;
            if (first_top_rank == 0 && top_rank_ok(coloring, supports, ranks))
                first_top_rank = coloring;
            if (!fixed_prefix_failed && !fixed_prefix_solution &&
                fixed_prefix_ok(coloring, supports, ranks))
                fixed_prefix_solution = true;
            if (!all_even_failed && !all_even_solution &&
                all_even_ok(coloring, supports, ranks))
                all_even_solution = true;
            if (quota_ok(coloring, supports, ranks)) {
                if (solution == 0) solution = coloring;
                if ((fixed_prefix_failed || fixed_prefix_solution) &&
                    (all_even_failed || all_even_solution))
                    break;
            }
        }
        coloring_nodes += nodes;
        if (nodes > maximum_nodes) {
            maximum_nodes = nodes;
            worst = state;
        }
        if (solution == 0) {
            std::cerr << "CANONICAL_BALANCED_HH_COUNTEREXAMPLE K=" << k
                      << " parent=" << show(state)
                      << " colorings=" << colorings.size() << '\n';
            for (int rank = 0; rank <= k; ++rank) {
                std::cerr << "rank=" << rank
                          << " degree=" << (1 << (k - rank))
                          << " quota=" << choose(k - 1, rank)
                          << " supports=";
                for (std::size_t column = 0; column < supports.size(); ++column)
                    if (ranks[column] == rank)
                        std::cerr << show_mask(supports[column], rows);
                std::cerr << '\n';
            }
            std::exit(1);
        }
        alternating = colorings.front();
        const int distance = std::popcount(solution ^ alternating) / 2;
        ++distance_histogram[distance];
        if (!quota_ok(alternating, supports, ranks)) {
            ++alternating_failures;
            if (first_alternating_failure.empty())
                first_alternating_failure = state;
        }
        if (first_top_rank == 0 ||
            !quota_ok(first_top_rank, supports, ranks)) {
            ++top_rank_first_failures;
            if (first_top_rank_first_failure.empty())
                first_top_rank_first_failure = state;
        }
        if (!greedy_success) {
            ++greedy_swap_failures;
            if (first_greedy_swap_failure.empty())
                first_greedy_swap_failure = state;
        }
        if (!fixed_prefix_failed && !fixed_prefix_solution) {
            fixed_prefix_failed = true;
            first_fixed_prefix_failure = state;
        }
        if (!all_even_failed && !all_even_solution) {
            all_even_failed = true;
            first_all_even_failure = state;
        }
        if (tested % 100000 == 0) {
            const double seconds = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - started).count();
            std::cout << "CANONICAL_BALANCED_HH_PROGRESS K=" << k
                      << " tested=" << tested << " skipped=" << skip
                      << " coloring_nodes=" << coloring_nodes
                      << " seconds=" << seconds << '\n';
        }
    }

    void enumerate(int remaining, int maximum, int prefix, Sequence &state) {
        if (done()) return;
        const int slots = rows - static_cast<int>(state.size());
        if (slots == 0) {
            if (remaining != 0) return;
            ++seen;
            if (seen > skip) inspect(state);
            return;
        }
        if (remaining < slots || remaining > slots * maximum) return;
        const int upper = std::min(maximum, remaining - (slots - 1));
        for (int value = upper; value >= 1; --value) {
            if (prefix + value >
                parent_prefix[static_cast<int>(state.size()) + 1])
                continue;
            state.push_back(value);
            enumerate(remaining - value, value, prefix + value, state);
            state.pop_back();
            if (done()) return;
        }
    }

    bool dominated(const Sequence &state) const {
        if (static_cast<int>(state.size()) != rows) return false;
        int prefix = 0;
        for (int index = 0; index < rows; ++index) {
            if (state[index] <= 0) return false;
            prefix += state[index];
            if (prefix > parent_prefix[index + 1]) return false;
        }
        return prefix == mass;
    }

    void report(bool complete, const std::string &mode) const {
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        std::cout << "CANONICAL_BALANCED_HH_CENSUS K=" << k
                  << " mode=" << mode
                  << " complete=" << (complete ? "YES" : "NO")
                  << " states=" << tested << " skipped=" << skip
                  << " coloring_nodes=" << coloring_nodes
                  << " max_nodes=" << maximum_nodes
                  << " worst=" << show(worst)
                  << " alternating_failures=" << alternating_failures
                  << " first_alternating_failure="
                  << show(first_alternating_failure)
                  << " top_rank_first_failures=" << top_rank_first_failures
                  << " first_top_rank_first_failure="
                  << show(first_top_rank_first_failure)
                  << " greedy_swap_failures=" << greedy_swap_failures
                  << " greedy_swaps_total=" << greedy_swaps_total
                  << " max_greedy_swaps=" << maximum_greedy_swaps
                  << " plateau_escapes=" << plateau_escapes
                  << " plateau_states_total=" << plateau_states_total
                  << " max_plateau_states=" << maximum_plateau_states
                  << " grouped_fallbacks=" << grouped_fallbacks
                  << " grouped_nodes_total=" << grouped_nodes_total
                  << " max_grouped_nodes=" << maximum_grouped_nodes
                  << " joint_steps_total=" << joint_steps_total
                  << " joint_row_swaps_total=" << joint_row_swaps_total
                  << " joint_incidence_switches_total="
                  << joint_incidence_switches_total
                  << " joint_neutral_moves_total="
                  << joint_neutral_moves_total
                  << " max_joint_steps=" << maximum_joint_steps
                  << " max_initial_joint_energy="
                  << maximum_initial_joint_energy
                  << " max_initial_column_imbalance="
                  << maximum_initial_column_imbalance
                  << " first_max_initial_energy="
                  << show(first_maximum_initial_energy)
                  << " first_max_column_imbalance="
                  << show(first_maximum_column_imbalance)
                  << " first_neutral_state=" << show(first_neutral_state)
                  << " first_neutral_trace='" << first_neutral_trace << "'"
                  << " first_incidence_state="
                  << show(first_incidence_state)
                  << " first_incidence_trace='" << first_incidence_trace
                  << "'"
                  << " first_greedy_swap_failure="
                  << show(first_greedy_swap_failure)
                  << " fixed_prefix_failure="
                  << show(first_fixed_prefix_failure)
                  << " all_even_failure=" << show(first_all_even_failure)
                  << " distance_histogram=" << show(Sequence(
                         distance_histogram.begin(), distance_histogram.end()))
                  << " seconds=" << seconds << '\n';
    }

    void run() {
        Sequence state;
        enumerate(mass, parent.front(), 0, state);
        const bool complete = limit == 0 && skip == 0;
        const std::uint64_t expected =
            k == 1 ? 1 : k == 2 ? 4 : k == 3 ? 160 : k == 4 ? 408776 : 0;
        if (complete && expected != 0 && tested != expected) {
            std::cerr << "CANONICAL_BALANCED_HH_STATE_COUNT_MISMATCH K=" << k
                      << " expected=" << expected << " actual=" << tested
                      << '\n';
            std::exit(2);
        }
        if (complete && k == 4 &&
            (alternating_failures != 69664 ||
             top_rank_first_failures != 3084 ||
             greedy_swap_failures != 0 ||
             greedy_swaps_total != 69664 ||
             joint_steps_total != 69664 ||
             joint_row_swaps_total != 69664 ||
             joint_incidence_switches_total != 0 ||
             joint_neutral_moves_total != 0 ||
             maximum_joint_steps != 1 ||
             maximum_initial_joint_energy != 2 ||
             maximum_initial_column_imbalance != 1)) {
            std::cerr << "CANONICAL_BALANCED_HH_K4_REGRESSION\n";
            std::exit(2);
        }
        report(complete, "partition-prefix");
    }
};

}  // namespace

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "usage: singleton_balanced_hh_census"
                  << " K [state-limit [state-skip]]\n"
                  << "       singleton_balanced_hh_census"
                  << " K random|walk samples seed\n"
                  << "       singleton_balanced_hh_census"
                  << " K hill iterations seed\n"
                  << "       singleton_balanced_hh_census"
                  << " K canonical-state rows...\n"
                  << "       singleton_balanced_hh_census K state rows...\n";
        return 2;
    }
    const int k = std::atoi(argv[1]);
    if (k < 1 || k > 6) {
        std::cerr << "invalid level\n";
        return 2;
    }
    if (argc >= 3 &&
        (std::string(argv[2]) == "state" ||
         std::string(argv[2]) == "canonical-state")) {
        const std::string mode = argv[2];
        Census census(k, 0, 0);
        Sequence state;
        for (int index = 3; index < argc; ++index)
            state.push_back(std::atoi(argv[index]));
        std::sort(state.begin(), state.end(), std::greater<int>());
        if (!census.dominated(state)) {
            std::cerr << "invalid or nonmajorized state " << show(state) << '\n';
            return 2;
        }
        if (mode == "canonical-state") {
            auto [supports, ranks] =
                census.canonical_havel_hakimi(state);
            GroupedColorSearch exact(k, census.rows, supports, ranks,
                                     100000000);
            exact.run();
            std::cout << "CANONICAL_STATE_COLORING K=" << k
                      << " parent=" << show(state)
                      << " found=" << (exact.found ? "YES" : "NO")
                      << " aborted=" << (exact.aborted ? "YES" : "NO")
                      << " nodes=" << exact.nodes << '\n';
            return exact.aborted ? 2 : 0;
        }
        census.inspect(state);
        census.report(false, "single-state");
        return 0;
    }
    if (argc == 5 &&
        (std::string(argv[2]) == "random" ||
         std::string(argv[2]) == "walk" ||
         std::string(argv[2]) == "hill")) {
        const std::string mode = argv[2];
        const int samples = std::atoi(argv[3]);
        const std::uint64_t seed = std::strtoull(argv[4], nullptr, 10);
        if (samples <= 0) {
            std::cerr << "invalid sample count\n";
            return 2;
        }
        Census census(k, 0, 0);
        std::mt19937_64 random(seed);
        Sequence state = census.parent;
        if (mode == "hill") {
            struct Difficulty {
                bool success = false;
                int score = 0;
                int steps = 0;
                int neutral = 0;
                int energy = 0;
            };
            const auto difficulty = [&](const Sequence &candidate) {
                auto [supports, ranks] =
                    census.canonical_havel_hakimi(candidate);
                std::uint64_t coloring = 0;
                for (int row = 0; row < census.rows; row += 2)
                    coloring |= std::uint64_t{1} << row;
                const int energy =
                    census.quota_energy(coloring, supports, ranks);
                int steps = 0;
                int row_swaps = 0;
                int incidence_switches = 0;
                int neutral = 0;
                const bool success = census.joint_descent(
                    coloring, supports, ranks, steps, row_swaps,
                    incidence_switches, neutral);
                return Difficulty{success,
                                  neutral * 100000 + steps * 1000 + energy,
                                  steps, neutral, energy};
            };
            Difficulty current = difficulty(state);
            Sequence best_state = state;
            Difficulty best = current;
            for (int iteration = 0; iteration < samples; ++iteration) {
                if (iteration != 0 && iteration % 100000 == 0) {
                    state = census.parent;
                    const int walk = 1 + random() % 5000;
                    for (int step = 0; step < walk; ++step) {
                        std::vector<std::pair<int, int>> transfers;
                        for (int donor = 0; donor < census.rows; ++donor)
                            for (int recipient = donor + 1;
                                 recipient < census.rows; ++recipient)
                                if (state[donor] >= state[recipient] + 2)
                                    transfers.emplace_back(donor, recipient);
                        if (transfers.empty()) break;
                        const auto [donor, recipient] =
                            transfers[random() % transfers.size()];
                        --state[donor];
                        ++state[recipient];
                        std::sort(state.begin(), state.end(),
                                  std::greater<int>());
                    }
                    current = difficulty(state);
                }
                int donor = random() % census.rows;
                int recipient = random() % census.rows;
                if (donor == recipient || state[donor] <= 1) continue;
                Sequence candidate = state;
                --candidate[donor];
                ++candidate[recipient];
                std::sort(candidate.begin(), candidate.end(),
                          std::greater<int>());
                if (!census.dominated(candidate)) continue;
                const Difficulty next = difficulty(candidate);
                if (!next.success) {
                    std::cerr << "HILL_JOINT_COUNTEREXAMPLE K=" << k
                              << " parent=" << show(candidate)
                              << " iteration=" << iteration << '\n';
                    census.inspect(candidate);
                    return 1;
                }
                if (next.score > best.score) {
                    best = next;
                    best_state = candidate;
                }
                if (next.score >= current.score || random() % 1000 == 0) {
                    state = std::move(candidate);
                    current = next;
                }
            }
            census.inspect(best_state);
            census.report(false, "hill");
            std::cout << "HILL_BEST K=" << k
                      << " parent=" << show(best_state)
                      << " score=" << best.score
                      << " steps=" << best.steps
                      << " neutral=" << best.neutral
                      << " initial_energy=" << best.energy
                      << " iterations=" << samples
                      << " seed=" << seed << '\n';
            return 0;
        }
        for (int sample = 0; sample < samples; ++sample) {
            if (mode == "random") {
                do {
                    state.assign(census.rows, 1);
                    for (int unit = census.rows; unit < census.mass; ++unit)
                        ++state[random() % census.rows];
                    std::sort(state.begin(), state.end(), std::greater<int>());
                } while (!census.dominated(state));
            } else {
                std::vector<std::pair<int, int>> transfers;
                for (int donor = 0; donor < census.rows; ++donor)
                    for (int recipient = donor + 1;
                         recipient < census.rows; ++recipient)
                        if (state[donor] >= state[recipient] + 2)
                            transfers.emplace_back(donor, recipient);
                if (transfers.empty()) state = census.parent;
                else {
                    const auto [donor, recipient] =
                        transfers[random() % transfers.size()];
                    --state[donor];
                    ++state[recipient];
                    std::sort(state.begin(), state.end(), std::greater<int>());
                }
            }
            census.inspect(state);
        }
        census.report(false, mode);
        return 0;
    }
    if (argc > 4) {
        std::cerr << "invalid census request\n";
        return 2;
    }
    const std::uint64_t limit =
        argc > 2 ? std::strtoull(argv[2], nullptr, 10) : 0;
    const std::uint64_t skip =
        argc > 3 ? std::strtoull(argv[3], nullptr, 10) : 0;
    if (k > 5 || (limit == 0 && skip != 0) ||
        (k == 5 && limit == 0)) {
        std::cerr << "invalid census request\n";
        return 2;
    }
    Census census(k, limit, skip);
    census.run();
    return 0;
}
