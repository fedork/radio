#include <algorithm>
#include <array>
#include <chrono>
#include <compare>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// Parent-first census of normalized singleton first-split child types.
//
// A parent row a has one of the forms (p,a-p,0) or (0,a-p,p).  Rows of the
// same value are indistinguishable, so their choices are enumerated in
// nondecreasing order.  Child parts are accumulated as value multiplicities;
// a partial child that already violates majorization cannot be completed.
// The ordinary mode stops at the second (L,M,R) orbit modulo L/R.  This makes
// K=4 plausible without enumerating all 1206^3 child triples.

namespace {

constexpr int MAX_VALUE = 16;
using Sequence = std::vector<int>;
using Counts = std::array<std::uint8_t, MAX_VALUE + 1>;
using AllocationKey = std::vector<std::array<std::uint8_t, 3>>;

Sequence singleton_base(int k) {
    Sequence cur{1};
    for (int level = 0; level < k; ++level) {
        Sequence next(2 * cur.size(), 0);
        for (std::size_t i = 0; i < cur.size(); ++i) {
            next[i] += cur[i];
            next[2 * i] += cur[i];
            next[2 * i + 1] += cur[i];
        }
        std::sort(next.begin(), next.end(), std::greater<int>());
        cur = std::move(next);
    }
    return cur;
}

std::string show(const Sequence &values) {
    std::string out = "(";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i) out += ',';
        out += std::to_string(values[i]);
    }
    return out + ')';
}

struct Child {
    Counts counts{};
    int mass = 0;
};

struct ChildKey {
    Counts left{};
    Counts mixed{};
    Counts right{};

    auto operator<=>(const ChildKey &) const = default;
};

struct Choice {
    int left = 0;
    int mixed = 0;
    int right = 0;
};

bool partition_less(const Counts &lhs, const Counts &rhs, int largest) {
    for (int value = largest; value >= 1; --value) {
        if (lhs[value] != rhs[value]) return lhs[value] < rhs[value];
    }
    return false;
}

ChildKey canonical_key(const Child &left, const Child &mixed, const Child &right,
                       int largest) {
    if (partition_less(right.counts, left.counts, largest))
        return {right.counts, mixed.counts, left.counts};
    return {left.counts, mixed.counts, right.counts};
}

AllocationKey allocation_key(const std::vector<Choice> &rows, bool swap_sides) {
    AllocationKey result;
    result.reserve(rows.size());
    for (const Choice &row : rows) {
        const int left = swap_sides ? row.right : row.left;
        const int right = swap_sides ? row.left : row.right;
        result.push_back({static_cast<std::uint8_t>(left),
                          static_cast<std::uint8_t>(row.mixed),
                          static_cast<std::uint8_t>(right)});
    }
    std::sort(result.begin(), result.end(), std::greater<>());
    return result;
}

Sequence expand(const Counts &counts, int largest) {
    Sequence result;
    for (int value = largest; value >= 1; --value)
        for (int count = 0; count < counts[value]; ++count) result.push_back(value);
    return result;
}

std::string show_key(const ChildKey &key, int largest) {
    return '(' + show(expand(key.left, largest)) + ',' +
           show(expand(key.mixed, largest)) + ',' +
           show(expand(key.right, largest)) + ')';
}

std::string show_allocation(const AllocationKey &allocation) {
    AllocationKey ordered = allocation;
    std::sort(ordered.begin(), ordered.end(), [](const auto &lhs, const auto &rhs) {
        const int lhs_sum = lhs[0] + lhs[1] + lhs[2];
        const int rhs_sum = rhs[0] + rhs[1] + rhs[2];
        return std::tuple{lhs_sum, lhs[0], lhs[1], lhs[2]} >
               std::tuple{rhs_sum, rhs[0], rhs[1], rhs[2]};
    });
    std::string out = "(";
    for (std::size_t i = 0; i < ordered.size(); ++i) {
        if (i) out += ',';
        out += '(' + std::to_string(ordered[i][0]) + ',' +
               std::to_string(ordered[i][1]) + ',' +
               std::to_string(ordered[i][2]) + ')';
    }
    return out + ')';
}

