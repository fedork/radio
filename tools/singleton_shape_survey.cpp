#include <algorithm>
#include <array>
#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>

// Survey a global "same coalescence shape" proposal for the open Singleton
// Row-Coloring Lemma.  A full-mass parent row either remains one child row or
// splits into two nonempty rows in the mixed and one pure child.  If s rows
// split, then r_0+r_1+r_2=r+s.
//
// For a parent with r rows, define its normalized coalescence shape
//
//   theta = (3^k-r)/(3^k-2^k).
//
// A row of width w represents w-1 binary joins.  If s parent rows split at
// the first test, the sum of the three child join counts is therefore
// (3^k-r)-s.  Since
//
//   3^k-2^k = 3(3^(k-1)-2^(k-1)) + 2^(k-1),
//
// the children have the same theta as the parent on average exactly when
// s=2^(k-1) theta.  A row of width one cannot split, so the tested target is
// the floor or ceiling of
//
//   min(2^(k-1) theta, number of parent rows of width at least two).
//
// This is an exact finite survey of the rule, not a proof of it.  Every accepted
// split is reconstructed directly and all three child partitions are checked
// against G_(k-1).

namespace {

using Sequence = std::vector<int>;

int power(int base, int exponent) {
    int result = 1;
    while (exponent-- > 0) result *= base;
    return result;
}

Sequence singleton_base(int k) {
    Sequence current{1};
    for (int level = 0; level < k; ++level) {
        Sequence next(2 * current.size(), 0);
        for (std::size_t i = 0; i < current.size(); ++i) {
            next[i] += current[i];
            next[2 * i] += current[i];
            next[2 * i + 1] += current[i];
        }
        std::sort(next.begin(), next.end(), std::greater<int>());
        current = std::move(next);
    }
    return current;
}

std::string show(const Sequence &values) {
    std::string result = "(";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i) result += ',';
        result += std::to_string(values[i]);
    }
    return result + ')';
}

struct LocalOption {
    std::array<int, 3> piece{};
    int split = 0;
};

struct ShapeSplitSearch {
    static constexpr int max_split_bits = 256;
    using SplitMask = std::bitset<max_split_bits>;
    const Sequence &parent;
    Sequence child_base;
    Sequence child_prefix{0};
    int child_mass;
    int child_min_rows;
    int child_max_width;
    int target_splits;
    bool enforce_full_profile;
    Sequence cut_lower;
    Sequence cut_upper;
    Sequence cut_used;
    std::vector<int> suffix_mass;
    std::vector<std::vector<LocalOption>> options_by_width;
    std::array<Sequence, 3> frequency;
    std::array<int, 3> mass{};
    std::array<int, 3> rows{};
    std::vector<std::array<int, 3>> selected;
    std::vector<std::array<int, 3>> witness;
    std::unordered_set<std::string> memo;
    std::unordered_map<std::string, SplitMask> all_memo;
    std::uint64_t nodes = 0;

