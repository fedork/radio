#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <random>
#include <limits>
#include <string>
#include <utility>
#include <vector>

// Exhaustive finite laboratory for the open Singleton Row-Coloring Lemma.
//
// A full-mass partition a <=_w G_k represents every underfull case after appending unit rows.
// The complete mode enumerates every such partition through k=4, finds an unrestricted normalized
// coloring, and compares several proposed forward rules.  In particular it tests the stronger
// Pair-Orientation Lemma: after sorting a, split each adjacent pair (a_1,a_2), (a_3,a_4), ...
// between colors A and B, choosing each pair's orientation independently, and require the exact
// Fixed-Color Hall inequalities
//
//   A_p + B_q <= H(p+q) + H(p) + H(q),   H = prefix(G_(k-1)).
//
// Equal pairs have only one normalized orientation.  Thus the search branches only where a
// pair straddles two distinct values; for k=4 there can be at most fifteen such decisions even
// though a full-mass partition can contain 81 rows.

namespace {

using Sequence = std::vector<int>;

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

std::string show(const Sequence &s) {
    std::string out = "(";
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (i) out += ',';
        out += std::to_string(s[i]);
    }
    return out + ')';
}

struct Hall {
    Sequence prefix;
    int child_rows = 0;

    explicit Hall(const Sequence &h) : prefix(1, 0), child_rows(static_cast<int>(h.size())) {
        for (int value : h) prefix.push_back(prefix.back() + value);
    }

    int H(int count) const {
        return prefix[std::min(count, child_rows)];
    }

    int capacity(int p, int q) const {
        return H(p + q) + H(p) + H(q);
    }
};

struct Coloring {
    Sequence a;
    Sequence b;
    Sequence pa{0};
    Sequence pb{0};
    int total_a = 0;
    int total_b = 0;

    void push_a(int value) {
        a.push_back(value);
        pa.push_back(pa.back() + value);
        total_a += value;
    }
    void push_b(int value) {
        b.push_back(value);
        pb.push_back(pb.back() + value);
        total_b += value;
    }
    void pop_a() {
        total_a -= a.back();
        a.pop_back();
        pa.pop_back();
    }
    void pop_b() {
        total_b -= b.back();
        b.pop_back();
        pb.pop_back();
    }
};

bool newest_inequalities_hold(const Hall &hall, const Coloring &c,
                              int old_a, int old_b) {
    const int na = static_cast<int>(c.a.size());
    const int nb = static_cast<int>(c.b.size());
    for (int p = old_a + 1; p <= na; ++p)
        for (int q = 0; q <= nb; ++q)
            if (c.pa[p] + c.pb[q] > hall.capacity(p, q)) return false;
    for (int q = old_b + 1; q <= nb; ++q)
        for (int p = 0; p <= old_a; ++p)
            if (c.pa[p] + c.pb[q] > hall.capacity(p, q)) return false;
    return true;
}

void append_orientation(Coloring &c, int av, int bv) {
    if (av) c.push_a(av);
    if (bv) c.push_b(bv);
}

void remove_orientation(Coloring &c, int av, int bv) {
    if (av) c.pop_a();
    if (bv) c.pop_b();
}

struct PairSearch {
    const Sequence &state;
    const Hall &hall;
    std::uint64_t nodes = 0;
    Coloring solution;

    PairSearch(const Sequence &s, const Hall &h) : state(s), hall(h) {}

    bool dfs(std::size_t index, Coloring &current) {
        ++nodes;
        if (index == state.size()) {
            solution = current;
            return true;
        }

        std::pair<int, int> options[2];
        int option_count = 0;
        if (index + 1 == state.size()) {
            options[option_count++] = {state[index], 0};
            options[option_count++] = {0, state[index]};
        } else {
            const int x = state[index];
            const int y = state[index + 1];
            options[option_count++] = {x, y};
            if (x != y) options[option_count++] = {y, x};
        }

        for (int choice = 0; choice < option_count; ++choice) {
            const auto [av, bv] = options[choice];
            const int old_a = static_cast<int>(current.a.size());
            const int old_b = static_cast<int>(current.b.size());
            append_orientation(current, av, bv);
            const bool legal = newest_inequalities_hold(hall, current, old_a, old_b);
            if (legal && dfs(index + (index + 1 < state.size() ? 2 : 1), current)) return true;
            remove_orientation(current, av, bv);
        }
        return false;
    }
};

struct GeneralSearch {
    struct Block {
        int value;
        int count;
    };

    const Hall &hall;
    std::vector<Block> blocks;
    std::uint64_t nodes = 0;
    Coloring solution;

    GeneralSearch(const Sequence &state, const Hall &h) : hall(h) {
        for (int value : state) {
            if (blocks.empty() || blocks.back().value != value)
                blocks.push_back({value, 1});
            else
                ++blocks.back().count;
        }
    }

    bool dfs(std::size_t block_index, Coloring &current) {
        ++nodes;
        if (block_index == blocks.size()) {
            solution = current;
            return true;
        }

        const auto [value, count] = blocks[block_index];
        std::vector<int> choices;
        for (int to_a = 0; to_a <= count; ++to_a) {
            // A/B complementation lets us normalize the first nonempty value block this way.
            if (block_index == 0 && to_a * 2 < count) continue;
            choices.push_back(to_a);
        }
        std::stable_sort(choices.begin(), choices.end(), [&](int lhs, int rhs) {
            const auto score = [&](int to_a) {
                const int to_b = count - to_a;
                const int difference = (current.total_a + to_a * value) -
                                       (current.total_b + to_b * value);
                // Complementation is normalized by putting a largest row in A.  When two
                // allocations are equally balanced, retain that orientation by favoring A.
                return std::pair{std::abs(difference), difference < 0};
            };
            return score(lhs) < score(rhs);
        });

        for (int to_a : choices) {
            const int to_b = count - to_a;
            const int old_a = static_cast<int>(current.a.size());
            const int old_b = static_cast<int>(current.b.size());
            for (int i = 0; i < to_a; ++i) current.push_a(value);
            for (int i = 0; i < to_b; ++i) current.push_b(value);
            const bool legal = newest_inequalities_hold(hall, current, old_a, old_b);
            if (legal && dfs(block_index + 1, current)) return true;
            for (int i = 0; i < to_a; ++i) current.pop_a();
            for (int i = 0; i < to_b; ++i) current.pop_b();
        }
        return false;
    }
};

