// Independent first-cut solver for singleton states.
//
// This file deliberately does not include or call radiobase, the Hall-coloring
// census, or singleton_pascal_interval_census.  It works directly with the
// legal per-row triples
//
//     (l,m,0), l+m=a_i,    or    (0,m,r), m+r=a_i,
//
// and checks the three sorted child multisets against G_(K-1).

// The optimized search uses only necessary conditions that are visible in
// those triples: partial child majorization, residual mass/support capacity,
// one pure side per row, and parent-prefix orientation counts.  A separate
// tiny oracle deliberately enumerates every row triple without any of those
// prunes and checks only at the leaves.

// Build and run with complete provenance:
//
//   CC=clang++ tools/build_radio.py -std=c++20 -O3 -Wall -Wextra -pedantic \
//       tools/singleton_direct_split_cleanroom.cpp -o /tmp/singleton-direct
//   tools/run_with_provenance.py /tmp/singleton-direct regression


#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {

using Partition = std::vector<int>;

struct RowTriple {
    int left = 0;
    int mixed = 0;
    int right = 0;

    auto key() const { return std::tie(left, mixed, right); }
};

bool operator==(const RowTriple &a, const RowTriple &b) {
    return a.key() == b.key();
}

Partition singleton_profile(int level) {
    if (level < 0) {
        throw std::invalid_argument("negative singleton-profile level");
    }
    Partition profile{1};
    for (int k = 1; k <= level; ++k) {
        const std::size_t old_size = profile.size();
        Partition next(2 * old_size, 0);
        for (std::size_t i = 0; i < old_size; ++i) {
            next[2 * i] += profile[i];
            next[i] += profile[i];
            next[2 * i + 1] += profile[i];
        }
        std::sort(next.begin(), next.end(), std::greater<int>());
        profile = std::move(next);
    }
    return profile;
}

std::vector<int> prefix_sums(const Partition &profile) {
    std::vector<int> prefix(profile.size() + 1, 0);
    for (std::size_t i = 0; i < profile.size(); ++i) {
        prefix[i + 1] = prefix[i] + profile[i];
    }
    return prefix;
}

int saturated_prefix(const std::vector<int> &prefix, std::size_t count) {
    return prefix[std::min(count, prefix.size() - 1)];
}

bool weakly_majorized(const Partition &values, const std::vector<int> &prefix) {
    long long sum = 0;
    for (std::size_t i = 0; i < values.size(); ++i) {
        sum += values[i];
        if (sum > saturated_prefix(prefix, i + 1)) {
            return false;
        }
    }
    return true;
}

std::string compact_partition(const Partition &values) {
    if (values.empty()) {
        return "()";
    }
    std::ostringstream out;
    out << '(';
    for (std::size_t i = 0; i < values.size();) {
        std::size_t j = i + 1;
        while (j < values.size() && values[j] == values[i]) {
            ++j;
        }
        if (i != 0) {
            out << ',';
        }
        out << values[i];
        if (j - i > 1) {
            out << '^' << (j - i);
        }
        i = j;
    }
    out << ')';
    return out.str();
}

std::string triple_string(const RowTriple &triple) {
    std::ostringstream out;
    out << '(' << triple.left << ',' << triple.mixed << ',' << triple.right << ')';
    return out.str();
}

void insert_descending(Partition &values, int value) {
    if (value <= 0) {
        return;
    }
    const auto it = std::lower_bound(
        values.begin(), values.end(), value, std::greater<int>());
    values.insert(it, value);
}

std::vector<RowTriple> legal_row_triples(int row, int max_child_part) {
    std::vector<RowTriple> options;
    for (int mixed = 0; mixed <= row; ++mixed) {
        const int pure = row - mixed;
        if (mixed > max_child_part || pure > max_child_part) {
            continue;
        }
        if (pure == 0) {
            options.push_back({0, mixed, 0});
        } else {
            options.push_back({pure, mixed, 0});
            options.push_back({0, mixed, pure});
        }
    }
    std::sort(options.begin(), options.end(), [](const RowTriple &a, const RowTriple &b) {
        return a.key() < b.key();
    });
    return options;
}

struct SearchStats {
    std::uint64_t nodes = 0;
    std::uint64_t options = 0;
    std::uint64_t child_prunes = 0;
    std::uint64_t mass_prunes = 0;
    std::uint64_t support_prunes = 0;
    std::uint64_t prefix_prunes = 0;
    std::size_t maximum_depth = 0;
};