    ShapeSplitSearch(const Sequence &state, int k, int target,
                     bool full_profile = false)
        : parent(state), child_base(singleton_base(k - 1)),
          child_mass(power(3, k - 1)),
          child_min_rows(power(2, k - 1)),
          child_max_width(child_base.front()), target_splits(target),
          enforce_full_profile(full_profile),
          cut_lower(state.empty() ? 1 : state.front() + 1, 0),
          cut_upper(state.empty() ? 1 : state.front() + 1,
                    std::numeric_limits<int>::max()),
          cut_used(state.empty() ? 1 : state.front() + 1, 0),
          suffix_mass(parent.size() + 1, 0),
          options_by_width(parent.empty() ? 1 : parent.front() + 1),
          selected(parent.size()) {
        for (int value : child_base) child_prefix.push_back(child_prefix.back() + value);
        for (int i = static_cast<int>(parent.size()) - 1; i >= 0; --i)
            suffix_mass[i] = suffix_mass[i + 1] + parent[i];
        for (auto &f : frequency) f.assign(child_max_width + 1, 0);

        if (enforce_full_profile) {
            const Sequence parent_base = singleton_base(k);
            for (int threshold = 1; threshold <= parent.front(); ++threshold) {
                const auto excess = [threshold](const Sequence &values) {
                    int result = 0;
                    for (int value : values) result += std::max(value - threshold, 0);
                    return result;
                };
                const int state_excess = excess(parent);
                const int base_excess = excess(parent_base);
                const int child_excess = excess(child_base);
                const int top_layer = base_excess - 3 * child_excess;
                int capacity = 0;
                for (int value : parent)
                    capacity += std::min(threshold, std::max(value - threshold, 0));
                if (base_excess == 0) continue;
                const std::int64_t numerator =
                    static_cast<std::int64_t>(state_excess) * top_layer;
                const std::int64_t capped = std::min(
                    numerator, static_cast<std::int64_t>(capacity) * base_excess);
                cut_lower[threshold] = static_cast<int>(capped / base_excess);
                cut_upper[threshold] = static_cast<int>(
                    (capped + base_excess - 1) / base_excess);
            }
        }

        for (int width = 1; width < static_cast<int>(options_by_width.size()); ++width) {
            std::vector<LocalOption> choices;
            for (int x = 0; x <= width; ++x) {
                for (const std::array<int, 3> pieces : {
                         std::array<int, 3>{width - x, x, 0},
                         std::array<int, 3>{0, width - x, x}}) {
                    if (*std::max_element(pieces.begin(), pieces.end()) > child_max_width)
                        continue;
                    if (std::any_of(choices.begin(), choices.end(), [&](const LocalOption &old) {
                            return old.piece == pieces;
                        }))
                        continue;
                    const int nonempty = static_cast<int>(pieces[0] > 0) +
                                         static_cast<int>(pieces[1] > 0) +
                                         static_cast<int>(pieces[2] > 0);
                    choices.push_back({pieces, nonempty - 1});
                }
            }
            options_by_width[width] = std::move(choices);
        }
    }

    int H(int count) const {
        return child_prefix[std::min(count, static_cast<int>(child_base.size()))];
    }

    bool child_majorized(int child) const {
        int count = 0;
        int sum = 0;
        for (int value = child_max_width; value >= 1; --value)
            for (int copy = 0; copy < frequency[child][value]; ++copy) {
                ++count;
                sum += value;
                if (sum > H(count)) return false;
            }
        return true;
    }

    std::string memo_key(int index, int splits) const {
        std::string key;
        key.reserve(2 + 3 * child_max_width);
        key.push_back(static_cast<char>(index));
        key.push_back(static_cast<char>(splits));
        // The pure children are interchangeable.  Canonicalize their frequency vectors.
        int first = 0;
        int third = 2;
        if (frequency[first] > frequency[third]) std::swap(first, third);
        for (int child : {first, 1, third})
            for (int value = 1; value <= child_max_width; ++value)
                key.push_back(static_cast<char>(frequency[child][value]));
        return key;
    }

    bool finish_unit_suffix(int index, int splits) {
        if (splits != target_splits) return false;
        if (enforce_full_profile)
            for (int threshold = 1; threshold < static_cast<int>(cut_used.size());
                 ++threshold)
                if (cut_used[threshold] < cut_lower[threshold] ||
                    cut_used[threshold] > cut_upper[threshold])
                    return false;
        const int units = static_cast<int>(parent.size()) - index;
        int required = 0;
        for (int child = 0; child < 3; ++child) {
            const int deficit = child_mass - mass[child];
            if (deficit < 0 || rows[child] + deficit < child_min_rows) return false;
            required += deficit;
        }
        if (required != units) return false;
        witness.assign(selected.begin(), selected.begin() + index);
        for (int child = 0; child < 3; ++child) {
            int deficit = child_mass - mass[child];
            for (int i = 0; i < deficit; ++i) {
                std::array<int, 3> pieces{};
                pieces[child] = 1;
                witness.push_back(pieces);
            }
        }
        return true;
    }

