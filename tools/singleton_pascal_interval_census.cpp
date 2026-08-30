#include <algorithm>
#include <chrono>
#include <compare>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// Exact parent-first census for the Pascal prefix/suffix allocation problems
// exposed by arbitrary tight parent prefixes.
//
// If h=G_(K-1), a tight parent prefix of size t chooses a pure-row count from
// the plateau argmax_p H(p)+H(t-p).  Consecutive tight ranks expose three
// contracted child bands.  The transition modes enumerate every exact-row
// parent refinement of such a band and search all admissible endpoint counts;
// the tail modes allow arbitrary positive-row refinements of a suffix.  The
// two-anchor modes delete the universal pure column and one maximum mixed
// column, then census the resulting capped residual-coloring problem and
// deliberately stronger assignment/transfer rules.

namespace {

using Sequence = std::vector<int>;

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

Sequence slice(const Sequence &values, int begin, int end) {
    begin = std::clamp(begin, 0, static_cast<int>(values.size()));
    end = std::clamp(end, begin, static_cast<int>(values.size()));
    return Sequence(values.begin() + begin, values.begin() + end);
}

std::string show(const Sequence &values) {
    std::string result = "(";
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index) result += ',';
        result += std::to_string(values[index]);
    }
    return result + ')';
}

struct Capacity {
    Sequence values;
    Sequence prefix{0};
    int mass = 0;
    int largest = 0;

    explicit Capacity(Sequence sequence) : values(std::move(sequence)) {
        if (!values.empty()) largest = values.front();
        for (int value : values) {
            mass += value;
            prefix.push_back(mass);
        }
    }

    int H(int rows) const {
        return prefix[std::min(rows, static_cast<int>(prefix.size()) - 1)];
    }
};

Sequence induced_parent(const Capacity &left, const Capacity &mixed,
                        const Capacity &right) {
    const int max_rows = static_cast<int>(left.values.size() +
                                          mixed.values.size() +
                                          right.values.size());
    Sequence result;
    int previous = 0;
    for (int rows = 1; rows <= max_rows; ++rows) {
        int outer = 0;
        for (int left_rows = 0; left_rows <= rows; ++left_rows)
            outer = std::max(outer, left.H(left_rows) +
                                      right.H(rows - left_rows));
        const int rank = mixed.H(rows) + outer;
        result.push_back(rank - previous);
        previous = rank;
    }
    while (!result.empty() && result.back() == 0) result.pop_back();
    if (!std::is_sorted(result.begin(), result.end(), std::greater<int>())) {
        std::cerr << "INDUCED_RANK_NOT_CONCAVE profile=" << show(result) << '\n';
        std::exit(1);
    }
    return result;
}

struct Child {
    std::vector<std::uint8_t> counts;
    int mass = 0;
    int parts = 0;

    explicit Child(int largest) : counts(largest + 1, 0) {}
};

struct Choice {
    int left = 0;
    int mixed = 0;
    int right = 0;

    auto operator<=>(const Choice &) const = default;
};

struct SplitSearch {
    const Sequence &state;
    const Capacity &left_capacity;
    const Capacity &mixed_capacity;
    const Capacity &right_capacity;
    int largest = 0;
    std::vector<std::vector<Choice>> choices;
    std::uint64_t nodes = 0;
    bool found = false;
    std::vector<Choice> path;
    std::vector<Choice> solution;
    int required_left_parts = -1;
    int required_right_parts = -1;
    int required_mixed_parts = -1;
    int required_mixed_prefix = -1;
    int fixed_orientation_rule = 0;
    int orientation_parameter = 0;
    bool require_exact_left = false;

    SplitSearch(const Sequence &parent, const Capacity &left,
                const Capacity &mixed, const Capacity &right,
                int left_parts = -1, int right_parts = -1,
                bool exact_left = false, int mixed_parts = -1,
                int mixed_prefix = -1, int orientation_rule = 0)
        : state(parent), left_capacity(left), mixed_capacity(mixed),
          right_capacity(right), required_left_parts(left_parts),
          required_right_parts(right_parts), required_mixed_parts(mixed_parts),
          required_mixed_prefix(mixed_prefix),
          fixed_orientation_rule(orientation_rule),
          require_exact_left(exact_left) {
        largest = std::max({left.largest, mixed.largest, right.largest});
        const int parent_largest = state.empty() ? 0 : state.front();
        choices.resize(parent_largest + 1);
        for (int value = 1; value <= parent_largest; ++value) {
            std::set<Choice> distinct;
            for (int pure = 0; pure <= std::min(value, left.largest); ++pure) {
                const int middle = value - pure;
                if (middle <= mixed.largest)
                    distinct.insert({pure, middle, 0});
            }
            for (int pure = 0; pure <= std::min(value, right.largest); ++pure) {
                const int middle = value - pure;
                if (middle <= mixed.largest)
                    distinct.insert({0, middle, pure});
            }
            choices[value].assign(distinct.begin(), distinct.end());
            std::stable_sort(choices[value].begin(), choices[value].end(),
                             [](const Choice &first, const Choice &second) {
                const auto score = [](const Choice &choice) {
                    const int pure = choice.left + choice.right;
                    return std::tuple{std::abs(pure - choice.mixed),
                                      choice.right != 0, pure};
                };
                return score(first) < score(second);
            });
        }
    }

    static void add(Child &child, int value) {
        if (value == 0) return;
        ++child.counts[value];
        child.mass += value;
        ++child.parts;
    }

    static void remove(Child &child, int value) {
        if (value == 0) return;
        --child.counts[value];
        child.mass -= value;
        --child.parts;
    }

    bool child_ok(const Child &child, const Capacity &capacity) const {
        if (child.mass > capacity.mass) return false;
        int prefix = 0;
        int rows = 0;
        for (int value = largest; value >= 1; --value) {
            for (int count = 0; count < child.counts[value]; ++count) {
                ++rows;
                prefix += value;
                if (prefix > capacity.H(rows)) return false;
            }
        }
        return true;
    }

    bool exact_left_prefix_ok(const Child &child) const {
        if (!require_exact_left) return true;
        std::vector<int> available(largest + 1, 0);
        for (int value : left_capacity.values) ++available[value];
        for (int value = 1; value <= largest; ++value)
            if (child.counts[value] > available[value]) return false;
        return true;
    }

    bool deficits_possible(const Child &left, const Child &mixed,
                           const Child &right, int remaining) const {
        const int left_deficit = left_capacity.mass - left.mass;
        const int mixed_deficit = mixed_capacity.mass - mixed.mass;
        const int right_deficit = right_capacity.mass - right.mass;
        return left_deficit >= 0 && mixed_deficit >= 0 && right_deficit >= 0 &&
               left_deficit + mixed_deficit + right_deficit == remaining;
    }

    void dfs(std::size_t row, int minimum_choice, int remaining,
             Child &left, Child &mixed, Child &right) {
        ++nodes;
        if (found || !deficits_possible(left, mixed, right, remaining)) return;
        if (required_left_parts >= 0) {
            const int rows_left = static_cast<int>(state.size() - row);
            const int missing_left = required_left_parts - left.parts;
            const int missing_right = required_right_parts - right.parts;
            if (missing_left < 0 || missing_right < 0 ||
                missing_left + missing_right > rows_left)
                return;
        }
        if (required_mixed_parts >= 0) {
            const int rows_left = static_cast<int>(state.size() - row);
            const int missing_mixed = required_mixed_parts - mixed.parts;
            if (missing_mixed < 0 || missing_mixed > rows_left) return;
        }
        if (row == state.size()) {
            found = left.mass == left_capacity.mass &&
                    mixed.mass == mixed_capacity.mass &&
                    right.mass == right_capacity.mass &&
                    (required_left_parts < 0 ||
                     (left.parts == required_left_parts &&
                      right.parts == required_right_parts)) &&
                    (required_mixed_parts < 0 ||
                     mixed.parts == required_mixed_parts);
            if (found) solution = path;
            return;
        }

        const int value = state[row];
        const bool same_value =
            row > 0 && state[row - 1] == value &&
            required_mixed_prefix != static_cast<int>(row) &&
            fixed_orientation_rule == 0;
        const int first = same_value ? minimum_choice : 0;
        for (int index = first; index < static_cast<int>(choices[value].size()); ++index) {
            const Choice &choice = choices[value][index];
            if (required_mixed_prefix >= 0 &&
                ((static_cast<int>(row) < required_mixed_prefix) !=
                 (choice.mixed > 0)))
                continue;
            if (fixed_orientation_rule == 1) {
                const int half = static_cast<int>(state.size()) / 2;
                const int local = static_cast<int>(row) % half;
                const bool left_orientation =
                    static_cast<int>(row) < half ? local % 2 == 0
                                                 : local % 2 != 0;
                if (left_orientation != (choice.left > 0)) continue;
            }
            if (fixed_orientation_rule == 2) {
                const int half = static_cast<int>(state.size()) / 2;
                const int local = static_cast<int>(row) % half;
                const bool left_orientation =
                    static_cast<int>(row) < half
                        ? local < orientation_parameter
                        : local >= orientation_parameter;
                if (left_orientation != (choice.left > 0)) continue;
            }
            add(left, choice.left);
            add(mixed, choice.mixed);
            add(right, choice.right);
            path.push_back(choice);
            if (exact_left_prefix_ok(left) &&
                child_ok(left, left_capacity) &&
                child_ok(mixed, mixed_capacity) &&
                child_ok(right, right_capacity))
                dfs(row + 1, index, remaining - value, left, mixed, right);
            path.pop_back();
            remove(right, choice.right);
            remove(mixed, choice.mixed);
            remove(left, choice.left);
            if (found) return;
        }
    }

    bool run() {
        Child left(largest), mixed(largest), right(largest);
        int total = 0;
        for (int value : state) total += value;
        dfs(0, 0, total, left, mixed, right);
        return found;
    }
};

struct Census {
    Capacity left;
    Capacity mixed;
    Capacity right;
    Capacity parent;
    std::uint64_t state_limit = 0;
    std::uint64_t state_skip = 0;
    std::uint64_t seen = 0;
    std::uint64_t tested = 0;
    std::uint64_t nodes = 0;
    std::uint64_t max_nodes = 0;
    Sequence worst;
    int exact_rows = 0;
    bool require_induced_profile = true;
    std::chrono::steady_clock::time_point started = std::chrono::steady_clock::now();

    Census(Sequence left_values, Sequence mixed_values, Sequence right_values,
           Sequence parent_values, std::uint64_t limit, std::uint64_t skip,
           int required_rows, bool check_induced_profile = true)
        : left(std::move(left_values)), mixed(std::move(mixed_values)),
          right(std::move(right_values)), parent(std::move(parent_values)),
          state_limit(limit), state_skip(skip), exact_rows(required_rows),
          require_induced_profile(check_induced_profile) {}

    bool done() const { return state_limit != 0 && tested >= state_limit; }

    void inspect(const Sequence &state) {
        SplitSearch search(state, left, mixed, right);
        if (!search.run()) {
            std::cerr << "PASCAL_INTERVAL_COUNTEREXAMPLE parent=" << show(state)
                      << " left=" << show(left.values)
                      << " mixed=" << show(mixed.values)
                      << " right=" << show(right.values)
                      << " nodes=" << search.nodes << '\n';
            std::exit(1);
        }
        ++tested;
        nodes += search.nodes;
        if (search.nodes > max_nodes) {
            max_nodes = search.nodes;
            worst = state;
        }
        if (tested % 100000 == 0) {
            const double seconds = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - started).count();
            std::cout << "PASCAL_INTERVAL_PROGRESS tested=" << tested
                      << " skipped=" << state_skip << " nodes=" << nodes
                      << " seconds=" << seconds << '\n';
        }
    }

    void enumerate(int remaining, int maximum, Sequence &state) {
        if (done()) return;
        if (remaining == 0) {
            if (exact_rows != 0 && static_cast<int>(state.size()) != exact_rows)
                return;
            ++seen;
            if (seen > state_skip) inspect(state);
            return;
        }
        if (exact_rows != 0 && static_cast<int>(state.size()) >= exact_rows)
            return;
        const int used = parent.mass - remaining;
        for (int value = std::min(maximum, remaining); value >= 1; --value) {
            if (used + value > parent.H(static_cast<int>(state.size()) + 1))
                continue;
            state.push_back(value);
            enumerate(remaining - value, value, state);
            state.pop_back();
            if (done()) return;
        }
    }

    void run(bool report = true) {
        Sequence induced = induced_parent(left, mixed, right);
        if (require_induced_profile && induced != parent.values) {
            std::cerr << "PASCAL_INTERVAL_PROFILE_MISMATCH expected="
                      << show(parent.values) << " induced=" << show(induced) << '\n';
            std::exit(1);
        }
        Sequence state;
        if (parent.mass == 0) {
            ++seen;
            if (state_skip == 0) inspect(state);
        } else {
            enumerate(parent.mass, parent.largest, state);
        }
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        if (!report) return;
        std::cout << "PASCAL_INTERVAL_CENSUS complete="
                  << (state_limit == 0 && state_skip == 0 ? "YES" : "NO")
                  << " states=" << tested << " skipped=" << state_skip
                  << " exact_rows=" << exact_rows
                  << " nodes=" << nodes << " max_nodes=" << max_nodes
                  << " worst_state=" << show(worst)
                  << " left=" << show(left.values)
                  << " mixed=" << show(mixed.values)
                  << " right=" << show(right.values)
                  << " parent=" << show(parent.values)
                  << " seconds=" << seconds << '\n';
    }
};

