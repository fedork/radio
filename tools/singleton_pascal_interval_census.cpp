#include <algorithm>
#include <chrono>
#include <compare>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
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
// the tail modes allow arbitrary positive-row refinements of a suffix.

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
    int required_left_parts = -1;
    int required_right_parts = -1;

    SplitSearch(const Sequence &parent, const Capacity &left,
                const Capacity &mixed, const Capacity &right,
                int left_parts = -1, int right_parts = -1)
        : state(parent), left_capacity(left), mixed_capacity(mixed),
          right_capacity(right), required_left_parts(left_parts),
          required_right_parts(right_parts) {
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
        if (row == state.size()) {
            found = left.mass == left_capacity.mass &&
                    mixed.mass == mixed_capacity.mass &&
                    right.mass == right_capacity.mass &&
                    (required_left_parts < 0 ||
                     (left.parts == required_left_parts &&
                      right.parts == required_right_parts));
            return;
        }

        const int value = state[row];
        const bool same_value = row > 0 && state[row - 1] == value;
        const int first = same_value ? minimum_choice : 0;
        for (int index = first; index < static_cast<int>(choices[value].size()); ++index) {
            const Choice &choice = choices[value][index];
            add(left, choice.left);
            add(mixed, choice.mixed);
            add(right, choice.right);
            if (child_ok(left, left_capacity) &&
                child_ok(mixed, mixed_capacity) &&
                child_ok(right, right_capacity))
                dfs(row + 1, index, remaining - value, left, mixed, right);
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