    bool dfs(int index, int splits) {
        ++nodes;
        const int remaining_rows = static_cast<int>(parent.size()) - index;
        if (splits > target_splits || splits + remaining_rows < target_splits) return false;
        if (index == static_cast<int>(parent.size())) {
            if (splits != target_splits) return false;
            for (int child = 0; child < 3; ++child)
                if (mass[child] != child_mass || rows[child] < child_min_rows) return false;
            witness = selected;
            return true;
        }
        if (parent[index] == 1) return finish_unit_suffix(index, splits);

        for (int child = 0; child < 3; ++child) {
            if (mass[child] > child_mass ||
                mass[child] + suffix_mass[index] < child_mass)
                return false;
        }

        const std::string key = memo_key(index, splits);
        if (!memo.insert(key).second) return false;

        auto choices = options_by_width[parent[index]];
        std::stable_sort(choices.begin(), choices.end(), [&](const LocalOption &left,
                                                             const LocalOption &right) {
            auto score = [&](const LocalOption &option) {
                int result = 0;
                for (int child = 0; child < 3; ++child) {
                    const int residual = child_mass - mass[child] - option.piece[child];
                    result += residual * residual;
                }
                return result;
            };
            return score(left) < score(right);
        });

        for (const LocalOption &option : choices) {
            bool legal = true;
            Sequence added_cut(cut_used.size(), 0);
            if (enforce_full_profile) {
                for (int threshold = 1;
                     threshold < static_cast<int>(cut_used.size()); ++threshold) {
                    int reduction = -std::min(parent[index], threshold);
                    for (int piece : option.piece)
                        reduction += std::min(piece, threshold);
                    added_cut[threshold] = reduction;
                    cut_used[threshold] += reduction;
                    if (cut_used[threshold] > cut_upper[threshold]) legal = false;
                }
            }
            for (int child = 0; child < 3; ++child) {
                const int piece = option.piece[child];
                if (!piece) continue;
                ++frequency[child][piece];
                mass[child] += piece;
                ++rows[child];
                if (mass[child] > child_mass || !child_majorized(child)) legal = false;
            }
            selected[index] = option.piece;
            if (legal && dfs(index + 1, splits + option.split)) return true;
            for (int child = 0; child < 3; ++child) {
                const int piece = option.piece[child];
                if (!piece) continue;
                --frequency[child][piece];
                mass[child] -= piece;
                --rows[child];
            }
            if (enforce_full_profile)
                for (int threshold = 1;
                     threshold < static_cast<int>(cut_used.size()); ++threshold)
                    cut_used[threshold] -= added_cut[threshold];
        }
        return false;
    }

    bool run() { return dfs(0, 0); }

    SplitMask finish_unit_suffix_all(int index) const {
        SplitMask result;
        const int units = static_cast<int>(parent.size()) - index;
        int required = 0;
        for (int child = 0; child < 3; ++child) {
            const int deficit = child_mass - mass[child];
            if (deficit < 0 || rows[child] + deficit < child_min_rows) return result;
            required += deficit;
        }
        if (required == units) result.set(0);
        return result;
    }

    SplitMask dfs_all(int index) {
        ++nodes;
        SplitMask result;
        if (index == static_cast<int>(parent.size())) {
            for (int child = 0; child < 3; ++child)
                if (mass[child] != child_mass || rows[child] < child_min_rows)
                    return result;
            result.set(0);
            return result;
        }
        if (parent[index] == 1) return finish_unit_suffix_all(index);

        for (int child = 0; child < 3; ++child)
            if (mass[child] > child_mass ||
                mass[child] + suffix_mass[index] < child_mass)
                return result;

        const std::string key = memo_key(index, 0);
        const auto old = all_memo.find(key);
        if (old != all_memo.end()) return old->second;

        auto choices = options_by_width[parent[index]];
        std::stable_sort(choices.begin(), choices.end(), [&](const LocalOption &left,
                                                             const LocalOption &right) {
            auto score = [&](const LocalOption &option) {
                int value = 0;
                for (int child = 0; child < 3; ++child) {
                    const int residual = child_mass - mass[child] - option.piece[child];
                    value += residual * residual;
                }
                return value;
            };
            return score(left) < score(right);
        });

        for (const LocalOption &option : choices) {
            bool legal = true;
            for (int child = 0; child < 3; ++child) {
                const int piece = option.piece[child];
                if (!piece) continue;
                ++frequency[child][piece];
                mass[child] += piece;
                ++rows[child];
                if (mass[child] > child_mass || !child_majorized(child)) legal = false;
            }
            if (legal) result |= dfs_all(index + 1) << option.split;
            for (int child = 0; child < 3; ++child) {
                const int piece = option.piece[child];
                if (!piece) continue;
                --frequency[child][piece];
                mass[child] -= piece;
                --rows[child];
            }
        }
        all_memo.emplace(key, result);
        return result;
    }