// A genuinely global candidate rule.  Ignore Hall feasibility at first and, among all
// bipartitions having at least one full child-base worth of rows on each side, minimize the
// final total-mass difference.  GlobalBalanceSearch asks whether at least one such optimum
// satisfies (C).  This is stronger than the Row-Coloring Lemma and is therefore only an
// experimental predicate, not a theorem.
struct GlobalBalanceSearch {
    const Hall &hall;
    std::vector<GeneralSearch::Block> blocks;
    int total_mass = 0;
    int total_rows = 0;
    int best_difference = std::numeric_limits<int>::max();
    std::uint64_t nodes = 0;
    Coloring solution;
    Coloring unrestricted_optimum;

    GlobalBalanceSearch(const Sequence &state, const Hall &h) : hall(h) {
        total_rows = static_cast<int>(state.size());
        for (int value : state) {
            total_mass += value;
            if (blocks.empty() || blocks.back().value != value)
                blocks.push_back({value, 1});
            else
                ++blocks.back().count;
        }

        std::vector<std::vector<unsigned char>> possible(
            total_rows + 1, std::vector<unsigned char>(total_mass + 1));
        possible[0][0] = 1;
        int used_rows = 0;
        int used_mass = 0;
        for (const auto [value, count] : blocks) {
            auto next = possible;
            for (auto &row : next) std::fill(row.begin(), row.end(), 0);
            for (int rows = 0; rows <= used_rows; ++rows)
                for (int mass = 0; mass <= used_mass; ++mass) {
                    if (!possible[rows][mass]) continue;
                    for (int to_a = 0; to_a <= count; ++to_a)
                        next[rows + to_a][mass + to_a * value] = 1;
                }
            possible = std::move(next);
            used_rows += count;
            used_mass += count * value;
        }

        for (int rows = hall.child_rows;
             rows <= total_rows - hall.child_rows; ++rows)
            for (int mass = 0; mass <= total_mass; ++mass)
                if (possible[rows][mass])
                    best_difference = std::min(
                        best_difference, std::abs(2 * mass - total_mass));
    }

    bool dfs(std::size_t block_index, Coloring &current,
             int remaining_rows, int remaining_mass) {
        ++nodes;
        if (block_index == blocks.size()) {
            if (static_cast<int>(current.a.size()) < hall.child_rows ||
                static_cast<int>(current.b.size()) < hall.child_rows ||
                std::abs(current.total_a - current.total_b) != best_difference)
                return false;
            solution = current;
            return true;
        }

        if (static_cast<int>(current.a.size()) + remaining_rows < hall.child_rows ||
            static_cast<int>(current.b.size()) + remaining_rows < hall.child_rows)
            return false;
        const int lowest_a = current.total_a;
        const int highest_a = current.total_a + remaining_mass;
        bool mass_reachable = false;
        for (int target = 0; target <= total_mass; ++target)
            if (std::abs(2 * target - total_mass) == best_difference &&
                lowest_a <= target && target <= highest_a) {
                mass_reachable = true;
                break;
            }
        if (!mass_reachable) return false;

        const auto [value, count] = blocks[block_index];
        std::vector<int> choices;
        for (int to_a = 0; to_a <= count; ++to_a) {
            if (block_index == 0 && to_a * 2 < count) continue;
            choices.push_back(to_a);
        }
        std::stable_sort(choices.begin(), choices.end(), [&](int lhs, int rhs) {
            return std::abs(2 * (current.total_a + lhs * value) - total_mass) <
                   std::abs(2 * (current.total_a + rhs * value) - total_mass);
        });

        for (int to_a : choices) {
            const int to_b = count - to_a;
            const int old_a = static_cast<int>(current.a.size());
            const int old_b = static_cast<int>(current.b.size());
            for (int i = 0; i < to_a; ++i) current.push_a(value);
            for (int i = 0; i < to_b; ++i) current.push_b(value);
            const bool legal = newest_inequalities_hold(hall, current, old_a, old_b);
            if (legal && dfs(block_index + 1, current,
                             remaining_rows - count,
                             remaining_mass - count * value))
                return true;
            for (int i = 0; i < to_a; ++i) current.pop_a();
            for (int i = 0; i < to_b; ++i) current.pop_b();
        }
        return false;
    }

    bool run() {
        if (best_difference == std::numeric_limits<int>::max()) return false;
        Coloring current;
        return dfs(0, current, total_rows, total_mass);
    }

    bool find_unrestricted_optimum(std::size_t block_index, Coloring &current) {
        if (block_index == blocks.size()) {
            if (static_cast<int>(current.a.size()) < hall.child_rows ||
                static_cast<int>(current.b.size()) < hall.child_rows ||
                std::abs(current.total_a - current.total_b) != best_difference)
                return false;
            unrestricted_optimum = current;
            return true;
        }
        const auto [value, count] = blocks[block_index];
        for (int to_a = 0; to_a <= count; ++to_a) {
            if (block_index == 0 && to_a * 2 < count) continue;
            const int to_b = count - to_a;
            for (int i = 0; i < to_a; ++i) current.push_a(value);
            for (int i = 0; i < to_b; ++i) current.push_b(value);
            if (find_unrestricted_optimum(block_index + 1, current)) return true;
            for (int i = 0; i < to_a; ++i) current.pop_a();
            for (int i = 0; i < to_b; ++i) current.pop_b();
        }
        return false;
    }

    bool find_unrestricted_optimum() {
        Coloring current;
        return find_unrestricted_optimum(0, current);
    }
};

enum class BlockOrder { Balanced, MinA, MaxA };