bool power_of_two(int value) {
    return value > 0 && (value & (value - 1)) == 0;
}

std::vector<int> maximizing_left_counts(const Capacity &capacity, int rows) {
    int best = -1;
    std::vector<int> result;
    for (int left = 0; left <= rows; ++left) {
        const int value = capacity.H(left) + capacity.H(rows - left);
        if (value > best) {
            best = value;
            result.clear();
        }
        if (value == best) result.push_back(left);
    }
    return result;
}

struct Transition {
    int from = 0;
    int to = 0;
    Capacity left;
    Capacity mixed;
    Capacity right;

    Transition(int start, int finish, Sequence left_values,
               Sequence mixed_values, Sequence right_values)
        : from(start), to(finish), left(std::move(left_values)),
          mixed(std::move(mixed_values)), right(std::move(right_values)) {}
};

struct TransitionBandCensus {
    int begin = 0;
    int end = 0;
    Capacity parent;
    std::vector<Transition> transitions;
    std::uint64_t states = 0;
    std::uint64_t nodes = 0;
    std::uint64_t max_nodes = 0;
    Sequence worst;
    std::vector<std::uint64_t> transition_uses;
    std::vector<std::uint64_t> feasible_transition_histogram;
    std::uint64_t left_total_failures = 0;
    std::uint64_t right_total_failures = 0;
    std::uint64_t incomplete_relations = 0;
    std::uint64_t strict_states = 0;
    std::uint64_t strict_incomplete_relations = 0;
    std::uint64_t alternating_failures = 0;
    std::uint64_t strict_alternating_failures = 0;
    std::map<std::string, std::uint64_t> relation_shapes;
    Sequence first_left_total_failure;
    Sequence first_right_total_failure;
    Sequence first_alternating_failure;
    int first_missing_from = -1;
    int first_missing_to = -1;
    int first_alternating_p = -1;
    int first_alternating_q = -1;
    bool left_extension_only = false;

    TransitionBandCensus(const Sequence &child, const Sequence &parent_base,
                         int first, int last, bool extension_only = false)
        : begin(first), end(last),
          parent(slice(parent_base, first, last)),
          left_extension_only(extension_only) {
        const Capacity child_capacity(child);
        const std::vector<int> starts =
            maximizing_left_counts(child_capacity, begin);
        const std::vector<int> finishes =
            maximizing_left_counts(child_capacity, end);
        for (int from : starts) {
            for (int to : finishes) {
                if (from > to || begin - from > end - to) continue;
                transitions.emplace_back(
                    from, to, slice(child, from, to), slice(child, begin, end),
                    slice(child, begin - from, end - to));
                const Transition &transition = transitions.back();
                const int mass = transition.left.mass + transition.mixed.mass +
                                 transition.right.mass;
                if (mass != parent.mass) {
                    std::cerr << "PASCAL_TRANSITION_MASS_MISMATCH band=("
                              << begin << ',' << end << ") from=" << from
                              << " to=" << to << " child_mass=" << mass
                              << " parent_mass=" << parent.mass << '\n';
                    std::exit(1);
                }
            }
        }
        transition_uses.assign(transitions.size(), 0);
        feasible_transition_histogram.assign(transitions.size() + 1, 0);
    }

    void inspect(const Sequence &state) {
        ++states;
        if (left_extension_only) {
            std::set<int> starts;
            for (const Transition &transition : transitions)
                starts.insert(transition.from);
            std::uint64_t state_nodes = 0;
            for (int from : starts) {
                bool extended = false;
                for (const Transition &transition : transitions) {
                    if (transition.from != from) continue;
                    SplitSearch search(state, transition.left, transition.mixed,
                                       transition.right,
                                       transition.to - transition.from,
                                       (end - transition.to) -
                                           (begin - transition.from));
                    const bool found = search.run();
                    state_nodes += search.nodes;
                    if (found) {
                        extended = true;
                        break;
                    }
                }
                if (!extended) {
                    std::cerr << "PASCAL_LEFT_EXTENSION_COUNTEREXAMPLE band=("
                              << begin << ',' << end << ") parent="
                              << show(state) << " from=" << from
                              << " nodes=" << state_nodes << '\n';
                    std::exit(1);
                }
            }
            nodes += state_nodes;
            if (state_nodes > max_nodes) {
                max_nodes = state_nodes;
                worst = state;
            }
            return;
        }
        int feasible = 0;
        std::uint64_t state_nodes = 0;
        std::string relation_shape;
        std::set<int> all_from;
        std::set<int> all_to;
        std::set<int> feasible_from;
        std::set<int> feasible_to;
        for (std::size_t index = 0; index < transitions.size(); ++index) {
            const Transition &transition = transitions[index];
            all_from.insert(transition.from);
            all_to.insert(transition.to);
            SplitSearch search(state, transition.left, transition.mixed,
                               transition.right,
                               transition.to - transition.from,
                               (end - transition.to) -
                                   (begin - transition.from));
            const bool found = search.run();
            relation_shape += found ? '1' : '0';
            state_nodes += search.nodes;
            if (found) {
                ++feasible;
                ++transition_uses[index];
                feasible_from.insert(transition.from);
                feasible_to.insert(transition.to);
            }
        }
        nodes += state_nodes;
        if (state_nodes > max_nodes) {
            max_nodes = state_nodes;
            worst = state;
        }
        if (feasible == 0) {
            std::cerr << "PASCAL_TRANSITION_COUNTEREXAMPLE band=(" << begin
                      << ',' << end << ") parent=" << show(state)
                      << " transitions=" << transitions.size()
                      << " nodes=" << state_nodes << '\n';
            std::exit(1);
        }
        ++feasible_transition_histogram[feasible];
        const bool incomplete =
            feasible != static_cast<int>(transitions.size());
        if (incomplete)
            ++incomplete_relations;
        int prefix = 0;
        bool has_internal_tight = false;
        for (int rank = 0; rank + 1 < static_cast<int>(state.size()); ++rank) {
            prefix += state[rank];
            if (prefix == parent.H(rank + 1)) {
                has_internal_tight = true;
                break;
            }
        }
        if (!has_internal_tight) {
            ++strict_states;
            if (incomplete) ++strict_incomplete_relations;
        }
        if (begin == 0 && transitions.size() == 1 &&
            end == 2 * static_cast<int>(transitions.front().mixed.values.size())) {
            Sequence left_prefix{0};
            Sequence right_prefix{0};
            for (std::size_t index = 0; index < state.size(); ++index) {
                Sequence &color_prefix =
                    index % 2 == 0 ? left_prefix : right_prefix;
                color_prefix.push_back(color_prefix.back() + state[index]);
            }
            bool alternating_ok = true;
            for (int left_rows = 0;
                 left_rows < static_cast<int>(left_prefix.size()) && alternating_ok;
                 ++left_rows) {
                for (int right_rows = 0;
                     right_rows < static_cast<int>(right_prefix.size());
                     ++right_rows) {
                    const int demand = left_prefix[left_rows] +
                                       right_prefix[right_rows];
                    const int capacity = transitions.front().left.H(left_rows) +
                                         transitions.front().mixed.H(
                                             left_rows + right_rows) +
                                         transitions.front().right.H(right_rows);
                    if (demand <= capacity) continue;
                    alternating_ok = false;
                    if (first_alternating_failure.empty()) {
                        first_alternating_failure = state;
                        first_alternating_p = left_rows;
                        first_alternating_q = right_rows;
                    }
                    break;
                }
            }
            if (!alternating_ok) {
                ++alternating_failures;
                if (!has_internal_tight) ++strict_alternating_failures;
            }
        }
        std::string relation_key;
        for (const Transition &transition : transitions) {
            relation_key += std::to_string(transition.from);
            relation_key += '>';
            relation_key += std::to_string(transition.to);
            relation_key += ',';
        }
        relation_key += ':';
        relation_key += relation_shape;
        ++relation_shapes[relation_key];
        if (feasible_from != all_from) {
            ++left_total_failures;
            if (first_left_total_failure.empty()) {
                first_left_total_failure = state;
                for (int value : all_from)
                    if (!feasible_from.contains(value)) {
                        first_missing_from = value;
                        break;
                    }
            }
        }
        if (feasible_to != all_to) {
            ++right_total_failures;
            if (first_right_total_failure.empty()) {
                first_right_total_failure = state;
                for (int value : all_to)
                    if (!feasible_to.contains(value)) {
                        first_missing_to = value;
                        break;
                    }
            }
        }
    }

    void enumerate(int remaining, int maximum, Sequence &state) {
        if (remaining == 0) {
            if (static_cast<int>(state.size()) == end - begin) inspect(state);
            return;
        }
        if (static_cast<int>(state.size()) >= end - begin) return;
        const int used = parent.mass - remaining;
        for (int value = std::min(maximum, remaining); value >= 1; --value) {
            if (used + value > parent.H(static_cast<int>(state.size()) + 1))
                continue;
            state.push_back(value);
            enumerate(remaining - value, value, state);
            state.pop_back();
        }
    }

    void run() {
        Sequence state;
        enumerate(parent.mass, parent.largest, state);
    }
};

struct BalancedColorSearch {
    struct Block {
        int value = 0;
        int count = 0;
    };

    const Capacity &capacity;
    int maximum_rows = 0;
    std::vector<Block> blocks;
    std::uint64_t nodes = 0;
    std::vector<int> path;
    std::vector<int> solution;
    int total_mass = 0;
    int required_difference = -1;

    BalancedColorSearch(const Sequence &state, const Capacity &child,
                        int row_limit)
        : capacity(child), maximum_rows(row_limit) {
        for (int value : state) {
            total_mass += value;
            if (value == 0) continue;
            if (blocks.empty() || blocks.back().value != value)
                blocks.push_back({value, 1});
            else
                ++blocks.back().count;
        }
    }

    bool newest_ok(const Sequence &left_prefix,
                   const Sequence &right_prefix,
                   int old_left, int old_right) const {
        const int left_rows = static_cast<int>(left_prefix.size()) - 1;
        const int right_rows = static_cast<int>(right_prefix.size()) - 1;
        const auto hall = [&](int p, int q) {
            return capacity.H(p) + capacity.H(p + q) + capacity.H(q);
        };
        for (int p = old_left + 1; p <= left_rows; ++p)
            for (int q = 0; q <= right_rows; ++q)
                if (left_prefix[p] + right_prefix[q] > hall(p, q))
                    return false;
        for (int q = old_right + 1; q <= right_rows; ++q)
            for (int p = 0; p <= old_left; ++p)
                if (left_prefix[p] + right_prefix[q] > hall(p, q))
                    return false;
        return true;
    }

    bool dfs(std::size_t block, Sequence &left_prefix,
             Sequence &right_prefix, int left_mass, int right_mass) {
        ++nodes;
        if (block == blocks.size()) {
            if (required_difference >= 0 &&
                std::abs(left_mass - right_mass) != required_difference)
                return false;
            solution = path;
            return true;
        }
        const int value = blocks[block].value;
        const int count = blocks[block].count;
        std::vector<int> choices;
        for (int to_left = 0; to_left <= count; ++to_left) {
            if (block == 0 && to_left * 2 < count) continue;
            if (static_cast<int>(left_prefix.size()) - 1 + to_left > maximum_rows)
                continue;
            if (static_cast<int>(right_prefix.size()) - 1 + count - to_left >
                maximum_rows)
                continue;
            choices.push_back(to_left);
        }
        std::stable_sort(choices.begin(), choices.end(), [&](int first, int second) {
            const auto score = [&](int to_left) {
                const int to_right = count - to_left;
                return std::abs((left_mass + to_left * value) -
                                (right_mass + to_right * value));
            };
            return score(first) < score(second);
        });
        for (int to_left : choices) {
            const int to_right = count - to_left;
            const int old_left = static_cast<int>(left_prefix.size()) - 1;
            const int old_right = static_cast<int>(right_prefix.size()) - 1;
            for (int index = 0; index < to_left; ++index)
                left_prefix.push_back(left_prefix.back() + value);
            for (int index = 0; index < to_right; ++index)
                right_prefix.push_back(right_prefix.back() + value);
            path.push_back(to_left);
            if (newest_ok(left_prefix, right_prefix, old_left, old_right) &&
                dfs(block + 1, left_prefix, right_prefix,
                    left_mass + to_left * value,
                    right_mass + to_right * value))
                return true;
            path.pop_back();
            left_prefix.resize(old_left + 1);
            right_prefix.resize(old_right + 1);
        }
        return false;
    }