    SplitMask all_feasible() { return dfs_all(0); }

    std::array<Sequence, 3> witness_children() const {
        std::array<Sequence, 3> result;
        for (const auto &pieces : witness)
            for (int child = 0; child < 3; ++child)
                if (pieces[child]) result[child].push_back(pieces[child]);
        for (auto &state : result)
            std::sort(state.begin(), state.end(), std::greater<int>());
        return result;
    }
};

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
            answer += count(remaining - value, value, length + 1);
        }
        return answer;
    }

    Sequence sample(std::mt19937_64 &random) {
        Sequence state;
        int remaining = total;
        int upper = maximum;
        while (remaining > 0) {
            struct Choice { int value; std::uint64_t weight; };
            std::vector<Choice> choices;
            std::uint64_t sum = 0;
            const int used = total - remaining;
            for (int value = std::min(upper, remaining); value >= 1; --value) {
                if (used + value > parent_H(static_cast<int>(state.size()) + 1)) continue;
                const auto weight = count(remaining - value, value,
                                          static_cast<int>(state.size()) + 1);
                choices.push_back({value, weight});
                sum += weight;
            }
            std::uniform_int_distribution<std::uint64_t> pick(0, sum - 1);
            std::uint64_t ticket = pick(random);
            for (const auto [value, weight] : choices) {
                if (ticket < weight) {
                    state.push_back(value);
                    remaining -= value;
                    upper = value;
                    break;
                }
                ticket -= weight;
            }
        }
        return state;
    }
};

std::vector<int> target_splits(const Sequence &state, int k) {
    const int total = power(3, k);
    const int minimum_rows = power(2, k);
    const int child_minimum_rows = power(2, k - 1);
    const int denominator = total - minimum_rows;
    const int splittable = static_cast<int>(std::count_if(
        state.begin(), state.end(), [](int value) { return value >= 2; }));
    const std::int64_t raw_numerator =
        static_cast<std::int64_t>(total - static_cast<int>(state.size())) *
        child_minimum_rows;
    const std::int64_t capped_numerator =
        std::min(raw_numerator, static_cast<std::int64_t>(splittable) * denominator);
    const int lower = static_cast<int>(capped_numerator / denominator);
    const int upper = static_cast<int>((capped_numerator + denominator - 1) / denominator);
    if (lower == upper) return {lower};
    return {lower, upper};
}

struct Survey {
    int k;
    Sequence base;
    Sequence prefix{0};
    int total;
    std::uint64_t checked = 0;
    std::uint64_t nodes = 0;
    std::uint64_t max_nodes = 0;
    Sequence failure;
    std::vector<int> failure_targets;
    std::vector<int> failure_feasible;
    std::array<Sequence, 3> failure_children;
    Sequence failure_profile_lower;
    Sequence failure_profile_upper;
    Sequence failure_profile_actual;
    bool full_profile;
    bool interval_mode;
    std::uint64_t interval_states = 0;
    std::uint64_t minimum_equals_hinge = 0;
    std::uint64_t maximum_equals_splittable = 0;
    std::uint64_t maximum_equals_mixed_bound = 0;
    std::uint64_t both_scalar_roundings_feasible = 0;
    std::uint64_t scalar_touches_minimum = 0;
    std::uint64_t scalar_touches_maximum = 0;
    int largest_minimum_gap = 0;
    int largest_maximum_gap = 0;
    int largest_mixed_upper_gap = 0;
    Sequence largest_minimum_gap_state;
    Sequence largest_maximum_gap_state;
    Sequence largest_mixed_upper_gap_state;
    std::vector<int> largest_minimum_gap_interval;
    std::vector<int> largest_maximum_gap_interval;
    std::vector<int> largest_mixed_upper_gap_interval;