struct SearchResult {
    bool feasible = false;
    std::array<Partition, 3> children;
    std::vector<RowTriple> allocation;
    SearchStats stats;
};

enum class ResidualFailure {
    none,
    mass,
    support,
    prefix,
};

struct SearchState {
    std::array<Partition, 3> children;
    std::array<int, 3> masses{0, 0, 0};
    int left_oriented = 0;
    int right_oriented = 0;
};

class DirectSplitSolver {
  public:
    DirectSplitSolver(int level, Partition rows,
                      std::optional<std::array<Partition, 3>> preference = std::nullopt)
        : level_(level), rows_(std::move(rows)), preference_(std::move(preference)) {
        if (level_ < 1) {
            throw std::invalid_argument("a first cut requires level at least one");
        }
        if (std::any_of(rows_.begin(), rows_.end(), [](int value) { return value <= 0; })) {
            throw std::invalid_argument("parent rows must be positive");
        }
        std::sort(rows_.begin(), rows_.end(), std::greater<int>());
        parent_profile_ = singleton_profile(level_);
        child_profile_ = singleton_profile(level_ - 1);
        parent_prefix_ = prefix_sums(parent_profile_);
        child_prefix_ = prefix_sums(child_profile_);
        child_mass_ = child_prefix_.back();
        parent_mass_ = std::accumulate(rows_.begin(), rows_.end(), 0);
        max_child_part_ = child_profile_.front();

        row_prefix_.assign(rows_.size() + 1, 0);
        suffix_mass_.assign(rows_.size() + 1, 0);
        suffix_piece_capacity_.assign(rows_.size() + 1, 0);
        for (std::size_t i = 0; i < rows_.size(); ++i) {
            row_prefix_[i + 1] = row_prefix_[i] + rows_[i];
        }
        for (std::size_t i = rows_.size(); i-- > 0;) {
            suffix_mass_[i] = suffix_mass_[i + 1] + rows_[i];
            suffix_piece_capacity_[i] = suffix_piece_capacity_[i + 1]
                + std::min(rows_[i], max_child_part_);
        }

        allowed_left_counts_.resize(rows_.size() + 1);
        for (std::size_t t = 0; t <= rows_.size(); ++t) {
            const int middle_capacity = saturated_prefix(child_prefix_, t);
            for (std::size_t p = 0; p <= t; ++p) {
                const int capacity = saturated_prefix(child_prefix_, p)
                    + middle_capacity
                    + saturated_prefix(child_prefix_, t - p);
                if (row_prefix_[t] <= capacity) {
                    allowed_left_counts_[t].push_back(static_cast<int>(p));
                }
            }
        }

        option_table_.reserve(rows_.size());
        for (int row : rows_) {
            option_table_.push_back(legal_row_triples(row, max_child_part_));
        }
    }

    SearchResult solve() {
        result_ = SearchResult{};
        current_allocation_.clear();

        if (!weakly_majorized(rows_, parent_prefix_)) {
            return result_;
        }
        SearchState state;
        if (future_feasible(0, state) != ResidualFailure::none) {
            return result_;
        }
        (void)dfs(0, state, 0);
        return result_;
    }

    const Partition &rows() const { return rows_; }
    const Partition &child_profile() const { return child_profile_; }

  private:
    int support_needed_for_mass(int mass) const {
        if (mass <= 0) {
            return 0;
        }
        for (std::size_t count = 1; count <= child_profile_.size(); ++count) {
            if (saturated_prefix(child_prefix_, count) >= mass) {
                return static_cast<int>(count);
            }
        }
        return std::numeric_limits<int>::max() / 4;
    }

    int preference_penalty(const SearchState &state) const {
        if (!preference_) {
            return 0;
        }
        int penalty = 0;
        for (int child = 0; child < 3; ++child) {
            const Partition &have = state.children[child];
            const Partition &want = (*preference_)[child];
            std::size_t j = 0;
            for (int value : have) {
                while (j < want.size() && want[j] > value) {
                    ++j;
                }
                if (j == want.size() || want[j] != value) {
                    ++penalty;
                } else {
                    ++j;
                }
            }
        }
        return penalty;
    }