    bool greedy(std::size_t block, Sequence &left_prefix,
                Sequence &right_prefix, int left_mass, int right_mass) {
        if (block == blocks.size()) return true;
        const int value = blocks[block].value;
        const int count = blocks[block].count;
        std::vector<int> choices;
        for (int to_left = 0; to_left <= count; ++to_left) {
            if (block == 0 && to_left * 2 < count) continue;
            if (static_cast<int>(left_prefix.size()) - 1 + to_left > maximum_rows)
                continue;
            if (static_cast<int>(right_prefix.size()) - 1 + count - to_left >
                maximum_rows)
                continue;
            choices.push_back(to_left);
        }
        std::stable_sort(choices.begin(), choices.end(), [&](int first, int second) {
            const auto score = [&](int to_left) {
                const int to_right = count - to_left;
                return std::abs((left_mass + to_left * value) -
                                (right_mass + to_right * value));
            };
            return score(first) < score(second);
        });
        for (int to_left : choices) {
            const int to_right = count - to_left;
            const int old_left = static_cast<int>(left_prefix.size()) - 1;
            const int old_right = static_cast<int>(right_prefix.size()) - 1;
            for (int index = 0; index < to_left; ++index)
                left_prefix.push_back(left_prefix.back() + value);
            for (int index = 0; index < to_right; ++index)
                right_prefix.push_back(right_prefix.back() + value);
            const bool legal = newest_ok(left_prefix, right_prefix,
                                         old_left, old_right);
            if (legal)
                return greedy(block + 1, left_prefix, right_prefix,
                              left_mass + to_left * value,
                              right_mass + to_right * value);
            left_prefix.resize(old_left + 1);
            right_prefix.resize(old_right + 1);
        }
        return false;
    }

    bool has_next_extension(std::size_t next_block,
                            Sequence &left_prefix,
                            Sequence &right_prefix) const {
        if (next_block == blocks.size()) return true;
        const int value = blocks[next_block].value;
        const int count = blocks[next_block].count;
        for (int to_left = 0; to_left <= count; ++to_left) {
            if (static_cast<int>(left_prefix.size()) - 1 + to_left > maximum_rows)
                continue;
            if (static_cast<int>(right_prefix.size()) - 1 + count - to_left >
                maximum_rows)
                continue;
            const int old_left = static_cast<int>(left_prefix.size()) - 1;
            const int old_right = static_cast<int>(right_prefix.size()) - 1;
            for (int index = 0; index < to_left; ++index)
                left_prefix.push_back(left_prefix.back() + value);
            for (int index = 0; index < count - to_left; ++index)
                right_prefix.push_back(right_prefix.back() + value);
            const bool legal = newest_ok(left_prefix, right_prefix,
                                         old_left, old_right);
            left_prefix.resize(old_left + 1);
            right_prefix.resize(old_right + 1);
            if (legal) return true;
        }
        return false;
    }

    bool lookahead(std::size_t block, Sequence &left_prefix,
                   Sequence &right_prefix, int left_mass, int right_mass) {
        if (block == blocks.size()) return true;
        const int value = blocks[block].value;
        const int count = blocks[block].count;
        std::vector<int> choices;
        for (int to_left = 0; to_left <= count; ++to_left) {
            if (block == 0 && to_left * 2 < count) continue;
            if (static_cast<int>(left_prefix.size()) - 1 + to_left > maximum_rows)
                continue;
            if (static_cast<int>(right_prefix.size()) - 1 + count - to_left >
                maximum_rows)
                continue;
            choices.push_back(to_left);
        }
        std::stable_sort(choices.begin(), choices.end(), [&](int first, int second) {
            const auto score = [&](int to_left) {
                const int to_right = count - to_left;
                return std::abs((left_mass + to_left * value) -
                                (right_mass + to_right * value));
            };
            return score(first) < score(second);
        });
        for (int to_left : choices) {
            const int to_right = count - to_left;
            const int old_left = static_cast<int>(left_prefix.size()) - 1;
            const int old_right = static_cast<int>(right_prefix.size()) - 1;
            for (int index = 0; index < to_left; ++index)
                left_prefix.push_back(left_prefix.back() + value);
            for (int index = 0; index < to_right; ++index)
                right_prefix.push_back(right_prefix.back() + value);
            const bool legal = newest_ok(left_prefix, right_prefix,
                                         old_left, old_right);
            const bool extendable =
                legal && has_next_extension(block + 1,
                                            left_prefix, right_prefix);
            if (extendable)
                return lookahead(block + 1, left_prefix, right_prefix,
                                 left_mass + to_left * value,
                                 right_mass + to_right * value);
            left_prefix.resize(old_left + 1);
            right_prefix.resize(old_right + 1);
        }
        return false;
    }

    bool run() {
        Sequence left_prefix{0};
        Sequence right_prefix{0};
        return dfs(0, left_prefix, right_prefix, 0, 0);
    }

    bool run_at_minimum_balance() {
        Sequence values;
        for (const Block &block : blocks)
            values.insert(values.end(), block.count, block.value);
        std::vector<std::vector<unsigned char>> reachable(
            values.size() + 1,
            std::vector<unsigned char>(total_mass + 1, 0));
        reachable[0][0] = 1;
        int used = 0;
        for (int value : values) {
            for (int count = used; count >= 0; --count)
                for (int mass = total_mass - value; mass >= 0; --mass)
                    if (reachable[count][mass])
                        reachable[count + 1][mass + value] = 1;
            ++used;
        }
        required_difference = total_mass;
        const int minimum_color_rows = static_cast<int>(std::count_if(
            capacity.values.begin(), capacity.values.end(),
            [](int value) { return value > 0; }));
        const int minimum_left_rows =
            std::max(minimum_color_rows,
                     static_cast<int>(values.size()) - maximum_rows);
        const int maximum_left_rows =
            std::min(maximum_rows,
                     static_cast<int>(values.size()) - minimum_color_rows);
        for (int count = minimum_left_rows; count <= maximum_left_rows; ++count)
            for (int mass = 0; mass <= total_mass; ++mass)
                if (reachable[count][mass])
                    required_difference = std::min(
                        required_difference, std::abs(2 * mass - total_mass));
        return run();
    }

    bool run_greedy() {
        Sequence left_prefix{0};
        Sequence right_prefix{0};
        return greedy(0, left_prefix, right_prefix, 0, 0);
    }

    bool run_lookahead() {
        Sequence left_prefix{0};
        Sequence right_prefix{0};
        return lookahead(0, left_prefix, right_prefix, 0, 0);
    }
};

struct CommonSameColorSearch {
    struct Block {
        int value = 0;
        int count = 0;
    };

    const Capacity &capacity;
    int maximum_rows = 0;
    int largest = 0;
    Sequence source;
    Sequence target;
    std::vector<Block> blocks;
    std::vector<int> source_left;
    std::vector<int> source_right;
    std::vector<int> target_left;
    std::vector<int> target_right;
    int source_left_rows = 0;
    int source_right_rows = 0;
    int target_left_rows = 0;
    int target_right_rows = 0;
    int target_left_mass = 0;
    int target_right_mass = 0;
    std::uint64_t nodes = 0;

    CommonSameColorSearch(const Sequence &target_values, int donor,
                          int recipient, const Capacity &child,
                          int row_limit)
        : capacity(child), maximum_rows(row_limit), source(target_values),
          target(target_values) {
        ++source[donor];
        --source[recipient];
        largest = std::max(*std::max_element(source.begin(), source.end()),
                           *std::max_element(target.begin(), target.end()));
        source_left.assign(largest + 1, 0);
        source_right.assign(largest + 1, 0);
        target_left.assign(largest + 1, 0);
        target_right.assign(largest + 1, 0);
        add(source_left, source[donor], source_left_rows);
        add(source_left, source[recipient], source_left_rows);
        add(target_left, target[donor], target_left_rows);
        add(target_left, target[recipient], target_left_rows);
        target_left_mass = target[donor] + target[recipient];
        for (int row = 0; row < static_cast<int>(target.size()); ++row) {
            if (row == donor || row == recipient || target[row] == 0) continue;
            if (blocks.empty() || blocks.back().value != target[row])
                blocks.push_back({target[row], 1});
            else
                ++blocks.back().count;
        }
    }

    static void add(std::vector<int> &counts, int value, int &rows) {
        if (value == 0) return;
        ++counts[value];
        ++rows;
    }

    static void remove(std::vector<int> &counts, int value, int &rows) {
        if (value == 0) return;
        --counts[value];
        --rows;
    }

    Sequence prefix(const std::vector<int> &counts) const {
        Sequence result{0};
        for (int value = largest; value >= 1; --value)
            for (int copy = 0; copy < counts[value]; ++copy)
                result.push_back(result.back() + value);
        return result;
    }

    bool hall_ok(const std::vector<int> &left,
                 const std::vector<int> &right) const {
        const Sequence left_prefix = prefix(left);
        const Sequence right_prefix = prefix(right);
        for (int p = 0; p < static_cast<int>(left_prefix.size()); ++p)
            for (int q = 0; q < static_cast<int>(right_prefix.size()); ++q)
                if (left_prefix[p] + right_prefix[q] >
                    capacity.H(p) + capacity.H(p + q) + capacity.H(q))
                    return false;
        return true;
    }

    bool dfs(std::size_t block) {
        ++nodes;
        if (source_left_rows > maximum_rows ||
            source_right_rows > maximum_rows ||
            target_left_rows > maximum_rows ||
            target_right_rows > maximum_rows)
            return false;
        if (!hall_ok(source_left, source_right) ||
            !hall_ok(target_left, target_right))
            return false;
        if (block == blocks.size()) return true;

        const int value = blocks[block].value;
        const int count = blocks[block].count;
        std::vector<int> choices;
        for (int to_left = 0; to_left <= count; ++to_left)
            choices.push_back(to_left);
        std::stable_sort(choices.begin(), choices.end(), [&](int first,
                                                             int second) {
            const auto score = [&](int to_left) {
                return std::abs((target_left_mass + to_left * value) -
                                (target_right_mass +
                                 (count - to_left) * value));
            };
            return score(first) < score(second);
        });
        for (int to_left : choices) {
            const int to_right = count - to_left;
            for (int copy = 0; copy < to_left; ++copy) {
                add(source_left, value, source_left_rows);
                add(target_left, value, target_left_rows);
            }
            for (int copy = 0; copy < to_right; ++copy) {
                add(source_right, value, source_right_rows);
                add(target_right, value, target_right_rows);
            }
            target_left_mass += to_left * value;
            target_right_mass += to_right * value;
            if (dfs(block + 1)) return true;
            target_right_mass -= to_right * value;
            target_left_mass -= to_left * value;
            for (int copy = 0; copy < to_right; ++copy) {
                remove(target_right, value, target_right_rows);
                remove(source_right, value, source_right_rows);
            }
            for (int copy = 0; copy < to_left; ++copy) {
                remove(target_left, value, target_left_rows);
                remove(source_left, value, source_left_rows);
            }
        }
        return false;
    }

    bool run() { return dfs(0); }
};

struct MajorizationProductSearch {
    struct Block {
        int value = 0;
        int count = 0;
    };

    Capacity left_capacity;
    Capacity right_capacity;
    int maximum_rows = 0;
    std::vector<Block> blocks;
    std::uint64_t nodes = 0;

    MajorizationProductSearch(const Sequence &state, const Sequence &left,
                              const Sequence &right, int row_limit)
        : left_capacity(left), right_capacity(right),
          maximum_rows(row_limit) {
        for (int value : state) {
            if (value == 0) continue;
            if (blocks.empty() || blocks.back().value != value)
                blocks.push_back({value, 1});
            else
                ++blocks.back().count;
        }
    }

