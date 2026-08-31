// Deterministic Tight-Band Capacity Obstruction certificates.
//
// For a singleton parent a majorized by G_K, let H be the prefix function
// of G_(K-1).  If parent prefixes u<v are tight, every legal first cut has a
// monotone endpoint transition p->q between the maximizing count sets I(u)
// and I(v).  Saturation fixes the three band masses.  Moreover, if
// delta=H(v)-H(v-1)>0, every row in the band sends at least delta coins to
// the mixed child.  A pure side assigned s band rows can therefore receive
// at most the sum of the s largest values b_i-delta.  If every endpoint
// transition exceeds one of those two pure capacities, no first cut exists.
//
// This implementation deliberately contains no row-split search, Hall code,
// or cache.  Besides checking a supplied state, it can enumerate all bands
// where delta is positive, count exact-support boundary spaces, and minimize
// transfer distance over the finite disjunction of capacity inequalities.
// The dyadic-family mode balances one canonical Pascal band and checks the
// resulting explicit no-first-cut construction through K=15.

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

using Value = std::int64_t;
using Count = std::uint64_t;
using Partition = std::vector<Value>;

Partition singleton_profile(int level) {
    if (level < 0 || level > 15) {
        throw std::invalid_argument("singleton level must lie in [0,15]");
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
        std::sort(next.begin(), next.end(), std::greater<Value>());
        profile = std::move(next);
    }
    return profile;
}

struct PrefixFunction {
    std::vector<Value> values;

    explicit PrefixFunction(const Partition &partition)
        : values(partition.size() + 1, 0) {
        for (std::size_t i = 0; i < partition.size(); ++i) {
            values[i + 1] = values[i] + partition[i];
        }
    }

    Value operator()(int count) const {
        if (count < 0) {
            throw std::invalid_argument("negative prefix rank");
        }
        return values[std::min<std::size_t>(
            static_cast<std::size_t>(count), values.size() - 1)];
    }

    Value mass() const { return values.back(); }
};

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

std::string compact_counts(const std::vector<int> &values) {
    std::ostringstream out;
    out << '(';
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            out << ',';
        }
        out << values[i];
    }
    out << ')';
    return out.str();
}

Partition repeated(std::initializer_list<std::pair<Value, int>> blocks) {
    Partition result;
    for (const auto &[value, count] : blocks) {
        result.insert(result.end(), static_cast<std::size_t>(count), value);
    }
    std::sort(result.begin(), result.end(), std::greater<Value>());
    return result;
}

bool weakly_majorized(const Partition &state, const PrefixFunction &capacity) {
    Value prefix = 0;
    for (std::size_t i = 0; i < state.size(); ++i) {
        prefix += state[i];
        if (prefix > capacity(static_cast<int>(i + 1))) {
            return false;
        }
    }
    return true;
}

struct TransitionCheck {
    int from = 0;
    int to = 0;
    int left_rows = 0;
    int right_rows = 0;
    Value mixed_required = 0;
    Value left_required = 0;
    Value right_required = 0;
    Value left_capacity = 0;
    Value right_capacity = 0;
    bool left_blocked = false;
    bool right_blocked = false;
};

struct Certificate {
    int lower_rank = 0;
    int upper_rank = 0;
    Value mixed_floor = 0;
    Partition band;
    std::vector<int> lower_counts;
    std::vector<int> upper_counts;
    std::vector<TransitionCheck> transitions;
    bool row_below_mixed_floor = false;
    bool no_monotone_transition = false;
};

struct Analysis {
    int level = 0;
    Partition state;
    bool majorized = false;
    std::vector<int> tight_ranks;
    std::vector<Certificate> certificates;
};

std::vector<int> endpoint_counts(
    int rank, const PrefixFunction &parent, const PrefixFunction &child) {
    std::vector<int> result;
    for (int left = 0; left <= rank; ++left) {
        if (parent(rank)
            == child(rank) + child(left) + child(rank - left)) {
            result.push_back(left);
        }
    }
    return result;
}

Value largest_sum(const Partition &values, int count) {
    if (count < 0 || static_cast<std::size_t>(count) > values.size()) {
        throw std::invalid_argument("invalid pure-row count");
    }
    return std::accumulate(values.begin(), values.begin() + count, Value{0});
}

Analysis analyze(int level, Partition state) {
    if (level < 1) {
        throw std::invalid_argument("a first cut requires K at least one");
    }
    if (std::any_of(state.begin(), state.end(), [](Value value) {
            return value <= 0;
        })) {
        throw std::invalid_argument("parent rows must be positive");
    }
    std::sort(state.begin(), state.end(), std::greater<Value>());
    const Partition parent_profile = singleton_profile(level);
    const Partition child_profile = singleton_profile(level - 1);
    const PrefixFunction parent(parent_profile);
    const PrefixFunction child(child_profile);

    Analysis result;
    result.level = level;
    result.state = state;
    result.majorized = weakly_majorized(state, parent);
    if (!result.majorized) {
        return result;
    }

    result.tight_ranks.push_back(0);
    Value state_prefix = 0;
    for (std::size_t i = 0; i < state.size(); ++i) {
        state_prefix += state[i];
        if (state_prefix == parent(static_cast<int>(i + 1))) {
            result.tight_ranks.push_back(static_cast<int>(i + 1));
        }
    }

    for (std::size_t lower_index = 0;
         lower_index < result.tight_ranks.size(); ++lower_index) {
        const int lower = result.tight_ranks[lower_index];
        for (std::size_t upper_index = lower_index + 1;
             upper_index < result.tight_ranks.size(); ++upper_index) {
            const int upper = result.tight_ranks[upper_index];
            const Value delta = child(upper) - child(upper - 1);
            if (delta <= 0) {
                continue;
            }

            Certificate candidate;
            candidate.lower_rank = lower;
            candidate.upper_rank = upper;
            candidate.mixed_floor = delta;
            candidate.band.assign(state.begin() + lower, state.begin() + upper);
            candidate.lower_counts = endpoint_counts(lower, parent, child);
            candidate.upper_counts = endpoint_counts(upper, parent, child);
            if (candidate.lower_counts.empty() || candidate.upper_counts.empty()) {
                throw std::logic_error("tight singleton prefix has no endpoint count");
            }

            Partition pure_ceilings;
            pure_ceilings.reserve(candidate.band.size());
            for (Value row : candidate.band) {
                if (row < delta) {
                    candidate.row_below_mixed_floor = true;
                }
                pure_ceilings.push_back(row - delta);
            }
            std::sort(
                pure_ceilings.begin(), pure_ceilings.end(), std::greater<Value>());

            for (int from : candidate.lower_counts) {
                for (int to : candidate.upper_counts) {
                    if (from > to || lower - from > upper - to) {
                        continue;
                    }
                    TransitionCheck check;
                    check.from = from;
                    check.to = to;
                    check.left_rows = to - from;
                    check.right_rows = (upper - to) - (lower - from);
                    check.mixed_required = child(upper) - child(lower);
                    check.left_required = child(to) - child(from);
                    check.right_required =
                        child(upper - to) - child(lower - from);
                    check.left_capacity =
                        largest_sum(pure_ceilings, check.left_rows);
                    check.right_capacity =
                        largest_sum(pure_ceilings, check.right_rows);
                    check.left_blocked =
                        check.left_required > check.left_capacity;
                    check.right_blocked =
                        check.right_required > check.right_capacity;
                    candidate.transitions.push_back(check);
                }
            }
            candidate.no_monotone_transition = candidate.transitions.empty();

            const bool every_transition_blocked = std::all_of(
                candidate.transitions.begin(), candidate.transitions.end(),
                [](const TransitionCheck &check) {
                    return check.left_blocked || check.right_blocked;
                });
            if (candidate.row_below_mixed_floor
                || candidate.no_monotone_transition
                || every_transition_blocked) {
                result.certificates.push_back(std::move(candidate));
            }
        }
    }
    return result;
}