struct SplitSearch {
    const Sequence &state;
    Sequence child_prefix{0};
    int child_mass = 0;
    int child_largest = 0;
    std::vector<std::vector<Choice>> choices;
    std::set<ChildKey> solutions;
    std::set<AllocationKey> allocation_orbits;
    std::map<ChildKey, std::set<AllocationKey>> allocations_by_solution;
    std::uint64_t nodes = 0;
    std::uint64_t complete_allocations = 0;
    int solution_limit = 2;

    SplitSearch(const Sequence &parent_state, const Sequence &child_base, int limit = 2)
        : state(parent_state), solution_limit(limit) {
        child_largest = child_base.front();
        for (int value : child_base) {
            child_mass += value;
            child_prefix.push_back(child_mass);
        }
        choices.resize(state.front() + 1);
        for (int value = 1; value <= state.front(); ++value) {
            const int low_pure = std::max(0, value - child_largest);
            const int high_pure = std::min(value, child_largest);
            for (int pure = low_pure; pure <= high_pure; ++pure) {
                const int mixed = value - pure;
                if (mixed > child_largest) continue;
                if (pure == 0) {
                    choices[value].push_back({0, mixed, 0});
                } else {
                    choices[value].push_back({pure, mixed, 0});
                    choices[value].push_back({0, mixed, pure});
                }
            }
            // Find child types quickly: balanced pieces before extreme pure/mixed choices.
            std::stable_sort(choices[value].begin(), choices[value].end(),
                             [](const Choice &lhs, const Choice &rhs) {
                const auto score = [](const Choice &choice) {
                    const int pure = choice.left + choice.right;
                    return std::tuple{std::abs(pure - choice.mixed),
                                      choice.left == 0 && choice.right == 0,
                                      choice.right != 0, pure};
                };
                return score(lhs) < score(rhs);
            });
        }
    }

    int H(int count) const {
        return child_prefix[std::min(count,
                                     static_cast<int>(child_prefix.size()) - 1)];
    }

    bool child_ok(const Child &child) const {
        if (child.mass > child_mass) return false;
        int prefix = 0;
        int rows = 0;
        for (int value = child_largest; value >= 1; --value) {
            for (int count = 0; count < child.counts[value]; ++count) {
                ++rows;
                prefix += value;
                if (prefix > H(rows)) return false;
            }
        }
        return true;
    }

    void add(Child &child, int value) {
        if (value == 0) return;
        ++child.counts[value];
        child.mass += value;
    }

    void remove(Child &child, int value) {
        if (value == 0) return;
        --child.counts[value];
        child.mass -= value;
    }

    bool deficits_possible(const Child &left, const Child &mixed, const Child &right,
                           int remaining_mass) const {
        const int dl = child_mass - left.mass;
        const int dm = child_mass - mixed.mass;
        const int dr = child_mass - right.mass;
        return dl >= 0 && dm >= 0 && dr >= 0 && dl + dm + dr == remaining_mass;
    }

    bool stopped() const {
        return solution_limit != 0 &&
               static_cast<int>(solutions.size()) >= solution_limit;
    }

    void dfs(std::size_t row, int minimum_choice, int remaining_mass,
             Child &left, Child &mixed, Child &right,
             std::vector<Choice> &rows) {
        ++nodes;
        if (stopped()) return;
        if (!deficits_possible(left, mixed, right, remaining_mass)) return;
        if (row == state.size()) {
            if (left.mass != child_mass || mixed.mass != child_mass ||
                right.mass != child_mass)
                return;
            ++complete_allocations;
            const bool swap_sides = partition_less(
                right.counts, left.counts, child_largest);
            const ChildKey key = canonical_key(left, mixed, right, child_largest);
            solutions.insert(key);
            AllocationKey allocation = allocation_key(rows, swap_sides);
            if (left.counts == right.counts) {
                const AllocationKey swapped = allocation_key(rows, !swap_sides);
                allocation = std::min(allocation, swapped);
            }
            allocation_orbits.insert(allocation);
            allocations_by_solution[key].insert(std::move(allocation));
            return;
        }

        const int value = state[row];
        const bool same_block = row > 0 && state[row - 1] == value;
        const int first = same_block ? minimum_choice : 0;
        const auto &row_choices = choices[value];
        for (int index = first; index < static_cast<int>(row_choices.size()); ++index) {
            const Choice &choice = row_choices[index];
            add(left, choice.left);
            add(mixed, choice.mixed);
            add(right, choice.right);
            rows.push_back(choice);
            const bool legal = child_ok(left) && child_ok(mixed) && child_ok(right);
            if (legal)
                dfs(row + 1, index, remaining_mass - value,
                    left, mixed, right, rows);
            rows.pop_back();
            remove(right, choice.right);
            remove(mixed, choice.mixed);
            remove(left, choice.left);
            if (stopped()) return;
        }
    }