bool greedy_block_coloring(const Sequence &state, const Hall &hall,
                           bool one_block_lookahead = false,
                           BlockOrder order = BlockOrder::Balanced,
                           bool reserve_child_rows = false) {
    GeneralSearch normalized(state, hall);
    Coloring current;
    for (std::size_t block_index = 0; block_index < normalized.blocks.size(); ++block_index) {
        const auto [value, count] = normalized.blocks[block_index];
        std::vector<int> choices;
        for (int to_a = 0; to_a <= count; ++to_a) {
            if (block_index == 0 && to_a * 2 < count) continue;
            choices.push_back(to_a);
        }
        if (order == BlockOrder::MaxA) {
            std::reverse(choices.begin(), choices.end());
        } else if (order == BlockOrder::Balanced) {
            std::stable_sort(choices.begin(), choices.end(), [&](int lhs, int rhs) {
                const auto score = [&](int to_a) {
                    const int to_b = count - to_a;
                    const int difference = (current.total_a + to_a * value) -
                                           (current.total_b + to_b * value);
                    return std::pair{std::abs(difference), difference < 0};
                };
                return score(lhs) < score(rhs);
            });
        }

        bool placed = false;
        for (int to_a : choices) {
            const int to_b = count - to_a;
            const int old_a = static_cast<int>(current.a.size());
            const int old_b = static_cast<int>(current.b.size());
            for (int i = 0; i < to_a; ++i) current.push_a(value);
            for (int i = 0; i < to_b; ++i) current.push_b(value);
            bool legal = newest_inequalities_hold(hall, current, old_a, old_b);
            if (legal && reserve_child_rows) {
                int remaining_rows = 0;
                for (std::size_t future = block_index + 1;
                     future < normalized.blocks.size(); ++future)
                    remaining_rows += normalized.blocks[future].count;
                legal = static_cast<int>(current.a.size()) + remaining_rows >=
                            hall.child_rows &&
                        static_cast<int>(current.b.size()) + remaining_rows >=
                            hall.child_rows;
            }
            if (legal && one_block_lookahead &&
                block_index + 1 < normalized.blocks.size()) {
                const auto [next_value, next_count] = normalized.blocks[block_index + 1];
                bool extendable = false;
                for (int next_to_a = 0; next_to_a <= next_count && !extendable;
                     ++next_to_a) {
                    const int next_to_b = next_count - next_to_a;
                    const int next_old_a = static_cast<int>(current.a.size());
                    const int next_old_b = static_cast<int>(current.b.size());
                    for (int i = 0; i < next_to_a; ++i) current.push_a(next_value);
                    for (int i = 0; i < next_to_b; ++i) current.push_b(next_value);
                    extendable = newest_inequalities_hold(
                        hall, current, next_old_a, next_old_b);
                    for (int i = 0; i < next_to_a; ++i) current.pop_a();
                    for (int i = 0; i < next_to_b; ++i) current.pop_b();
                }
                legal = extendable;
            }
            if (legal) {
                placed = true;
                break;
            }
            for (int i = 0; i < to_a; ++i) current.pop_a();
            for (int i = 0; i < to_b; ++i) current.pop_b();
        }
        if (!placed) return false;
    }
    return true;
}

enum class GreedyRule { Fixed, LowerMass };

bool greedy_pair_coloring(const Sequence &state, const Hall &hall, GreedyRule rule,
                          bool use_safe_fallback) {
    Coloring c;
    for (std::size_t index = 0; index < state.size();) {
        std::pair<int, int> options[2];
        int option_count = 0;
        std::size_t advance = 1;
        if (index + 1 == state.size()) {
            std::pair<int, int> first{state[index], 0};
            std::pair<int, int> second{0, state[index]};
            if (rule == GreedyRule::LowerMass && c.total_a > c.total_b)
                std::swap(first, second);
            options[option_count++] = first;
            options[option_count++] = second;
        } else {
            advance = 2;
            const int x = state[index];
            const int y = state[index + 1];
            std::pair<int, int> first{x, y};
            std::pair<int, int> second{y, x};
            if (rule == GreedyRule::LowerMass && c.total_a > c.total_b)
                std::swap(first, second);
            options[option_count++] = first;
            if (x != y) options[option_count++] = second;
        }

        bool placed = false;
        const int tries = use_safe_fallback ? option_count : 1;
        for (int choice = 0; choice < tries; ++choice) {
            const auto [av, bv] = options[choice];
            const int old_a = static_cast<int>(c.a.size());
            const int old_b = static_cast<int>(c.b.size());
            append_orientation(c, av, bv);
            if (newest_inequalities_hold(hall, c, old_a, old_b)) {
                placed = true;
                break;
            }
            remove_orientation(c, av, bv);
        }
        if (!placed) return false;
        index += advance;
    }
    return true;
}

struct Census {
    int k;
    Sequence parent;
    Sequence child;
    Hall hall;
    Sequence parent_prefix{0};
    int total;
    std::uint64_t states = 0;
    std::uint64_t pair_ok = 0;
    std::uint64_t fixed_ok = 0;
    std::uint64_t fixed_safe_ok = 0;
    std::uint64_t mass_ok = 0;
    std::uint64_t mass_safe_ok = 0;
    std::uint64_t search_nodes = 0;
    std::uint64_t max_search_nodes = 0;
    std::uint64_t general_search_nodes = 0;
    std::uint64_t max_general_search_nodes = 0;
    std::uint64_t general_direct_nodes = 0;
    std::uint64_t block_greedy_ok = 0;
    std::uint64_t block_lookahead_ok = 0;
    std::uint64_t block_lookahead_min_ok = 0;
    std::uint64_t block_lookahead_max_ok = 0;
    std::uint64_t block_reserve_ok = 0;
    Sequence first_pair_failure;
    Sequence first_block_greedy_failure;
    Sequence first_block_lookahead_failure;
    Sequence first_fixed_failure;
    Sequence first_fixed_safe_failure;
    Sequence first_mass_failure;
    Sequence first_mass_safe_failure;
    Sequence worst_state;
    Coloring worst_solution;
    Sequence worst_general_state;
    Coloring worst_general_solution;