    ResidualFailure future_feasible(std::size_t next_row, const SearchState &state) const {
        const int remaining_rows = static_cast<int>(rows_.size() - next_row);
        const int remaining_mass = suffix_mass_[next_row];
        const int remaining_piece_capacity = suffix_piece_capacity_[next_row];
        const int lower_child_mass = std::max(0, parent_mass_ - 2 * child_mass_);

        int total_mass_deficit = 0;
        for (int child = 0; child < 3; ++child) {
            if (state.masses[child] > child_mass_) {
                return ResidualFailure::mass;
            }
            const int deficit = std::max(0, lower_child_mass - state.masses[child]);
            total_mass_deficit += deficit;
            if (deficit > remaining_piece_capacity) {
                return ResidualFailure::mass;
            }
            const int final_mass_floor = std::max(lower_child_mass, state.masses[child]);
            const int support_floor = support_needed_for_mass(final_mass_floor);
            if (static_cast<int>(state.children[child].size()) + remaining_rows < support_floor) {
                return ResidualFailure::support;
            }
        }
        if (total_mass_deficit > remaining_mass) {
            return ResidualFailure::mass;
        }

        const int left_mass_deficit = std::max(0, lower_child_mass - state.masses[0]);
        const int right_mass_deficit = std::max(0, lower_child_mass - state.masses[2]);
        if (left_mass_deficit + right_mass_deficit > remaining_piece_capacity) {
            return ResidualFailure::mass;
        }

        const int left_support_floor = support_needed_for_mass(
            std::max(lower_child_mass, state.masses[0]));
        const int right_support_floor = support_needed_for_mass(
            std::max(lower_child_mass, state.masses[2]));
        const int missing_pure_support =
            std::max(0, left_support_floor - static_cast<int>(state.children[0].size()))
            + std::max(0, right_support_floor - static_cast<int>(state.children[2].size()));
        if (missing_pure_support > remaining_rows) {
            return ResidualFailure::support;
        }

        // For every future parent prefix, there must be some possible number p of
        // left-oriented rows.  Existing nonzero pure pieces fix lower bounds on p
        // and t-p.  Rows sent wholly to the mixed child remain freely orientable.
        // Existing pure masses must also fit the corresponding child prefixes.
        for (std::size_t t = next_row; t <= rows_.size(); ++t) {
            bool found = false;
            for (int p : allowed_left_counts_[t]) {
                if (p < state.left_oriented
                    || static_cast<int>(t) - p < state.right_oriented) {
                    continue;
                }
                if (state.masses[0] > saturated_prefix(child_prefix_, p)
                    || state.masses[1] > saturated_prefix(child_prefix_, t)
                    || state.masses[2] > saturated_prefix(child_prefix_, t - p)) {
                    continue;
                }
                found = true;
                break;
            }
            if (!found) {
                return ResidualFailure::prefix;
            }
        }
        return ResidualFailure::none;
    }

    bool apply_option(const SearchState &state, const RowTriple &option,
                      SearchState &next) const {
        next = state;
        insert_descending(next.children[0], option.left);
        insert_descending(next.children[1], option.mixed);
        insert_descending(next.children[2], option.right);
        next.masses[0] += option.left;
        next.masses[1] += option.mixed;
        next.masses[2] += option.right;
        if (option.left > 0) {
            ++next.left_oriented;
        } else if (option.right > 0) {
            ++next.right_oriented;
        }
        for (int child = 0; child < 3; ++child) {
            if (!weakly_majorized(next.children[child], child_prefix_)) {
                return false;
            }
        }
        return true;
    }