    explicit Survey(int level, bool enforce_full_profile = false,
                    bool survey_intervals = false)
        : k(level), base(singleton_base(level)), total(power(3, level)) {
        full_profile = enforce_full_profile;
        interval_mode = survey_intervals;
        for (int value : base) prefix.push_back(prefix.back() + value);
    }

    int H(int count) const {
        return prefix[std::min(count, static_cast<int>(base.size()))];
    }

    int hinge_minimum_splits(const Sequence &state) const {
        const Sequence child = singleton_base(k - 1);
        int result = 0;
        for (int threshold = 1; threshold <= state.front(); ++threshold) {
            int required = 0;
            for (int value : state) required += std::max(value - threshold, 0);
            for (int value : child)
                required -= 3 * std::max(value - threshold, 0);
            if (required <= 0) continue;
            Sequence capacities;
            for (int value : state)
                capacities.push_back(
                    std::min(threshold, std::max(value - threshold, 0)));
            std::sort(capacities.begin(), capacities.end(), std::greater<int>());
            int supplied = 0;
            int used = 0;
            while (used < static_cast<int>(capacities.size()) && supplied < required)
                supplied += capacities[used++];
            if (supplied < required) return static_cast<int>(state.size()) + 1;
            result = std::max(result, used);
        }
        return result;
    }

    bool inspect_interval(const Sequence &state) {
        ++checked;
        ++interval_states;
        const int splittable = static_cast<int>(std::count_if(
            state.begin(), state.end(), [](int value) { return value >= 2; }));
        std::vector<int> feasible;
        ShapeSplitSearch search(state, k, 0);
        const auto mask = search.all_feasible();
        for (int target = 0; target <= splittable; ++target)
            if (mask.test(target)) feasible.push_back(target);
        nodes += search.nodes;
        max_nodes = std::max(max_nodes, search.nodes);
        bool contiguous = !feasible.empty();
        for (std::size_t i = 1; i < feasible.size(); ++i)
            if (feasible[i] != feasible[i - 1] + 1) contiguous = false;
        if (!contiguous) {
            failure = state;
            failure_feasible = feasible;
            return false;
        }

        const int hinge_minimum = hinge_minimum_splits(state);
        const int child_mass = power(3, k - 1);
        const int child_width = power(2, k - 1);
        int mandatory = 0;
        int mandatory_mixed_mass = 0;
        for (int value : state)
            if (value > child_width) {
                ++mandatory;
                mandatory_mixed_mass += value - child_width;
            }
        const int mixed_upper = std::min(
            splittable, mandatory + child_mass - mandatory_mixed_mass);
        const int minimum_gap = feasible.front() - hinge_minimum;
        const int maximum_gap = splittable - feasible.back();
        const int mixed_upper_gap = mixed_upper - feasible.back();
        minimum_equals_hinge += minimum_gap == 0;
        maximum_equals_splittable += maximum_gap == 0;
        maximum_equals_mixed_bound += feasible.back() == mixed_upper;
        const auto scalar_targets = target_splits(state, k);
        bool lower_target = std::binary_search(
            feasible.begin(), feasible.end(), scalar_targets.front());
        bool upper_target = std::binary_search(
            feasible.begin(), feasible.end(), scalar_targets.back());
        both_scalar_roundings_feasible += lower_target && upper_target;
        scalar_touches_minimum +=
            scalar_targets.front() <= feasible.front() &&
            feasible.front() <= scalar_targets.back();
        scalar_touches_maximum +=
            scalar_targets.front() <= feasible.back() &&
            feasible.back() <= scalar_targets.back();
        if (minimum_gap > largest_minimum_gap) {
            largest_minimum_gap = minimum_gap;
            largest_minimum_gap_state = state;
            largest_minimum_gap_interval = feasible;
        }
        if (maximum_gap > largest_maximum_gap) {
            largest_maximum_gap = maximum_gap;
            largest_maximum_gap_state = state;
            largest_maximum_gap_interval = feasible;
        }
        if (mixed_upper_gap > largest_mixed_upper_gap) {
            largest_mixed_upper_gap = mixed_upper_gap;
            largest_mixed_upper_gap_state = state;
            largest_mixed_upper_gap_interval = feasible;
        }
        return true;
    }