    void run() {
        Child left, mixed, right;
        std::vector<Choice> rows;
        rows.reserve(state.size());
        int total = 0;
        for (int value : state) total += value;
        dfs(0, 0, total, left, mixed, right, rows);
    }
};

struct Census {
    struct LowParent {
        Sequence parent;
        std::set<ChildKey> children;
        int allocation_orbits = 0;
        std::map<ChildKey, std::set<AllocationKey>> allocations_by_solution;
    };

    struct UniqueParent {
        Sequence parent;
        ChildKey children;
        int allocation_orbits = 0;
    };

    int k;
    Sequence parent;
    Sequence child;
    Sequence parent_prefix{0};
    int total = 0;
    std::uint64_t state_limit = 0;
    std::uint64_t state_skip = 0;
    int orbit_limit = 2;
    std::uint64_t states_seen = 0;
    std::uint64_t states = 0;
    std::uint64_t unique = 0;
    std::uint64_t multiple = 0;
    std::uint64_t search_nodes = 0;
    std::uint64_t complete_allocations = 0;
    std::uint64_t recorded_child_orbits = 0;
    std::uint64_t recorded_allocation_orbits = 0;
    std::uint64_t max_nodes = 0;
    Sequence worst_state;
    std::vector<UniqueParent> unique_states;
    std::vector<LowParent> low_states;
    std::map<int, std::uint64_t> orbit_counts;
    std::map<int, std::uint64_t> cut_counts;
    std::chrono::steady_clock::time_point started = std::chrono::steady_clock::now();

    Census(int level, std::uint64_t limit, std::uint64_t skip, int split_limit)
        : k(level), parent(singleton_base(level)), child(singleton_base(level - 1)),
          state_limit(limit), state_skip(skip), orbit_limit(split_limit) {
        for (int value : parent) {
            total += value;
            parent_prefix.push_back(total);
        }
    }

    int parent_H(int count) const {
        return parent_prefix[std::min(count, static_cast<int>(parent.size()))];
    }

    bool limit_reached() const {
        return state_limit != 0 && states >= state_limit;
    }

