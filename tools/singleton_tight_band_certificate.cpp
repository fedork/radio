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
// or cache.  It only evaluates the preceding integer inequalities.

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
#include <utility>
#include <vector>

namespace {

using Value = std::int64_t;
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
        std::cerr << "usage: " << argv[0]
                  << " regression | survey-k6-band15-32 | check K ROW [ROW ...]\n";
        return 64;
    } catch (const std::exception &error) {
        std::cerr << "error: " << error.what() << '\n';
        return 70;
    }
}