    explicit Census(int level)
        : k(level), parent(singleton_base(level)), child(singleton_base(level - 1)),
          hall(child), total(0) {
        for (int value : parent) {
            total += value;
            parent_prefix.push_back(total);
        }
    }

    int parent_H(int count) const {
        return parent_prefix[std::min(count, static_cast<int>(parent.size()))];
    }

    void inspect(const Sequence &state) {
        ++states;
        PairSearch search(state, hall);
        Coloring current;
        const bool pair = search.dfs(0, current);
        pair_ok += pair;
        if (!pair && first_pair_failure.empty()) first_pair_failure = state;
        search_nodes += search.nodes;
        if (pair && search.nodes > max_search_nodes) {
            max_search_nodes = search.nodes;
            worst_state = state;
            worst_solution = search.solution;
        }

        GeneralSearch general(state, hall);
        Coloring general_current;
        if (!general.dfs(0, general_current)) {
            std::cerr << "ROW_COLORING_COUNTEREXAMPLE k=" << k
                      << " state=" << show(state) << '\n';
            std::exit(1);
        }
        general_search_nodes += general.nodes;
        general_direct_nodes += general.blocks.size() + 1;
        if (general.nodes > max_general_search_nodes) {
            max_general_search_nodes = general.nodes;
            worst_general_state = state;
            worst_general_solution = general.solution;
        }

        const bool block_greedy = greedy_block_coloring(state, hall);
        const bool block_lookahead = greedy_block_coloring(state, hall, true);
        const bool block_lookahead_min =
            greedy_block_coloring(state, hall, true, BlockOrder::MinA);
        const bool block_lookahead_max =
            greedy_block_coloring(state, hall, true, BlockOrder::MaxA);
        const bool block_reserve =
            greedy_block_coloring(state, hall, false, BlockOrder::Balanced, true);
        block_greedy_ok += block_greedy;
        block_lookahead_ok += block_lookahead;
        block_lookahead_min_ok += block_lookahead_min;
        block_lookahead_max_ok += block_lookahead_max;
        block_reserve_ok += block_reserve;
        if (!block_greedy && first_block_greedy_failure.empty())
            first_block_greedy_failure = state;
        if (!block_lookahead && first_block_lookahead_failure.empty())
            first_block_lookahead_failure = state;

        const bool fixed = greedy_pair_coloring(state, hall, GreedyRule::Fixed, false);
        const bool fixed_safe = greedy_pair_coloring(state, hall, GreedyRule::Fixed, true);
        const bool mass = greedy_pair_coloring(state, hall, GreedyRule::LowerMass, false);
        const bool mass_safe = greedy_pair_coloring(state, hall, GreedyRule::LowerMass, true);
        fixed_ok += fixed;
        fixed_safe_ok += fixed_safe;
        mass_ok += mass;
        mass_safe_ok += mass_safe;
        if (!fixed && first_fixed_failure.empty()) first_fixed_failure = state;
        if (!fixed_safe && first_fixed_safe_failure.empty()) first_fixed_safe_failure = state;
        if (!mass && first_mass_failure.empty()) first_mass_failure = state;
        if (!mass_safe && first_mass_safe_failure.empty()) first_mass_safe_failure = state;
    }

    void enumerate(int remaining, int maximum, Sequence &state) {
        if (remaining == 0) {
            inspect(state);
            return;
        }
        const int used = total - remaining;
        for (int value = std::min(maximum, remaining); value >= 1; --value) {
            if (used + value > parent_H(static_cast<int>(state.size()) + 1)) continue;
            state.push_back(value);
            enumerate(remaining - value, value, state);
            state.pop_back();
        }
    }

    void run() {
        Sequence state;
        enumerate(total, parent.front(), state);
        std::cout << "PAIR_CENSUS k=" << k << " states=" << states
                  << " pair_orientation_ok=" << pair_ok
                  << " search_nodes=" << search_nodes
                  << " max_search_nodes=" << max_search_nodes << '\n';
        if (!first_pair_failure.empty())
            std::cout << "FIRST_PAIR_FAILURE " << show(first_pair_failure) << '\n';
        std::cout << "GENERAL_COLORING ok=" << states << '/' << states
                  << " search_nodes=" << general_search_nodes
                  << " direct_nodes=" << general_direct_nodes
                  << " max_search_nodes=" << max_general_search_nodes << '\n';
        std::cout << "BLOCK_GREEDY ok=" << block_greedy_ok << '/' << states << '\n';
        std::cout << "BLOCK_LOOKAHEAD ok=" << block_lookahead_ok << '/' << states << '\n';
        std::cout << "BLOCK_LOOKAHEAD_EXTREMES minA=" << block_lookahead_min_ok
                  << '/' << states << " maxA=" << block_lookahead_max_ok
                  << '/' << states << '\n';
        std::cout << "BLOCK_ROW_RESERVE ok=" << block_reserve_ok << '/' << states << '\n';
        if (!first_block_greedy_failure.empty())
            std::cout << "FIRST_BLOCK_GREEDY_FAILURE "
                      << show(first_block_greedy_failure) << '\n';
        if (!first_block_lookahead_failure.empty())
            std::cout << "FIRST_BLOCK_LOOKAHEAD_FAILURE "
                      << show(first_block_lookahead_failure) << '\n';
        std::cout << "GREEDY fixed=" << fixed_ok << '/' << states
                  << " fixed_safe=" << fixed_safe_ok << '/' << states
                  << " lower_mass=" << mass_ok << '/' << states
                  << " lower_mass_safe=" << mass_safe_ok << '/' << states << '\n';
        if (!first_fixed_failure.empty())
            std::cout << "FIRST_FIXED_FAILURE " << show(first_fixed_failure) << '\n';
        if (!first_fixed_safe_failure.empty())
            std::cout << "FIRST_FIXED_SAFE_FAILURE " << show(first_fixed_safe_failure) << '\n';
        if (!first_mass_failure.empty())
            std::cout << "FIRST_LOWER_MASS_FAILURE " << show(first_mass_failure) << '\n';
        if (!first_mass_safe_failure.empty())
            std::cout << "FIRST_LOWER_MASS_SAFE_FAILURE " << show(first_mass_safe_failure) << '\n';
        std::cout << "WORST_BACKTRACK_STATE " << show(worst_state)
                  << " nodes=" << max_search_nodes
                  << " A=" << show(worst_solution.a)
                  << " B=" << show(worst_solution.b) << '\n';
        std::cout << "WORST_GENERAL_STATE " << show(worst_general_state)
                  << " nodes=" << max_general_search_nodes
                  << " A=" << show(worst_general_solution.a)
                  << " B=" << show(worst_general_solution.b) << '\n';
    }
};