    void inspect(const Sequence &state) {
        SplitSearch search(state, child, orbit_limit);
        search.run();
        ++states;
        search_nodes += search.nodes;
        complete_allocations += search.complete_allocations;
        recorded_child_orbits += search.solutions.size();
        recorded_allocation_orbits += search.allocation_orbits.size();
        if (search.nodes > max_nodes) {
            max_nodes = search.nodes;
            worst_state = state;
        }
        if (search.solutions.empty()) {
            std::cerr << "SPLIT_COUNTEREXAMPLE k=" << k
                      << " state=" << show(state)
                      << " nodes=" << search.nodes << '\n';
            std::exit(1);
        }
        if (search.solutions.size() == 1) {
            ++unique;
            unique_states.push_back({state, *search.solutions.begin(),
                                     static_cast<int>(search.allocation_orbits.size())});
        } else {
            ++multiple;
        }
        ++orbit_counts[static_cast<int>(search.solutions.size())];
        if (!search.stopped()) {
            ++cut_counts[static_cast<int>(search.allocation_orbits.size())];
            low_states.push_back({state, search.solutions,
                                  static_cast<int>(search.allocation_orbits.size()),
                                  search.allocations_by_solution});
        }

        if (states % 100000 == 0) {
            const double seconds = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - started).count();
            std::cout << "PROGRESS k=" << k << " skipped=" << state_skip
                      << " states=" << states << " unique=" << unique
                      << " nodes=" << search_nodes << " seconds=" << seconds << '\n';
        }
    }

    void enumerate(int remaining, int maximum, Sequence &state) {
        if (limit_reached()) return;
        if (remaining == 0) {
            ++states_seen;
            if (states_seen > state_skip) inspect(state);
            return;
        }
        const int used = total - remaining;
        for (int value = std::min(maximum, remaining); value >= 1; --value) {
            if (used + value > parent_H(static_cast<int>(state.size()) + 1)) continue;
            state.push_back(value);
            enumerate(remaining - value, value, state);
            state.pop_back();
            if (limit_reached()) return;
        }
    }

    void run(bool report = true, bool report_low = false) {
        Sequence state;
        enumerate(total, parent.front(), state);
        if (state_limit == 0 && state_skip == 0 && orbit_limit == 4) {
            const std::array<std::uint64_t, 5> expected_states{
                0, 2, 15, 1206, 5997038};
            const std::array<std::array<std::uint64_t, 4>, 5> expected_orbits{{
                {0, 0, 0, 0},
                {2, 0, 0, 0},
                {4, 5, 2, 4},
                {9, 19, 6, 1172},
                {30, 123, 106, 5996779},
            }};
            const std::array<std::array<std::uint64_t, 3>, 5> expected_cuts{{
                {0, 0, 0},
                {2, 0, 0},
                {3, 3, 4},
                {6, 4, 8},
                {8, 19, 32},
            }};
            const auto count_or_zero = [](const auto &counts, int key) {
                const auto found = counts.find(key);
                return found == counts.end() ? std::uint64_t{0} : found->second;
            };
            bool valid = states == expected_states[k];
            for (int count = 1; count <= 4; ++count)
                valid = valid && count_or_zero(orbit_counts, count) ==
                    expected_orbits[k][count - 1];
            for (int count = 1; count <= 3; ++count)
                valid = valid && count_or_zero(cut_counts, count) ==
                    expected_cuts[k][count - 1];
            if (!valid) {
                std::cerr << "MULTIPLICITY_REGRESSION k=" << k << '\n';
                std::exit(1);
            }
        }
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        if (!report) return;
        std::cout << "SPLIT_MULTIPLICITY_CENSUS k=" << k
                  << " complete=" << (state_limit == 0 && state_skip == 0 ? "YES" : "NO")
                  << " skipped=" << state_skip << " states=" << states
                  << " unique=" << unique << " multiple=" << multiple
                  << " nodes=" << search_nodes
                  << " complete_allocations=" << complete_allocations
                  << " recorded_child_orbits=" << recorded_child_orbits
                  << " recorded_allocation_orbits=" << recorded_allocation_orbits
                  << " max_nodes=" << max_nodes
                  << " worst_state=" << show(worst_state)
                  << " seconds=" << seconds << '\n';
        for (const auto &[count, parents] : orbit_counts)
            std::cout << "ORBIT_COUNT child_orbits="
                      << (count == orbit_limit ? ">=" : "") << count
                      << " parents=" << parents << '\n';
        for (const auto &[count, parents] : cut_counts)
            if (count <= 3)
                std::cout << "CUT_COUNT allocation_orbits=" << count
                          << " parents=" << parents << '\n';
        for (const UniqueParent &item : unique_states)
            std::cout << "CHILD_UNIQUE parent=" << show(item.parent)
                      << " children=" << show_key(item.children, child.front())
                      << " allocation_orbits=" << item.allocation_orbits << '\n';
        if (report_low) {
            for (const LowParent &item : low_states) {
                std::cout << "LOW_PARENT parent=" << show(item.parent)
                          << " child_orbits=" << item.children.size()
                          << " allocation_orbits=" << item.allocation_orbits << '\n';
                int solution = 0;
                for (const ChildKey &key : item.children) {
                    const auto &allocations = item.allocations_by_solution.at(key);
                    std::cout << "LOW_SOLUTION index=" << ++solution
                              << " children=" << show_key(key, child.front())
                              << " allocation_orbits=" << allocations.size()
                              << " representative="
                              << show_allocation(*allocations.begin()) << '\n';
                }
            }
        }
    }
};

}  // namespace