    bool dfs(std::size_t row_index, const SearchState &state, std::size_t minimum_option) {
        ++result_.stats.nodes;
        result_.stats.maximum_depth = std::max(result_.stats.maximum_depth, row_index);
        if (row_index == rows_.size()) {
            result_.feasible = true;
            result_.children = state.children;
            result_.allocation = current_allocation_;
            return true;
        }

        const auto &options = option_table_[row_index];
        struct Candidate {
            std::size_t option_index = 0;
            SearchState next;
            std::tuple<int, int, long long, std::size_t> score;
        };
        std::vector<Candidate> candidates;
        candidates.reserve(options.size() - std::min(minimum_option, options.size()));
        for (std::size_t i = minimum_option; i < options.size(); ++i) {
            ++result_.stats.options;
            SearchState next;
            if (!apply_option(state, options[i], next)) {
                ++result_.stats.child_prunes;
                continue;
            }
            const int penalty = preference_penalty(next);
            const int minimum_mass =
                std::min({next.masses[0], next.masses[1], next.masses[2]});
            const int maximum_mass =
                std::max({next.masses[0], next.masses[1], next.masses[2]});
            const long long used_mass = row_prefix_[row_index + 1];
            long long square_error = 0;
            for (int mass : next.masses) {
                const long long error = 3LL * mass - used_mass;
                square_error += error * error;
            }
            candidates.push_back(Candidate{
                i, std::move(next),
                std::tuple<int, int, long long, std::size_t>(
                    penalty, maximum_mass - minimum_mass, square_error, i)});
        }

        // This affects positive-witness discovery only.  It never prunes: when a
        // requested child triple is known, stay inside its submultisets first;
        // otherwise keep the three accumulated child masses roughly balanced.
        // Each next state is built once; the former comparator rebuilt it O(log n)
        // times per option and dominated the larger cleanroom controls.
        std::sort(
            candidates.begin(), candidates.end(),
            [](const Candidate &lhs, const Candidate &rhs) {
                return lhs.score < rhs.score;
            });

        for (Candidate &candidate : candidates) {
            const std::size_t option_index = candidate.option_index;
            const RowTriple &option = options[option_index];
            const SearchState &next = candidate.next;
            const ResidualFailure failure = future_feasible(row_index + 1, next);
            if (failure != ResidualFailure::none) {
                if (failure == ResidualFailure::mass) {
                    ++result_.stats.mass_prunes;
                } else if (failure == ResidualFailure::support) {
                    ++result_.stats.support_prunes;
                } else {
                    ++result_.stats.prefix_prunes;
                }
                continue;
            }

            current_allocation_.push_back(option);
            const bool same_block = row_index + 1 < rows_.size()
                && rows_[row_index + 1] == rows_[row_index];
            // Equal parent rows are indistinguishable.  Requiring their static
            // option indices to be nondecreasing enumerates every option
            // multiset once; no left/right symmetry reduction is mixed into it.
            if (dfs(row_index + 1, next, same_block ? option_index : 0)) {
                return true;
            }
            current_allocation_.pop_back();
        }
        return false;
    }

    int level_;
    Partition rows_;
    Partition parent_profile_;
    Partition child_profile_;
    std::vector<int> parent_prefix_;
    std::vector<int> child_prefix_;
    int child_mass_ = 0;
    int parent_mass_ = 0;
    int max_child_part_ = 0;
    std::vector<int> row_prefix_;
    std::vector<int> suffix_mass_;
    std::vector<int> suffix_piece_capacity_;
    std::vector<std::vector<int>> allowed_left_counts_;
    std::vector<std::vector<RowTriple>> option_table_;
    std::optional<std::array<Partition, 3>> preference_;
    std::vector<RowTriple> current_allocation_;
    SearchResult result_;
};

bool verify_solution(const Partition &rows, const Partition &child_profile,
                     const SearchResult &result, std::string &error) {
    if (!result.feasible) {
        error = "result is not feasible";
        return false;
    }
    if (result.allocation.size() != rows.size()) {
        error = "allocation has the wrong number of rows";
        return false;
    }
    std::array<Partition, 3> rebuilt;
    for (std::size_t i = 0; i < rows.size(); ++i) {
        const RowTriple &triple = result.allocation[i];
        if (triple.left < 0 || triple.mixed < 0 || triple.right < 0
            || triple.left + triple.mixed + triple.right != rows[i]
            || (triple.left > 0 && triple.right > 0)) {
            error = "illegal row triple at row " + std::to_string(i);
            return false;
        }
        insert_descending(rebuilt[0], triple.left);
        insert_descending(rebuilt[1], triple.mixed);
        insert_descending(rebuilt[2], triple.right);
    }
    const auto prefix = prefix_sums(child_profile);
    for (int child = 0; child < 3; ++child) {
        if (rebuilt[child] != result.children[child]) {
            error = "reported child does not match replayed allocation";
            return false;
        }
        if (!weakly_majorized(rebuilt[child], prefix)) {
            error = "replayed child violates majorization";
            return false;
        }
    }
    return true;
}