// In the 3M-slot padded formulation, reserve the M lightest slots for rows that may use only the
// mixed child, then alternate the remaining 2M slots between the two pure orientations.  The
// reserved rows are necessarily zero or one.  If their total is c, contracting them from the
// mixed child replaces H(t) by min(H(t), M-c).  This census tests that particular deterministic
// orientation; it is stronger than the Row-Coloring Lemma and is only a diagnostic.
struct PaddedThreeCensus {
    int k;
    Sequence parent;
    Hall hall;
    Sequence parent_prefix{0};
    int total = 0;
    int block_size = 0;
    std::uint64_t states = 0;
    std::vector<std::uint64_t> states_by_c;
    std::vector<std::uint64_t> failures_by_c;
    std::vector<std::uint64_t> failures_by_support;
    Sequence first_failure;
    int first_failure_c = -1;
    int first_p = -1;
    int first_q = -1;
    int first_lhs = -1;
    int first_rhs = -1;

    explicit PaddedThreeCensus(int level)
        : k(level), parent(singleton_base(level)), hall(singleton_base(level - 1)) {
        for (int value : parent) {
            total += value;
            parent_prefix.push_back(total);
        }
        block_size = total / 3;
        states_by_c.assign(block_size + 1, 0);
        failures_by_c.assign(block_size + 1, 0);
        failures_by_support.assign(total + 1, 0);
    }

    int parent_H(int count) const {
        return parent_prefix[std::min(count, static_cast<int>(parent.size()))];
    }

    int value_at(const Sequence &state, int index) const {
        return index < static_cast<int>(state.size()) ? state[index] : 0;
    }

    void inspect(const Sequence &state) {
        ++states;
        int c = 0;
        for (int i = 2 * block_size; i < total; ++i) {
            const int value = value_at(state, i);
            if (value > 1) {
                std::cerr << "PADDED_TAIL_NOT_UNIT k=" << k
                          << " state=" << show(state) << '\n';
                std::exit(1);
            }
            c += value;
        }
        ++states_by_c[c];

        Sequence pa(1, 0);
        Sequence pb(1, 0);
        for (int i = 0; i < 2 * block_size; ++i) {
            Sequence &prefix = (i % 2 == 0) ? pa : pb;
            prefix.push_back(prefix.back() + value_at(state, i));
        }

        bool legal = true;
        int bad_p = -1;
        int bad_q = -1;
        int bad_lhs = -1;
        int bad_rhs = -1;
        for (int p = 0; p <= block_size && legal; ++p) {
            for (int q = 0; q <= block_size; ++q) {
                const int mixed = std::min(hall.H(p + q), block_size - c);
                const int lhs = pa[p] + pb[q];
                const int rhs = hall.H(p) + hall.H(q) + mixed;
                if (lhs > rhs) {
                    legal = false;
                    bad_p = p;
                    bad_q = q;
                    bad_lhs = lhs;
                    bad_rhs = rhs;
                    break;
                }
            }
        }
        if (legal) return;
        ++failures_by_c[c];
        ++failures_by_support[state.size()];
        if (first_failure.empty()) {
            first_failure = state;
            first_failure_c = c;
            first_p = bad_p;
            first_q = bad_q;
            first_lhs = bad_lhs;
            first_rhs = bad_rhs;
        }
    }

    void enumerate(int remaining, int maximum, Sequence &state) {
        if (remaining == 0) {
            inspect(state);
            return;
        }
        const int used = total - remaining;
        for (int value = std::min(maximum, remaining); value >= 1; --value) {
            if (used + value > parent_H(static_cast<int>(state.size()) + 1)) continue;
            state.push_back(value);
            enumerate(remaining - value, value, state);
            state.pop_back();
        }
    }

    void run() {
        Sequence state;
        enumerate(total, parent.front(), state);
        std::cout << "PADDED_THREE_CENSUS k=" << k << " states=" << states << '\n';
        for (int c = 0; c <= block_size; ++c) {
            if (states_by_c[c] == 0) continue;
            std::cout << "TAIL_MASS c=" << c
                      << " states=" << states_by_c[c]
                      << " alternating_failures=" << failures_by_c[c] << '\n';
        }
        for (int support = 0; support <= total; ++support) {
            if (failures_by_support[support] == 0) continue;
            std::cout << "FAILURE_SUPPORT rows=" << support
                      << " failures=" << failures_by_support[support] << '\n';
        }
        if (!first_failure.empty())
            std::cout << "FIRST_PADDED_ALTERNATING_FAILURE state=" << show(first_failure)
                      << " c=" << first_failure_c
                      << " p=" << first_p << " q=" << first_q
                      << " lhs=" << first_lhs << " rhs=" << first_rhs << '\n';
    }
};