const Certificate *primary_certificate(const Analysis &analysis) {
    if (analysis.certificates.empty()) {
        return nullptr;
    }
    return &*std::min_element(
        analysis.certificates.begin(), analysis.certificates.end(),
        [](const Certificate &a, const Certificate &b) {
            return std::tuple<std::size_t, int, int, int>(
                       a.transitions.size(), a.upper_rank - a.lower_rank,
                       a.lower_rank, a.upper_rank)
                < std::tuple<std::size_t, int, int, int>(
                       b.transitions.size(), b.upper_rank - b.lower_rank,
                       b.lower_rank, b.upper_rank);
        });
}

void print_certificate(const Certificate &certificate) {
    std::cout << "CERTIFICATE anchors=(" << certificate.lower_rank << ','
              << certificate.upper_rank << ')'
              << " mixed_floor=" << certificate.mixed_floor
              << " lower_counts=" << compact_counts(certificate.lower_counts)
              << " upper_counts=" << compact_counts(certificate.upper_counts)
              << " band=" << compact_partition(certificate.band)
              << " transitions=" << certificate.transitions.size()
              << " floor_failure="
              << (certificate.row_below_mixed_floor ? "YES" : "NO")
              << " path_failure="
              << (certificate.no_monotone_transition ? "YES" : "NO") << '\n';
    for (const TransitionCheck &check : certificate.transitions) {
        std::string blocked = "NONE";
        if (check.left_blocked && check.right_blocked) {
            blocked = "BOTH";
        } else if (check.left_blocked) {
            blocked = "LEFT";
        } else if (check.right_blocked) {
            blocked = "RIGHT";
        }
        std::cout << "  TRANSITION from=" << check.from
                  << " to=" << check.to
                  << " mixed_required=" << check.mixed_required
                  << " left_rows=" << check.left_rows
                  << " left_required=" << check.left_required
                  << " left_capacity=" << check.left_capacity
                  << " right_rows=" << check.right_rows
                  << " right_required=" << check.right_required
                  << " right_capacity=" << check.right_capacity
                  << " blocked=" << blocked << '\n';
    }
}

Partition transfer_state(int step, bool padded) {
    if (step < 0 || step > 14) {
        throw std::invalid_argument("transfer step must lie in [0,14]");
    }
    Partition state = repeated(
        {{64, 1}, {63, 1}, {57, 2}, {42, 4}, {22, 7}});
    state.push_back(22 - step);
    state.insert(state.end(), static_cast<std::size_t>(step), 8);
    state.insert(state.end(), static_cast<std::size_t>(15 - step), 7);
    state.push_back(7);
    if (padded) {
        state.insert(state.end(), 32, 1);
    }
    std::sort(state.begin(), state.end(), std::greater<Value>());
    return state;
}

void enumerate_partitions(
    Value remaining, Value maximum, Partition &current,
    const std::function<void(const Partition &)> &accept) {
    if (remaining == 0) {
        accept(current);
        return;
    }
    for (Value value = std::min(remaining, maximum); value >= 1; --value) {
        current.push_back(value);
        enumerate_partitions(remaining - value, value, current, accept);
        current.pop_back();
    }
}