    bool dfs(std::size_t block, Sequence &left_prefix,
             Sequence &right_prefix, int left_mass, int right_mass,
             int remaining) {
        ++nodes;
        if (left_mass > left_capacity.mass ||
            right_mass > right_capacity.mass)
            return false;
        if (left_mass + remaining < left_capacity.mass ||
            right_mass + remaining < right_capacity.mass)
            return false;
        if (block == blocks.size())
            return left_mass == left_capacity.mass &&
                   right_mass == right_capacity.mass;

        const int value = blocks[block].value;
        const int count = blocks[block].count;
        std::vector<int> choices;
        for (int to_left = 0; to_left <= count; ++to_left) {
            if (static_cast<int>(left_prefix.size()) - 1 + to_left >
                maximum_rows)
                continue;
            if (static_cast<int>(right_prefix.size()) - 1 + count - to_left >
                maximum_rows)
                continue;
            choices.push_back(to_left);
        }
        std::stable_sort(choices.begin(), choices.end(), [&](int first,
                                                             int second) {
            const auto score = [&](int to_left) {
                return std::abs((left_mass + to_left * value) -
                                left_capacity.mass);
            };
            return score(first) < score(second);
        });
        for (int to_left : choices) {
            const int to_right = count - to_left;
            const int old_left = static_cast<int>(left_prefix.size()) - 1;
            const int old_right = static_cast<int>(right_prefix.size()) - 1;
            bool legal = true;
            for (int copy = 0; copy < to_left; ++copy) {
                left_prefix.push_back(left_prefix.back() + value);
                if (left_prefix.back() >
                    left_capacity.H(static_cast<int>(left_prefix.size()) - 1))
                    legal = false;
            }
            for (int copy = 0; copy < to_right; ++copy) {
                right_prefix.push_back(right_prefix.back() + value);
                if (right_prefix.back() >
                    right_capacity.H(static_cast<int>(right_prefix.size()) - 1))
                    legal = false;
            }
            if (legal &&
                dfs(block + 1, left_prefix, right_prefix,
                    left_mass + to_left * value,
                    right_mass + to_right * value,
                    remaining - count * value))
                return true;
            left_prefix.resize(old_left + 1);
            right_prefix.resize(old_right + 1);
        }
        return false;
    }

    bool run() {
        Sequence left_prefix{0};
        Sequence right_prefix{0};
        int mass = 0;
        for (const Block &block : blocks) mass += block.value * block.count;
        return dfs(0, left_prefix, right_prefix, 0, 0, mass);
    }
};

bool fixed_color_hall_ok(const Sequence &left, const Sequence &right,
                         const Capacity &capacity) {
    Sequence left_prefix{0};
    Sequence right_prefix{0};
    for (int value : left)
        left_prefix.push_back(left_prefix.back() + value);
    for (int value : right)
        right_prefix.push_back(right_prefix.back() + value);
    for (int p = 0; p < static_cast<int>(left_prefix.size()); ++p)
        for (int q = 0; q < static_cast<int>(right_prefix.size()); ++q)
            if (left_prefix[p] + right_prefix[q] >
                capacity.H(p) + capacity.H(p + q) + capacity.H(q))
                return false;
    return true;
}

std::vector<std::pair<Sequence, Sequence>> boundary_colorings(
    const Sequence &state, const Capacity &capacity, int maximum_rows) {
    std::vector<BalancedColorSearch::Block> blocks;
    for (int value : state) {
        if (blocks.empty() || blocks.back().value != value)
            blocks.push_back({value, 1});
        else
            ++blocks.back().count;
    }
    std::set<std::pair<Sequence, Sequence>> unique;
    Sequence left;
    Sequence right;
    const auto enumerate = [&](auto &&self, std::size_t block) -> void {
        if (block == blocks.size()) {
            if (fixed_color_hall_ok(left, right, capacity)) {
                auto coloring = std::make_pair(left, right);
                if (coloring.second < coloring.first)
                    std::swap(coloring.first, coloring.second);
                unique.insert(std::move(coloring));
            }
            return;
        }
        const int value = blocks[block].value;
        const int count = blocks[block].count;
        for (int to_left = 0; to_left <= count; ++to_left) {
            if (static_cast<int>(left.size()) + to_left > maximum_rows ||
                static_cast<int>(right.size()) + count - to_left >
                    maximum_rows)
                continue;
            left.insert(left.end(), to_left, value);
            right.insert(right.end(), count - to_left, value);
            self(self, block + 1);
            right.resize(right.size() - (count - to_left));
            left.resize(left.size() - to_left);
        }
    };
    enumerate(enumerate, 0);
    return {unique.begin(), unique.end()};
}

}  // namespace