// Deliberately naive comparison oracle: no prefix pruning, no row quotient,
// no orientation bounds, and no residual bounds.  It enumerates all distinct
// legal row triples and checks the sorted children only at complete leaves.
bool naive_first_cut(const Partition &input_rows, int level, std::uint64_t &leaves) {
    Partition rows = input_rows;
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    const Partition child_profile = singleton_profile(level - 1);
    const auto child_prefix = prefix_sums(child_profile);
    std::array<Partition, 3> children;
    leaves = 0;

    std::function<bool(std::size_t)> visit = [&](std::size_t index) {
        if (index == rows.size()) {
            ++leaves;
            std::array<Partition, 3> sorted = children;
            for (Partition &child : sorted) {
                std::sort(child.begin(), child.end(), std::greater<int>());
                child.erase(std::remove(child.begin(), child.end(), 0), child.end());
                if (!weakly_majorized(child, child_prefix)) {
                    return false;
                }
            }
            return true;
        }
        const int row = rows[index];
        for (int mixed = 0; mixed <= row; ++mixed) {
            const int pure = row - mixed;
            children[0].push_back(pure);
            children[1].push_back(mixed);
            children[2].push_back(0);
            if (visit(index + 1)) {
                return true;
            }
            children[0].pop_back();
            children[1].pop_back();
            children[2].pop_back();
            if (pure == 0) {
                continue;
            }
            children[0].push_back(0);
            children[1].push_back(mixed);
            children[2].push_back(pure);
            if (visit(index + 1)) {
                return true;
            }
            children[0].pop_back();
            children[1].pop_back();
            children[2].pop_back();
        }
        return false;
    };
    return visit(0);
}

void enumerate_partitions(int remaining, int maximum, Partition &current,
                          const std::function<void(const Partition &)> &accept) {
    if (remaining == 0) {
        accept(current);
        return;
    }
    for (int value = std::min(remaining, maximum); value >= 1; --value) {
        current.push_back(value);
        enumerate_partitions(remaining - value, value, current, accept);
        current.pop_back();
    }
}

Partition repeated(std::initializer_list<std::pair<int, int>> blocks) {
    Partition result;
    for (const auto &[value, count] : blocks) {
        result.insert(result.end(), static_cast<std::size_t>(count), value);
    }
    std::sort(result.begin(), result.end(), std::greater<int>());
    return result;
}

Partition transfer_state(int step, bool padded) {
    if (step < 0 || step > 14) {
        throw std::invalid_argument("transfer step must lie in [0,14]");
    }
    Partition state = repeated({{64, 1}, {63, 1}, {57, 2}, {42, 4}, {22, 7}});
    state.push_back(22 - step);
    state.insert(state.end(), static_cast<std::size_t>(step), 8);
    state.insert(state.end(), static_cast<std::size_t>(15 - step), 7);
    state.push_back(7);  // the unchanged rank-32 row
    if (padded) {
        state.insert(state.end(), 32, 1);
    }
    std::sort(state.begin(), state.end(), std::greater<int>());
    return state;
}

void enumerate_dominated_exact_length(
    const Partition &capacity,
    const std::function<void(const Partition &)> &accept) {
    const std::vector<int> capacity_prefix = prefix_sums(capacity);
    const int total_mass = capacity_prefix.back();
    Partition current;
    std::function<void(int, int)> visit = [&](int remaining, int maximum) {
        const std::size_t position = current.size();
        if (position == capacity.size()) {
            if (remaining == 0) {
                accept(current);
            }
            return;
        }
        const int remaining_slots =
            static_cast<int>(capacity.size() - position - 1);
        const int used_mass = total_mass - remaining;
        for (int value = std::min(maximum, remaining); value >= 1; --value) {
            if (used_mass + value > capacity_prefix[position + 1]) {
                continue;
            }
            const int mass_after = remaining - value;
            if (mass_after < remaining_slots
                || mass_after > remaining_slots * value) {
                continue;
            }
            current.push_back(value);
            visit(mass_after, value);
            current.pop_back();
        }
    };
    visit(total_mass, capacity.front());
}

int transfer_distance(const Partition &from, const Partition &to) {
    if (from.size() != to.size()) {
        throw std::invalid_argument("transfer distance requires equal support");
    }
    int l1_distance = 0;
    for (std::size_t i = 0; i < from.size(); ++i) {
        l1_distance += std::abs(from[i] - to[i]);
    }
    if (l1_distance % 2 != 0) {
        throw std::logic_error("equal-mass transfer distance is not integral");
    }
    return l1_distance / 2;
}