// For a state with at least 2M nonzero rows, put its M lightest padded slots in the mixed-only
// block.  If that block has mass c, write E=M-c and
//
//   U_E(t)=min(H_k(t), E+t),
//
// the joint parent-majorization/support bound on the first t remaining rows.  Under strict
// alternation, the Hall inequalities with q>=p follow from concavity.  For p>q it is enough that
//
//   floor((U_E(2q+1)+U_E(2p-1))/2)
//       <= H(p)+H(q)+min(H(p+q),E).
//
// Each side is piecewise linear in integer E, with breakpoints only where one displayed min
// changes branch.  Checking those breakpoints and their two neighbors is therefore exhaustive.
void check_padded_prefix_arithmetic(int k) {
    const Sequence child = singleton_base(k - 1);
    const Hall hall(child);
    int mass = 0;
    for (int value : child) mass += value;
    const int child_rows = static_cast<int>(child.size());
    const auto parent_H = [&](int count) {
        return hall.H(count) + hall.H((count + 1) / 2) + hall.H(count / 2);
    };
    const auto U = [&](int count, int excess_mass) {
        return std::min(parent_H(count), excess_mass + count);
    };

    std::uint64_t pairs = 0;
    std::uint64_t values = 0;
    for (int p = 1; p <= child_rows; ++p) {
        for (int q = 0; q < p; ++q) {
            ++pairs;
            const int total_rows = p + q;
            const int first_index = 2 * q + 1;
            const int second_index = 2 * p - 1;
            const int switches[] = {
                0,
                mass - 1,
                mass,
                parent_H(first_index) - first_index,
                parent_H(second_index) - second_index,
                hall.H(total_rows),
            };
            std::vector<int> candidates;
            for (int point : switches) {
                for (int delta = -2; delta <= 2; ++delta) {
                    const int excess_mass = point + delta;
                    if (excess_mass < 0 || excess_mass > mass) continue;
                    candidates.push_back(excess_mass);
                }
            }
            std::sort(candidates.begin(), candidates.end());
            candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());
            for (int excess_mass : candidates) {
                ++values;
                const int lhs =
                    (U(first_index, excess_mass) + U(second_index, excess_mass)) / 2;
                const int rhs = hall.H(p) + hall.H(q) +
                                std::min(hall.H(total_rows), excess_mass);
                if (lhs > rhs) {
                    std::cout << "PADDED_PREFIX_FAILURE k=" << k
                              << " E=" << excess_mass
                              << " p=" << p << " q=" << q
                              << " lhs=" << lhs << " rhs=" << rhs << '\n';
                    return;
                }
            }
        }
    }
    std::cout << "PADDED_PREFIX_CHECK k=" << k
              << " pairs=" << pairs
              << " breakpoint_values=" << values
              << " result=PASS\n";
}

void sample_transfer_states(int k, std::uint64_t sample_count, std::uint64_t seed) {
    const Sequence parent = singleton_base(k);
    const Hall hall(singleton_base(k - 1));
    int total = 0;
    for (int value : parent) total += value;

    std::mt19937_64 random(seed);
    Sequence padded(total, 0);
    auto reset = [&] {
        std::fill(padded.begin(), padded.end(), 0);
        std::copy(parent.begin(), parent.end(), padded.begin());
    };
    reset();
    std::uint64_t walk_left = 0;
    std::uint64_t block_ok = 0;
    std::uint64_t lookahead_ok = 0;
    std::uint64_t reserve_ok = 0;
    Sequence first_block_failure;
    Sequence first_lookahead_failure;
    Sequence first_reserve_failure;
    Coloring first_lookahead_solution;
    Coloring first_reserve_solution;

    for (std::uint64_t sample = 0; sample < sample_count; ++sample) {
        if (walk_left == 0) {
            reset();
            walk_left = 1 + random() % static_cast<std::uint64_t>(2 * total);
        }
        Sequence state = padded;
        while (!state.empty() && state.back() == 0) state.pop_back();

        const bool block = greedy_block_coloring(state, hall);
        const bool lookahead = greedy_block_coloring(state, hall, true);
        const bool reserve =
            greedy_block_coloring(state, hall, false, BlockOrder::Balanced, true);
        block_ok += block;
        lookahead_ok += lookahead;
        reserve_ok += reserve;
        if (!block && first_block_failure.empty()) first_block_failure = state;
        if (!reserve && first_reserve_failure.empty()) {
            first_reserve_failure = state;
            GeneralSearch exact(state, hall);
            Coloring current;
            if (!exact.dfs(0, current)) {
                std::cerr << "ROW_COLORING_COUNTEREXAMPLE k=" << k
                          << " state=" << show(state) << '\n';
                std::exit(1);
            }
            first_reserve_solution = exact.solution;
        }
        if (!lookahead && first_lookahead_failure.empty()) {
            first_lookahead_failure = state;
            GeneralSearch exact(state, hall);
            Coloring current;
            if (!exact.dfs(0, current)) {
                std::cerr << "ROW_COLORING_COUNTEREXAMPLE k=" << k
                          << " state=" << show(state) << '\n';
                std::exit(1);
            }
            first_lookahead_solution = exact.solution;
        }

        std::vector<int> donors;
        for (int i = 0; i < total; ++i)
            if (padded[i] >= 2) donors.push_back(i);
        if (donors.empty()) {
            walk_left = 0;
            continue;
        }
        bool moved = false;
        for (int attempt = 0; attempt < 64 && !moved; ++attempt) {
            const int donor = donors[random() % donors.size()];
            const int recipient = static_cast<int>(random() % total);
            if (donor == recipient || padded[donor] < padded[recipient] + 2) continue;
            --padded[donor];
            ++padded[recipient];
            std::sort(padded.begin(), padded.end(), std::greater<int>());
            moved = true;
        }
        if (!moved) walk_left = 0;
        else --walk_left;
    }

    std::cout << "TRANSFER_SAMPLE k=" << k << " samples=" << sample_count
              << " seed=" << seed << '\n';
    std::cout << "BLOCK_GREEDY ok=" << block_ok << '/' << sample_count << '\n';
    std::cout << "BLOCK_LOOKAHEAD ok=" << lookahead_ok << '/' << sample_count << '\n';
    std::cout << "BLOCK_ROW_RESERVE ok=" << reserve_ok << '/' << sample_count << '\n';
    if (!first_block_failure.empty())
        std::cout << "FIRST_BLOCK_GREEDY_FAILURE " << show(first_block_failure) << '\n';
    if (!first_lookahead_failure.empty())
        std::cout << "FIRST_BLOCK_LOOKAHEAD_FAILURE "
                  << show(first_lookahead_failure)
                  << " exact_A=" << show(first_lookahead_solution.a)
                  << " exact_B=" << show(first_lookahead_solution.b) << '\n';
    if (!first_reserve_failure.empty())
        std::cout << "FIRST_BLOCK_ROW_RESERVE_FAILURE "
                  << show(first_reserve_failure)
                  << " exact_A=" << show(first_reserve_solution.a)
                  << " exact_B=" << show(first_reserve_solution.b) << '\n';
}