int main(int argc, char **argv) {
    if (argc == 2 &&
        std::string(argv[1]) == "strict-exact-alternation-counterexample") {
        const int k = 6;
        const Sequence parent = singleton_base(k);
        const Capacity child(singleton_base(k - 1));
        Sequence state{63, 63, 57, 57, 42, 42, 42};
        state.insert(state.end(), 5, 23);
        state.insert(state.end(), 5, 22);
        state.insert(state.end(), 44, 3);
        state.insert(state.end(), 3, 2);
        if (state.size() != parent.size()) {
            std::cerr << "STRICT_ALTERNATION_INTERNAL_ERROR support\n";
            return 1;
        }
        int state_prefix = 0;
        int parent_prefix = 0;
        int maximum_slack = 0;
        for (std::size_t index = 0; index < state.size(); ++index) {
            state_prefix += state[index];
            parent_prefix += parent[index];
            maximum_slack = std::max(maximum_slack,
                                     parent_prefix - state_prefix);
            if (state_prefix > parent_prefix ||
                (index + 1 < state.size() && state_prefix == parent_prefix)) {
                std::cerr << "STRICT_ALTERNATION_INTERNAL_ERROR dominance rank="
                          << index + 1 << '\n';
                return 1;
            }
        }
        if (maximum_slack != 61) {
            std::cerr << "STRICT_ALTERNATION_INTERNAL_ERROR maximum_slack="
                      << maximum_slack << '\n';
            return 1;
        }
        Sequence left_prefix{0};
        Sequence right_prefix{0};
        for (std::size_t index = 0; index < state.size(); ++index) {
            Sequence &color_prefix =
                index % 2 == 0 ? left_prefix : right_prefix;
            color_prefix.push_back(color_prefix.back() + state[index]);
        }
        const int p = 9;
        const int q = 3;
        const int lhs = left_prefix[p] + right_prefix[q];
        const int rhs = child.H(p) + child.H(p + q) + child.H(q);
        if (lhs <= rhs) {
            std::cerr << "STRICT_ALTERNATION_INTERNAL_ERROR no Hall failure\n";
            return 1;
        }
        std::cout << "STRICT_EXACT_ALTERNATION_COUNTEREXAMPLE k=" << k
                  << " state=" << show(state)
                  << " support=" << state.size()
                  << " internal_tight_prefixes=0"
                  << " maximum_prefix_slack=" << maximum_slack
                  << " p=" << p << " q=" << q
                  << " lhs=" << lhs << " rhs=" << rhs
                  << " excess=" << lhs - rhs << '\n';
        return 0;
    }

    if (argc == 3 && std::string(argv[2]) == "tight-state-count") {
        const int k = std::atoi(argv[1]);
        if (k < 1 || k > 4) {
            std::cerr << "invalid tight-state-count request\n";
            return 2;
        }
        const Sequence parent_values = singleton_base(k);
        const Capacity parent(parent_values);
        std::uint64_t states = 0;
        std::uint64_t with_internal_tight = 0;
        std::vector<std::uint64_t> first_tight(parent_values.size(), 0);
        Sequence state;
        const auto started = std::chrono::steady_clock::now();
        const auto enumerate = [&](auto &&self, int remaining, int maximum,
                                   int first_tight_rank) -> void {
            if (remaining == 0) {
                ++states;
                if (first_tight_rank > 0) {
                    ++with_internal_tight;
                    ++first_tight[first_tight_rank];
                }
                return;
            }
            const int used = parent.mass - remaining;
            for (int value = std::min(maximum, remaining); value >= 1; --value) {
                const int rows = static_cast<int>(state.size()) + 1;
                const int next_used = used + value;
                if (next_used > parent.H(rows)) continue;
                int next_first = first_tight_rank;
                if (next_first == 0 && rows < static_cast<int>(parent_values.size()) &&
                    next_used == parent.H(rows))
                    next_first = rows;
                state.push_back(value);
                self(self, remaining - value, value, next_first);
                state.pop_back();
            }
        };
        enumerate(enumerate, parent.mass, parent.largest, 0);
        if ((k == 2 && (states != 15 || with_internal_tight != 5)) ||
            (k == 3 && (states != 1206 || with_internal_tight != 294)) ||
            (k == 4 &&
             (states != 5997038 || with_internal_tight != 1000432))) {
            std::cerr << "PASCAL_TIGHT_STATE_COUNT_REGRESSION k=" << k
                      << " states=" << states
                      << " with_internal_tight=" << with_internal_tight << '\n';
            return 1;
        }
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        std::cout << "PASCAL_TIGHT_STATE_COUNT k=" << k
                  << " states=" << states
                  << " with_internal_tight=" << with_internal_tight
                  << " without_internal_tight="
                  << states - with_internal_tight
                  << " seconds=" << seconds << '\n';
        for (std::size_t rank = 1; rank < first_tight.size(); ++rank)
            if (first_tight[rank] != 0)
                std::cout << "FIRST_TIGHT rank=" << rank
                          << " states=" << first_tight[rank] << '\n';
        return 0;
    }

    if (argc >= 4 && std::string(argv[2]) == "prefix-mixed-state") {
        const int k = std::atoi(argv[1]);
        if (k < 1 || k > 6 || argc != 3 + (1 << k)) {
            std::cerr << "usage: singleton_pascal_interval_census"
                      << " K prefix-mixed-state ROW...\n";
            return 2;
        }
        Sequence state;
        for (int index = 3; index < argc; ++index)
            state.push_back(std::atoi(argv[index]));
        const Sequence child_values = singleton_base(k - 1);
        const Capacity child(child_values);
        const int half_rows = static_cast<int>(child_values.size());
        SplitSearch search(state, child, child, child,
                           half_rows, half_rows, false,
                           half_rows, half_rows);
        const bool found = search.run();
        std::cout << "PREFIX_MIXED_STATE k=" << k
                  << " state=" << show(state)
                  << " found=" << (found ? "YES" : "NO")
                  << " nodes=" << search.nodes << '\n';
        if (found) {
            for (std::size_t index = 0; index < state.size(); ++index) {
                const Choice &choice = search.solution[index];
                std::cout << "ROW index=" << index + 1
                          << " value=" << state[index]
                          << " split=(" << choice.left << ','
                          << choice.mixed << ',' << choice.right << ")\n";
            }
        }
        return found ? 0 : 1;
    }

    if (argc >= 4 && std::string(argv[2]) == "balanced-residual-state") {
        const int k = std::atoi(argv[1]);
        if (k < 2 || k > 6) return 2;
        Sequence state;
        for (int index = 3; index < argc; ++index)
            state.push_back(std::atoi(argv[index]));
        Sequence residual_child_values = singleton_base(k - 1);
        const int color_rows = static_cast<int>(residual_child_values.size());
        for (int &value : residual_child_values) --value;
        while (!residual_child_values.empty() && residual_child_values.back() == 0)
            residual_child_values.pop_back();
        const Capacity residual_child(residual_child_values);
        BalancedColorSearch exact(state, residual_child, color_rows);
        BalancedColorSearch greedy(state, residual_child, color_rows);
        BalancedColorSearch lookahead(state, residual_child, color_rows);
        BalancedColorSearch minimum_balance(state, residual_child, color_rows);
        const bool exact_result = exact.run();
        const bool greedy_result = greedy.run_greedy();
        const bool lookahead_result = lookahead.run_lookahead();
        const bool minimum_balance_result =
            minimum_balance.run_at_minimum_balance();
        Sequence alternating_left{0};
        Sequence alternating_right{0};
        for (std::size_t index = 0; index < state.size(); ++index) {
            Sequence &prefix = index % 2 == 0
                                   ? alternating_left : alternating_right;
            prefix.push_back(prefix.back() + state[index]);
        }
        bool alternating_result = true;
        for (int p = 0; p < static_cast<int>(alternating_left.size()); ++p)
            for (int q = 0; q < static_cast<int>(alternating_right.size()); ++q)
                if (alternating_left[p] + alternating_right[q] >
                    residual_child.H(p) + residual_child.H(p + q) +
                        residual_child.H(q))
                    alternating_result = false;
        std::cout << "BALANCED_RESIDUAL_STATE k=" << k
                  << " state=" << show(state)
                  << " exact=" << (exact_result ? "YES" : "NO")
                  << " greedy=" << (greedy_result ? "YES" : "NO")
                  << " lookahead=" << (lookahead_result ? "YES" : "NO")
                  << " minimum_balance="
                  << (minimum_balance_result ? "YES" : "NO")
                  << " minimum_difference="
                  << minimum_balance.required_difference
                  << " alternating=" << (alternating_result ? "YES" : "NO")
                  << " nodes=" << exact.nodes << '\n';
        if (exact_result) {
            Sequence left;
            Sequence right;
            for (std::size_t block = 0; block < exact.blocks.size(); ++block) {
                const int to_left = exact.solution[block];
                left.insert(left.end(), to_left, exact.blocks[block].value);
                right.insert(right.end(),
                             exact.blocks[block].count - to_left,
                             exact.blocks[block].value);
            }
            std::cout << "BALANCED_RESIDUAL_SOLUTION left=" << show(left)
                      << " right=" << show(right) << '\n';
        }
        return exact_result ? 0 : 1;
    }

    if (argc == 6 && std::string(argv[2]) == "prefix-mixed-walk") {
        const int k = std::atoi(argv[1]);
        const int samples = std::atoi(argv[3]);
        const int maximum_steps = std::atoi(argv[4]);
        const std::uint64_t seed = std::strtoull(argv[5], nullptr, 10);
        if (k < 2 || k > 6 || samples < 1 || maximum_steps < 1) {
            std::cerr << "invalid prefix-mixed-walk request\n";
            return 2;
        }
        const Sequence child_values = singleton_base(k - 1);
        const Capacity child(child_values);
        const int half_rows = static_cast<int>(child_values.size());
        std::mt19937_64 random(seed);
        std::uint64_t nodes = 0;
        std::uint64_t max_nodes = 0;
        Sequence worst;
        for (int sample = 0; sample < samples; ++sample) {
            Sequence state = singleton_base(k);
            const int steps = 1 + static_cast<int>(random() % maximum_steps);
            for (int step = 0; step < steps; ++step) {
                std::vector<std::pair<int, int>> transfers;
                for (int donor = 0; donor < static_cast<int>(state.size()); ++donor)
                    for (int recipient = donor + 1;
                         recipient < static_cast<int>(state.size()); ++recipient)
                        if (state[donor] >= state[recipient] + 2)
                            transfers.emplace_back(donor, recipient);
                if (transfers.empty()) break;
                const auto [donor, recipient] =
                    transfers[random() % transfers.size()];
                --state[donor];
                ++state[recipient];
                std::sort(state.begin(), state.end(), std::greater<int>());
            }
            SplitSearch search(state, child, child, child,
                               half_rows, half_rows, false,
                               half_rows, half_rows);
            const bool found = search.run();
            nodes += search.nodes;
            if (search.nodes > max_nodes) {
                max_nodes = search.nodes;
                worst = state;
            }
            if (!found) {
                std::cout << "PREFIX_MIXED_WALK_FAILURE k=" << k
                          << " sample=" << sample
                          << " state=" << show(state)
                          << " nodes=" << search.nodes
                          << " seed=" << seed << '\n';
                return 1;
            }
        }
        std::cout << "PREFIX_MIXED_WALK_PASS k=" << k
                  << " samples=" << samples
                  << " maximum_steps=" << maximum_steps
                  << " seed=" << seed
                  << " nodes=" << nodes
                  << " max_nodes=" << max_nodes
                  << " worst_state=" << show(worst) << '\n';
        return 0;
    }

    if (argc == 6 && std::string(argv[2]) == "two-anchor-balanced-walk") {
        const int k = std::atoi(argv[1]);
        const int samples = std::atoi(argv[3]);
        const int maximum_steps = std::atoi(argv[4]);
        const std::uint64_t seed = std::strtoull(argv[5], nullptr, 10);
        if (k < 2 || k > 7 || samples < 1 || maximum_steps < 1) {
            std::cerr << "invalid two-anchor-balanced-walk request\n";
            return 2;
        }
        const Sequence parent = singleton_base(k);
        Sequence residual_child_values = singleton_base(k - 1);
        const int color_rows = static_cast<int>(residual_child_values.size());
        for (int &value : residual_child_values) --value;
        while (!residual_child_values.empty() && residual_child_values.back() == 0)
            residual_child_values.pop_back();
        const Capacity residual_child(residual_child_values);
        std::mt19937_64 random(seed);
        std::uint64_t greedy_failures = 0;
        std::uint64_t lookahead_failures = 0;
        std::uint64_t exact_nodes = 0;
        std::uint64_t maximum_exact_nodes = 0;
        for (int sample = 0; sample < samples; ++sample) {
            Sequence state = parent;
            const int steps = 1 + static_cast<int>(random() % maximum_steps);
            for (int step = 0; step < steps; ++step) {
                std::vector<std::pair<int, int>> transfers;
                for (int donor = 0; donor < static_cast<int>(state.size()); ++donor)
                    for (int recipient = donor + 1;
                         recipient < static_cast<int>(state.size()); ++recipient)
                        if (state[donor] >= state[recipient] + 2)
                            transfers.emplace_back(donor, recipient);
                if (transfers.empty()) break;
                const auto [donor, recipient] =
                    transfers[random() % transfers.size()];
                --state[donor];
                ++state[recipient];
                std::sort(state.begin(), state.end(), std::greater<int>());
            }
            Sequence residual;
            for (int row = 0; row < static_cast<int>(state.size()); ++row) {
                const int value = state[row] - (row < color_rows ? 2 : 1);
                if (value > 0) residual.push_back(value);
            }
            std::sort(residual.begin(), residual.end(), std::greater<int>());
            BalancedColorSearch greedy(residual, residual_child, color_rows);
            BalancedColorSearch lookahead(residual, residual_child, color_rows);
            BalancedColorSearch exact(residual, residual_child, color_rows);
            if (!greedy.run_greedy()) ++greedy_failures;
            if (!lookahead.run_lookahead()) ++lookahead_failures;
            if (!exact.run()) {
                std::cout << "TWO_ANCHOR_BALANCED_WALK_FAILURE k=" << k
                          << " sample=" << sample
                          << " state=" << show(state)
                          << " residual=" << show(residual)
                          << " seed=" << seed << '\n';
                return 1;
            }
            exact_nodes += exact.nodes;
            maximum_exact_nodes = std::max(maximum_exact_nodes, exact.nodes);
        }
        std::cout << "TWO_ANCHOR_BALANCED_WALK_PASS k=" << k
                  << " samples=" << samples
                  << " maximum_steps=" << maximum_steps
                  << " seed=" << seed
                  << " greedy_failures=" << greedy_failures
                  << " lookahead_failures=" << lookahead_failures
                  << " exact_nodes=" << exact_nodes
                  << " maximum_exact_nodes=" << maximum_exact_nodes << '\n';
        return 0;
    }

    if (argc == 3 &&
        (std::string(argv[2]) == "canonical-pure-full-band" ||
         std::string(argv[2]) == "exact-children-full-band" ||
         std::string(argv[2]) == "prefix-mixed-full-band" ||
         std::string(argv[2]) == "cross-alternating-full-band" ||
         std::string(argv[2]) == "cross-threshold-full-band")) {
        const bool canonical_pure =
            std::string(argv[2]) == "canonical-pure-full-band";
        const bool prefix_mixed =
            std::string(argv[2]) == "prefix-mixed-full-band" ||
            std::string(argv[2]) == "cross-alternating-full-band" ||
            std::string(argv[2]) == "cross-threshold-full-band";
        const bool cross_alternating =
            std::string(argv[2]) == "cross-alternating-full-band";
        const bool cross_threshold =
            std::string(argv[2]) == "cross-threshold-full-band";
        const int k = std::atoi(argv[1]);
        if (k < 1 || k > 4) {
            std::cerr << "invalid full-band refinement request\n";
            return 2;
        }
        const Sequence child_values = singleton_base(k - 1);
        const Sequence parent_values = singleton_base(k);
        const Capacity child(child_values);
        const Capacity parent(parent_values);
        std::uint64_t states = 0;
        std::uint64_t failures = 0;
        std::uint64_t nodes = 0;
        std::uint64_t max_nodes = 0;
        Sequence first_failure;
        Sequence worst;
        Sequence state;
        const auto started = std::chrono::steady_clock::now();
        const auto enumerate = [&](auto &&self, int remaining,
                                   int maximum) -> void {
            if (remaining == 0) {
                if (state.size() != parent_values.size()) return;
                ++states;
                const int half_rows = static_cast<int>(child_values.size());
                bool found = false;
                std::uint64_t state_nodes = 0;
                const int first_parameter = cross_threshold ? 0 : -1;
                const int last_parameter = cross_threshold ? half_rows : -1;
                for (int parameter = first_parameter;
                     parameter <= last_parameter && !found; ++parameter) {
                    SplitSearch search(
                        state, child, child, child,
                        canonical_pure ? -1 : half_rows,
                        canonical_pure ? -1 : half_rows,
                        canonical_pure,
                        canonical_pure ? -1 : half_rows,
                        prefix_mixed ? half_rows : -1,
                        cross_alternating ? 1 : (cross_threshold ? 2 : 0));
                    search.orientation_parameter = std::max(parameter, 0);
                    found = search.run();
                    state_nodes += search.nodes;
                }
                nodes += state_nodes;
                if (state_nodes > max_nodes) {
                    max_nodes = state_nodes;
                    worst = state;
                }
                if (!found) {
                    ++failures;
                    if (first_failure.empty()) first_failure = state;
                }
                return;
            }
            if (state.size() >= parent_values.size()) return;
            const int used = parent.mass - remaining;
            for (int value = std::min(maximum, remaining); value >= 1; --value) {
                const int rows = static_cast<int>(state.size()) + 1;
                if (used + value > parent.H(rows)) continue;
                state.push_back(value);
                self(self, remaining - value, value);
                state.pop_back();
            }
        };
        enumerate(enumerate, parent.mass, parent.largest);
        const std::uint64_t expected_failures =
            canonical_pure && k == 3
                ? 1
                : (cross_alternating && k == 3
                       ? 6
                       : (cross_threshold && k == 3 ? 9 : 0));
        const bool check_failures =
            k <= 3 ||
            (k == 4 && !canonical_pure && !cross_alternating &&
             !cross_threshold);
        if ((k == 3 && states != 160) ||
            (k == 4 && states != 408776) ||
            (check_failures && failures != expected_failures)) {
            std::cerr << "FULL_BAND_REFINEMENT_REGRESSION mode=" << argv[2]
                      << " k=" << k << " states=" << states
                      << " failures=" << failures
                      << " expected_failures=" << expected_failures << '\n';
            return 1;
        }
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        std::cout << (canonical_pure
                          ? "CANONICAL_PURE_FULL_BAND"
                          : (cross_alternating
                                 ? "CROSS_ALTERNATING_FULL_BAND"
                                 : (cross_threshold
                                        ? "CROSS_THRESHOLD_FULL_BAND"
                                        : (prefix_mixed
                                               ? "PREFIX_MIXED_FULL_BAND"
                                               : "EXACT_CHILDREN_FULL_BAND"))))
                  << " k=" << k
                  << " complete=YES states=" << states
                  << " failures=" << failures
                  << " nodes=" << nodes
                  << " max_nodes=" << max_nodes
                  << " worst_state=" << show(worst);
        if (!first_failure.empty())
            std::cout << " first_failure=" << show(first_failure);
        std::cout << " seconds=" << seconds << '\n';
        return failures == 0 ? 0 : 1;
    }

    if (argc == 3 && std::string(argv[2]) == "binary-duplicate-census") {
        const int k = std::atoi(argv[1]);
        if (k < 1 || k > 4) {
            std::cerr << "invalid binary-duplicate-census request\n";
            return 2;
        }
        const Sequence child = singleton_base(k - 1);
        Sequence duplicate;
        duplicate.reserve(2 * child.size());
        for (int value : child) {
            duplicate.push_back(value);
            duplicate.push_back(value);
        }
        Census census(child, Sequence{}, child, duplicate, 0, 0,
                      static_cast<int>(duplicate.size()));
        census.run();
        return 0;
    }

    if (argc == 3 && std::string(argv[2]) == "anchored-residual-census") {
        const int k = std::atoi(argv[1]);
        if (k < 2 || k > 4) {
            std::cerr << "invalid anchored-residual-census request\n";
            return 2;
        }
        Sequence residual_child = singleton_base(k - 1);
        for (int &value : residual_child) --value;
        while (!residual_child.empty() && residual_child.back() == 0)
            residual_child.pop_back();
        const Capacity capacity(residual_child);
        const Sequence residual_parent =
            induced_parent(capacity, capacity, capacity);
        Census census(residual_child, residual_child, residual_child,
                      residual_parent, 0, 0, 0);
        census.run();
        return 0;
    }

    if (argc == 3 &&
        std::string(argv[2]) == "anchored-residual-balanced-coloring") {
        const int k = std::atoi(argv[1]);
        if (k < 2 || k > 4) {
            std::cerr << "invalid anchored-residual-balanced-coloring request\n";
            return 2;
        }
        Sequence residual_child_values = singleton_base(k - 1);
        const int color_rows = static_cast<int>(residual_child_values.size());
        for (int &value : residual_child_values) --value;
        while (!residual_child_values.empty() && residual_child_values.back() == 0)
            residual_child_values.pop_back();
        const Capacity residual_child(residual_child_values);
        const Sequence residual_parent_values =
            induced_parent(residual_child, residual_child, residual_child);
        const Capacity residual_parent(residual_parent_values);
        std::uint64_t states = 0;
        std::uint64_t failures = 0;
        std::uint64_t greedy_failures = 0;
        std::uint64_t lookahead_failures = 0;
        std::uint64_t minimum_balance_failures = 0;
        std::uint64_t nodes = 0;
        std::uint64_t max_nodes = 0;
        Sequence first_failure;
        Sequence first_greedy_failure;
        Sequence first_minimum_balance_failure;
        Sequence worst;
        Sequence state;
        const auto inspect = [&]() {
            ++states;
            BalancedColorSearch search(state, residual_child, color_rows);
            BalancedColorSearch greedy_search(state, residual_child, color_rows);
            BalancedColorSearch lookahead_search(state, residual_child, color_rows);
            BalancedColorSearch minimum_balance_search(
                state, residual_child, color_rows);
            if (!greedy_search.run_greedy()) {
                ++greedy_failures;
                if (first_greedy_failure.empty()) first_greedy_failure = state;
            }
            if (!lookahead_search.run_lookahead()) ++lookahead_failures;
            if (!minimum_balance_search.run_at_minimum_balance()) {
                ++minimum_balance_failures;
                if (first_minimum_balance_failure.empty())
                    first_minimum_balance_failure = state;
            }
            const bool found = search.run();
            nodes += search.nodes;
            if (search.nodes > max_nodes) {
                max_nodes = search.nodes;
                worst = state;
            }
            if (!found) {
                ++failures;
                if (first_failure.empty()) first_failure = state;
            }
        };
        const auto enumerate = [&](auto &&self, int remaining,
                                   int maximum) -> void {
            if (remaining == 0) {
                inspect();
                return;
            }
            if (static_cast<int>(state.size()) >= 2 * color_rows) return;
            const int used = residual_parent.mass - remaining;
            for (int value = std::min(maximum, remaining); value >= 1; --value) {
                const int rows = static_cast<int>(state.size()) + 1;
                if (used + value > residual_parent.H(rows)) continue;
                state.push_back(value);
                self(self, remaining - value, value);
                state.pop_back();
            }
        };
        enumerate(enumerate, residual_parent.mass, residual_parent.largest);
        if ((k == 3 &&
             (states != 73 || failures != 0 || greedy_failures != 0 ||
              lookahead_failures != 0 || minimum_balance_failures != 0)) ||
            (k == 4 &&
             (states != 160492 || failures != 0 || greedy_failures != 403 ||
              lookahead_failures != 0 ||
              minimum_balance_failures != 1067))) {
            std::cerr << "ANCHORED_RESIDUAL_BALANCED_REGRESSION k=" << k
                      << " states=" << states << " failures=" << failures
                      << " greedy_failures=" << greedy_failures
                      << " lookahead_failures=" << lookahead_failures
                      << " minimum_balance_failures="
                      << minimum_balance_failures << '\n';
            return 1;
        }
        std::cout << "ANCHORED_RESIDUAL_BALANCED_COLORING k=" << k
                  << " complete=YES states=" << states
                  << " failures=" << failures
                  << " greedy_failures=" << greedy_failures
                  << " lookahead_failures=" << lookahead_failures
                  << " minimum_balance_failures="
                  << minimum_balance_failures
                  << " nodes=" << nodes
                  << " max_nodes=" << max_nodes
                  << " worst_state=" << show(worst);
        if (!first_failure.empty())
            std::cout << " first_failure=" << show(first_failure);
        if (!first_greedy_failure.empty())
            std::cout << " first_greedy_failure="
                      << show(first_greedy_failure);
        if (!first_minimum_balance_failure.empty())
            std::cout << " first_minimum_balance_failure="
                      << show(first_minimum_balance_failure);
        std::cout << '\n';
        return failures == 0 ? 0 : 1;
    }

    if (argc == 3 &&
        std::string(argv[2]) ==
            "anchored-residual-same-color-predecessor") {
        const int k = std::atoi(argv[1]);
        if (k < 2 || k > 4) {
            std::cerr << "invalid anchored-residual-same-color-predecessor"
                      << " request\n";
            return 2;
        }
        Sequence residual_child_values = singleton_base(k - 1);
        const int color_rows = static_cast<int>(residual_child_values.size());
        for (int &value : residual_child_values) --value;
        while (!residual_child_values.empty() &&
               residual_child_values.back() == 0)
            residual_child_values.pop_back();
        const Capacity residual_child(residual_child_values);
        const Sequence residual_parent_values =
            induced_parent(residual_child, residual_child, residual_child);
        const Capacity residual_parent(residual_parent_values);
        std::uint64_t states = 0;
        std::uint64_t noncanonical_states = 0;
        std::uint64_t failures = 0;
        std::uint64_t no_predecessor = 0;
        std::uint64_t candidates = 0;
        std::uint64_t first_candidate_failures = 0;
        std::uint64_t equal_first_candidate_failures = 0;
        std::uint64_t distinct_repairs_after_first_failure = 0;
        std::uint64_t max_candidates_per_state = 0;
        std::uint64_t nodes = 0;
        std::uint64_t max_nodes = 0;
        Sequence first_failure;
        Sequence first_first_candidate_failure;
        Sequence first_distinct_candidate_failure;
        std::pair<int, int> first_failed_values{0, 0};
        std::pair<int, int> first_repair_values{0, 0};
        Sequence state;
        const auto dominated = [&](Sequence values) {
            std::sort(values.begin(), values.end(), std::greater<int>());
            while (!values.empty() && values.back() == 0) values.pop_back();
            int prefix = 0;
            for (int row = 0; row < static_cast<int>(values.size()); ++row) {
                prefix += values[row];
                if (prefix > residual_parent.H(row + 1)) return false;
            }
            return prefix == residual_parent.mass;
        };
        const auto inspect = [&]() {
            ++states;
            if (state == residual_parent_values) return;
            ++noncanonical_states;
            Sequence target = state;
            target.resize(2 * color_rows, 0);
            bool has_predecessor = false;
            bool found = false;
            bool first_candidate_failed = false;
            bool first_failed_values_equal = false;
            std::uint64_t state_candidates = 0;
            std::set<std::pair<int, int>> value_pairs;
            for (int donor = 0;
                 donor < static_cast<int>(state.size()) && !found; ++donor) {
                for (int recipient = donor + 1;
                     recipient < static_cast<int>(state.size()) && !found;
                     ++recipient) {
                    if (!value_pairs.insert(
                            {state[donor], state[recipient]}).second)
                        continue;
                    Sequence source = target;
                    ++source[donor];
                    --source[recipient];
                    if (!dominated(source)) continue;
                    has_predecessor = true;
                    ++candidates;
                    ++state_candidates;
                    CommonSameColorSearch search(
                        target, donor, recipient, residual_child, color_rows);
                    found = search.run();
                    nodes += search.nodes;
                    max_nodes = std::max(max_nodes, search.nodes);
                    if (state_candidates == 1 && !found) {
                        first_candidate_failed = true;
                        first_failed_values_equal =
                            state[donor] == state[recipient];
                        if (first_first_candidate_failure.empty()) {
                            first_first_candidate_failure = state;
                            first_failed_values =
                                {state[donor], state[recipient]};
                        }
                        if (!first_failed_values_equal &&
                            first_distinct_candidate_failure.empty())
                            first_distinct_candidate_failure = state;
                    }
                    if (found && first_candidate_failed &&
                        first_first_candidate_failure == state)
                        first_repair_values =
                            {state[donor], state[recipient]};
                    if (found && first_candidate_failed &&
                        state[donor] != state[recipient])
                        ++distinct_repairs_after_first_failure;
                }
            }
            max_candidates_per_state =
                std::max(max_candidates_per_state, state_candidates);
            if (first_candidate_failed) ++first_candidate_failures;
            if (first_candidate_failed && first_failed_values_equal)
                ++equal_first_candidate_failures;
            if (!has_predecessor) ++no_predecessor;
            if (!found) {
                ++failures;
                if (first_failure.empty()) first_failure = state;
            }
        };
        const auto enumerate = [&](auto &&self, int remaining,
                                   int maximum) -> void {
            if (remaining == 0) {
                inspect();
                return;
            }
            if (static_cast<int>(state.size()) >= 2 * color_rows) return;
            const int used = residual_parent.mass - remaining;
            for (int value = std::min(maximum, remaining); value >= 1; --value) {
                const int rows = static_cast<int>(state.size()) + 1;
                if (used + value > residual_parent.H(rows)) continue;
                state.push_back(value);
                self(self, remaining - value, value);
                state.pop_back();
            }
        };
        enumerate(enumerate, residual_parent.mass, residual_parent.largest);
        if ((k == 3 &&
             (states != 73 || noncanonical_states != 72 || failures != 0 ||
              no_predecessor != 0 || candidates != 74 ||
              first_candidate_failures != 2 ||
              equal_first_candidate_failures != 1 ||
              distinct_repairs_after_first_failure != 2 ||
              max_candidates_per_state != 2)) ||
            (k == 4 &&
             (states != 160492 || noncanonical_states != 160491 ||
              failures != 0 || no_predecessor != 0 ||
              candidates != 160568 || first_candidate_failures != 77 ||
              equal_first_candidate_failures != 61 ||
              distinct_repairs_after_first_failure != 77 ||
              max_candidates_per_state != 2))) {
            std::cerr << "ANCHORED_RESIDUAL_PREDECESSOR_REGRESSION k=" << k
                      << " states=" << states
                      << " noncanonical_states=" << noncanonical_states
                      << " failures=" << failures
                      << " no_predecessor=" << no_predecessor
                      << " candidates=" << candidates << '\n';
            return 1;
        }
        std::cout << "ANCHORED_RESIDUAL_SAME_COLOR_PREDECESSOR k=" << k
                  << " complete=YES states=" << states
                  << " noncanonical_states=" << noncanonical_states
                  << " failures=" << failures
                  << " no_predecessor=" << no_predecessor
                  << " candidates=" << candidates
                  << " first_candidate_failures="
                  << first_candidate_failures
                  << " equal_first_candidate_failures="
                  << equal_first_candidate_failures
                  << " distinct_repairs_after_first_failure="
                  << distinct_repairs_after_first_failure
                  << " max_candidates_per_state="
                  << max_candidates_per_state
                  << " nodes=" << nodes
                  << " max_nodes=" << max_nodes;
        if (!first_failure.empty())
            std::cout << " first_failure=" << show(first_failure);
        if (!first_first_candidate_failure.empty())
            std::cout << " first_first_candidate_failure="
                      << show(first_first_candidate_failure)
                      << " failed_values=(" << first_failed_values.first
                      << ',' << first_failed_values.second << ')'
                      << " repair_values=(" << first_repair_values.first
                      << ',' << first_repair_values.second << ')';
        if (!first_distinct_candidate_failure.empty())
            std::cout << " first_distinct_candidate_failure="
                      << show(first_distinct_candidate_failure);
        std::cout << '\n';
        return failures == 0 ? 0 : 1;
    }

    if (argc == 3 &&
        std::string(argv[2]) == "anchored-residual-boundary-product") {
        const int k = std::atoi(argv[1]);
        if (k < 2 || k > 4) {
            std::cerr << "invalid anchored-residual-boundary-product request\n";
            return 2;
        }
        Sequence residual_child_values = singleton_base(k - 1);
        const int color_rows = static_cast<int>(residual_child_values.size());
        for (int &value : residual_child_values) --value;
        while (!residual_child_values.empty() &&
               residual_child_values.back() == 0)
            residual_child_values.pop_back();
        const Capacity residual_child(residual_child_values);
        const Sequence residual_parent_values =
            induced_parent(residual_child, residual_child, residual_child);
        const Capacity residual_parent(residual_parent_values);
        const auto boundaries = boundary_colorings(
            residual_parent_values, residual_child, color_rows);
        std::uint64_t states = 0;
        std::uint64_t failures = 0;
        std::uint64_t searches = 0;
        std::uint64_t nodes = 0;
        std::uint64_t max_nodes = 0;
        Sequence first_failure;
        Sequence state;
        const auto inspect = [&]() {
            ++states;
            bool found = false;
            for (const auto &[left, right] : boundaries) {
                ++searches;
                MajorizationProductSearch search(
                    state, left, right, color_rows);
                found = search.run();
                nodes += search.nodes;
                max_nodes = std::max(max_nodes, search.nodes);
                if (found) break;
            }
            if (!found) {
                ++failures;
                if (first_failure.empty()) first_failure = state;
            }
        };
        const auto enumerate = [&](auto &&self, int remaining,
                                   int maximum) -> void {
            if (remaining == 0) {
                inspect();
                return;
            }
            if (static_cast<int>(state.size()) >= 2 * color_rows) return;
            const int used = residual_parent.mass - remaining;
            for (int value = std::min(maximum, remaining); value >= 1; --value) {
                const int rows = static_cast<int>(state.size()) + 1;
                if (used + value > residual_parent.H(rows)) continue;
                state.push_back(value);
                self(self, remaining - value, value);
                state.pop_back();
            }
        };
        enumerate(enumerate, residual_parent.mass, residual_parent.largest);
        if (k == 3 &&
            (states != 73 || boundaries.size() != 1 || failures != 4 ||
             first_failure != Sequence{6, 3, 3, 3})) {
            std::cerr << "ANCHORED_RESIDUAL_PRODUCT_REGRESSION k=" << k
                      << " states=" << states
                      << " boundary_colorings=" << boundaries.size()
                      << " failures=" << failures
                      << " first_failure=" << show(first_failure) << '\n';
            return 1;
        }
        std::cout << "ANCHORED_RESIDUAL_BOUNDARY_PRODUCT k=" << k
                  << " complete=YES states=" << states
                  << " boundary_colorings=" << boundaries.size()
                  << " failures=" << failures
                  << " searches=" << searches
                  << " nodes=" << nodes
                  << " max_nodes=" << max_nodes;
        if (!first_failure.empty())
            std::cout << " first_failure=" << show(first_failure);
        std::cout << '\n';
        for (std::size_t index = 0; index < boundaries.size(); ++index)
            std::cout << "BOUNDARY_COLORING index=" << index + 1
                      << " left=" << show(boundaries[index].first)
                      << " right=" << show(boundaries[index].second)
                      << '\n';
        return failures == 0 ? 0 : 1;
    }

    if (argc == 3 && std::string(argv[2]) == "two-anchor-coalescence") {
        const int k = std::atoi(argv[1]);
        if (k < 2 || k > 4) {
            std::cerr << "invalid two-anchor-coalescence request\n";
            return 2;
        }
        const Sequence parent_values = singleton_base(k);
        const Capacity parent(parent_values);
        Sequence residual_child = singleton_base(k - 1);
        for (int &value : residual_child) --value;
        while (!residual_child.empty() && residual_child.back() == 0)
            residual_child.pop_back();
        const Capacity residual_capacity(residual_child);
        const Sequence residual_parent_values =
            induced_parent(residual_capacity, residual_capacity,
                           residual_capacity);
        const Capacity residual_parent(residual_parent_values);
        std::uint64_t states = 0;
        std::uint64_t failures = 0;
        Sequence first_failure;
        Sequence state;
        const int half = static_cast<int>(parent_values.size()) / 2;
        const auto dominated = [](const Sequence &values,
                                  const Capacity &capacity) {
            int prefix = 0;
            for (std::size_t index = 0; index < values.size(); ++index) {
                prefix += values[index];
                if (prefix > capacity.H(static_cast<int>(index) + 1))
                    return false;
            }
            return prefix == capacity.mass;
        };
        const auto inspect = [&]() {
            ++states;
            Sequence residual;
            for (int index = 0; index < static_cast<int>(state.size()); ++index) {
                const int value = state[index] - (index < half ? 2 : 1);
                if (value > 0) residual.push_back(value);
            }
            std::sort(residual.begin(), residual.end(), std::greater<int>());
            if (!dominated(residual, residual_parent)) {
                std::cerr << "TWO_ANCHOR_INTERNAL_DOMINANCE_FAILURE state="
                          << show(state) << " residual=" << show(residual) << '\n';
                std::exit(1);
            }
            while (residual.size() > residual_parent_values.size()) {
                const int merged = residual.back() + residual[residual.size() - 2];
                residual.pop_back();
                residual.pop_back();
                residual.push_back(merged);
                std::sort(residual.begin(), residual.end(), std::greater<int>());
                if (!dominated(residual, residual_parent)) {
                    ++failures;
                    if (first_failure.empty()) first_failure = state;
                    return;
                }
            }
        };
        const auto enumerate = [&](auto &&self, int remaining,
                                   int maximum) -> void {
            if (remaining == 0) {
                if (state.size() == parent_values.size()) inspect();
                return;
            }
            if (state.size() >= parent_values.size()) return;
            const int used = parent.mass - remaining;
            for (int value = std::min(maximum, remaining); value >= 1; --value) {
                const int rows = static_cast<int>(state.size()) + 1;
                if (used + value > parent.H(rows)) continue;
                state.push_back(value);
                self(self, remaining - value, value);
                state.pop_back();
            }
        };
        enumerate(enumerate, parent.mass, parent.largest);
        if ((k == 3 && (states != 160 || failures != 0)) ||
            (k == 4 && (states != 408776 || failures != 9804))) {
            std::cerr << "TWO_ANCHOR_COALESCENCE_REGRESSION k=" << k
                      << " states=" << states
                      << " failures=" << failures << '\n';
            return 1;
        }
        std::cout << "TWO_ANCHOR_COALESCENCE k=" << k
                  << " complete=YES states=" << states
                  << " failures=" << failures;
        if (!first_failure.empty())
            std::cout << " first_failure=" << show(first_failure);
        std::cout << '\n';
        return failures == 0 ? 0 : 1;
    }

    if (argc == 3 && std::string(argv[2]) == "two-anchor-balanced-coloring") {
        const int k = std::atoi(argv[1]);
        if (k < 2 || k > 4) {
            std::cerr << "invalid two-anchor-balanced-coloring request\n";
            return 2;
        }
        const Sequence parent_values = singleton_base(k);
        const Capacity parent(parent_values);
        Sequence residual_child_values = singleton_base(k - 1);
        for (int &value : residual_child_values) --value;
        while (!residual_child_values.empty() && residual_child_values.back() == 0)
            residual_child_values.pop_back();
        const Capacity residual_child(residual_child_values);
        std::uint64_t states = 0;
        std::uint64_t failures = 0;
        std::uint64_t greedy_failures = 0;
        std::uint64_t lookahead_failures = 0;
        std::uint64_t minimum_balance_failures = 0;
        std::uint64_t nodes = 0;
        std::uint64_t max_nodes = 0;
        Sequence first_failure;
        Sequence first_greedy_failure;
        Sequence first_minimum_balance_failure;
        Sequence worst;
        Sequence state;
        const int half = static_cast<int>(parent_values.size()) / 2;
        const auto inspect = [&]() {
            ++states;
            Sequence residual;
            for (int index = 0; index < static_cast<int>(state.size()); ++index) {
                const int value = state[index] - (index < half ? 2 : 1);
                if (value > 0) residual.push_back(value);
            }
            std::sort(residual.begin(), residual.end(), std::greater<int>());
            BalancedColorSearch search(residual, residual_child, half);
            BalancedColorSearch greedy_search(residual, residual_child, half);
            BalancedColorSearch lookahead_search(residual, residual_child, half);
            BalancedColorSearch minimum_balance_search(
                residual, residual_child, half);
            if (!greedy_search.run_greedy()) {
                ++greedy_failures;
                if (first_greedy_failure.empty()) first_greedy_failure = state;
            }
            if (!lookahead_search.run_lookahead()) ++lookahead_failures;
            if (!minimum_balance_search.run_at_minimum_balance()) {
                ++minimum_balance_failures;
                if (first_minimum_balance_failure.empty())
                    first_minimum_balance_failure = state;
            }
            const bool found = search.run();
            nodes += search.nodes;
            if (search.nodes > max_nodes) {
                max_nodes = search.nodes;
                worst = state;
            }
            if (!found) {
                ++failures;
                if (first_failure.empty()) first_failure = state;
            }
        };
        const auto enumerate = [&](auto &&self, int remaining,
                                   int maximum) -> void {
            if (remaining == 0) {
                if (state.size() == parent_values.size()) inspect();
                return;
            }
            if (state.size() >= parent_values.size()) return;
            const int used = parent.mass - remaining;
            for (int value = std::min(maximum, remaining); value >= 1; --value) {
                const int rows = static_cast<int>(state.size()) + 1;
                if (used + value > parent.H(rows)) continue;
                state.push_back(value);
                self(self, remaining - value, value);
                state.pop_back();
            }
        };
        enumerate(enumerate, parent.mass, parent.largest);
        if ((k == 3 &&
             (states != 160 || failures != 0 || greedy_failures != 0 ||
              lookahead_failures != 0 || minimum_balance_failures != 0)) ||
            (k == 4 &&
             (states != 408776 || failures != 0 || greedy_failures != 515 ||
              lookahead_failures != 0 ||
              minimum_balance_failures != 2499))) {
            std::cerr << "TWO_ANCHOR_BALANCED_REGRESSION k=" << k
                      << " states=" << states << " failures=" << failures
                      << " greedy_failures=" << greedy_failures
                      << " lookahead_failures=" << lookahead_failures
                      << " minimum_balance_failures="
                      << minimum_balance_failures << '\n';
            return 1;
        }
        std::cout << "TWO_ANCHOR_BALANCED_COLORING k=" << k
                  << " complete=YES states=" << states
                  << " failures=" << failures
                  << " greedy_failures=" << greedy_failures
                  << " lookahead_failures=" << lookahead_failures
                  << " minimum_balance_failures="
                  << minimum_balance_failures
                  << " nodes=" << nodes
                  << " max_nodes=" << max_nodes
                  << " worst_state=" << show(worst);
        if (!first_failure.empty())
            std::cout << " first_failure=" << show(first_failure);
        if (!first_greedy_failure.empty())
            std::cout << " first_greedy_failure="
                      << show(first_greedy_failure);
        if (!first_minimum_balance_failure.empty())
            std::cout << " first_minimum_balance_failure="
                      << show(first_minimum_balance_failure);
        std::cout << '\n';
        return failures == 0 ? 0 : 1;
    }

    if (argc == 3 && std::string(argv[2]) == "all-tail-extensions") {
        const int k = std::atoi(argv[1]);
        if (k < 1 || k > 4) {
            std::cerr << "invalid all-tail-extensions request\n";
            return 2;
        }
        const Sequence child = singleton_base(k - 1);
        const Sequence parent = singleton_base(k);
        const Capacity child_capacity(child);
        std::uint64_t cases = 0;
        std::uint64_t states = 0;
        std::uint64_t nodes = 0;
        std::uint64_t maximum_nodes = 0;
        int worst_begin = -1;
        int worst_count = -1;
        const auto started = std::chrono::steady_clock::now();
        for (int begin = 1; begin < static_cast<int>(parent.size()); ++begin) {
            for (int from : maximizing_left_counts(child_capacity, begin)) {
                Census census(slice(child, from, static_cast<int>(child.size())),
                              slice(child, begin, static_cast<int>(child.size())),
                              slice(child, begin - from,
                                    static_cast<int>(child.size())),
                              slice(parent, begin, static_cast<int>(parent.size())),
                              0, 0, 0, false);
                census.run(false);
                ++cases;
                states += census.tested;
                nodes += census.nodes;
                if (census.max_nodes > maximum_nodes) {
                    maximum_nodes = census.max_nodes;
                    worst_begin = begin;
                    worst_count = from;
                }
            }
        }
        if ((k == 3 && (cases != 13 || states != 443)) ||
            (k == 4 && (cases != 37 || states != 1422304))) {
            std::cerr << "PASCAL_TAIL_EXTENSION_REGRESSION k=" << k
                      << " cases=" << cases << " states=" << states << '\n';
            return 1;
        }
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        std::cout << "PASCAL_TAIL_EXTENSION_CENSUS k=" << k
                  << " complete=YES cases=" << cases
                  << " states=" << states << " nodes=" << nodes
                  << " max_nodes=" << maximum_nodes
                  << " worst_case=(" << worst_begin << ',' << worst_count << ')'
                  << " seconds=" << seconds << '\n';
        return 0;
    }

    if (argc == 5 && std::string(argv[2]) == "tail-start") {
        const int k = std::atoi(argv[1]);
        const int begin = std::atoi(argv[3]);
        const int from = std::atoi(argv[4]);
        if (k < 1 || k > 4 || begin <= 0 || begin >= (1 << k)) {
            std::cerr << "invalid tail-start request\n";
            return 2;
        }
        const Sequence child = singleton_base(k - 1);
        const Sequence parent = singleton_base(k);
        const Capacity child_capacity(child);
        const std::vector<int> starts =
            maximizing_left_counts(child_capacity, begin);
        if (std::find(starts.begin(), starts.end(), from) == starts.end()) {
            std::cerr << "tail start is not a Pascal maximizer\n";
            return 2;
        }
        Census census(slice(child, from, static_cast<int>(child.size())),
                      slice(child, begin, static_cast<int>(child.size())),
                      slice(child, begin - from,
                            static_cast<int>(child.size())),
                      slice(parent, begin, static_cast<int>(parent.size())),
                      0, 0, 0, false);
        census.run();
        return 0;
    }

    if (argc == 3 && std::string(argv[2]) == "all-left-extension-bands") {
        const int k = std::atoi(argv[1]);
        if (k < 1 || k > 4) {
            std::cerr << "invalid all-left-extension-bands request\n";
            return 2;
        }
        const Sequence child = singleton_base(k - 1);
        const Sequence parent = singleton_base(k);
        std::uint64_t bands = 0;
        std::uint64_t states = 0;
        std::uint64_t nodes = 0;
        std::uint64_t maximum_nodes = 0;
        int worst_begin = -1;
        int worst_end = -1;
        const auto started = std::chrono::steady_clock::now();
        for (int begin = 0; begin < static_cast<int>(parent.size()); ++begin) {
            for (int end = begin + 1; end <= static_cast<int>(parent.size()); ++end) {
                TransitionBandCensus census(child, parent, begin, end, true);
                census.run();
                ++bands;
                states += census.states;
                nodes += census.nodes;
                if (census.max_nodes > maximum_nodes) {
                    maximum_nodes = census.max_nodes;
                    worst_begin = begin;
                    worst_end = end;
                }
            }
        }
        if ((k == 3 && (bands != 36 || states != 561)) ||
            (k == 4 && (bands != 136 || states != 1722516))) {
            std::cerr << "PASCAL_LEFT_EXTENSION_REGRESSION k=" << k
                      << " bands=" << bands << " states=" << states << '\n';
            return 1;
        }
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        std::cout << "PASCAL_LEFT_EXTENSION_BAND_CENSUS k=" << k
                  << " complete=YES bands=" << bands
                  << " states=" << states << " nodes=" << nodes
                  << " max_nodes=" << maximum_nodes
                  << " worst_band=(" << worst_begin << ',' << worst_end << ')'
                  << " seconds=" << seconds << '\n';
        return 0;
    }

    if (argc == 3 && std::string(argv[2]) == "all-transition-bands") {
        const int k = std::atoi(argv[1]);
        if (k < 1 || k > 4) {
            std::cerr << "invalid all-transition-bands request\n";
            return 2;
        }
        const Sequence child = singleton_base(k - 1);
        const Sequence parent = singleton_base(k);
        std::uint64_t bands = 0;
        std::uint64_t states = 0;
        std::uint64_t nodes = 0;
        std::uint64_t maximum_nodes = 0;
        std::uint64_t left_total_failures = 0;
        std::uint64_t right_total_failures = 0;
        std::uint64_t incomplete_relations = 0;
        std::uint64_t strict_states = 0;
        std::uint64_t strict_incomplete_relations = 0;
        std::map<std::string, std::uint64_t> relation_shapes;
        int worst_begin = -1;
        int worst_end = -1;
        const auto started = std::chrono::steady_clock::now();
        for (int begin = 0; begin < static_cast<int>(parent.size()); ++begin) {
            for (int end = begin + 1; end <= static_cast<int>(parent.size()); ++end) {
                TransitionBandCensus census(child, parent, begin, end);
                census.run();
                ++bands;
                states += census.states;
                nodes += census.nodes;
                left_total_failures += census.left_total_failures;
                right_total_failures += census.right_total_failures;
                incomplete_relations += census.incomplete_relations;
                strict_states += census.strict_states;
                strict_incomplete_relations +=
                    census.strict_incomplete_relations;
                for (const auto &[shape, count] : census.relation_shapes)
                    relation_shapes[shape] += count;
                if (census.max_nodes > maximum_nodes) {
                    maximum_nodes = census.max_nodes;
                    worst_begin = begin;
                    worst_end = end;
                }
            }
        }
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        std::cout << "PASCAL_TRANSITION_BAND_CENSUS k=" << k
                  << " complete=YES bands=" << bands
                  << " states=" << states << " nodes=" << nodes
                  << " incomplete_relations=" << incomplete_relations
                  << " left_total_failures=" << left_total_failures
                  << " right_total_failures=" << right_total_failures
                  << " strict_states=" << strict_states
                  << " strict_incomplete_relations="
                  << strict_incomplete_relations
                  << " max_nodes=" << maximum_nodes
                  << " worst_band=(" << worst_begin << ',' << worst_end << ')'
                  << " seconds=" << seconds << '\n';
        for (const auto &[shape, count] : relation_shapes)
            std::cout << "RELATION_SHAPE shape=" << shape
                      << " states=" << count << '\n';
        return 0;
    }

    if (argc == 5 && std::string(argv[2]) == "transition-band") {
        const int k = std::atoi(argv[1]);
        const int begin = std::atoi(argv[3]);
        const int end = std::atoi(argv[4]);
        if (k < 1 || k > 4 || begin < 0 || end <= begin ||
            end > (1 << k)) {
            std::cerr << "invalid transition-band request\n";
            return 2;
        }
        const Sequence child = singleton_base(k - 1);
        const Sequence parent = singleton_base(k);
        TransitionBandCensus census(child, parent, begin, end);
        census.run();
        if (begin == 0 && end == (1 << k) &&
            ((k == 3 &&
              (census.states != 160 || census.strict_states != 33 ||
               census.alternating_failures != 0 ||
               census.strict_alternating_failures != 0)) ||
             (k == 4 &&
              (census.states != 408776 || census.strict_states != 63329 ||
               census.alternating_failures != 1968 ||
               census.strict_alternating_failures != 0)))) {
            std::cerr << "PASCAL_EXACT_SUPPORT_REGRESSION k=" << k
                      << " states=" << census.states
                      << " strict=" << census.strict_states
                      << " alternating_failures="
                      << census.alternating_failures
                      << " strict_alternating_failures="
                      << census.strict_alternating_failures << '\n';
            return 1;
        }
        std::cout << "PASCAL_TRANSITION_BAND band=(" << begin << ',' << end
                  << ") complete=YES states=" << census.states
                  << " transitions=" << census.transitions.size()
                  << " nodes=" << census.nodes
                  << " max_nodes=" << census.max_nodes
                  << " worst_state=" << show(census.worst) << '\n';
        std::cout << "TRANSITION_COVERAGE incomplete_relations="
                  << census.incomplete_relations
                  << " left_total_failures=" << census.left_total_failures
                  << " right_total_failures=" << census.right_total_failures;
        std::cout << " strict_states=" << census.strict_states
                  << " strict_incomplete_relations="
                  << census.strict_incomplete_relations
                  << " alternating_failures=" << census.alternating_failures
                  << " strict_alternating_failures="
                  << census.strict_alternating_failures;
        if (!census.first_alternating_failure.empty())
            std::cout << " first_alternating_state="
                      << show(census.first_alternating_failure)
                      << " first_alternating_cut=("
                      << census.first_alternating_p << ','
                      << census.first_alternating_q << ')';
        if (!census.first_left_total_failure.empty())
            std::cout << " first_missing_from=" << census.first_missing_from
                      << " first_left_state="
                      << show(census.first_left_total_failure);
        if (!census.first_right_total_failure.empty())
            std::cout << " first_missing_to=" << census.first_missing_to
                      << " first_right_state="
                      << show(census.first_right_total_failure);
        std::cout << '\n';
        for (std::size_t index = 0; index < census.transitions.size(); ++index) {
            const Transition &transition = census.transitions[index];
            std::cout << "TRANSITION from=" << transition.from
                      << " to=" << transition.to
                      << " states=" << census.transition_uses[index]
                      << " left=" << show(transition.left.values)
                      << " mixed=" << show(transition.mixed.values)
                      << " right=" << show(transition.right.values) << '\n';
        }
        for (std::size_t count = 1;
             count < census.feasible_transition_histogram.size(); ++count) {
            if (census.feasible_transition_histogram[count] != 0)
                std::cout << "FEASIBLE_TRANSITIONS count=" << count
                          << " states="
                          << census.feasible_transition_histogram[count] << '\n';
        }
        for (const auto &[shape, count] : census.relation_shapes)
            std::cout << "RELATION_SHAPE shape=" << shape
                      << " states=" << count << '\n';
        return 0;
    }

    if (argc == 3 && std::string(argv[2]) == "all-bands") {
        const int k = std::atoi(argv[1]);
        if (k < 1 || k > 4) {
            std::cerr << "invalid all-bands request\n";
            return 2;
        }
        const Sequence child = singleton_base(k - 1);
        const Sequence parent = singleton_base(k);
        std::uint64_t bands = 0;
        std::uint64_t states = 0;
        std::uint64_t nodes = 0;
        std::uint64_t maximum_nodes = 0;
        int worst_begin = -1;
        int worst_end = -1;
        const auto started = std::chrono::steady_clock::now();
        for (int begin = 0; begin < static_cast<int>(parent.size()); ++begin) {
            for (int end = begin + 1; end <= static_cast<int>(parent.size()); ++end) {
                Sequence left = slice(child, (begin + 1) / 2, (end + 1) / 2);
                Sequence mixed = slice(child, begin, end);
                Sequence right = slice(child, begin / 2, end / 2);
                Sequence target = slice(parent, begin, end);
                Census census(std::move(left), std::move(mixed), std::move(right),
                              std::move(target), 0, 0, end - begin);
                census.run(false);
                ++bands;
                states += census.tested;
                nodes += census.nodes;
                if (census.max_nodes > maximum_nodes) {
                    maximum_nodes = census.max_nodes;
                    worst_begin = begin;
                    worst_end = end;
                }
            }
        }
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        std::cout << "PASCAL_BAND_CENSUS k=" << k
                  << " complete=YES bands=" << bands
                  << " states=" << states << " nodes=" << nodes
                  << " max_nodes=" << maximum_nodes
                  << " worst_band=(" << worst_begin << ',' << worst_end << ')'
                  << " seconds=" << seconds << '\n';
        return 0;
    }

    if (argc >= 3 && std::string(argv[2]) == "band") {
        if (argc < 5 || argc > 7) {
            std::cerr << "usage: singleton_pascal_interval_census"
                      << " K band begin end [state-limit [state-skip]]\n";
            return 2;
        }
        const int k = std::atoi(argv[1]);
        const int begin = std::atoi(argv[3]);
        const int end = std::atoi(argv[4]);
        const std::uint64_t limit =
            argc > 5 ? std::strtoull(argv[5], nullptr, 10) : 0;
        const std::uint64_t skip =
            argc > 6 ? std::strtoull(argv[6], nullptr, 10) : 0;
        if (k < 1 || k > 4 || begin < 0 || end <= begin ||
            end > (1 << k) || (limit == 0 && skip != 0)) {
            std::cerr << "invalid band request\n";
            return 2;
        }
        const Sequence child = singleton_base(k - 1);
        const Sequence parent = singleton_base(k);
        Sequence left = slice(child, (begin + 1) / 2, (end + 1) / 2);
        Sequence mixed = slice(child, begin, end);
        Sequence right = slice(child, begin / 2, end / 2);
        Sequence target = slice(parent, begin, end);
        Census census(std::move(left), std::move(mixed), std::move(right),
                      std::move(target), limit, skip, end - begin);
        census.run();
        return 0;
    }

    if (argc < 4 || argc > 6) {
        std::cerr << "usage: singleton_pascal_interval_census"
                  << " K t head|head-exact|tail [state-limit [state-skip]]\n";
        return 2;
    }
    const int k = std::atoi(argv[1]);
    const int t = std::atoi(argv[2]);
    const std::string side = argv[3];
    const std::uint64_t limit = argc > 4 ? std::strtoull(argv[4], nullptr, 10) : 0;
    const std::uint64_t skip = argc > 5 ? std::strtoull(argv[5], nullptr, 10) : 0;
    if (k < 1 || k > 4 || !power_of_two(t) ||
        t > (1 << k) ||
        (side != "head" && side != "head-exact" && side != "tail") ||
        (limit == 0 && skip != 0)) {
        std::cerr << "invalid interval request\n";
        return 2;
    }

    const Sequence child = singleton_base(k - 1);
    const Sequence parent = singleton_base(k);
    const int p = t / 2;
    const int q = t - p;
    Sequence left;
    Sequence mixed;
    Sequence right;
    Sequence target;
    if (side == "head" || side == "head-exact") {
        left = slice(child, 0, p);
        mixed = slice(child, 0, t);
        right = slice(child, 0, q);
        target = slice(parent, 0, t);
    } else {
        left = slice(child, p, static_cast<int>(child.size()));
        mixed = slice(child, t, static_cast<int>(child.size()));
        right = slice(child, q, static_cast<int>(child.size()));
        target = slice(parent, t, static_cast<int>(parent.size()));
    }
    Census census(std::move(left), std::move(mixed), std::move(right),
                  std::move(target), limit, skip,
                  side == "head-exact" ? t : 0);
    census.run();
    return 0;
}