int main(int argc, char **argv) {
    const int k = argc > 1 ? std::atoi(argv[1]) : 3;
    const std::uint64_t limit = argc > 2 ? std::strtoull(argv[2], nullptr, 10) : 0;
    const std::uint64_t skip = argc > 3 ? std::strtoull(argv[3], nullptr, 10) : 0;
    const int orbit_limit = argc > 4 ? std::atoi(argv[4]) : 2;
    const bool report_low = argc > 5 && std::atoi(argv[5]) != 0;
    if (k < 1 || k > 4 || (limit == 0 && skip != 0) || orbit_limit < 2) {
        std::cerr << "usage: singleton_split_multiplicity_census"
                  << " k [state-limit [state-skip [orbit-limit [report-low]]]]\n";
        return 2;
    }
    Census census(k, limit, skip, orbit_limit);
    census.run(true, report_low);
    if (k >= 2 && limit == 0 && skip == 0) {
        Census lower(k - 1, 0, 0, 4);
        lower.run(false);
        std::set<Counts> rigid_children;
        std::map<Counts, int> lower_multiplicity;
        for (const Census::LowParent &item : lower.low_states) {
            Counts counts{};
            for (int value : item.parent) ++counts[value];
            lower_multiplicity[counts] = static_cast<int>(item.children.size());
            if (item.children.size() == 1) rigid_children.insert(counts);
        }
        std::map<int, std::uint64_t> rigid_histogram;
        std::uint64_t repeated = 0;
        for (const Census::UniqueParent &item : census.unique_states) {
            const ChildKey &key = item.children;
            const int rigid = rigid_children.contains(key.left) +
                              rigid_children.contains(key.mixed) +
                              rigid_children.contains(key.right);
            ++rigid_histogram[rigid];
            repeated += key.left == key.mixed || key.left == key.right ||
                        key.mixed == key.right;
        }
        for (const auto &[rigid, parents] : rigid_histogram)
            std::cout << "UNIQUE_RIGID_CHILDREN count=" << rigid
                      << " parents=" << parents << '\n';
        std::cout << "UNIQUE_REPEATED_CHILD parents=" << repeated
                  << " total=" << census.unique_states.size() << '\n';
        if (orbit_limit == 4) {
            const bool structure_valid =
                (k == 2 && rigid_histogram == std::map<int, std::uint64_t>{{3, 4}} &&
                 repeated == 4) ||
                (k == 3 && rigid_histogram == std::map<int, std::uint64_t>{{3, 9}} &&
                 repeated == 9) ||
                (k == 4 && rigid_histogram ==
                     std::map<int, std::uint64_t>{{1, 5}, {2, 15}, {3, 10}} &&
                 repeated == 13);
            if (!structure_valid) {
                std::cerr << "MULTIPLICITY_STRUCTURE_REGRESSION k=" << k << '\n';
                return 1;
            }
        }

        std::map<std::pair<int, int>, std::uint64_t> best_child_layer;
        for (const Census::LowParent &item : census.low_states) {
            int best = 4;
            for (const ChildKey &key : item.children) {
                int worst_child = 1;
                for (const Counts *counts : {&key.left, &key.mixed, &key.right}) {
                    const auto found = lower_multiplicity.find(*counts);
                    worst_child = std::max(
                        worst_child,
                        found == lower_multiplicity.end() ? 4 : found->second);
                }
                best = std::min(best, worst_child);
            }
            ++best_child_layer[{static_cast<int>(item.children.size()), best}];
        }
        for (const auto &[layers, parents] : best_child_layer)
            std::cout << "LOW_PARENT_CHILD_LAYER parent_orbits=" << layers.first
                      << " best_max_child_orbits="
                      << (layers.second == 4 ? ">=" : "") << layers.second
                      << " parents=" << parents << '\n';
    }
    return 0;
}