    bool inspect(const Sequence &state) {
        if (interval_mode) return inspect_interval(state);
        ++checked;
        const auto targets = target_splits(state, k);
        std::uint64_t state_nodes = 0;
        for (int target : targets) {
            ShapeSplitSearch search(state, k, target, full_profile);
            if (search.run()) {
                state_nodes += search.nodes;
                nodes += state_nodes;
                max_nodes = std::max(max_nodes, state_nodes);
                return true;
            }
            state_nodes += search.nodes;
        }
        nodes += state_nodes;
        max_nodes = std::max(max_nodes, state_nodes);
        failure = state;
        failure_targets = targets;
        if (full_profile) {
            ShapeSplitSearch bounds(state, k, targets.front(), true);
            failure_profile_lower = bounds.cut_lower;
            failure_profile_upper = bounds.cut_upper;
        }
        const int max_splits = static_cast<int>(std::count_if(
            state.begin(), state.end(), [](int value) { return value >= 2; }));
        for (int target = 0; target <= max_splits; ++target) {
            ShapeSplitSearch search(state, k, target);
            if (search.run()) {
                failure_feasible.push_back(target);
                if (failure_children[0].empty())
                    failure_children = search.witness_children();
            }
        }
        if (full_profile && !failure_children[0].empty()) {
            failure_profile_actual.assign(state.front() + 1, 0);
            for (int threshold = 1; threshold <= state.front(); ++threshold) {
                for (int value : state)
                    failure_profile_actual[threshold] +=
                        std::max(value - threshold, 0);
                for (const auto &child : failure_children)
                    for (int value : child)
                        failure_profile_actual[threshold] -=
                            std::max(value - threshold, 0);
            }
        }
        return false;
    }

    void enumerate(int remaining, int maximum, Sequence &state) {
        if (!failure.empty()) return;
        if (remaining == 0) {
            inspect(state);
            return;
        }
        const int used = total - remaining;
        for (int value = std::min(maximum, remaining); value >= 1; --value) {
            if (used + value > H(static_cast<int>(state.size()) + 1)) continue;
            state.push_back(value);
            enumerate(remaining - value, value, state);
            state.pop_back();
            if (!failure.empty()) return;
        }
    }