bool k6_tight_band_face_survey() {
    const Partition head = repeated(
        {{64, 1}, {63, 1}, {57, 2}, {42, 4}, {22, 7}});
    const Partition canonical_band = repeated({{22, 1}, {7, 16}});
    const Partition expected_hole = repeated({{8, 15}, {7, 2}});
    const Partition tail(32, 1);

    std::uint64_t states = 0;
    std::uint64_t feasible = 0;
    std::uint64_t holes = 0;
    std::uint64_t total_nodes = 0;
    std::uint64_t maximum_nodes = 0;
    Partition worst_band;
    Partition unique_hole;
    int minimum_hole_distance = std::numeric_limits<int>::max();
    bool replay_ok = true;

    enumerate_dominated_exact_length(canonical_band, [&](const Partition &band) {
        Partition parent = head;
        parent.insert(parent.end(), band.begin(), band.end());
        parent.insert(parent.end(), tail.begin(), tail.end());
        DirectSplitSolver solver(6, parent);
        const SearchResult result = solver.solve();
        ++states;
        total_nodes += result.stats.nodes;
        if (result.stats.nodes > maximum_nodes) {
            maximum_nodes = result.stats.nodes;
            worst_band = band;
        }
        if (result.feasible) {
            ++feasible;
            std::string error;
            if (!verify_solution(solver.rows(), solver.child_profile(), result, error)) {
                std::cerr << "FAIL K6 tight-band face replay band="
                          << compact_partition(band) << ": " << error << '\n';
                replay_ok = false;
            }
        } else {
            ++holes;
            unique_hole = band;
            minimum_hole_distance = std::min(
                minimum_hole_distance, transfer_distance(canonical_band, band));
        }
    });

    const bool ok = replay_ok && states == 176 && feasible == 175 && holes == 1
        && unique_hole == expected_hole && minimum_hole_distance == 14
        && total_nodes == 141216 && maximum_nodes == 9345
        && worst_band == expected_hole;
    std::cout << "K6_TIGHT_BAND_FACE_SURVEY"
              << " complete=" << (ok ? "YES" : "NO")
              << " band=(15,32)"
              << " states=" << states
              << " feasible=" << feasible
              << " holes=" << holes
              << " unique_hole=" << compact_partition(unique_hole)
              << " minimum_transfer_distance="
              << (holes == 0 ? -1 : minimum_hole_distance)
              << " total_nodes=" << total_nodes
              << " max_nodes=" << maximum_nodes
              << " worst_band=" << compact_partition(worst_band) << '\n';
    return ok;
}

void print_stats(const SearchStats &stats);

bool k7_dyadic_family_control() {
    const Partition parent = repeated({
        {128, 1}, {127, 1}, {120, 2}, {99, 4}, {64, 7},
        {32, 1}, {31, 16}, {8, 32}, {1, 64}});
    DirectSplitSolver solver(7, parent);
    const SearchResult result = solver.solve();
    const bool ok = !result.feasible
        && result.stats.nodes == 21489353
        && result.stats.options == 238217814
        && result.stats.maximum_depth == 31
        && result.stats.child_prunes == 216728462
        && result.stats.mass_prunes == 0
        && result.stats.support_prunes == 0
        && result.stats.prefix_prunes == 0;
    std::cout << "K7_DYADIC_FAMILY_DIRECT_CONTROL"
              << " verified=" << (ok ? "YES" : "NO")
              << " feasible=" << (result.feasible ? "YES" : "NO");
    print_stats(result.stats);
    std::cout << '\n';
    return ok;
}

void print_stats(const SearchStats &stats) {
    std::cout << " nodes=" << stats.nodes
              << " options=" << stats.options
              << " max_depth=" << stats.maximum_depth
              << " prunes(child=" << stats.child_prunes
              << ",mass=" << stats.mass_prunes
              << ",support=" << stats.support_prunes
              << ",prefix=" << stats.prefix_prunes << ')';
}

void print_allocation(const Partition &rows, const SearchResult &result) {
    std::cout << "  allocation";
    for (std::size_t i = 0; i < rows.size();) {
        std::size_t j = i + 1;
        while (j < rows.size() && rows[j] == rows[i]
               && result.allocation[j] == result.allocation[i]) {
            ++j;
        }
        std::cout << ' ' << rows[i] << "->" << triple_string(result.allocation[i]);
        if (j - i > 1) {
            std::cout << '^' << (j - i);
        }
        i = j;
    }
    std::cout << '\n';
    std::cout << "  children L=" << compact_partition(result.children[0])
              << " M=" << compact_partition(result.children[1])
              << " R=" << compact_partition(result.children[2]) << '\n';
}