struct DominatedPartitionSampler {
    Sequence parent_prefix{0};
    int total = 0;
    int maximum = 0;
    std::vector<std::uint64_t> memo;

    explicit DominatedPartitionSampler(const Sequence &parent) : maximum(parent.front()) {
        for (int value : parent) {
            total += value;
            parent_prefix.push_back(total);
        }
        memo.assign(static_cast<std::size_t>(total + 1) * (maximum + 1) * (total + 1),
                    std::numeric_limits<std::uint64_t>::max());
    }

    int parent_H(int count) const {
        return parent_prefix[std::min(count, static_cast<int>(parent_prefix.size()) - 1)];
    }

    std::size_t key(int remaining, int upper, int length) const {
        return (static_cast<std::size_t>(remaining) * (maximum + 1) + upper) *
                   (total + 1) +
               length;
    }

    std::uint64_t count(int remaining, int upper, int length) {
        if (remaining == 0) return 1;
        auto &answer = memo[key(remaining, upper, length)];
        if (answer != std::numeric_limits<std::uint64_t>::max()) return answer;
        answer = 0;
        const int used = total - remaining;
        for (int value = std::min(upper, remaining); value >= 1; --value) {
            if (used + value > parent_H(length + 1)) continue;
            const std::uint64_t branch = count(remaining - value, value, length + 1);
            if (std::numeric_limits<std::uint64_t>::max() - answer <= branch) {
                answer = std::numeric_limits<std::uint64_t>::max() - 1;
                break;
            }
            answer += branch;
        }
        return answer;
    }

    Sequence sample(std::mt19937_64 &random) {
        Sequence state;
        int remaining = total;
        int upper = maximum;
        while (remaining > 0) {
            struct Choice {
                int value;
                std::uint64_t weight;
            };
            std::vector<Choice> choices;
            std::uint64_t sum = 0;
            const int used = total - remaining;
            for (int value = std::min(upper, remaining); value >= 1; --value) {
                if (used + value > parent_H(static_cast<int>(state.size()) + 1)) continue;
                const std::uint64_t weight =
                    count(remaining - value, value, static_cast<int>(state.size()) + 1);
                choices.push_back({value, weight});
                sum += weight;
            }
            std::uniform_int_distribution<std::uint64_t> pick(0, sum - 1);
            std::uint64_t ticket = pick(random);
            int selected = choices.back().value;
            for (const auto [value, weight] : choices) {
                if (ticket < weight) {
                    selected = value;
                    break;
                }
                ticket -= weight;
            }
            state.push_back(selected);
            remaining -= selected;
            upper = selected;
        }
        return state;
    }
};

void sample_uniform_states(int k, std::uint64_t sample_count, std::uint64_t seed) {
    const Sequence parent = singleton_base(k);
    const Hall hall(singleton_base(k - 1));
    DominatedPartitionSampler sampler(parent);
    const std::uint64_t universe = sampler.count(sampler.total, sampler.maximum, 0);
    std::mt19937_64 random(seed);
    std::uint64_t block_ok = 0;
    std::uint64_t lookahead_ok = 0;
    std::uint64_t reserve_ok = 0;
    Sequence first_block_failure;
    Sequence first_lookahead_failure;
    Sequence first_reserve_failure;
    Coloring first_lookahead_solution;
    Coloring first_reserve_solution;

    for (std::uint64_t sample = 0; sample < sample_count; ++sample) {
        const Sequence state = sampler.sample(random);
        const bool block = greedy_block_coloring(state, hall);
        const bool lookahead = greedy_block_coloring(state, hall, true);
        const bool reserve =
            greedy_block_coloring(state, hall, false, BlockOrder::Balanced, true);
        block_ok += block;
        lookahead_ok += lookahead;
        reserve_ok += reserve;
        if (!block && first_block_failure.empty()) first_block_failure = state;
        if (!reserve && first_reserve_failure.empty()) {
            first_reserve_failure = state;
            GeneralSearch exact(state, hall);
            Coloring current;
            if (!exact.dfs(0, current)) {
                std::cerr << "ROW_COLORING_COUNTEREXAMPLE k=" << k
                          << " state=" << show(state) << '\n';
                std::exit(1);
            }
            first_reserve_solution = exact.solution;
        }
        if (!lookahead && first_lookahead_failure.empty()) {
            first_lookahead_failure = state;
            GeneralSearch exact(state, hall);
            Coloring current;
            if (!exact.dfs(0, current)) {
                std::cerr << "ROW_COLORING_COUNTEREXAMPLE k=" << k
                          << " state=" << show(state) << '\n';
                std::exit(1);
            }
            first_lookahead_solution = exact.solution;
        }
    }

    std::cout << "UNIFORM_SAMPLE k=" << k << " universe=" << universe
              << " samples=" << sample_count << " seed=" << seed << '\n';
    std::cout << "BLOCK_GREEDY ok=" << block_ok << '/' << sample_count << '\n';
    std::cout << "BLOCK_LOOKAHEAD ok=" << lookahead_ok << '/' << sample_count << '\n';
    std::cout << "BLOCK_ROW_RESERVE ok=" << reserve_ok << '/' << sample_count << '\n';
    if (!first_block_failure.empty())
        std::cout << "FIRST_BLOCK_GREEDY_FAILURE " << show(first_block_failure) << '\n';
    if (!first_lookahead_failure.empty())
        std::cout << "FIRST_BLOCK_LOOKAHEAD_FAILURE "
                  << show(first_lookahead_failure)
                  << " exact_A=" << show(first_lookahead_solution.a)
                  << " exact_B=" << show(first_lookahead_solution.b) << '\n';
    if (!first_reserve_failure.empty())
        std::cout << "FIRST_BLOCK_ROW_RESERVE_FAILURE "
                  << show(first_reserve_failure)
                  << " exact_A=" << show(first_reserve_solution.a)
                  << " exact_B=" << show(first_reserve_solution.b) << '\n';
}