    void report(const std::string &mode, std::uint64_t requested = 0,
                std::uint64_t universe = 0, std::uint64_t seed = 0) const {
        std::cout << "SHAPE_SURVEY mode=" << mode << " k=" << k;
        if (universe) std::cout << " universe=" << universe;
        if (requested) std::cout << " requested=" << requested << " seed=" << seed;
        std::cout << " checked=" << checked << " nodes=" << nodes
                  << " max_nodes=" << max_nodes;
        if (failure.empty()) {
            std::cout << " result=NO_FAILURE\n";
            if (interval_mode) {
                std::cout << "INTERVAL_SUMMARY states=" << interval_states
                          << " minimum_equals_hinge=" << minimum_equals_hinge
                          << " maximum_equals_splittable="
                          << maximum_equals_splittable
                          << " maximum_equals_mixed_bound="
                          << maximum_equals_mixed_bound
                          << " both_scalar_roundings_feasible="
                          << both_scalar_roundings_feasible
                          << " scalar_touches_minimum=" << scalar_touches_minimum
                          << " scalar_touches_maximum=" << scalar_touches_maximum
                          << " largest_minimum_gap=" << largest_minimum_gap
                          << " minimum_gap_state=" << show(largest_minimum_gap_state)
                          << " minimum_gap_interval=";
                for (int value : largest_minimum_gap_interval) std::cout << value << ',';
                std::cout << " largest_maximum_gap=" << largest_maximum_gap
                          << " maximum_gap_state=" << show(largest_maximum_gap_state)
                          << " maximum_gap_interval=";
                for (int value : largest_maximum_gap_interval) std::cout << value << ',';
                std::cout << " largest_mixed_upper_gap=" << largest_mixed_upper_gap
                          << " mixed_upper_gap_state="
                          << show(largest_mixed_upper_gap_state)
                          << " mixed_upper_gap_interval=";
                for (int value : largest_mixed_upper_gap_interval)
                    std::cout << value << ',';
                std::cout << '\n';
            }
            return;
        }
        std::cout << " result=FAIL state=" << show(failure) << " targets=";
        for (int value : failure_targets) std::cout << value << ',';
        std::cout << " feasible=";
        for (int value : failure_feasible) std::cout << value << ',';
        std::cout << '\n';
        if (!failure_children[0].empty())
            std::cout << "FIRST_FEASIBLE_CHILDREN L=" << show(failure_children[0])
                      << " M=" << show(failure_children[1])
                      << " R=" << show(failure_children[2]) << '\n';
        if (full_profile && !failure_profile_actual.empty()) {
            std::cout << "PROFILE threshold:lower-upper/first_feasible";
            for (int threshold = 1;
                 threshold < static_cast<int>(failure_profile_actual.size()); ++threshold) {
                if (failure_profile_upper[threshold] !=
                    std::numeric_limits<int>::max())
                    std::cout << ' ' << threshold << ':'
                              << failure_profile_lower[threshold] << '-'
                              << failure_profile_upper[threshold] << '/'
                              << failure_profile_actual[threshold];
            }
            std::cout << '\n';
        }
    }
};

}  // namespace

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "usage: singleton_shape_survey --census k\n"
                     "       singleton_shape_survey --uniform k samples [seed]\n"
                     "       singleton_shape_survey --profile-census k\n"
                     "       singleton_shape_survey --profile-uniform k samples [seed]\n"
                     "       singleton_shape_survey --interval-census k\n"
                     "       singleton_shape_survey --interval-uniform k samples [seed]\n"
                     "       singleton_shape_survey --state k row...\n";
        return 2;
    }
    const std::string mode = argv[1];
    const int k = std::atoi(argv[2]);
    if (k < 1 || k > 5) {
        std::cerr << "k must be in 1..5\n";
        return 2;
    }
    if (mode == "--state") {
        Sequence state;
        for (int i = 3; i < argc; ++i) state.push_back(std::atoi(argv[i]));
        std::sort(state.begin(), state.end(), std::greater<int>());
        const int expected_mass = power(3, k);
        int actual_mass = 0;
        for (int value : state) actual_mass += value;
        if (state.empty() || actual_mass != expected_mass) {
            std::cerr << "state mass must be " << expected_mass << '\n';
            return 2;
        }
        const int splittable = static_cast<int>(std::count_if(
            state.begin(), state.end(), [](int value) { return value >= 2; }));
        std::cout << "STATE_SPLITS k=" << k << " state=" << show(state)
                  << " splittable=" << splittable << '\n';
        for (int target = 0; target <= splittable; ++target) {
            ShapeSplitSearch search(state, k, target);
            if (!search.run()) continue;
            const auto children = search.witness_children();
            std::cout << "s=" << target << " L=" << show(children[0])
                      << " M=" << show(children[1])
                      << " R=" << show(children[2]) << '\n';
        }
        return 0;
    }
    const bool full_profile = mode == "--profile-census" ||
                              mode == "--profile-uniform";
    const bool intervals = mode == "--interval-census" ||
                           mode == "--interval-uniform";
    Survey survey(k, full_profile, intervals);
    if (mode == "--census" || mode == "--profile-census" ||
        mode == "--interval-census") {
        Sequence state;
        survey.enumerate(survey.total, survey.base.front(), state);
        survey.report(full_profile ? "profile-census" :
                      intervals ? "interval-census" : "census");
        return 0;
    }
    if (mode == "--uniform" || mode == "--profile-uniform" ||
        mode == "--interval-uniform") {
        const std::uint64_t samples = argc > 3 ? std::strtoull(argv[3], nullptr, 10) : 10000;
        const std::uint64_t seed = argc > 4 ? std::strtoull(argv[4], nullptr, 10) : 1;
        DominatedPartitionSampler sampler(survey.base);
        const auto universe = sampler.count(sampler.total, sampler.maximum, 0);
        std::mt19937_64 random(seed);
        for (std::uint64_t i = 0; i < samples && survey.failure.empty(); ++i)
            survey.inspect(sampler.sample(random));
        survey.report(full_profile ? "profile-uniform" :
                      intervals ? "interval-uniform" : "uniform",
                      samples, universe, seed);
        return 0;
    }
    std::cerr << "unknown mode: " << mode << '\n';
    return 2;
}