bool same_children(const std::array<Partition, 3> &actual,
                   const std::array<Partition, 3> &expected) {
    return actual == expected
        || (actual[0] == expected[2] && actual[1] == expected[1]
            && actual[2] == expected[0]);
}

bool run_named_case(const std::string &name, int level, const Partition &rows,
                    bool expected_feasible,
                    std::optional<std::array<Partition, 3>> expected_children = std::nullopt) {
    DirectSplitSolver solver(level, rows, expected_children);
    const SearchResult result = solver.solve();
    std::cout << "CASE " << name
              << " mass=" << std::accumulate(rows.begin(), rows.end(), 0)
              << " support=" << rows.size()
              << " feasible=" << (result.feasible ? "YES" : "NO");
    print_stats(result.stats);
    std::cout << '\n';

    if (result.feasible) {
        std::string error;
        if (!verify_solution(solver.rows(), solver.child_profile(), result, error)) {
            std::cerr << "FAIL " << name << ": replay failed: " << error << '\n';
            return false;
        }
        print_allocation(solver.rows(), result);
    }
    if (result.feasible != expected_feasible) {
        std::cerr << "FAIL " << name << ": expected feasible="
                  << (expected_feasible ? "YES" : "NO") << '\n';
        return false;
    }
    if (expected_children && !same_children(result.children, *expected_children)) {
        std::cerr << "FAIL " << name << ": did not reproduce the requested child triple\n";
        return false;
    }
    return true;
}

bool tiny_comparisons() {
    bool ok = true;
    std::uint64_t compared_states = 0;
    std::uint64_t naive_leaves = 0;

    for (int level = 1; level <= 3; ++level) {
        int capacity = 1;
        for (int i = 0; i < level; ++i) {
            capacity *= 3;
        }
        const int comparison_mass = std::min(capacity, 9);
        for (int mass = 0; mass <= comparison_mass; ++mass) {
            Partition current;
            enumerate_partitions(mass, mass, current, [&](const Partition &state) {
                DirectSplitSolver solver(level, state);
                const bool optimized = solver.solve().feasible;
                std::uint64_t leaves = 0;
                const bool naive = naive_first_cut(state, level, leaves);
                ++compared_states;
                naive_leaves += leaves;
                if (optimized != naive) {
                    std::cerr << "FAIL tiny comparison K=" << level
                              << " state=" << compact_partition(state)
                              << " optimized=" << optimized << " naive=" << naive << '\n';
                    ok = false;
                }
            });
        }
    }

    const std::array<std::uint64_t, 4> expected_full_mass{0, 2, 15, 1206};
    const std::array<std::uint64_t, 4> expected_exact_support{0, 1, 4, 160};
    std::uint64_t total_majorized = 0;
    std::uint64_t total_search_nodes = 0;
    for (int level = 1; level <= 3; ++level) {
        int mass = 1;
        for (int i = 0; i < level; ++i) {
            mass *= 3;
        }
        const Partition parent_profile = singleton_profile(level);
        const auto parent_prefix = prefix_sums(parent_profile);
        std::uint64_t majorized = 0;
        std::uint64_t exact_support = 0;
        std::uint64_t search_nodes = 0;
        bool level_ok = true;
        Partition current;
        enumerate_partitions(mass, mass, current, [&](const Partition &state) {
            if (!weakly_majorized(state, parent_prefix)) {
                return;
            }
            ++majorized;
            if (state.size() == parent_profile.size()) {
                ++exact_support;
            }
            DirectSplitSolver solver(level, state);
            const SearchResult result = solver.solve();
            search_nodes += result.stats.nodes;
            if (!result.feasible) {
                std::cerr << "FAIL tiny closure K=" << level
                          << " state=" << compact_partition(state) << '\n';
                ok = false;
                level_ok = false;
            }
        });
        std::cout << "TINY_FULL_MASS K=" << level
                  << " majorized=" << majorized
                  << " exact_support=" << exact_support
                  << " all_first_cut_feasible=" << (level_ok ? "YES" : "NO")
                  << " search_nodes=" << search_nodes << '\n';
        if (majorized != expected_full_mass[level]) {
            std::cerr << "FAIL tiny K=" << level << ": expected "
                      << expected_full_mass[level] << " full-mass types, got "
                      << majorized << '\n';
            ok = false;
        }
        if (exact_support != expected_exact_support[level]) {
            std::cerr << "FAIL tiny K=" << level << ": expected "
                      << expected_exact_support[level] << " exact-support types, got "
                      << exact_support << '\n';
            ok = false;
        }
        total_majorized += majorized;
        total_search_nodes += search_nodes;
    }
    std::cout << "TINY_COMPARISON optimized_vs_naive_states=" << compared_states
              << " naive_leaf_checks=" << naive_leaves
              << " full_mass_majorized_states=" << total_majorized
              << " optimized_search_nodes=" << total_search_nodes
              << " verified=" << (ok ? "YES" : "NO") << '\n';
    return ok;
}