struct GlobalBalanceCensus {
    int k;
    Sequence parent;
    Hall hall;
    Sequence parent_prefix{0};
    int total = 0;
    std::uint64_t states = 0;
    std::uint64_t nodes = 0;
    Sequence first_failure;
    int first_best_difference = 0;
    Coloring first_unrestricted_optimum;
    Coloring first_legal;

    explicit GlobalBalanceCensus(int level)
        : k(level), parent(singleton_base(level)),
          hall(singleton_base(level - 1)) {
        for (int value : parent) {
            total += value;
            parent_prefix.push_back(total);
        }
    }

    int parent_H(int count) const {
        return parent_prefix[std::min(count, static_cast<int>(parent.size()))];
    }

    void inspect(const Sequence &state) {
        ++states;
        GlobalBalanceSearch search(state, hall);
        const bool ok = search.run();
        nodes += search.nodes;
        if (!ok && first_failure.empty()) {
            first_failure = state;
            first_best_difference = search.best_difference;
            if (!search.find_unrestricted_optimum()) {
                std::cerr << "NO_GLOBAL_BALANCE_OPTIMUM k=" << k
                          << " state=" << show(state) << '\n';
                std::exit(1);
            }
            first_unrestricted_optimum = search.unrestricted_optimum;
            GeneralSearch exact(state, hall);
            Coloring current;
            if (!exact.dfs(0, current)) {
                std::cerr << "ROW_COLORING_COUNTEREXAMPLE k=" << k
                          << " state=" << show(state) << '\n';
                std::exit(1);
            }
            first_legal = exact.solution;
        }
    }

    void enumerate(int remaining, int maximum, Sequence &state) {
        if (!first_failure.empty()) return;
        if (remaining == 0) {
            inspect(state);
            return;
        }
        const int used = total - remaining;
        for (int value = std::min(maximum, remaining); value >= 1; --value) {
            if (used + value > parent_H(static_cast<int>(state.size()) + 1)) continue;
            state.push_back(value);
            enumerate(remaining - value, value, state);
            state.pop_back();
            if (!first_failure.empty()) return;
        }
    }

    void run() {
        Sequence state;
        enumerate(total, parent.front(), state);
        std::cout << "GLOBAL_BALANCE_CENSUS k=" << k
                  << " states=" << states
                  << " nodes=" << nodes;
        if (first_failure.empty())
            std::cout << " ok=" << states << '/' << states << '\n';
        else
            std::cout << " first_failure=" << show(first_failure)
                      << " best_difference=" << first_best_difference
                      << " optimum_A=" << show(first_unrestricted_optimum.a)
                      << " optimum_B=" << show(first_unrestricted_optimum.b)
                      << " legal_A=" << show(first_legal.a)
                      << " legal_B=" << show(first_legal.b) << '\n';
    }
};

}  // namespace

int main(int argc, char **argv) {
    if (argc >= 2 && std::string(argv[1]) == "--padded-prefix-check") {
        const int k = argc > 2 ? std::atoi(argv[2]) : 8;
        if (k < 1 || k > 12) {
            std::cerr << "usage: singleton_pair_coloring_census --padded-prefix-check k\n";
            return 2;
        }
        check_padded_prefix_arithmetic(k);
        return 0;
    }
    if (argc >= 2 && std::string(argv[1]) == "--padded-three-census") {
        const int k = argc > 2 ? std::atoi(argv[2]) : 4;
        if (k < 1 || k > 4) {
            std::cerr << "usage: singleton_pair_coloring_census --padded-three-census k\n";
            return 2;
        }
        PaddedThreeCensus census(k);
        census.run();
        return 0;
    }
    if (argc >= 2 && std::string(argv[1]) == "--global-census") {
        const int k = argc > 2 ? std::atoi(argv[2]) : 4;
        if (k < 1 || k > 4) {
            std::cerr << "usage: singleton_pair_coloring_census --global-census k\n";
            return 2;
        }
        GlobalBalanceCensus census(k);
        census.run();
        return 0;
    }
    if (argc >= 2 && std::string(argv[1]) == "--uniform") {
        const int k = argc > 2 ? std::atoi(argv[2]) : 5;
        const std::uint64_t samples = argc > 3 ? std::strtoull(argv[3], nullptr, 10) : 100000;
        const std::uint64_t seed = argc > 4 ? std::strtoull(argv[4], nullptr, 10) : 1;
        if (k < 1 || k > 5) {
            std::cerr << "usage: singleton_pair_coloring_census --uniform k samples [seed]\n";
            return 2;
        }
        sample_uniform_states(k, samples, seed);
        return 0;
    }
    if (argc >= 2 && std::string(argv[1]) == "--sample") {
        const int k = argc > 2 ? std::atoi(argv[2]) : 5;
        const std::uint64_t samples = argc > 3 ? std::strtoull(argv[3], nullptr, 10) : 100000;
        const std::uint64_t seed = argc > 4 ? std::strtoull(argv[4], nullptr, 10) : 1;
        if (k < 1 || k > 8) {
            std::cerr << "usage: singleton_pair_coloring_census --sample k samples [seed]\n";
            return 2;
        }
        sample_transfer_states(k, samples, seed);
        return 0;
    }
    const int k = argc > 1 ? std::atoi(argv[1]) : 4;
    if (k < 1 || k > 4) {
        std::cerr << "usage: singleton_pair_coloring_census [k=1..4]\n";
        return 2;
    }
    Census census(k);
    census.run();
    return 0;
}