void enumerate_dominated_exact_length(
    const Partition &capacity,
    const std::function<void(const Partition &)> &accept) {
    const PrefixFunction prefix(capacity);
    const Value total_mass = prefix.mass();
    Partition current;
    std::function<void(Value, Value)> visit =
        [&](Value remaining, Value maximum) {
            const std::size_t position = current.size();
            if (position == capacity.size()) {
                if (remaining == 0) {
                    accept(current);
                }
                return;
            }
            const Value remaining_slots = static_cast<Value>(
                capacity.size() - position - 1);
            const Value used_mass = total_mass - remaining;
            for (Value value = std::min(maximum, remaining);
                 value >= 1; --value) {
                if (used_mass + value > prefix(static_cast<int>(position + 1))) {
                    continue;
                }
                const Value mass_after = remaining - value;
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

Count checked_count_sum(Count first, Count second) {
    if (std::numeric_limits<Count>::max() - first < second) {
        throw std::overflow_error("dominated-partition count exceeds uint64_t");
    }
    return first + second;
}

struct DominatedExactLengthCounter {
    PrefixFunction capacity;
    int length = 0;
    int total_mass = 0;
    int largest = 0;
    bool strict_internal = false;
    std::vector<Count> memo;
    std::vector<unsigned char> known;

    explicit DominatedExactLengthCounter(
        const Partition &values, bool require_strict_internal = false)
        : capacity(values), length(static_cast<int>(values.size())),
          total_mass(static_cast<int>(capacity.mass())),
          largest(static_cast<int>(values.front())),
          strict_internal(require_strict_internal),
          memo(static_cast<std::size_t>(length + 1)
                   * static_cast<std::size_t>(total_mass + 1)
                   * static_cast<std::size_t>(largest + 1),
               0),
          known(memo.size(), 0) {}

    std::size_t key(int position, int remaining, int maximum) const {
        return (static_cast<std::size_t>(position)
                    * static_cast<std::size_t>(total_mass + 1)
                + static_cast<std::size_t>(remaining))
                   * static_cast<std::size_t>(largest + 1)
            + static_cast<std::size_t>(maximum);
    }

    Count visit(int position, int remaining, int maximum) {
        if (position == length) {
            return remaining == 0 ? 1 : 0;
        }
        const int slots = length - position;
        if (remaining < slots || remaining > slots * maximum) {
            return 0;
        }
        const std::size_t index = key(position, remaining, maximum);
        if (known[index]) {
            return memo[index];
        }
        known[index] = 1;
        Count result = 0;
        const int used = total_mass - remaining;
        for (int value = std::min(maximum, remaining); value >= 1; --value) {
            const int new_prefix = used + value;
            if (new_prefix > capacity(position + 1)) {
                continue;
            }
            if (strict_internal && position + 1 < length
                && new_prefix == capacity(position + 1)) {
                continue;
            }
            result = checked_count_sum(
                result, visit(position + 1, remaining - value, value));
        }
        memo[index] = result;
        return result;
    }

    Count run() { return visit(0, total_mass, largest); }
};

int transfer_distance(const Partition &from, const Partition &to);

bool count_boundary_space(int level) {
    if (level < 1 || level > 5) {
        throw std::invalid_argument("boundary count level must lie in [1,5]");
    }
    const Partition parent = singleton_profile(level);
    DominatedExactLengthCounter full_counter(parent);
    DominatedExactLengthCounter strict_counter(parent, true);
    const Count exact_support = full_counter.run();
    const Count strict_interior = strict_counter.run();

    Count band_instances = 0;
    Count positive_floor_band_instances = 0;
    Count bands = 0;
    Count positive_floor_bands = 0;
    Count largest_band_space = 0;
    Count largest_positive_floor_band_space = 0;
    int largest_begin = -1;
    int largest_end = -1;
    int largest_positive_floor_begin = -1;
    int largest_positive_floor_end = -1;
    const int child_support = 1 << (level - 1);
    for (int begin = 0; begin < static_cast<int>(parent.size()); ++begin) {
        for (int end = begin + 1; end <= static_cast<int>(parent.size()); ++end) {
            const Partition band(parent.begin() + begin, parent.begin() + end);
            DominatedExactLengthCounter band_counter(band);
            const Count count = band_counter.run();
            band_instances = checked_count_sum(band_instances, count);
            ++bands;
            if (end <= child_support) {
                positive_floor_band_instances = checked_count_sum(
                    positive_floor_band_instances, count);
                ++positive_floor_bands;
                if (count > largest_positive_floor_band_space) {
                    largest_positive_floor_band_space = count;
                    largest_positive_floor_begin = begin;
                    largest_positive_floor_end = end;
                }
            }
            if (count > largest_band_space) {
                largest_band_space = count;
                largest_begin = begin;
                largest_end = end;
            }
        }
    }

    bool ok = true;
    if (level == 3) {
        ok = exact_support == 160 && strict_interior == 33
            && bands == 36 && band_instances == 561
            && largest_begin == 0 && largest_end == 8
            && largest_band_space == 160
            && positive_floor_bands == 10
            && positive_floor_band_instances == 22
            && largest_positive_floor_begin == 0
            && largest_positive_floor_end == 4
            && largest_positive_floor_band_space == 7;
    } else if (level == 4) {
        ok = exact_support == 408776 && strict_interior == 63329
            && bands == 136 && band_instances == 1722516
            && largest_begin == 0 && largest_end == 16
            && largest_band_space == 408776
            && positive_floor_bands == 36
            && positive_floor_band_instances == 3863
            && largest_positive_floor_begin == 0
            && largest_positive_floor_end == 8
            && largest_positive_floor_band_space == 1567;
    } else if (level == 5) {
        ok = exact_support == 1431800647444ULL
            && strict_interior == 147422086892ULL
            && bands == 528 && band_instances == 8973226867713ULL
            && largest_begin == 0 && largest_end == 32
            && largest_band_space == 1431800647444ULL
            && positive_floor_bands == 136
            && positive_floor_band_instances == 613689090
            && largest_positive_floor_begin == 0
            && largest_positive_floor_end == 16
            && largest_positive_floor_band_space == 228246747;
    }
    std::cout << "TIGHT_BAND_BOUNDARY_SPACE K=" << level
              << " verified=" << (ok ? "YES" : "NO")
              << " exact_support=" << exact_support
              << " strict_interior=" << strict_interior
              << " tight_skeleton=" << exact_support - strict_interior
              << " rank_bands=" << bands
              << " band_state_instances=" << band_instances
              << " largest_band=(" << largest_begin << ',' << largest_end << ')'
              << " largest_band_states=" << largest_band_space
              << " positive_floor_bands=" << positive_floor_bands
              << " positive_floor_band_instances="
              << positive_floor_band_instances
              << " largest_positive_floor_band=("
              << largest_positive_floor_begin << ','
              << largest_positive_floor_end << ')'
              << " largest_positive_floor_band_states="
              << largest_positive_floor_band_space << '\n';
    return ok;
}

struct CapacityBandSurvey {
    int lower = 0;
    int upper = 0;
    Value mixed_floor = 0;
    Partition canonical_band;
    PrefixFunction capacity;
    std::vector<TransitionCheck> transitions;
    Partition current;
    std::vector<Value> current_prefix{0};
    Count states = 0;
    Count certified_states = 0;
    Partition first_certified;
    int minimum_transfer_distance = std::numeric_limits<int>::max();

    CapacityBandSurvey(
        int lower_rank, int upper_rank, const Partition &parent_profile,
        const PrefixFunction &parent, const PrefixFunction &child)
        : lower(lower_rank), upper(upper_rank),
          mixed_floor(child(upper_rank) - child(upper_rank - 1)),
          canonical_band(parent_profile.begin() + lower_rank,
                         parent_profile.begin() + upper_rank),
          capacity(canonical_band) {
        const std::vector<int> lower_counts =
            endpoint_counts(lower, parent, child);
        const std::vector<int> upper_counts =
            endpoint_counts(upper, parent, child);
        for (int from : lower_counts) {
            for (int to : upper_counts) {
                if (from > to || lower - from > upper - to) {
                    continue;
                }
                TransitionCheck check;
                check.from = from;
                check.to = to;
                check.left_rows = to - from;
                check.right_rows = (upper - to) - (lower - from);
                check.mixed_required = child(upper) - child(lower);
                check.left_required = child(to) - child(from);
                check.right_required =
                    child(upper - to) - child(lower - from);
                transitions.push_back(check);
            }
        }
    }

    bool certified() const {
        if (current.back() < mixed_floor || transitions.empty()) {
            return true;
        }
        for (const TransitionCheck &check : transitions) {
            const Value left_capacity =
                current_prefix[check.left_rows]
                - check.left_rows * mixed_floor;
            const Value right_capacity =
                current_prefix[check.right_rows]
                - check.right_rows * mixed_floor;
            if (check.left_required <= left_capacity
                && check.right_required <= right_capacity) {
                return false;
            }
        }
        return true;
    }

    void inspect() {
        ++states;
        if (!certified()) {
            return;
        }
        ++certified_states;
        if (first_certified.empty()) {
            first_certified = current;
        }
        minimum_transfer_distance = std::min(
            minimum_transfer_distance,
            transfer_distance(canonical_band, current));
    }

    void enumerate(Value remaining, Value maximum) {
        const std::size_t position = current.size();
        if (position == canonical_band.size()) {
            if (remaining == 0) {
                inspect();
            }
            return;
        }
        const Value remaining_slots = static_cast<Value>(
            canonical_band.size() - position - 1);
        const Value used_mass = capacity.mass() - remaining;
        for (Value value = std::min(maximum, remaining);
             value >= 1; --value) {
            if (used_mass + value > capacity(static_cast<int>(position + 1))) {
                continue;
            }
            const Value mass_after = remaining - value;
            if (mass_after < remaining_slots
                || mass_after > remaining_slots * value) {
                continue;
            }
            current.push_back(value);
            current_prefix.push_back(current_prefix.back() + value);
            enumerate(mass_after, value);
            current_prefix.pop_back();
            current.pop_back();
        }
    }

    void run() {
        enumerate(capacity.mass(), canonical_band.front());
    }
};

bool survey_capacity_bands(int level) {
    if (level < 1 || level > 5) {
        throw std::invalid_argument("capacity-band survey level must lie in [1,5]");
    }
    const Partition parent_profile = singleton_profile(level);
    const PrefixFunction parent(parent_profile);
    const PrefixFunction child(singleton_profile(level - 1));
    const int positive_floor_support = 1 << (level - 1);
    Count bands = 0;
    Count states = 0;
    Count certified_states = 0;
    int certified_bands = 0;
    int first_lower = -1;
    int first_upper = -1;
    Partition first_certified;
    int minimum_distance = std::numeric_limits<int>::max();

    for (int upper = 1; upper <= positive_floor_support; ++upper) {
        for (int lower = 0; lower < upper; ++lower) {
            CapacityBandSurvey survey(
                lower, upper, parent_profile, parent, child);
            survey.run();
            states = checked_count_sum(states, survey.states);
            certified_states = checked_count_sum(
                certified_states, survey.certified_states);
            ++bands;
            if (survey.certified_states != 0) {
                ++certified_bands;
                if (first_certified.empty()) {
                    first_lower = lower;
                    first_upper = upper;
                    first_certified = survey.first_certified;
                }
                minimum_distance = std::min(
                    minimum_distance, survey.minimum_transfer_distance);
            }
            std::cout << "TIGHT_BAND_CAPACITY_FACE K=" << level
                      << " anchors=(" << lower << ',' << upper << ')'
                      << " transitions=" << survey.transitions.size()
                      << " states=" << survey.states
                      << " certified=" << survey.certified_states;
            if (!survey.first_certified.empty()) {
                std::cout << " first="
                          << compact_partition(survey.first_certified)
                          << " minimum_transfer_distance="
                          << survey.minimum_transfer_distance;
            }
            std::cout << '\n' << std::flush;
        }
    }

    bool ok = true;
    if (level == 3) {
        ok = bands == 10 && states == 22
            && certified_states == 0;
    } else if (level == 4) {
        ok = bands == 36 && states == 3863
            && certified_states == 0;
    } else if (level == 5) {
        ok = bands == 136 && states == 613689090
            && certified_states == 0;
    }
    std::cout << "TIGHT_BAND_CAPACITY_SURVEY K=" << level
              << " complete=YES"
              << " verified=" << (ok ? "YES" : "NO")
              << " bands=" << bands
              << " states=" << states
              << " certified_bands=" << certified_bands
              << " certified_states=" << certified_states;
    if (!first_certified.empty()) {
        std::cout << " first_anchors=(" << first_lower << ',' << first_upper << ')'
                  << " first_certified=" << compact_partition(first_certified)
                  << " minimum_transfer_distance=" << minimum_distance;
    }
    std::cout << '\n';
    return ok;
}

int transfer_distance(const Partition &from, const Partition &to) {
    if (from.size() != to.size()) {
        throw std::invalid_argument("transfer distance requires equal support");
    }
    Value l1_distance = 0;
    for (std::size_t i = 0; i < from.size(); ++i) {
        l1_distance += std::llabs(from[i] - to[i]);
    }
    if (l1_distance % 2 != 0
        || l1_distance / 2 > std::numeric_limits<int>::max()) {
        throw std::logic_error("invalid equal-mass transfer distance");
    }
    return static_cast<int>(l1_distance / 2);
}

struct TransferShellCounter {
    const Partition &canonical;
    PrefixFunction capacity;
    int length = 0;
    int total_mass = 0;
    int largest = 0;
    int target_l1 = 0;
    std::vector<int> canonical_suffix;
    std::unordered_map<std::uint64_t, Count> memo;

    TransferShellCounter(const Partition &profile, int distance)
        : canonical(profile), capacity(profile),
          length(static_cast<int>(profile.size())),
          total_mass(static_cast<int>(capacity.mass())),
          largest(static_cast<int>(profile.front())),
          target_l1(2 * distance), canonical_suffix(length + 1, 0) {
        for (int index = length - 1; index >= 0; --index) {
            canonical_suffix[index] =
                canonical_suffix[index + 1] + static_cast<int>(canonical[index]);
        }
    }

    std::uint64_t key(
        int position, int remaining, int maximum, int used_l1) const {
        std::uint64_t result = static_cast<std::uint64_t>(position);
        result = result * static_cast<std::uint64_t>(total_mass + 1)
            + static_cast<std::uint64_t>(remaining);
        result = result * static_cast<std::uint64_t>(largest + 1)
            + static_cast<std::uint64_t>(maximum);
        result = result * static_cast<std::uint64_t>(target_l1 + 1)
            + static_cast<std::uint64_t>(used_l1);
        return result;
    }

    Count visit(int position, int remaining, int maximum, int used_l1) {
        if (position == length) {
            return remaining == 0 && used_l1 == target_l1 ? 1 : 0;
        }
        const int slots = length - position;
        if (remaining < slots || remaining > slots * maximum
            || used_l1 > target_l1) {
            return 0;
        }
        const int remaining_difference =
            remaining - canonical_suffix[position];
        const int available_l1 = target_l1 - used_l1;
        if (remaining_difference < 0
            || remaining_difference > available_l1
            || ((available_l1 - remaining_difference) & 1) != 0) {
            return 0;
        }

        const std::uint64_t memo_key =
            key(position, remaining, maximum, used_l1);
        if (const auto found = memo.find(memo_key); found != memo.end()) {
            return found->second;
        }

        Count result = 0;
        const int used_mass = total_mass - remaining;
        const int canonical_value = static_cast<int>(canonical[position]);
        const int smallest = std::max(1, canonical_value - available_l1);
        const int greatest = std::min(
            {maximum, remaining, canonical_value + available_l1});
        for (int value = greatest; value >= smallest; --value) {
            if (used_mass + value > capacity(position + 1)) {
                continue;
            }
            const int new_l1 = used_l1 + std::abs(value - canonical_value);
            if (new_l1 > target_l1) {
                continue;
            }
            result = checked_count_sum(
                result,
                visit(position + 1, remaining - value, value, new_l1));
        }
        memo.emplace(memo_key, result);
        return result;
    }

    Count run() { return visit(0, total_mass, largest, 0); }
};

bool count_transfer_shells(int level, int maximum_distance) {
    if (level < 1 || level > 6 || maximum_distance < 0) {
        throw std::invalid_argument("invalid transfer-shell count request");
    }
    const Partition canonical = singleton_profile(level);
    std::vector<Count> shell_counts;
    Count ball = 0;
    for (int distance = 0; distance <= maximum_distance; ++distance) {
        TransferShellCounter counter(canonical, distance);
        const Count count = counter.run();
        shell_counts.push_back(count);
        ball = checked_count_sum(ball, count);
        std::cout << "TRANSFER_SHELL K=" << level
                  << " distance=" << distance
                  << " states=" << count
                  << " memo_states=" << counter.memo.size() << '\n';
    }

    bool ok = !shell_counts.empty() && shell_counts.front() == 1;
    const bool direct_cross_check_run = level <= 4;
    if (direct_cross_check_run) {
        std::vector<Count> direct(shell_counts.size(), 0);
        enumerate_dominated_exact_length(canonical, [&](const Partition &state) {
            const int distance = transfer_distance(canonical, state);
            if (distance <= maximum_distance) {
                ++direct[distance];
            }
        });
        ok = ok && direct == shell_counts;
    }
    std::cout << "TRANSFER_BALL K=" << level
              << " maximum_distance=" << maximum_distance
              << " states=" << ball
              << " internal_checks=" << (ok ? "YES" : "NO")
              << " direct_cross_check="
              << (direct_cross_check_run ? (ok ? "YES" : "NO") : "NOT_RUN")
              << '\n';
    return ok;
}

struct PrefixCapOptimizer {
    static constexpr int impossible = std::numeric_limits<int>::max() / 4;

    const Partition &canonical;
    PrefixFunction capacity;
    const std::vector<Value> &prefix_caps;
    int minimum_value = 1;
    int final_maximum = std::numeric_limits<int>::max();
    int length = 0;
    int total_mass = 0;
    int largest = 0;
    std::unordered_map<std::uint64_t, int> memo;
    std::unordered_map<std::uint64_t, int> choices;

    PrefixCapOptimizer(
        const Partition &values, const std::vector<Value> &caps,
        int row_minimum, int last_maximum)
        : canonical(values), capacity(values), prefix_caps(caps),
          minimum_value(row_minimum), final_maximum(last_maximum),
          length(static_cast<int>(values.size())),
          total_mass(static_cast<int>(capacity.mass())),
          largest(static_cast<int>(values.front())) {}

    std::uint64_t key(int position, int remaining, int maximum) const {
        std::uint64_t result = static_cast<std::uint64_t>(position);
        result = result * static_cast<std::uint64_t>(total_mass + 1)
            + static_cast<std::uint64_t>(remaining);
        result = result * static_cast<std::uint64_t>(largest + 1)
            + static_cast<std::uint64_t>(maximum);
        return result;
    }

    int visit(int position, int remaining, int maximum) {
        if (position == length) {
            return remaining == 0 ? 0 : impossible;
        }
        const int slots = length - position;
        if (remaining < slots * minimum_value
            || remaining > slots * maximum) {
            return impossible;
        }
        const std::uint64_t memo_key = key(position, remaining, maximum);
        if (const auto found = memo.find(memo_key); found != memo.end()) {
            return found->second;
        }

        const int used_mass = total_mass - remaining;
        int greatest = std::min(maximum, remaining);
        if (position + 1 == length) {
            greatest = std::min(greatest, final_maximum);
        }
        int best = impossible;
        int best_value = -1;
        for (int value = greatest; value >= minimum_value; --value) {
            const int new_prefix = used_mass + value;
            if (new_prefix > prefix_caps[position + 1]) {
                continue;
            }
            const int tail = visit(position + 1, remaining - value, value);
            if (tail == impossible) {
                continue;
            }
            const int candidate =
                std::abs(value - static_cast<int>(canonical[position])) + tail;
            if (candidate < best) {
                best = candidate;
                best_value = value;
            }
        }
        memo.emplace(memo_key, best);
        if (best_value >= 0) {
            choices.emplace(memo_key, best_value);
        }
        return best;
    }

    std::pair<int, Partition> run() {
        const int objective = visit(0, total_mass, largest);
        if (objective == impossible) {
            return {impossible, {}};
        }
        Partition result;
        int remaining = total_mass;
        int maximum = largest;
        for (int position = 0; position < length; ++position) {
            const std::uint64_t state_key = key(position, remaining, maximum);
            const int value = choices.at(state_key);
            result.push_back(value);
            remaining -= value;
            maximum = value;
        }
        return {objective, result};
    }
};

struct MinimumBandCertificate {
    int lower = -1;
    int upper = -1;
    int distance = PrefixCapOptimizer::impossible;
    Partition band;
};

struct FaceOptimization {
    int objective = PrefixCapOptimizer::impossible;
    Partition band;
    Count blocking_cap_vectors = 0;
};

bool cap_vector_dominates(
    const std::vector<Value> &weaker,
    const std::vector<Value> &stronger) {
    if (weaker.size() != stronger.size()) {
        throw std::logic_error("incomparable prefix-cap dimensions");
    }
    for (std::size_t index = 0; index < weaker.size(); ++index) {
        if (weaker[index] < stronger[index]) {
            return false;
        }
    }
    return true;
}

std::vector<std::vector<Value>> maximal_blocking_cap_vectors(
    const Partition &canonical_band, Value mixed_floor,
    const std::vector<TransitionCheck> &transitions) {
    const PrefixFunction capacity(canonical_band);
    std::vector<Value> initial(canonical_band.size() + 1, 0);
    for (std::size_t count = 0; count < initial.size(); ++count) {
        initial[count] = capacity(static_cast<int>(count));
    }
    std::vector<std::vector<Value>> vectors{initial};
    for (const TransitionCheck &transition : transitions) {
        const int left_rank = transition.left_rows;
        const int right_rank = transition.right_rows;
        const Value left_cap =
            transition.left_required + left_rank * mixed_floor - 1;
        const Value right_cap =
            transition.right_required + right_rank * mixed_floor - 1;
        std::vector<std::vector<Value>> next;
        for (const std::vector<Value> &caps : vectors) {
            if (caps[left_rank] <= left_cap
                || caps[right_rank] <= right_cap) {
                next.push_back(caps);
                continue;
            }
            std::vector<Value> left = caps;
            left[left_rank] = left_cap;
            next.push_back(std::move(left));
            std::vector<Value> right = caps;
            right[right_rank] = right_cap;
            next.push_back(std::move(right));
        }
        std::sort(next.begin(), next.end());
        next.erase(std::unique(next.begin(), next.end()), next.end());
        std::vector<unsigned char> discard(next.size(), 0);
        for (std::size_t first = 0; first < next.size(); ++first) {
            if (discard[first]) {
                continue;
            }
            for (std::size_t second = 0; second < next.size(); ++second) {
                if (first == second || discard[second]) {
                    continue;
                }
                if (cap_vector_dominates(next[first], next[second])) {
                    discard[second] = 1;
                }
            }
        }
        vectors.clear();
        for (std::size_t index = 0; index < next.size(); ++index) {
            if (!discard[index]) {
                vectors.push_back(std::move(next[index]));
            }
        }
    }
    return vectors;
}

FaceOptimization optimize_capacity_face(const CapacityBandSurvey &survey) {
    const std::vector<std::vector<Value>> blocking_caps =
        maximal_blocking_cap_vectors(
            survey.canonical_band, survey.mixed_floor, survey.transitions);
    FaceOptimization result;
    result.blocking_cap_vectors = static_cast<Count>(blocking_caps.size());

    std::vector<Value> canonical_caps(
        survey.canonical_band.size() + 1, 0);
    for (std::size_t count = 0; count < canonical_caps.size(); ++count) {
        canonical_caps[count] = survey.capacity(static_cast<int>(count));
    }
    PrefixCapOptimizer floor_optimizer(
        survey.canonical_band, canonical_caps, 1,
        static_cast<int>(survey.mixed_floor - 1));
    auto [floor_objective, floor_band] = floor_optimizer.run();
    if (floor_objective < result.objective) {
        result.objective = floor_objective;
        result.band = std::move(floor_band);
    }

    for (const std::vector<Value> &caps : blocking_caps) {
        // A rank-zero blocking inequality is 0 <= -1 and therefore
        // impossible; the cap vector records it at index zero only so the
        // disjunctive simplifier can retain a uniform representation.
        if (caps.front() < 0) {
            continue;
        }
        PrefixCapOptimizer optimizer(
            survey.canonical_band, caps,
            static_cast<int>(survey.mixed_floor),
            std::numeric_limits<int>::max());
        auto [objective, band] = optimizer.run();
        if (objective < result.objective) {
            result.objective = objective;
            result.band = std::move(band);
        }
    }
    return result;
}

MinimumBandCertificate optimize_capacity_certificates(int level, bool print) {
    if (level < 1 || level > 6) {
        throw std::invalid_argument("certificate optimization level must lie in [1,6]");
    }
    const Partition parent_profile = singleton_profile(level);
    const PrefixFunction parent(parent_profile);
    const PrefixFunction child(singleton_profile(level - 1));
    const int positive_floor_support = 1 << (level - 1);
    MinimumBandCertificate best;
    int bands = 0;
    int feasible_bands = 0;
    Count cap_vectors = 0;

    for (int upper = 1; upper <= positive_floor_support; ++upper) {
        for (int lower = 0; lower < upper; ++lower) {
            CapacityBandSurvey survey(
                lower, upper, parent_profile, parent, child);
            FaceOptimization face = optimize_capacity_face(survey);
            cap_vectors = checked_count_sum(
                cap_vectors, face.blocking_cap_vectors);
            ++bands;
            if (face.objective != PrefixCapOptimizer::impossible) {
                if ((face.objective & 1) != 0) {
                    throw std::logic_error("certificate objective is not even");
                }
                ++feasible_bands;
                const int distance = face.objective / 2;
                if (distance < best.distance) {
                    best.lower = lower;
                    best.upper = upper;
                    best.distance = distance;
                    best.band = std::move(face.band);
                }
            }
        }
    }
    if (print) {
        bool verified = true;
        if (level == 3) {
            verified = bands == 10 && cap_vectors == 17
                && feasible_bands == 0
                && best.distance == PrefixCapOptimizer::impossible;
        } else if (level == 4) {
            verified = bands == 36 && cap_vectors == 74
                && feasible_bands == 0
                && best.distance == PrefixCapOptimizer::impossible;
        } else if (level == 5) {
            verified = bands == 136 && cap_vectors == 528
                && feasible_bands == 0
                && best.distance == PrefixCapOptimizer::impossible;
        } else if (level == 6) {
            verified = bands == 528 && cap_vectors == 38131
                && feasible_bands == 3 && best.distance == 14
                && best.lower == 15 && best.upper == 30
                && best.band == repeated({{8, 15}});
        }
        if (best.distance != PrefixCapOptimizer::impossible) {
            Partition state(
                parent_profile.begin(), parent_profile.begin() + best.lower);
            state.insert(state.end(), best.band.begin(), best.band.end());
            state.insert(
                state.end(), parent_profile.begin() + best.upper,
                parent_profile.end());
            const Analysis replay = analyze(level, state);
            const bool matching_certificate = std::any_of(
                replay.certificates.begin(), replay.certificates.end(),
                [&](const Certificate &certificate) {
                    return certificate.lower_rank == best.lower
                        && certificate.upper_rank == best.upper;
                });
            verified = verified && replay.majorized && matching_certificate
                && transfer_distance(parent_profile, state) == best.distance;
        }
        std::cout << "MINIMUM_TIGHT_BAND_CERTIFICATE K=" << level
                  << " complete=YES"
                  << " verified=" << (verified ? "YES" : "NO")
                  << " bands=" << bands
                  << " blocking_cap_vectors=" << cap_vectors
                  << " feasible_bands=" << feasible_bands;
        if (best.distance == PrefixCapOptimizer::impossible) {
            std::cout << " minimum_transfer_distance=NONE";
        } else {
            std::cout << " minimum_transfer_distance=" << best.distance
                      << " anchors=(" << best.lower << ',' << best.upper << ')'
                      << " band=" << compact_partition(best.band);
        }
        std::cout << '\n';
        if (!verified) {
            throw std::logic_error("minimum certificate regression mismatch");
        }
    }
    return best;
}

bool optimize_capacity_certificate_cli(int level) {
    const MinimumBandCertificate best =
        optimize_capacity_certificates(level, true);
    if (level <= 5) {
        return best.distance == PrefixCapOptimizer::impossible;
    }
    if (level == 6) {
        return best.distance == 14 && best.lower == 15 && best.upper == 30
            && best.band == repeated({{8, 15}});
    }
    return true;
}

bool optimize_capacity_face_cli(int level, int lower, int upper) {
    if (level < 1 || level > 15) {
        throw std::invalid_argument("certificate face level must lie in [1,15]");
    }
    const Partition parent_profile = singleton_profile(level);
    const int positive_floor_support = 1 << (level - 1);
    if (lower < 0 || upper <= lower || upper > positive_floor_support) {
        throw std::invalid_argument("invalid positive-floor anchor pair");
    }
    const PrefixFunction parent(parent_profile);
    const PrefixFunction child(singleton_profile(level - 1));
    CapacityBandSurvey survey(lower, upper, parent_profile, parent, child);
    FaceOptimization result = optimize_capacity_face(survey);
    bool verified = true;
    Partition state;
    int distance = -1;
    if (result.objective != PrefixCapOptimizer::impossible) {
        if ((result.objective & 1) != 0) {
            throw std::logic_error("certificate objective is not even");
        }
        distance = result.objective / 2;
        state.assign(parent_profile.begin(), parent_profile.begin() + lower);
        state.insert(state.end(), result.band.begin(), result.band.end());
        state.insert(
            state.end(), parent_profile.begin() + upper,
            parent_profile.end());
        const Analysis replay = analyze(level, state);
        verified = replay.majorized
            && transfer_distance(parent_profile, state) == distance
            && std::any_of(
                replay.certificates.begin(), replay.certificates.end(),
                [&](const Certificate &certificate) {
                    return certificate.lower_rank == lower
                        && certificate.upper_rank == upper;
                });
    }
    std::cout << "MINIMUM_TIGHT_BAND_CERTIFICATE_FACE K=" << level
              << " anchors=(" << lower << ',' << upper << ')'
              << " complete=YES verified=" << (verified ? "YES" : "NO")
              << " transitions=" << survey.transitions.size()
              << " blocking_cap_vectors=" << result.blocking_cap_vectors
              << " canonical_band="
              << compact_partition(survey.canonical_band);
    if (result.objective == PrefixCapOptimizer::impossible) {
        std::cout << " minimum_transfer_distance=NONE";
    } else {
        std::cout << " minimum_transfer_distance=" << distance
                  << " band=" << compact_partition(result.band)
                  << " parent=" << compact_partition(state);
    }
    std::cout << '\n';
    return verified;
}

bool scan_capacity_faces_cli(int level, int maximum_transitions) {
    if (level < 1 || level > 15 || maximum_transitions < 0) {
        throw std::invalid_argument("invalid certificate face scan request");
    }
    const Partition parent_profile = singleton_profile(level);
    const PrefixFunction parent(parent_profile);
    const PrefixFunction child(singleton_profile(level - 1));
    const int positive_floor_support = 1 << (level - 1);
    int examined = 0;
    int skipped = 0;
    int feasible = 0;
    Count cap_vectors = 0;
    MinimumBandCertificate best;
    bool verified = true;
    for (int upper = 1; upper <= positive_floor_support; ++upper) {
        for (int lower = 0; lower < upper; ++lower) {
            CapacityBandSurvey survey(
                lower, upper, parent_profile, parent, child);
            if (static_cast<int>(survey.transitions.size())
                > maximum_transitions) {
                ++skipped;
                continue;
            }
            ++examined;
            FaceOptimization result = optimize_capacity_face(survey);
            cap_vectors = checked_count_sum(
                cap_vectors, result.blocking_cap_vectors);
            if (result.objective == PrefixCapOptimizer::impossible) {
                continue;
            }
            if ((result.objective & 1) != 0) {
                throw std::logic_error("certificate objective is not even");
            }
            ++feasible;
            const int distance = result.objective / 2;
            Partition state(
                parent_profile.begin(), parent_profile.begin() + lower);
            state.insert(state.end(), result.band.begin(), result.band.end());
            state.insert(
                state.end(), parent_profile.begin() + upper,
                parent_profile.end());
            const Analysis replay = analyze(level, state);
            const bool face_verified = replay.majorized
                && transfer_distance(parent_profile, state) == distance
                && std::any_of(
                    replay.certificates.begin(), replay.certificates.end(),
                    [&](const Certificate &certificate) {
                        return certificate.lower_rank == lower
                            && certificate.upper_rank == upper;
                    });
            verified = verified && face_verified;
            std::cout << "TIGHT_BAND_CERTIFICATE_FACE_HIT K=" << level
                      << " anchors=(" << lower << ',' << upper << ')'
                      << " transitions=" << survey.transitions.size()
                      << " blocking_cap_vectors="
                      << result.blocking_cap_vectors
                      << " distance=" << distance
                      << " band=" << compact_partition(result.band)
                      << " parent=" << compact_partition(state)
                      << " verified="
                      << (face_verified ? "YES" : "NO") << '\n';
            if (distance < best.distance) {
                best.lower = lower;
                best.upper = upper;
                best.distance = distance;
                best.band = std::move(result.band);
            }
        }
    }
    std::cout << "TIGHT_BAND_CERTIFICATE_FACE_SCAN K=" << level
              << " maximum_transitions=" << maximum_transitions
              << " complete_within_filter=YES"
              << " verified=" << (verified ? "YES" : "NO")
              << " examined_faces=" << examined
              << " skipped_faces=" << skipped
              << " blocking_cap_vectors=" << cap_vectors
              << " feasible_faces=" << feasible;
    if (best.distance == PrefixCapOptimizer::impossible) {
        std::cout << " minimum_transfer_distance=NONE";
    } else {
        std::cout << " minimum_transfer_distance=" << best.distance
                  << " first_minimizing_anchors=(" << best.lower << ','
                  << best.upper << ')'
                  << " first_minimizing_band="
                  << compact_partition(best.band);
    }
    std::cout << '\n';
    return verified;
}

bool survey_dyadic_family(int maximum_level) {
    if (maximum_level < 3 || maximum_level > 15) {
        throw std::invalid_argument("dyadic family maximum K must lie in [3,15]");
    }
    int total_faces = 0;
    int total_hits = 0;
    bool verified = true;
    for (int level = 3; level <= maximum_level; ++level) {
        const Partition parent_profile = singleton_profile(level);
        const PrefixFunction parent(parent_profile);
        const PrefixFunction child(singleton_profile(level - 1));
        int level_hits = 0;
        for (int exponent = 1; exponent <= level - 2; ++exponent) {
            const int half = 1 << exponent;
            const int lower = half - 1;
            const int upper = 2 * half;
            ++total_faces;
            CapacityBandSurvey survey(
                lower, upper, parent_profile, parent, child);
            const Value mass = survey.capacity.mass();
            const Value length = static_cast<Value>(survey.canonical_band.size());
            const Value quotient = mass / length;
            const int remainder = static_cast<int>(mass % length);
            Partition balanced(
                static_cast<std::size_t>(remainder), quotient + 1);
            balanced.insert(
                balanced.end(),
                static_cast<std::size_t>(length - remainder), quotient);

            survey.current = balanced;
            survey.current_prefix.assign(1, 0);
            for (Value value : balanced) {
                survey.current_prefix.push_back(
                    survey.current_prefix.back() + value);
            }
            const bool majorized =
                weakly_majorized(balanced, survey.capacity);
            bool every_transition_blocked = !survey.transitions.empty();
            Value minimum_margin = std::numeric_limits<Value>::max();
            for (const TransitionCheck &transition : survey.transitions) {
                const Value left_capacity =
                    survey.current_prefix[transition.left_rows]
                    - transition.left_rows * survey.mixed_floor;
                const Value right_capacity =
                    survey.current_prefix[transition.right_rows]
                    - transition.right_rows * survey.mixed_floor;
                const Value left_margin =
                    transition.left_required - left_capacity;
                const Value right_margin =
                    transition.right_required - right_capacity;
                every_transition_blocked = every_transition_blocked
                    && (left_margin > 0 || right_margin > 0);
                if (left_margin > 0) {
                    minimum_margin = std::min(minimum_margin, left_margin);
                }
                if (right_margin > 0) {
                    minimum_margin = std::min(minimum_margin, right_margin);
                }
            }
            const bool hit = balanced.back() >= survey.mixed_floor
                && every_transition_blocked;
            verified = verified && majorized
                && survey.transitions.size() == 2
                && hit == survey.certified();
            if (!hit) {
                continue;
            }
            ++level_hits;
            ++total_hits;
            Partition state(
                parent_profile.begin(), parent_profile.begin() + lower);
            state.insert(state.end(), balanced.begin(), balanced.end());
            state.insert(
                state.end(), parent_profile.begin() + upper,
                parent_profile.end());
            std::cout << "DYADIC_TIGHT_BAND_COUNTEREXAMPLE K=" << level
                      << " exponent=" << exponent
                      << " anchors=(" << lower << ',' << upper << ')'
                      << " canonical_band="
                      << compact_partition(survey.canonical_band)
                      << " balanced_band=" << compact_partition(balanced)
                      << " mixed_floor=" << survey.mixed_floor
                      << " blocking_margin=" << minimum_margin
                      << " transfer_distance="
                      << transfer_distance(survey.canonical_band, balanced)
                      << " parent=" << compact_partition(state) << '\n';
        }
        const int expected = level <= 5 ? 0
            : (level <= 8 ? 1 : (level <= 12 ? 2 : 3));
        if (level <= 15) {
            verified = verified && level_hits == expected;
        }
        std::cout << "DYADIC_TIGHT_BAND_LEVEL K=" << level
                  << " faces=" << level - 2
                  << " counterexamples=" << level_hits << '\n';
    }
    std::cout << "DYADIC_TIGHT_BAND_FAMILY maximum_K=" << maximum_level
              << " complete_within_family=YES"
              << " verified=" << (verified ? "YES" : "NO")
              << " faces=" << total_faces
              << " counterexamples=" << total_hits << '\n';
    return verified;
}

bool check_known_certificate(bool print) {
    const Analysis full = analyze(6, transfer_state(14, true));
    const Analysis core = analyze(6, transfer_state(14, false));
    const Certificate *full_primary = primary_certificate(full);
    const Certificate *core_primary = primary_certificate(core);
    bool ok = full.majorized && core.majorized
        && full.certificates.size() == 3 && core.certificates.size() == 3
        && full_primary != nullptr && core_primary != nullptr;
    if (!ok) {
        return false;
    }
    std::vector<std::pair<int, int>> full_anchors;
    std::vector<std::pair<int, int>> core_anchors;
    for (const Certificate &certificate : full.certificates) {
        full_anchors.emplace_back(certificate.lower_rank, certificate.upper_rank);
    }
    for (const Certificate &certificate : core.certificates) {
        core_anchors.emplace_back(certificate.lower_rank, certificate.upper_rank);
    }
    const std::vector<std::pair<int, int>> expected_anchors{
        {15, 30}, {15, 31}, {15, 32}};
    ok = full_anchors == expected_anchors && core_anchors == expected_anchors
        && full_primary->lower_rank == 15 && full_primary->upper_rank == 32
        && full_primary->mixed_floor == 1
        && full_primary->lower_counts == std::vector<int>({7, 8})
        && full_primary->upper_counts == std::vector<int>({16})
        && full_primary->band == repeated({{8, 15}, {7, 2}})
        && full_primary->transitions.size() == 2
        && core_primary->lower_rank == 15 && core_primary->upper_rank == 32;
    if (!ok) {
        return false;
    }
    const TransitionCheck &first = full_primary->transitions[0];
    const TransitionCheck &second = full_primary->transitions[1];
    ok = first.from == 7 && first.to == 16
        && first.mixed_required == 22
        && first.left_rows == 9 && first.left_required == 64
        && first.left_capacity == 63 && first.left_blocked
        && first.right_rows == 8 && first.right_required == 48
        && first.right_capacity == 56 && !first.right_blocked
        && second.from == 8 && second.to == 16
        && second.mixed_required == 22
        && second.left_rows == 8 && second.left_required == 48
        && second.left_capacity == 56 && !second.left_blocked
        && second.right_rows == 9 && second.right_required == 64
        && second.right_capacity == 63 && second.right_blocked;
    if (print && ok) {
        std::cout << "K6_TIGHT_BAND_PRIMARY full_mass_certificates="
                  << full.certificates.size()
                  << " core_certificates=" << core.certificates.size()
                  << " all_anchors=((15,30),(15,31),(15,32))\n";
        print_certificate(*full_primary);
    }
    return ok;
}

bool transfer_path_control() {
    bool ok = true;
    for (int step = 0; step <= 14; ++step) {
        const Analysis analysis = analyze(6, transfer_state(step, false));
        const bool certified = !analysis.certificates.empty();
        const bool expected = step == 14;
        if (!analysis.majorized || certified != expected) {
            ok = false;
        }
        std::cout << "TIGHT_BAND_TRANSFER_PATH step=" << step
                  << " certified=" << (certified ? "YES" : "NO")
                  << " certificates=" << analysis.certificates.size() << '\n';
    }
    return ok;
}

bool low_level_control() {
    const std::array<std::uint64_t, 4> expected{0, 2, 15, 1206};
    std::uint64_t total_states = 0;
    std::uint64_t total_certified = 0;
    bool ok = true;
    for (int level = 1; level <= 3; ++level) {
        Value mass = 1;
        for (int i = 0; i < level; ++i) {
            mass *= 3;
        }
        const PrefixFunction parent(singleton_profile(level));
        std::uint64_t states = 0;
        std::uint64_t certified = 0;
        Partition current;
        enumerate_partitions(mass, mass, current, [&](const Partition &state) {
            if (!weakly_majorized(state, parent)) {
                return;
            }
            ++states;
            if (!analyze(level, state).certificates.empty()) {
                ++certified;
            }
        });
        std::cout << "TIGHT_BAND_LOW_LEVEL K=" << level
                  << " full_mass_majorized=" << states
                  << " certified=" << certified << '\n';
        ok = ok && states == expected[level] && certified == 0;
        total_states += states;
        total_certified += certified;
    }
    std::cout << "TIGHT_BAND_LOW_LEVEL_SUMMARY states=" << total_states
              << " certified=" << total_certified << '\n';
    return ok && total_states == 1223 && total_certified == 0;
}

bool k6_band_face_certificate_survey() {
    const Partition head = repeated(
        {{64, 1}, {63, 1}, {57, 2}, {42, 4}, {22, 7}});
    const Partition canonical_band = repeated({{22, 1}, {7, 16}});
    const Partition expected_hole = repeated({{8, 15}, {7, 2}});
    std::uint64_t states = 0;
    std::uint64_t certified_states = 0;
    std::uint64_t certificate_pairs = 0;
    Partition unique_certified;
    int minimum_distance = std::numeric_limits<int>::max();

    enumerate_dominated_exact_length(canonical_band, [&](const Partition &band) {
        Partition state = head;
        state.insert(state.end(), band.begin(), band.end());
        const Analysis analysis = analyze(6, state);
        if (!analysis.majorized) {
            throw std::logic_error("enumerated K6 band is not parent-majorized");
        }
        ++states;
        if (!analysis.certificates.empty()) {
            ++certified_states;
            certificate_pairs += analysis.certificates.size();
            unique_certified = band;
            minimum_distance = std::min(
                minimum_distance, transfer_distance(canonical_band, band));
        }
    });

    const bool ok = states == 176 && certified_states == 1
        && certificate_pairs == 3 && unique_certified == expected_hole
        && minimum_distance == 14;
    std::cout << "K6_TIGHT_BAND_CERTIFICATE_SURVEY"
              << " complete=" << (ok ? "YES" : "NO")
              << " band=(15,32)"
              << " states=" << states
              << " certified_states=" << certified_states
              << " certificate_pairs=" << certificate_pairs
              << " unique_certified=" << compact_partition(unique_certified)
              << " minimum_transfer_distance="
              << (certified_states == 0 ? -1 : minimum_distance) << '\n';
    return ok;
}

int regression() {
    const Partition expected_g5 = repeated(
        {{32, 1}, {31, 1}, {26, 2}, {16, 4}, {6, 8}, {1, 16}});
    const Partition expected_g6 = repeated(
        {{64, 1}, {63, 1}, {57, 2}, {42, 4}, {22, 8}, {7, 16}, {1, 32}});
    bool ok = singleton_profile(5) == expected_g5
        && singleton_profile(6) == expected_g6;
    ok = check_known_certificate(true) && ok;
    ok = transfer_path_control() && ok;
    ok = low_level_control() && ok;
    ok = count_boundary_space(3) && ok;
    ok = count_boundary_space(4) && ok;
    ok = count_boundary_space(5) && ok;
    ok = optimize_capacity_certificate_cli(3) && ok;
    ok = optimize_capacity_certificate_cli(4) && ok;
    ok = optimize_capacity_certificate_cli(5) && ok;
    ok = optimize_capacity_certificate_cli(6) && ok;
    ok = k6_band_face_certificate_survey() && ok;
    std::cout << "TIGHT_BAND_CAPACITY_REGRESSION verified="
              << (ok ? "YES" : "NO")
              << " implementation=prefix-capacity-inequalities"
              << " split_search=NONE hall_code=NONE cache=NONE\n";
    return ok ? 0 : 1;
}

int check_cli(int argc, char **argv) {
    if (argc < 4) {
        std::cerr << "usage: " << argv[0] << " check K ROW [ROW ...]\n";
        return 64;
    }
    const int level = std::stoi(argv[2]);
    Partition state;
    for (int i = 3; i < argc; ++i) {
        state.push_back(std::stoll(argv[i]));
    }
    const Analysis analysis = analyze(level, state);
    std::cout << "TIGHT_BAND_ANALYSIS K=" << level
              << " parent=" << compact_partition(analysis.state)
              << " majorized=" << (analysis.majorized ? "YES" : "NO")
              << " tight_ranks=" << compact_counts(analysis.tight_ranks)
              << " certificates=" << analysis.certificates.size() << '\n';
    if (const Certificate *primary = primary_certificate(analysis)) {
        print_certificate(*primary);
    }
    return 0;
}

}  // namespace

int main(int argc, char **argv) {
    try {
        if (argc == 1 || std::string(argv[1]) == "regression") {
            return regression();
        }
        if (std::string(argv[1]) == "check") {
            return check_cli(argc, argv);
        }
        if (std::string(argv[1]) == "survey-k6-band15-32") {
            return k6_band_face_certificate_survey() ? 0 : 1;
        }
        if (std::string(argv[1]) == "count-boundary-space") {
            if (argc != 3) {
                std::cerr << "usage: " << argv[0]
                          << " count-boundary-space K\n";
                return 64;
            }
            return count_boundary_space(std::stoi(argv[2])) ? 0 : 1;
        }
        if (std::string(argv[1]) == "survey-capacity-bands") {
            if (argc != 3) {
                std::cerr << "usage: " << argv[0]
                          << " survey-capacity-bands K\n";
                return 64;
            }
            return survey_capacity_bands(std::stoi(argv[2])) ? 0 : 1;
        }
        if (std::string(argv[1]) == "count-transfer-shells") {
            if (argc != 4) {
                std::cerr << "usage: " << argv[0]
                          << " count-transfer-shells K MAX_DISTANCE\n";
                return 64;
            }
            return count_transfer_shells(
                       std::stoi(argv[2]), std::stoi(argv[3]))
                ? 0 : 1;
        }
        if (std::string(argv[1]) == "optimize-capacity-certificate") {
            if (argc != 3) {
                std::cerr << "usage: " << argv[0]
                          << " optimize-capacity-certificate K\n";
                return 64;
            }
            return optimize_capacity_certificate_cli(std::stoi(argv[2]))
                ? 0 : 1;
        }
        if (std::string(argv[1]) == "optimize-capacity-face") {
            if (argc != 5) {
                std::cerr << "usage: " << argv[0]
                          << " optimize-capacity-face K LOWER UPPER\n";
                return 64;
            }
            return optimize_capacity_face_cli(
                       std::stoi(argv[2]), std::stoi(argv[3]),
                       std::stoi(argv[4]))
                ? 0 : 1;
        }
        if (std::string(argv[1]) == "scan-capacity-faces") {
            if (argc != 4) {
                std::cerr << "usage: " << argv[0]
                          << " scan-capacity-faces K MAX_TRANSITIONS\n";
                return 64;
            }
            return scan_capacity_faces_cli(
                       std::stoi(argv[2]), std::stoi(argv[3]))
                ? 0 : 1;
        }
        if (std::string(argv[1]) == "survey-dyadic-family") {
            if (argc != 3) {
                std::cerr << "usage: " << argv[0]
                          << " survey-dyadic-family MAX_K\n";
                return 64;
            }
            return survey_dyadic_family(std::stoi(argv[2])) ? 0 : 1;
        }
        std::cerr << "usage: " << argv[0]
                  << " regression | survey-k6-band15-32"
                  << " | count-boundary-space K | survey-capacity-bands K"
                  << " | count-transfer-shells K MAX_DISTANCE"
                  << " | optimize-capacity-certificate K"
                  << " | optimize-capacity-face K LOWER UPPER"
                  << " | scan-capacity-faces K MAX_TRANSITIONS"
                  << " | survey-dyadic-family MAX_K"
                  << " | check K ROW [ROW ...]\n";
        return 64;
    } catch (const std::exception &error) {
        std::cerr << "error: " << error.what() << '\n';
        return 70;
    }
}