int regression() {
    bool ok = true;
    const Partition g5 = singleton_profile(5);
    const Partition g6 = singleton_profile(6);
    const Partition expected_g5 = repeated(
        {{32, 1}, {31, 1}, {26, 2}, {16, 4}, {6, 8}, {1, 16}});
    const Partition expected_g6 = repeated(
        {{64, 1}, {63, 1}, {57, 2}, {42, 4}, {22, 8}, {7, 16}, {1, 32}});
    if (g5 != expected_g5 || g6 != expected_g6) {
        std::cerr << "FAIL canonical singleton profiles do not match G5/G6 controls\n";
        return 1;
    }

    const Partition j13_mixed = repeated(
        {{32, 1}, {31, 1}, {26, 2}, {16, 4}, {6, 7}, {2, 5}, {1, 12}});
    const Partition j13_right = repeated(
        {{32, 1}, {31, 1}, {26, 2}, {16, 3}, {8, 1}, {7, 8}, {1, 16}});

    ok &= run_named_case("canonical-G6", 6, g6, true,
                         std::array<Partition, 3>{g5, g5, g5});
    ok &= run_named_case("transfer-j13", 6, transfer_state(13, true), true,
                         std::array<Partition, 3>{g5, j13_mixed, j13_right});
    ok &= run_named_case("transfer-j14-padded", 6, transfer_state(14, true), false);
    ok &= run_named_case("transfer-j14-core", 6, transfer_state(14, false), false);
    ok &= k6_tight_band_face_survey();
    ok &= tiny_comparisons();

    std::cout << "CLEANROOM_SINGLETON_DIRECT_SPLIT verified=" << (ok ? "YES" : "NO")
              << " implementation=direct-row-triples"
              << " hall_code=NONE shared_cache=NONE\n";
    return ok ? 0 : 1;
}

int solve_cli(int argc, char **argv) {
    if (argc < 4) {
        std::cerr << "usage: " << argv[0] << " solve K ROW [ROW ...]\n";
        return 64;
    }
    const int level = std::stoi(argv[2]);
    Partition rows;
    for (int i = 3; i < argc; ++i) {
        rows.push_back(std::stoi(argv[i]));
    }
    DirectSplitSolver solver(level, rows);
    const SearchResult result = solver.solve();
    std::cout << "RESULT K=" << level
              << " parent=" << compact_partition(solver.rows())
              << " feasible=" << (result.feasible ? "YES" : "NO");
    print_stats(result.stats);
    std::cout << '\n';
    if (result.feasible) {
        std::string error;
        if (!verify_solution(solver.rows(), solver.child_profile(), result, error)) {
            std::cerr << "internal replay failure: " << error << '\n';
            return 70;
        }
        print_allocation(solver.rows(), result);
    }
    return 0;
}

}  // namespace

int main(int argc, char **argv) {
    try {
        if (argc == 1 || std::string(argv[1]) == "regression") {
            return regression();
        }
        if (std::string(argv[1]) == "solve") {
            return solve_cli(argc, argv);
        }
        if (std::string(argv[1]) == "survey-k6-band15-32") {
            return k6_tight_band_face_survey() ? 0 : 1;
        }
        if (std::string(argv[1]) == "k7-dyadic-family-control") {
            return k7_dyadic_family_control() ? 0 : 1;
        }
        std::cerr << "usage: " << argv[0]
                  << " regression | survey-k6-band15-32"
                  << " | k7-dyadic-family-control | solve K ROW [ROW ...]\n";
        return 64;
    } catch (const std::exception &error) {
        std::cerr << "error: " << error.what() << '\n';
        return 70;
    }
}
