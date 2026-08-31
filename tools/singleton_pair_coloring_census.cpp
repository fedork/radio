#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <random>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

// Exhaustive finite laboratory for the Singleton Row-Coloring conjecture, now
// known to fail at K=6.  The lower-level census results remain exact.
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
//
// The transfer-shell modes instead keep exactly 2^k positive rows and full mass.  They stream a
// fixed half-l1 shell around G_k, prove the shell count by memoized suffix DP, and can split its
// deterministic DFS rank interval across independent threads through k=6.

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

// A fixed prefix of a sorted exact-support parent defines a cylinder of completions.  Color the
// fixed rows, then color the unknown suffix alternately.  If r suffix rows of A and s suffix rows
// of B occur in a Hall inequality, their union has r+s entries and hence weighs no more than the
// first r+s rows of the parent suffix.  Dominance, positivity and sortedness give the uniform bound
//
//   U(j) = min(G_k(t+j)-prefix_mass, suffix_mass-(suffix_rows-j), j*maximum).
//
// Thus one finite set of inequalities can certify every completion of the prefix.  This search
// enumerates only the multiplicities of equal fixed rows; the alternating suffix start is tried in
// both colors, so A/B complementation remains a sound normalization of the first fixed block.
struct PrefixCylinderSearch {
    struct Block {
        int value;
        int count;
    };

    const Hall &hall;
    const Sequence &parent_prefix;
    int fixed_rows;
    int fixed_mass;
    int suffix_rows;
    int suffix_mass;
    int suffix_maximum;
    const std::array<std::vector<std::vector<int>>, 2> *exact_tail_bounds;
    std::uint64_t node_limit;
    std::vector<Block> blocks;
    std::uint64_t nodes = 0;
    bool cutoff = false;
    Coloring solution;
    int solution_suffix_start = -1;

    PrefixCylinderSearch(const Sequence &prefix, const Hall &child_hall,
                         const Sequence &canonical_parent_prefix,
                         int remaining_mass, int maximum,
                         int total_parent_rows, std::uint64_t search_node_limit,
                         const std::array<std::vector<std::vector<int>>, 2> *tail_bounds)
        : hall(child_hall), parent_prefix(canonical_parent_prefix),
          fixed_rows(static_cast<int>(prefix.size())),
          fixed_mass(parent_prefix_mass(prefix)),
          suffix_rows(total_parent_rows - fixed_rows), suffix_mass(remaining_mass),
          suffix_maximum(maximum), exact_tail_bounds(tail_bounds),
          node_limit(search_node_limit) {
        for (int value : prefix) {
            if (blocks.empty() || blocks.back().value != value)
                blocks.push_back({value, 1});
            else
                ++blocks.back().count;
        }
    }

    static int parent_prefix_mass(const Sequence &prefix) {
        int result = 0;
        for (int value : prefix) result += value;
        return result;
    }

    int suffix_upper_bound(int count) const {
        if (count == 0) return 0;
        const int dominance =
            parent_prefix[fixed_rows + count] - fixed_mass;
        const int positivity = suffix_mass - (suffix_rows - count);
        const int sortedness = count * suffix_maximum;
        return std::min({dominance, positivity, sortedness});
    }

    bool certifies(const Coloring &current, int suffix_start) const {
        const int suffix_a = (suffix_rows + (suffix_start == 0 ? 1 : 0)) / 2;
        const int suffix_b = suffix_rows - suffix_a;
        const int fixed_a = static_cast<int>(current.a.size());
        const int fixed_b = static_cast<int>(current.b.size());
        for (int p = 0; p <= fixed_a + suffix_a; ++p) {
            const int tail_a = std::max(0, p - fixed_a);
            const int demand_a = current.pa[std::min(p, fixed_a)];
            for (int q = 0; q <= fixed_b + suffix_b; ++q) {
                const int tail_b = std::max(0, q - fixed_b);
                const int demand = demand_a
                    + current.pb[std::min(q, fixed_b)]
                    + (exact_tail_bounds == nullptr
                        ? suffix_upper_bound(tail_a + tail_b)
                        : (*exact_tail_bounds)[suffix_start][tail_a][tail_b]);
                if (demand > hall.capacity(p, q)) return false;
            }
        }
        return true;
    }

    bool dfs(std::size_t block_index, Coloring &current) {
        if (node_limit != 0 && nodes >= node_limit) {
            cutoff = true;
            return false;
        }
        ++nodes;
        if (block_index == blocks.size()) {
            for (int suffix_start = 0; suffix_start < 2; ++suffix_start) {
                if (certifies(current, suffix_start)) {
                    solution = current;
                    solution_suffix_start = suffix_start;
                    return true;
                }
            }
            return false;
        }

        const auto [value, count] = blocks[block_index];
        std::vector<int> choices;
        for (int to_a = 0; to_a <= count; ++to_a) {
            if (block_index == 0 && to_a * 2 < count) continue;
            choices.push_back(to_a);
        }
        std::stable_sort(choices.begin(), choices.end(), [&](int lhs, int rhs) {
            const auto score = [&](int to_a) {
                const int to_b = count - to_a;
                const int difference = (current.total_a + to_a * value)
                    - (current.total_b + to_b * value);
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
            if (node_limit != 0 && nodes >= node_limit) {
                cutoff = true;
                return false;
            }
        }
        return false;
    }
};

bool full_coloring_holds(const Hall &hall, const Coloring &coloring) {
    return newest_inequalities_hold(hall, coloring, 0, 0);
}

bool labelled_coloring_holds(const Hall &hall, const Sequence &values,
                             const std::vector<unsigned char> &in_a,
                             int donor = -1, int recipient = -1) {
    Coloring coloring;
    for (int row = 0; row < static_cast<int>(values.size()); ++row) {
        int value = values[row];
        if (row == donor) --value;
        if (row == recipient) ++value;
        if (in_a[row]) coloring.a.push_back(value);
        else coloring.b.push_back(value);
    }
    std::sort(coloring.a.begin(), coloring.a.end(), std::greater<int>());
    std::sort(coloring.b.begin(), coloring.b.end(), std::greater<int>());
    coloring.pa.assign(1, 0);
    coloring.pb.assign(1, 0);
    for (int value : coloring.a) coloring.pa.push_back(coloring.pa.back() + value);
    for (int value : coloring.b) coloring.pb.push_back(coloring.pb.back() + value);
    return full_coloring_holds(hall, coloring);
}

bool remove_one(Sequence &values, int value) {
    const auto found = std::find(values.begin(), values.end(), value);
    if (found == values.end()) return false;
    values.erase(found);
    return true;
}

// For a fixed labelled Robin--Hood transfer, compute the minimum x-slack among row sets that
// contain the recipient but not the donor.  The donor is normalized to color A.  All unmarked
// zero rows can be omitted: adding one increases a row count without increasing demand, so it
// cannot decrease Hall slack.  A zero recipient remains explicit through recipient_count=1.
struct SeparatorWitness {
    int margin = std::numeric_limits<int>::max();
    int p = -1;
    int q = -1;
    int demand = 0;
    int capacity = 0;
    int minimizers = 0;
};

SeparatorWitness separator_witness(const Hall &hall, const Coloring &coloring,
                                   int donor_value, int recipient_value,
                                   bool recipient_in_a) {
    Sequence pool_a = coloring.a;
    Sequence pool_b = coloring.b;
    if (!remove_one(pool_a, donor_value)) {
        std::cerr << "ADJACENT_FIBER_INTERNAL_ERROR missing donor\n";
        std::exit(1);
    }
    if (recipient_value > 0) {
        Sequence &recipient_pool = recipient_in_a ? pool_a : pool_b;
        if (!remove_one(recipient_pool, recipient_value)) {
            std::cerr << "ADJACENT_FIBER_INTERNAL_ERROR missing recipient\n";
            std::exit(1);
        }
    }

    Sequence prefix_a(1, 0);
    Sequence prefix_b(1, 0);
    for (int value : pool_a) prefix_a.push_back(prefix_a.back() + value);
    for (int value : pool_b) prefix_b.push_back(prefix_b.back() + value);

    const int include_a = recipient_in_a ? 1 : 0;
    const int include_b = recipient_in_a ? 0 : 1;
    const int include_mass_a = recipient_in_a ? recipient_value : 0;
    const int include_mass_b = recipient_in_a ? 0 : recipient_value;
    SeparatorWitness witness;
    for (int p = include_a; p <= include_a + static_cast<int>(pool_a.size()); ++p) {
        const int demand_a = include_mass_a + prefix_a[p - include_a];
        for (int q = include_b; q <= include_b + static_cast<int>(pool_b.size()); ++q) {
            const int demand_b = include_mass_b + prefix_b[q - include_b];
            const int demand = demand_a + demand_b;
            const int capacity = hall.capacity(p, q);
            const int margin = capacity - demand;
            if (margin < witness.margin) {
                witness = {margin, p, q, demand, capacity, 1};
            } else if (margin == witness.margin) {
                ++witness.minimizers;
            }
        }
    }
    return witness;
}

int separator_margin(const Hall &hall, const Coloring &coloring,
                     int donor_value, int recipient_value, bool recipient_in_a) {
    return separator_witness(
        hall, coloring, donor_value, recipient_value, recipient_in_a).margin;
}

bool transferred_coloring_holds(const Hall &hall, const Coloring &coloring,
                                int donor_value, int recipient_value,
                                bool recipient_in_a) {
    Sequence a = coloring.a;
    Sequence b = coloring.b;
    if (!remove_one(a, donor_value)) {
        std::cerr << "ADJACENT_FIBER_INTERNAL_ERROR missing transferred donor\n";
        std::exit(1);
    }
    if (donor_value > 1) a.push_back(donor_value - 1);
    Sequence &recipient_side = recipient_in_a ? a : b;
    if (recipient_value > 0 && !remove_one(recipient_side, recipient_value)) {
        std::cerr << "ADJACENT_FIBER_INTERNAL_ERROR missing transferred recipient\n";
        std::exit(1);
    }
    recipient_side.push_back(recipient_value + 1);
    std::sort(a.begin(), a.end(), std::greater<int>());
    std::sort(b.begin(), b.end(), std::greater<int>());

    Coloring transferred;
    for (int value : a) transferred.push_a(value);
    for (int value : b) transferred.push_b(value);
    return full_coloring_holds(hall, transferred);
}

// Enumerate every coloring up to permutations of unmarked equal rows.  Complementation is
// normalized by forcing the labelled donor into A; the labelled recipient may use either side.
// With stop_at_one=true the search is still an exact test of the Adjacent-Fiber Lemma, but it
// stops once a common coloring (separator margin >=1) is found instead of maximizing larger
// margins that are irrelevant to the lemma.
struct AdjacentFiberSearch {
    const Hall &hall;
    std::vector<GeneralSearch::Block> blocks;
    int donor_value;
    int recipient_value;
    bool stop_at_one;
    int recipient_mode;
    std::uint64_t nodes = 0;
    std::uint64_t complete_colorings = 0;
    int best_margin = std::numeric_limits<int>::min();
    int best_same_margin = std::numeric_limits<int>::min();
    int best_opposite_margin = std::numeric_limits<int>::min();
    Coloring best_coloring;
    bool best_recipient_in_a = false;
    std::function<void(const Coloring &, bool, int)> observer;

    AdjacentFiberSearch(const Sequence &state, const Hall &h, int donor, int recipient,
                        bool stop, int mode = -1)
        : hall(h), donor_value(donor), recipient_value(recipient), stop_at_one(stop),
          recipient_mode(mode) {
        for (int value : state) {
            if (blocks.empty() || blocks.back().value != value)
                blocks.push_back({value, 1});
            else
                ++blocks.back().count;
        }
    }

    bool finish(Coloring &current, bool recipient_in_a) {
        ++complete_colorings;
        const int margin = separator_margin(
            hall, current, donor_value, recipient_value, recipient_in_a);
        const bool transferred_legal = transferred_coloring_holds(
            hall, current, donor_value, recipient_value, recipient_in_a);
        if ((margin >= 1) != transferred_legal) {
            std::cerr << "ADJACENT_FIBER_INTERNAL_ERROR margin mismatch"
                      << " donor=" << donor_value
                      << " recipient=" << recipient_value
                      << " margin=" << margin << '\n';
            std::exit(1);
        }
        if (observer) observer(current, recipient_in_a, margin);
        if (margin > best_margin) {
            best_margin = margin;
            best_coloring = current;
            best_recipient_in_a = recipient_in_a;
        }
        int &oriented_best = recipient_in_a ? best_same_margin : best_opposite_margin;
        oriented_best = std::max(oriented_best, margin);
        return stop_at_one && margin >= 1;
    }

    bool dfs(std::size_t block_index, Coloring &current, int recipient_color) {
        ++nodes;
        if (block_index == blocks.size()) {
            if (recipient_color < 0) {
                // The recipient is one of the padded zero rows, which was not present in blocks.
                if (recipient_mode != 0 && finish(current, true)) return true;
                return recipient_mode != 1 && finish(current, false);
            }
            return finish(current, recipient_color == 1);
        }

        const auto [value, count] = blocks[block_index];
        const bool has_donor = value == donor_value;
        const bool has_recipient = value == recipient_value;
        const int ordinary_count = count - static_cast<int>(has_donor) -
                                   static_cast<int>(has_recipient);
        if (ordinary_count < 0 || (has_donor && has_recipient)) {
            std::cerr << "ADJACENT_FIBER_INTERNAL_ERROR bad marked block\n";
            std::exit(1);
        }

        const int first_recipient_choice = has_recipient && recipient_mode == 0 ? 1 : 0;
        const int recipient_choice_end = has_recipient && recipient_mode == 1 ? 1 :
                                         (has_recipient ? 2 : 1);
        for (int recipient_choice = first_recipient_choice;
             recipient_choice < recipient_choice_end; ++recipient_choice) {
            // Try putting donor and recipient together first; it often certifies the transfer
            // immediately, but both choices are searched when necessary.
            const bool marked_recipient_in_a = has_recipient && recipient_choice == 0;
            for (int ordinary_to_a = 0; ordinary_to_a <= ordinary_count; ++ordinary_to_a) {
                const int to_a = ordinary_to_a + static_cast<int>(has_donor) +
                                 static_cast<int>(marked_recipient_in_a);
                const int to_b = ordinary_count - ordinary_to_a +
                                 static_cast<int>(has_recipient &&
                                                  !marked_recipient_in_a);
                const int old_a = static_cast<int>(current.a.size());
                const int old_b = static_cast<int>(current.b.size());
                for (int i = 0; i < to_a; ++i) current.push_a(value);
                for (int i = 0; i < to_b; ++i) current.push_b(value);
                const bool legal = newest_inequalities_hold(hall, current, old_a, old_b);
                const int next_recipient_color = has_recipient
                    ? static_cast<int>(marked_recipient_in_a)
                    : recipient_color;
                if (legal && dfs(block_index + 1, current, next_recipient_color)) return true;
                for (int i = 0; i < to_a; ++i) current.pop_a();
                for (int i = 0; i < to_b; ++i) current.pop_b();
            }
        }
        return false;
    }

    void run() {
        Coloring current;
        dfs(0, current, -1);
    }
};

// Diagnostic landscape for the corrected plateau-descent proposal.  A fixed-color tight cut
// proves that an orientation-preserving reroute cannot work.  The relevant finite state space is
// therefore the feasible colorings of the original state.  Two normalized colorings have row
// distance d when d rows must change color, minimized over equal-row matchings and global A/B
// complementation.
// This mode asks whether every failed coloring has a distance-one or distance-two neighbor that
// either raises the separating margin or narrows a minimum tight Pascal band.
struct LandscapePoint {
    Coloring coloring;
    bool recipient_in_a = false;
    int margin = 0;
    std::vector<int> signature;
    std::vector<int> opposite_signature;
    std::tuple<int, int, int, int, int> band;
    int cut_p = -1;
    int cut_q = -1;
};

std::vector<int> coloring_signature(const Coloring &coloring,
                                    const std::vector<GeneralSearch::Block> &blocks,
                                    int donor_value, int recipient_value,
                                    bool recipient_in_a) {
    std::vector<int> signature;
    for (const auto &[value, count] : blocks) {
        (void)count;
        int to_a = static_cast<int>(
            std::count(coloring.a.begin(), coloring.a.end(), value));
        if (value == donor_value) --to_a;
        if (value == recipient_value && recipient_in_a) --to_a;
        signature.push_back(to_a);
    }
    signature.push_back(recipient_in_a ? 1 : 0);
    return signature;
}

std::vector<int> unlabelled_coloring_signature(
        const Coloring &coloring, const std::vector<GeneralSearch::Block> &blocks) {
    std::vector<int> signature;
    for (const auto &[value, count] : blocks) {
        (void)count;
        signature.push_back(static_cast<int>(
            std::count(coloring.a.begin(), coloring.a.end(), value)));
    }
    return signature;
}

int signature_distance(const std::vector<int> &lhs, const std::vector<int> &rhs) {
    int distance = 0;
    for (std::size_t i = 0; i < lhs.size(); ++i)
        distance += std::abs(lhs[i] - rhs[i]);
    return distance;
}

int coloring_distance(const LandscapePoint &lhs, const LandscapePoint &rhs) {
    return std::min(signature_distance(lhs.signature, rhs.signature),
                    signature_distance(lhs.signature, rhs.opposite_signature));
}

std::tuple<int, int, int, int, int> band_key(int p, int q, const Sequence &child) {
    int levels = 0;
    int columns = 0;
    int previous = -1;
    for (int column = 1; column <= child.front(); ++column) {
        const int capacity = static_cast<int>(
            std::count_if(child.begin(), child.end(),
                          [&](int value) { return value >= column; }));
        if (p < capacity && capacity < q) {
            ++columns;
            if (capacity != previous) {
                ++levels;
                previous = capacity;
            }
        }
    }
    return {levels, columns, q - p, p, q};
}

LandscapePoint make_landscape_point(const Hall &hall, const Sequence &child,
                                    const std::vector<GeneralSearch::Block> &blocks,
                                    const Coloring &coloring, int donor_value,
                                    int recipient_value, bool recipient_in_a, int margin) {
    LandscapePoint point;
    point.coloring = coloring;
    point.recipient_in_a = recipient_in_a;
    point.margin = margin;
    point.signature = coloring_signature(
        coloring, blocks, donor_value, recipient_value, recipient_in_a);
    if (margin >= 1) {
        point.band = {-1, -1, -1, -1, -1};
        return point;
    }

    Sequence pool_a = coloring.a;
    Sequence pool_b = coloring.b;
    if (!remove_one(pool_a, donor_value)) std::exit(1);
    if (recipient_value > 0) {
        Sequence &recipient_pool = recipient_in_a ? pool_a : pool_b;
        if (!remove_one(recipient_pool, recipient_value)) std::exit(1);
    }
    Sequence prefix_a(1, 0), prefix_b(1, 0);
    for (int value : pool_a) prefix_a.push_back(prefix_a.back() + value);
    for (int value : pool_b) prefix_b.push_back(prefix_b.back() + value);
    const int include_a = recipient_in_a ? 1 : 0;
    const int include_b = recipient_in_a ? 0 : 1;
    const int include_mass_a = recipient_in_a ? recipient_value : 0;
    const int include_mass_b = recipient_in_a ? 0 : recipient_value;
    point.band = {std::numeric_limits<int>::max(), std::numeric_limits<int>::max(),
                  std::numeric_limits<int>::max(), -1, -1};
    for (int p = include_a; p <= include_a + static_cast<int>(pool_a.size()); ++p) {
        const int demand_a = include_mass_a + prefix_a[p - include_a];
        for (int q = include_b; q <= include_b + static_cast<int>(pool_b.size()); ++q) {
            const int demand = demand_a + include_mass_b + prefix_b[q - include_b];
            if (hall.capacity(p, q) - demand != margin) continue;
            const auto candidate = band_key(p, q, child);
            if (candidate < point.band) {
                point.band = candidate;
                point.cut_p = p;
                point.cut_q = q;
            }
        }
    }
    return point;
}

bool landscape_improves(const LandscapePoint &from, const LandscapePoint &to) {
    return to.margin > from.margin || (to.margin == from.margin && to.band < from.band);
}

std::vector<LandscapePoint> enumerate_landscape(const Sequence &state, const Sequence &child,
                                                const Hall &hall, int donor, int recipient) {
    AdjacentFiberSearch search(state, hall, donor, recipient, false);
    std::vector<LandscapePoint> points;
    search.observer = [&](const Coloring &coloring, bool recipient_in_a, int margin) {
        points.push_back(make_landscape_point(
            hall, child, search.blocks, coloring, donor, recipient, recipient_in_a, margin));
    };
    search.run();
    return points;
}

// A Robin--Hood move between partition values does not distinguish equal donor or recipient
// rows, and the pure sides are globally interchangeable.  Collapse marked-row realizations that
// induce the same unordered pair of value multisets, retaining the best donor/recipient identities
// and orientation.  This quotient is the landscape relevant to majorization.
std::vector<LandscapePoint> collapse_unlabelled_landscape(
        const std::vector<LandscapePoint> &raw,
        const std::vector<GeneralSearch::Block> &blocks) {
    std::map<std::vector<int>, LandscapePoint> best;
    for (LandscapePoint point : raw) {
        const std::vector<int> signature =
            unlabelled_coloring_signature(point.coloring, blocks);
        std::vector<int> opposite;
        for (std::size_t i = 0; i < blocks.size(); ++i)
            opposite.push_back(blocks[i].count - signature[i]);
        const std::vector<int> key = std::min(signature, opposite);
        point.signature = signature;
        point.opposite_signature = opposite;
        const auto found = best.find(key);
        if (found == best.end() || landscape_improves(found->second, point))
            best[key] = std::move(point);
    }
    std::vector<LandscapePoint> result;
    for (auto &[signature, point] : best) {
        (void)signature;
        result.push_back(std::move(point));
    }
    return result;
}

struct LandscapeSummary {
    int failed = 0;
    int stuck_one = 0;
    int stuck_two = 0;
    int stuck_swap = 0;
    int no_direct_success_one = 0;
    int no_direct_success_two = 0;
    int no_direct_success_swap = 0;
    int no_direct_success_one_or_swap = 0;
    int first_stuck_one = -1;
    int first_stuck_two = -1;
    int first_stuck_swap = -1;
    int first_no_direct_flip_above_floor = -1;
    int canonical_crossing_swap_failures = 0;
    int first_canonical_crossing_swap_failure = -1;
    int maximum_mass_gain_failures = 0;
    int first_maximum_mass_gain_failure = -1;
    int maximum_mass_gain_bad_ties = 0;
    int first_maximum_mass_gain_bad_tie = -1;
    int nonpositive_maximum_mass_gain = 0;
    int first_nonpositive_maximum_mass_gain = -1;
    int minimum_maximum_mass_gain = std::numeric_limits<int>::max();
    int failed_recipient_side_at_floor = 0;
    int failed_recipient_side_above_floor = 0;
    int no_direct_flip_above_floor = 0;
    int no_direct_swap_at_floor = 0;
    std::vector<int> minimum_success_distances;
    int maximum_minimum_success_distance = -1;
    int maximum_distance_point = -1;
};

bool canonical_crossing_swap_succeeds(const LandscapePoint &point, const Hall &hall,
                                      int donor_value, int recipient_value) {
    if (point.margin >= 1 || point.recipient_in_a || point.cut_p < 0 || point.cut_q < 1)
        return true;
    if (recipient_value == 0) return true;  // This diagnostic only classifies material rows.

    Sequence outside_a = point.coloring.a;
    Sequence inside_b = point.coloring.b;
    if (!remove_one(outside_a, donor_value) ||
        (recipient_value > 0 && !remove_one(inside_b, recipient_value)))
        return false;
    if (point.cut_p > static_cast<int>(outside_a.size()) ||
        point.cut_q - 1 > static_cast<int>(inside_b.size()))
        return false;

    // The separator witness uses the top p unmarked A rows.  Swap the marked recipient
    // itself with the closest smaller non-donor row just outside that prefix.
    outside_a.erase(outside_a.begin(), outside_a.begin() + point.cut_p);
    for (int u : outside_a) {
        if (u >= recipient_value) continue;
        Coloring candidate = point.coloring;
        if (!remove_one(candidate.a, u) ||
            !remove_one(candidate.b, recipient_value))
            return false;
        candidate.a.push_back(recipient_value);
        candidate.b.push_back(u);
        std::sort(candidate.a.begin(), candidate.a.end(), std::greater<int>());
        std::sort(candidate.b.begin(), candidate.b.end(), std::greater<int>());
        if (!full_coloring_holds(hall, candidate)) continue;
        if (transferred_coloring_holds(hall, candidate, donor_value,
                                      recipient_value, true))
            return true;
        break;  // Values are descending, so this was the closest smaller boundary row.
    }

    // The other crossing direction keeps the marked recipient in B: exchange the largest
    // outside A row with the least unmarked-or-marked Y row above it.
    if (outside_a.empty()) return false;
    const int u = outside_a.front();
    inside_b.resize(point.cut_q - 1);
    inside_b.push_back(recipient_value);
    std::sort(inside_b.begin(), inside_b.end(), std::greater<int>());
    for (auto it = inside_b.rbegin(); it != inside_b.rend(); ++it) {
        const int v = *it;
        if (v <= u) continue;
        Coloring candidate = point.coloring;
        if (!remove_one(candidate.a, u) || !remove_one(candidate.b, v)) return false;
        candidate.a.push_back(v);
        candidate.b.push_back(u);
        std::sort(candidate.a.begin(), candidate.a.end(), std::greater<int>());
        std::sort(candidate.b.begin(), candidate.b.end(), std::greater<int>());
        if (!full_coloring_holds(hall, candidate)) return false;
        if (v == recipient_value &&
            transferred_coloring_holds(hall, candidate, donor_value,
                                       recipient_value, true))
            return true;
        if ((v != recipient_value ||
             std::count(candidate.b.begin(), candidate.b.end(), recipient_value) > 0) &&
            transferred_coloring_holds(hall, candidate, donor_value,
                                       recipient_value, false))
            return true;
        return false;  // This is the unique closest-above value block.
    }
    return false;
}

LandscapeSummary summarize_landscape(const std::vector<LandscapePoint> &points,
                                     const Hall &hall, int donor_value,
                                     int recipient_value) {
    LandscapeSummary summary;
    for (std::size_t i = 0; i < points.size(); ++i) {
        if (points[i].margin >= 1) continue;
        ++summary.failed;
        bool improves_one = false;
        bool improves_two = false;
        bool improves_swap = false;
        bool reaches_one = false;
        bool reaches_two = false;
        bool reaches_swap = false;
        int maximum_mass_gain = std::numeric_limits<int>::min();
        bool maximum_mass_gain_reaches_success = false;
        bool maximum_mass_gain_has_failure = false;
        int mass_i = 0;
        int total_mass = 0;
        for (int value : points[i].coloring.a) mass_i += value;
        total_mass = mass_i;
        for (int value : points[i].coloring.b) total_mass += value;
        Sequence separator_b = points[i].coloring.b;
        Sequence outside_a = points[i].coloring.a;
        if (recipient_value > 0 && !remove_one(separator_b, recipient_value)) std::exit(1);
        if (!remove_one(outside_a, donor_value)) std::exit(1);
        if (points[i].cut_q > 0) {
            separator_b.resize(points[i].cut_q - 1);
            if (recipient_value > 0) separator_b.push_back(recipient_value);
        } else {
            separator_b.clear();
        }
        if (points[i].cut_p > static_cast<int>(outside_a.size())) std::exit(1);
        outside_a.erase(outside_a.begin(), outside_a.begin() + points[i].cut_p);
        outside_a.push_back(donor_value);
        const auto crossing_move = [&](const Sequence &candidate_a) {
            std::map<int, int> difference;
            for (int value : candidate_a) ++difference[value];
            for (int value : points[i].coloring.a) --difference[value];
            int added = -1, removed = -1, additions = 0, removals = 0;
            for (const auto &[value, count] : difference) {
                if (count == 1) {
                    added = value;
                    ++additions;
                } else if (count == -1) {
                    removed = value;
                    ++removals;
                } else if (count != 0) {
                    return false;
                }
            }
            if (additions != 1 ||
                std::find(separator_b.begin(), separator_b.end(), added) == separator_b.end())
                return false;
            if (removals == 0) return true;
            return removals == 1 &&
                   std::find(outside_a.begin(), outside_a.end(), removed) != outside_a.end();
        };
        int minimum_success_distance = std::numeric_limits<int>::max();
        for (std::size_t j = 0; j < points.size(); ++j) {
            const int distance = coloring_distance(points[i], points[j]);
            if (points[j].margin >= 1)
                minimum_success_distance = std::min(minimum_success_distance, distance);
            if (distance <= 1) {
                improves_one = improves_one || landscape_improves(points[i], points[j]);
                reaches_one = reaches_one || points[j].margin >= 1;
            }
            if (distance <= 2) {
                improves_two = improves_two || landscape_improves(points[i], points[j]);
                reaches_two = reaches_two || points[j].margin >= 1;
            }
            int rows_i = 0;
            int rows_j = 0;
            int opposite_rows_j = 0;
            for (int count : points[i].signature) rows_i += count;
            for (int count : points[j].signature) rows_j += count;
            for (int count : points[j].opposite_signature) opposite_rows_j += count;
            const bool is_swap =
                (signature_distance(points[i].signature, points[j].signature) == 2 &&
                 rows_i == rows_j) ||
                (signature_distance(points[i].signature, points[j].opposite_signature) == 2 &&
                 rows_i == opposite_rows_j);
            if (is_swap) {
                improves_swap = improves_swap || landscape_improves(points[i], points[j]);
                reaches_swap = reaches_swap || points[j].margin >= 1;
            }

            int mass_j = 0;
            for (int value : points[j].coloring.a) mass_j += value;
            const int direct_distance =
                signature_distance(points[i].signature, points[j].signature);
            const int opposite_distance =
                signature_distance(points[i].signature, points[j].opposite_signature);
            const auto consider_gain = [&](const Sequence &candidate_a,
                                           int candidate_distance, int candidate_rows,
                                           int candidate_mass) {
                const bool one_flip = candidate_distance == 1;
                const bool one_swap = candidate_distance == 2 && rows_i == candidate_rows;
                if ((!one_flip && !one_swap) || !crossing_move(candidate_a)) return;
                const int gain = candidate_mass - mass_i;
                if (gain > maximum_mass_gain) {
                    maximum_mass_gain = gain;
                    maximum_mass_gain_reaches_success = points[j].margin >= 1;
                    maximum_mass_gain_has_failure = points[j].margin < 1;
                } else if (gain == maximum_mass_gain && points[j].margin >= 1) {
                    maximum_mass_gain_reaches_success = true;
                } else if (gain == maximum_mass_gain) {
                    maximum_mass_gain_has_failure = true;
                }
            };
            consider_gain(points[j].coloring.a, direct_distance, rows_j, mass_j);
            consider_gain(points[j].coloring.b, opposite_distance, opposite_rows_j,
                          total_mass - mass_j);
        }
        if (!improves_one) {
            ++summary.stuck_one;
            if (summary.first_stuck_one < 0) summary.first_stuck_one = static_cast<int>(i);
        }
        if (!improves_two) {
            ++summary.stuck_two;
            if (summary.first_stuck_two < 0) summary.first_stuck_two = static_cast<int>(i);
        }
        if (!improves_swap) {
            ++summary.stuck_swap;
            if (summary.first_stuck_swap < 0) summary.first_stuck_swap = static_cast<int>(i);
        }
        summary.no_direct_success_one += !reaches_one;
        summary.no_direct_success_two += !reaches_two;
        summary.no_direct_success_swap += !reaches_swap;
        summary.no_direct_success_one_or_swap += !reaches_one && !reaches_swap;
        const bool canonical_swap = reaches_one || canonical_crossing_swap_succeeds(
            points[i], hall, donor_value, recipient_value);
        if (!canonical_swap) {
            ++summary.canonical_crossing_swap_failures;
            if (summary.first_canonical_crossing_swap_failure < 0)
                summary.first_canonical_crossing_swap_failure = static_cast<int>(i);
        }
        if (recipient_value > 0 && !maximum_mass_gain_reaches_success) {
            ++summary.maximum_mass_gain_failures;
            if (summary.first_maximum_mass_gain_failure < 0)
                summary.first_maximum_mass_gain_failure = static_cast<int>(i);
        }
        if (recipient_value > 0 && maximum_mass_gain_has_failure) {
            ++summary.maximum_mass_gain_bad_ties;
            if (summary.first_maximum_mass_gain_bad_tie < 0)
                summary.first_maximum_mass_gain_bad_tie = static_cast<int>(i);
        }
        if (recipient_value > 0) {
            summary.minimum_maximum_mass_gain =
                std::min(summary.minimum_maximum_mass_gain, maximum_mass_gain);
            if (maximum_mass_gain <= 0) {
                ++summary.nonpositive_maximum_mass_gain;
                if (summary.first_nonpositive_maximum_mass_gain < 0)
                    summary.first_nonpositive_maximum_mass_gain = static_cast<int>(i);
            }
        }
        const int recipient_side_rows = static_cast<int>(points[i].coloring.b.size());
        if (recipient_side_rows == hall.child_rows) {
            ++summary.failed_recipient_side_at_floor;
            summary.no_direct_swap_at_floor += !reaches_swap;
        } else {
            ++summary.failed_recipient_side_above_floor;
            summary.no_direct_flip_above_floor += !reaches_one;
            if (!reaches_one && summary.first_no_direct_flip_above_floor < 0)
                summary.first_no_direct_flip_above_floor = static_cast<int>(i);
        }
        summary.minimum_success_distances.push_back(minimum_success_distance);
        if (minimum_success_distance > summary.maximum_minimum_success_distance) {
            summary.maximum_minimum_success_distance = minimum_success_distance;
            summary.maximum_distance_point = static_cast<int>(i);
        }
    }
    return summary;
}

void inspect_adjacent_fiber_landscape(int k, int donor, int recipient, Sequence state) {
    const Sequence child = singleton_base(k - 1);
    const Hall hall(child);
    std::sort(state.begin(), state.end(), std::greater<int>());
    AdjacentFiberSearch signature_source(state, hall, donor, recipient, false);
    const std::vector<LandscapePoint> raw =
        enumerate_landscape(state, child, hall, donor, recipient);
    const std::vector<LandscapePoint> points =
        collapse_unlabelled_landscape(raw, signature_source.blocks);
    const LandscapeSummary summary = summarize_landscape(
        points, hall, donor, recipient);

    std::cout << "ADJACENT_FIBER_LANDSCAPE k=" << k
              << " state=" << show(state)
              << " donor=" << donor
              << " recipient=" << recipient
              << " feasible_markings=" << raw.size()
              << " feasible_colorings=" << points.size()
              << " failed_colorings=" << summary.failed
              << " stuck_d1=" << summary.stuck_one
              << " stuck_d2=" << summary.stuck_two
              << " stuck_swap=" << summary.stuck_swap
              << " no_direct_success_d1=" << summary.no_direct_success_one
              << " no_direct_success_d2=" << summary.no_direct_success_two
              << " no_direct_success_swap=" << summary.no_direct_success_swap
              << " failed_recipient_floor=" << summary.failed_recipient_side_at_floor
              << " failed_recipient_above_floor="
              << summary.failed_recipient_side_above_floor
              << " no_direct_flip_above_floor=" << summary.no_direct_flip_above_floor
              << " no_direct_swap_at_floor=" << summary.no_direct_swap_at_floor
              << " canonical_crossing_swap_failures="
              << summary.canonical_crossing_swap_failures << '\n';
    for (std::size_t i = 0; i < points.size(); ++i) {
        const auto &[levels, columns, width, p, q] = points[i].band;
        std::cout << "LANDSCAPE_POINT index=" << i
                  << " margin=" << points[i].margin
                  << " recipient_color=" << (points[i].recipient_in_a ? 'A' : 'B')
                  << " band_levels=" << levels
                  << " band_columns=" << columns
                  << " band_width=" << width
                  << " cut_p=" << p
                  << " cut_q=" << q
                  << " A=" << show(points[i].coloring.a)
                  << " B=" << show(points[i].coloring.b) << '\n';
    }
}

// Inspect the exact obstruction created by every recoloring that crosses one selected minimum
// separator.  This is a labelled case mode: it is meant to test the proposed Pascal-band descent,
// not to contribute to the large censuses.  A rank-loss blocker with old color counts (a,b) loses
// exactly the Ferrers columns whose heights lie between the two counts.  We encode that closed
// height interval [lo,hi] as band_key(lo-1,hi+1).
void inspect_boundary_blockers(int k, int donor_value, int recipient_value,
                               int point_index, Sequence state) {
    std::sort(state.begin(), state.end(), std::greater<int>());
    const Sequence child = singleton_base(k - 1);
    const Hall hall(child);
    AdjacentFiberSearch signature_source(state, hall, donor_value, recipient_value, false);
    const std::vector<LandscapePoint> raw =
        enumerate_landscape(state, child, hall, donor_value, recipient_value);
    const std::vector<LandscapePoint> points =
        collapse_unlabelled_landscape(raw, signature_source.blocks);
    if (point_index < 0 || point_index >= static_cast<int>(points.size())) {
        std::cerr << "BOUNDARY_BLOCKERS point index out of range\n";
        std::exit(2);
    }
    const LandscapePoint &point = points[point_index];
    if (point.margin >= 1 || point.cut_p < 0 || point.cut_q < 0) {
        std::cerr << "BOUNDARY_BLOCKERS requires a failed landscape point\n";
        std::exit(2);
    }

    Sequence values;
    std::vector<unsigned char> old_a;
    for (int value : point.coloring.a) {
        values.push_back(value);
        old_a.push_back(1);
    }
    for (int value : point.coloring.b) {
        values.push_back(value);
        old_a.push_back(0);
    }
    const int n = static_cast<int>(values.size());
    if (n > 24) {
        std::cerr << "BOUNDARY_BLOCKERS supports at most 24 material rows\n";
        std::exit(2);
    }
    int donor = -1, recipient = -1;
    for (int row = 0; row < n; ++row) {
        if (donor < 0 && old_a[row] && values[row] == donor_value) donor = row;
        if (recipient < 0 && static_cast<bool>(old_a[row]) == point.recipient_in_a &&
            values[row] == recipient_value)
            recipient = row;
    }
    if (donor < 0 || recipient < 0) {
        std::cerr << "BOUNDARY_BLOCKERS could not label donor/recipient\n";
        std::exit(2);
    }

    const std::uint64_t masks = std::uint64_t{1} << n;
    std::uint64_t dangerous_intersection = masks - 1;
    std::uint64_t dangerous_union = 0;
    int dangerous_tight_sets = 0;
    for (std::uint64_t mask = 1; mask < masks; ++mask) {
        if (((mask >> recipient) & 1U) == 0 || ((mask >> donor) & 1U) != 0) continue;
        int demand = 0, count_a = 0, count_b = 0;
        for (int row = 0; row < n; ++row) {
            if (((mask >> row) & 1U) == 0) continue;
            demand += values[row];
            (old_a[row] ? count_a : count_b)++;
        }
        if (hall.capacity(count_a, count_b) != demand) continue;
        ++dangerous_tight_sets;
        dangerous_intersection &= mask;
        dangerous_union |= mask;
    }
    if (dangerous_tight_sets == 0) {
        std::cerr << "BOUNDARY_BLOCKERS found no dangerous tight set\n";
        std::exit(1);
    }

    const auto print_rows = [&](std::uint64_t mask) {
        std::cout << '{';
        bool first = true;
        for (int row = 0; row < n; ++row) {
            if (((mask >> row) & 1U) == 0) continue;
            if (!first) std::cout << ',';
            first = false;
            std::cout << values[row] << '@' << row << (old_a[row] ? 'A' : 'B');
        }
        std::cout << '}';
    };

    std::vector<unsigned char> in_separator(n, 0);
    in_separator[recipient] = 1;
    int need_a = point.cut_p;
    int need_b = point.cut_q - (point.recipient_in_a ? 0 : 1);
    if (point.recipient_in_a) --need_a;
    for (int row = 0; row < n && need_a > 0; ++row) {
        if (row != donor && old_a[row] && row != recipient) {
            in_separator[row] = 1;
            --need_a;
        }
    }
    for (int row = 0; row < n && need_b > 0; ++row) {
        if (!old_a[row] && row != recipient) {
            in_separator[row] = 1;
            --need_b;
        }
    }
    if (need_a != 0 || need_b != 0) {
        std::cerr << "BOUNDARY_BLOCKERS could not reconstruct separator\n";
        std::exit(2);
    }

    const auto target_band = band_key(point.cut_p, point.cut_q, child);
    const auto print_band = [](const std::tuple<int, int, int, int, int> &band) {
        const auto &[levels, columns, width, p, q] = band;
        std::cout << "levels=" << levels << ",columns=" << columns
                  << ",width=" << width << ",p=" << p << ",q=" << q;
    };
    std::cout << "BOUNDARY_BLOCKERS k=" << k << " state=" << show(state)
              << " donor=" << donor_value << " recipient=" << recipient_value
              << " point=" << point_index << " cut=(" << point.cut_p << ','
              << point.cut_q << ") target_band=";
    print_band(target_band);
    std::cout << " A=" << show(point.coloring.a)
              << " B=" << show(point.coloring.b) << '\n';
    std::cout << "DANGEROUS_LATTICE tight_sets=" << dangerous_tight_sets << " core=";
    print_rows(dangerous_intersection);
    std::cout << " hull=";
    print_rows(dangerous_union);
    std::cout << '\n';

    const auto inspect_move = [&](const char *kind, int u, int v,
                                  const std::vector<unsigned char> &new_a) {
        Coloring candidate;
        for (int row = 0; row < n; ++row) {
            if (new_a[row]) candidate.push_a(values[row]);
            else candidate.push_b(values[row]);
        }
        std::sort(candidate.a.begin(), candidate.a.end(), std::greater<int>());
        std::sort(candidate.b.begin(), candidate.b.end(), std::greater<int>());

        int target_old_a = 0, target_old_b = 0, target_new_a = 0, target_new_b = 0;
        for (int row = 0; row < n; ++row) {
            if (!in_separator[row]) continue;
            (old_a[row] ? target_old_a : target_old_b)++;
            (new_a[row] ? target_new_a : target_new_b)++;
        }
        const int target_gain = hall.capacity(target_new_a, target_new_b) -
                                hall.capacity(target_old_a, target_old_b);

        int minimum_x_margin = 0;
        int violations = 0;
        bool all_blocker_bands_smaller = true;
        bool have_blocker_band = false;
        std::uint64_t blocker_intersection = masks - 1;
        std::uint64_t blocker_union = 0;
        std::tuple<int, int, int, int, int> largest_blocker_band{-1, -1, -1, -1, -1};
        int example_old_a = -1, example_old_b = -1, example_new_a = -1,
            example_new_b = -1, example_old_slack = -1, example_new_margin = -1;
        for (std::uint64_t mask = 1; mask < masks; ++mask) {
            int demand = 0, oa = 0, ob = 0, na = 0, nb = 0;
            for (int row = 0; row < n; ++row) {
                if (((mask >> row) & 1U) == 0) continue;
                demand += values[row];
                (old_a[row] ? oa : ob)++;
                (new_a[row] ? na : nb)++;
            }
            const int new_margin = hall.capacity(na, nb) - demand;
            minimum_x_margin = std::min(minimum_x_margin, new_margin);
            if (new_margin >= 0) continue;
            ++violations;
            blocker_intersection &= mask;
            blocker_union |= mask;
            const int old_slack = hall.capacity(oa, ob) - demand;
            if (old_slack < 0 || oa == na || ob == nb) {
                std::cerr << "BOUNDARY_BLOCKERS inconsistent rank-loss blocker\n";
                std::exit(1);
            }
            const int lo = std::min(oa, ob);
            const int hi = std::max(oa, ob);
            const auto blocker_band = band_key(lo - 1, hi + 1, child);
            all_blocker_bands_smaller &= blocker_band < target_band;
            if (!have_blocker_band || largest_blocker_band < blocker_band) {
                have_blocker_band = true;
                largest_blocker_band = blocker_band;
                example_old_a = oa;
                example_old_b = ob;
                example_new_a = na;
                example_new_b = nb;
                example_old_slack = old_slack;
                example_new_margin = new_margin;
            }
        }

        std::cout << "CROSSING_MOVE kind=" << kind;
        if (u >= 0) std::cout << " u=" << values[u] << "@" << u;
        std::cout << " v=" << values[v] << "@" << v
                  << " target_gain=" << target_gain
                  << " x_margin=" << minimum_x_margin;
        if (violations > 0) {
            std::cout << " blockers=" << violations
                      << " all_blocker_bands_smaller="
                      << (all_blocker_bands_smaller ? "YES" : "NO")
                      << " largest_blocker_band=";
            print_band(largest_blocker_band);
            std::cout << " example_counts=(" << example_old_a << ',' << example_old_b
                      << ")->(" << example_new_a << ',' << example_new_b << ')'
                      << " old_slack=" << example_old_slack
                      << " new_margin=" << example_new_margin;
            std::cout << " blocker_core=";
            print_rows(blocker_intersection);
            std::cout << " blocker_hull=";
            print_rows(blocker_union);
        } else {
            Coloring normalized = candidate;
            bool recipient_in_a = new_a[recipient] != 0;
            if (!new_a[donor]) {
                std::swap(normalized.a, normalized.b);
                recipient_in_a = !recipient_in_a;
            }
            const SeparatorWitness next = separator_witness(
                hall, normalized, donor_value, recipient_value, recipient_in_a);
            std::cout << " next_margin=" << next.margin;
            if (next.margin < 1) {
                const auto next_band = band_key(next.p, next.q, child);
                std::cout << " next_cut=(" << next.p << ',' << next.q << ")"
                          << " next_band=";
                print_band(next_band);
                std::cout << " next_band_smaller=" << (next_band < target_band ? "YES" : "NO");
            } else {
                std::cout << " transfer=SUCCESS";
            }
        }
        std::cout << '\n';
    };

    // The relevant crossing directions move a B-row inside the B-heavy separator to A, either
    // alone or while moving an A-row outside the separator back to B.
    for (int v = 0; v < n; ++v) {
        if (old_a[v] || !in_separator[v]) continue;
        std::vector<unsigned char> flipped = old_a;
        flipped[v] = 1;
        inspect_move("flip", -1, v, flipped);
        for (int u = 0; u < n; ++u) {
            if (!old_a[u] || in_separator[u]) continue;
            std::vector<unsigned char> swapped = old_a;
            swapped[u] = 0;
            swapped[v] = 1;
            inspect_move("swap", u, v, swapped);
        }
    }
}

// Test whether the labelled feasible-coloring family of one fixed state satisfies Bouchet's
// symmetric-exchange axiom.  This is deliberately a case inspector rather than a census: its
// 2^n mask table is useful for small theoretical examples, but is not the representation used by
// the full partition census above.
void inspect_boundary_delta_case(int k, Sequence state) {
    std::sort(state.begin(), state.end(), std::greater<int>());
    const int n = static_cast<int>(state.size());
    if (n > 24) {
        std::cerr << "BOUNDARY_DELTA_CASE supports at most 24 labelled rows\n";
        std::exit(2);
    }
    const Hall hall(singleton_base(k - 1));
    const std::uint64_t mask_count = std::uint64_t{1} << n;
    std::vector<unsigned char> feasible(mask_count, 0);
    std::vector<std::uint64_t> family;
    for (std::uint64_t mask = 0; mask < mask_count; ++mask) {
        Coloring coloring;
        for (int row = 0; row < n; ++row) {
            if ((mask >> row) & 1U)
                coloring.push_a(state[row]);
            else
                coloring.push_b(state[row]);
        }
        if (full_coloring_holds(hall, coloring)) {
            feasible[mask] = 1;
            family.push_back(mask);
        }
    }

    for (std::uint64_t x : family) {
        for (std::uint64_t y : family) {
            std::uint64_t difference = x ^ y;
            for (int e = 0; e < n; ++e) {
                if (((difference >> e) & 1U) == 0) continue;
                bool exchanged = feasible[x ^ (std::uint64_t{1} << e)] != 0;
                for (int f = 0; !exchanged && f < n; ++f) {
                    if (f == e || ((difference >> f) & 1U) == 0) continue;
                    exchanged = feasible[x ^ (std::uint64_t{1} << e) ^
                                           (std::uint64_t{1} << f)] != 0;
                }
                if (!exchanged) {
                    std::cout << "BOUNDARY_DELTA_CASE k=" << k
                              << " state=" << show(state)
                              << " feasible_colorings=" << family.size()
                              << " delta=NO x=" << x << " y=" << y
                              << " e=" << e << '\n';
                    return;
                }
            }
        }
    }
    std::cout << "BOUNDARY_DELTA_CASE k=" << k
              << " state=" << show(state)
              << " feasible_colorings=" << family.size()
              << " delta=YES\n";
}

struct LandscapeCensus {
    int k;
    Sequence parent;
    Sequence child;
    Hall hall;
    Sequence parent_prefix{0};
    int total = 0;
    std::uint64_t state_limit = 0;
    std::uint64_t state_skip = 0;
    std::uint64_t states_seen = 0;
    std::uint64_t states = 0;
    std::uint64_t transfers = 0;
    std::uint64_t feasible_colorings = 0;
    std::uint64_t failed_colorings = 0;
    std::uint64_t stuck_one = 0;
    std::uint64_t stuck_two = 0;
    std::uint64_t stuck_swap = 0;
    std::uint64_t no_direct_success_two = 0;
    std::uint64_t no_direct_success_swap = 0;
    std::uint64_t no_direct_success_one_or_swap = 0;
    std::uint64_t failed_recipient_side_at_floor = 0;
    std::uint64_t failed_recipient_side_above_floor = 0;
    std::uint64_t no_direct_flip_above_floor = 0;
    std::uint64_t no_direct_swap_at_floor = 0;
    std::uint64_t canonical_crossing_swap_failures = 0;
    std::uint64_t maximum_mass_gain_failures = 0;
    std::uint64_t maximum_mass_gain_bad_ties = 0;
    std::uint64_t nonpositive_maximum_mass_gain = 0;
    int minimum_maximum_mass_gain = std::numeric_limits<int>::max();
    std::map<int, std::uint64_t> success_distance_counts;
    int maximum_success_distance = -1;
    Sequence maximum_distance_state;
    int maximum_distance_donor = -1;
    int maximum_distance_recipient = -1;
    LandscapePoint maximum_distance_point;
    Sequence first_stuck_one_state;
    int first_stuck_one_donor = -1;
    int first_stuck_one_recipient = -1;
    LandscapePoint first_stuck_one_point;
    Sequence first_stuck_swap_state;
    int first_stuck_swap_donor = -1;
    int first_stuck_swap_recipient = -1;
    LandscapePoint first_stuck_swap_point;
    Sequence first_stuck_state;
    int first_stuck_donor = -1;
    int first_stuck_recipient = -1;
    LandscapePoint first_stuck_point;
    Sequence first_no_direct_flip_above_floor_state;
    int first_no_direct_flip_above_floor_donor = -1;
    int first_no_direct_flip_above_floor_recipient = -1;
    LandscapePoint first_no_direct_flip_above_floor_point;
    Sequence first_canonical_crossing_swap_failure_state;
    int first_canonical_crossing_swap_failure_donor = -1;
    int first_canonical_crossing_swap_failure_recipient = -1;
    LandscapePoint first_canonical_crossing_swap_failure_point;
    Sequence first_maximum_mass_gain_failure_state;
    int first_maximum_mass_gain_failure_donor = -1;
    int first_maximum_mass_gain_failure_recipient = -1;
    LandscapePoint first_maximum_mass_gain_failure_point;

    LandscapeCensus(int level, std::uint64_t limit, std::uint64_t skip = 0)
        : k(level), parent(singleton_base(level)), child(singleton_base(level - 1)),
          hall(child), state_limit(limit), state_skip(skip) {
        for (int value : parent) {
            total += value;
            parent_prefix.push_back(total);
        }
    }

    int parent_H(int count) const {
        return parent_prefix[std::min(count, static_cast<int>(parent.size()))];
    }

    void inspect_transfer(const Sequence &state, int donor, int recipient) {
        ++transfers;
        const std::vector<LandscapePoint> points =
            enumerate_landscape(state, child, hall, donor, recipient);
        AdjacentFiberSearch signature_source(state, hall, donor, recipient, false);
        const std::vector<LandscapePoint> collapsed =
            collapse_unlabelled_landscape(points, signature_source.blocks);
        const LandscapeSummary summary = summarize_landscape(
            collapsed, hall, donor, recipient);
        feasible_colorings += collapsed.size();
        failed_colorings += summary.failed;
        stuck_one += summary.stuck_one;
        stuck_two += summary.stuck_two;
        stuck_swap += summary.stuck_swap;
        no_direct_success_two += summary.no_direct_success_two;
        no_direct_success_swap += summary.no_direct_success_swap;
        no_direct_success_one_or_swap += summary.no_direct_success_one_or_swap;
        failed_recipient_side_at_floor += summary.failed_recipient_side_at_floor;
        failed_recipient_side_above_floor += summary.failed_recipient_side_above_floor;
        no_direct_flip_above_floor += summary.no_direct_flip_above_floor;
        no_direct_swap_at_floor += summary.no_direct_swap_at_floor;
        canonical_crossing_swap_failures +=
            summary.canonical_crossing_swap_failures;
        maximum_mass_gain_failures += summary.maximum_mass_gain_failures;
        maximum_mass_gain_bad_ties += summary.maximum_mass_gain_bad_ties;
        nonpositive_maximum_mass_gain += summary.nonpositive_maximum_mass_gain;
        minimum_maximum_mass_gain =
            std::min(minimum_maximum_mass_gain, summary.minimum_maximum_mass_gain);
        for (int distance : summary.minimum_success_distances)
            ++success_distance_counts[distance];
        if (summary.maximum_minimum_success_distance > maximum_success_distance) {
            maximum_success_distance = summary.maximum_minimum_success_distance;
            maximum_distance_state = state;
            maximum_distance_donor = donor;
            maximum_distance_recipient = recipient;
            maximum_distance_point = collapsed[summary.maximum_distance_point];
        }
        if (summary.first_stuck_one >= 0 && first_stuck_one_state.empty()) {
            first_stuck_one_state = state;
            first_stuck_one_donor = donor;
            first_stuck_one_recipient = recipient;
            first_stuck_one_point = collapsed[summary.first_stuck_one];
        }
        if (summary.first_stuck_swap >= 0 && first_stuck_swap_state.empty()) {
            first_stuck_swap_state = state;
            first_stuck_swap_donor = donor;
            first_stuck_swap_recipient = recipient;
            first_stuck_swap_point = collapsed[summary.first_stuck_swap];
        }
        if (summary.first_stuck_two >= 0 && first_stuck_state.empty()) {
            first_stuck_state = state;
            first_stuck_donor = donor;
            first_stuck_recipient = recipient;
            first_stuck_point = collapsed[summary.first_stuck_two];
        }
        if (summary.first_no_direct_flip_above_floor >= 0 &&
            first_no_direct_flip_above_floor_state.empty()) {
            first_no_direct_flip_above_floor_state = state;
            first_no_direct_flip_above_floor_donor = donor;
            first_no_direct_flip_above_floor_recipient = recipient;
            first_no_direct_flip_above_floor_point =
                collapsed[summary.first_no_direct_flip_above_floor];
        }
        if (summary.first_canonical_crossing_swap_failure >= 0 &&
            first_canonical_crossing_swap_failure_state.empty()) {
            first_canonical_crossing_swap_failure_state = state;
            first_canonical_crossing_swap_failure_donor = donor;
            first_canonical_crossing_swap_failure_recipient = recipient;
            first_canonical_crossing_swap_failure_point =
                collapsed[summary.first_canonical_crossing_swap_failure];
        }
        if (summary.first_maximum_mass_gain_failure >= 0 &&
            first_maximum_mass_gain_failure_state.empty()) {
            first_maximum_mass_gain_failure_state = state;
            first_maximum_mass_gain_failure_donor = donor;
            first_maximum_mass_gain_failure_recipient = recipient;
            first_maximum_mass_gain_failure_point =
                collapsed[summary.first_maximum_mass_gain_failure];
        }
    }

    void inspect(const Sequence &state) {
        ++states;
        std::vector<int> values;
        for (int value : state)
            if (values.empty() || values.back() != value) values.push_back(value);
        for (int donor : values) {
            if (donor < 2) continue;
            for (int recipient : values)
                if (donor >= recipient + 2) inspect_transfer(state, donor, recipient);
            if (static_cast<int>(state.size()) < total) inspect_transfer(state, donor, 0);
        }
    }

    bool limit_reached() const {
        return state_limit != 0 && states >= state_limit;
    }

    void enumerate(int remaining, int maximum, Sequence &state) {
        if (limit_reached()) return;
        if (remaining == 0) {
            ++states_seen;
            if (states_seen <= state_skip) return;
            inspect(state);
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

    void run() {
        Sequence state;
        enumerate(total, parent.front(), state);
        std::cout << "ADJACENT_FIBER_LANDSCAPE_CENSUS k=" << k
                  << " complete=" << (state_limit == 0 && state_skip == 0 ? "YES" : "NO")
                  << " skipped_states=" << state_skip
                  << " states=" << states
                  << " transfers=" << transfers
                  << " feasible_colorings=" << feasible_colorings
                  << " failed_colorings=" << failed_colorings
                  << " stuck_d1=" << stuck_one
                  << " stuck_d2=" << stuck_two
                  << " stuck_swap=" << stuck_swap
                  << " no_direct_success_d2=" << no_direct_success_two
                  << " no_direct_success_swap=" << no_direct_success_swap
                  << " no_direct_success_one_or_swap="
                  << no_direct_success_one_or_swap
                  << " failed_recipient_floor=" << failed_recipient_side_at_floor
                  << " failed_recipient_above_floor="
                  << failed_recipient_side_above_floor
                  << " no_direct_flip_above_floor=" << no_direct_flip_above_floor
                  << " no_direct_swap_at_floor=" << no_direct_swap_at_floor
                  << " canonical_crossing_swap_failures="
                  << canonical_crossing_swap_failures
                  << " maximum_mass_gain_failures="
                  << maximum_mass_gain_failures
                  << " maximum_mass_gain_bad_ties="
                  << maximum_mass_gain_bad_ties
                  << " nonpositive_maximum_mass_gain="
                  << nonpositive_maximum_mass_gain
                  << " minimum_maximum_mass_gain="
                  << (minimum_maximum_mass_gain == std::numeric_limits<int>::max()
                          ? -1 : minimum_maximum_mass_gain)
                  << '\n';
        for (const auto &[distance, count] : success_distance_counts)
            std::cout << "LANDSCAPE_SUCCESS_DISTANCE distance=" << distance
                      << " failed_colorings=" << count << '\n';
        if (!maximum_distance_state.empty())
            std::cout << "LANDSCAPE_MAX_DISTANCE distance=" << maximum_success_distance
                      << " state=" << show(maximum_distance_state)
                      << " donor=" << maximum_distance_donor
                      << " recipient=" << maximum_distance_recipient
                      << " A=" << show(maximum_distance_point.coloring.a)
                      << " B=" << show(maximum_distance_point.coloring.b) << '\n';
        if (!first_stuck_one_state.empty()) {
            const auto &[levels, columns, width, p, q] = first_stuck_one_point.band;
            std::cout << "LANDSCAPE_D1_COUNTEREXAMPLE state=" << show(first_stuck_one_state)
                      << " donor=" << first_stuck_one_donor
                      << " recipient=" << first_stuck_one_recipient
                      << " margin=" << first_stuck_one_point.margin
                      << " band_levels=" << levels
                      << " band_columns=" << columns
                      << " band_width=" << width
                      << " cut_p=" << p
                      << " cut_q=" << q
                      << " A=" << show(first_stuck_one_point.coloring.a)
                      << " B=" << show(first_stuck_one_point.coloring.b) << '\n';
        }
        if (!first_stuck_swap_state.empty())
            std::cout << "LANDSCAPE_SWAP_COUNTEREXAMPLE state="
                      << show(first_stuck_swap_state)
                      << " donor=" << first_stuck_swap_donor
                      << " recipient=" << first_stuck_swap_recipient
                      << " A=" << show(first_stuck_swap_point.coloring.a)
                      << " B=" << show(first_stuck_swap_point.coloring.b) << '\n';
        if (!first_stuck_state.empty()) {
            const auto &[levels, columns, width, p, q] = first_stuck_point.band;
            std::cout << "LANDSCAPE_D2_COUNTEREXAMPLE state=" << show(first_stuck_state)
                      << " donor=" << first_stuck_donor
                      << " recipient=" << first_stuck_recipient
                      << " margin=" << first_stuck_point.margin
                      << " band_levels=" << levels
                      << " band_columns=" << columns
                      << " band_width=" << width
                      << " cut_p=" << p
                      << " cut_q=" << q
                      << " A=" << show(first_stuck_point.coloring.a)
                      << " B=" << show(first_stuck_point.coloring.b) << '\n';
        }
        if (!first_no_direct_flip_above_floor_state.empty())
            std::cout << "LANDSCAPE_ABOVE_FLOOR_NO_FLIP state="
                      << show(first_no_direct_flip_above_floor_state)
                      << " donor=" << first_no_direct_flip_above_floor_donor
                      << " recipient=" << first_no_direct_flip_above_floor_recipient
                      << " A=" << show(first_no_direct_flip_above_floor_point.coloring.a)
                      << " B=" << show(first_no_direct_flip_above_floor_point.coloring.b)
                      << '\n';
        if (!first_canonical_crossing_swap_failure_state.empty())
            std::cout << "LANDSCAPE_CANONICAL_CROSSING_SWAP_FAILURE state="
                      << show(first_canonical_crossing_swap_failure_state)
                      << " donor=" << first_canonical_crossing_swap_failure_donor
                      << " recipient=" << first_canonical_crossing_swap_failure_recipient
                      << " A="
                      << show(first_canonical_crossing_swap_failure_point.coloring.a)
                      << " B="
                      << show(first_canonical_crossing_swap_failure_point.coloring.b)
                      << '\n';
        if (!first_maximum_mass_gain_failure_state.empty())
            std::cout << "LANDSCAPE_MAXIMUM_MASS_GAIN_FAILURE state="
                      << show(first_maximum_mass_gain_failure_state)
                      << " donor=" << first_maximum_mass_gain_failure_donor
                      << " recipient=" << first_maximum_mass_gain_failure_recipient
                      << " A=" << show(first_maximum_mass_gain_failure_point.coloring.a)
                      << " B=" << show(first_maximum_mass_gain_failure_point.coloring.b)
                      << '\n';
    }
};

void inspect_adjacent_fiber_case(int k, int donor, int recipient, Sequence state) {
    const Sequence parent = singleton_base(k);
    const Hall hall(singleton_base(k - 1));
    std::sort(state.begin(), state.end(), std::greater<int>());
    int total = 0;
    int parent_total = 0;
    int prefix = 0;
    int parent_prefix = 0;
    bool majorized = true;
    for (int value : parent) parent_total += value;
    for (std::size_t i = 0; i < state.size(); ++i) {
        total += state[i];
        prefix += state[i];
        if (i < parent.size()) parent_prefix += parent[i];
        if (prefix > (i < parent.size() ? parent_prefix : parent_total)) majorized = false;
    }
    if (total != parent_total || !majorized || donor < recipient + 2) {
        std::cerr << "usage error: case must be full-mass, majorized, and a Robin--Hood pair\n";
        std::exit(2);
    }

    AdjacentFiberSearch same(state, hall, donor, recipient, false, 1);
    same.run();
    AdjacentFiberSearch opposite(state, hall, donor, recipient, false, 0);
    opposite.run();
    std::cout << "ADJACENT_FIBER_CASE k=" << k
              << " state=" << show(state)
              << " donor=" << donor
              << " recipient=" << recipient << '\n';
    std::cout << "CASE_SAME best_margin=";
    if (same.best_margin == std::numeric_limits<int>::min())
        std::cout << "NO_FEASIBLE_COLORING";
    else
        std::cout << same.best_margin;
    std::cout << " nodes=" << same.nodes
              << " complete_colorings=" << same.complete_colorings;
    if (same.best_margin != std::numeric_limits<int>::min())
        std::cout << " A=" << show(same.best_coloring.a)
                  << " B=" << show(same.best_coloring.b);
    std::cout << '\n';
    std::cout << "CASE_OPPOSITE best_margin=";
    if (opposite.best_margin == std::numeric_limits<int>::min())
        std::cout << "NO_FEASIBLE_COLORING";
    else
        std::cout << opposite.best_margin;
    std::cout << " nodes=" << opposite.nodes
              << " complete_colorings=" << opposite.complete_colorings;
    if (opposite.best_margin != std::numeric_limits<int>::min()) {
        const SeparatorWitness witness = separator_witness(
            hall, opposite.best_coloring, donor, recipient, false);
        std::cout << " cut_p=" << witness.p
                  << " cut_q=" << witness.q
                  << " minimizing_cuts=" << witness.minimizers
                  << " A=" << show(opposite.best_coloring.a)
                  << " B=" << show(opposite.best_coloring.b);
    }
    std::cout << '\n';
}

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

struct AdjacentFiberCensus {
    int k;
    Sequence parent;
    Hall hall;
    Sequence parent_prefix{0};
    int total = 0;
    std::uint64_t state_limit = 0;
    bool exact_margins = false;
    std::uint64_t states = 0;
    std::uint64_t transfers = 0;
    std::uint64_t common = 0;
    std::uint64_t common_with_same_color = 0;
    std::uint64_t nodes = 0;
    std::uint64_t complete_colorings = 0;
    int minimum_best_margin = std::numeric_limits<int>::max();
    int maximum_best_margin = std::numeric_limits<int>::min();
    std::vector<std::uint64_t> margin_counts;
    Sequence worst_state;
    int worst_donor = -1;
    int worst_recipient = -1;
    Coloring worst_coloring;
    bool worst_recipient_in_a = false;
    Sequence first_failure;
    int first_failure_donor = -1;
    int first_failure_recipient = -1;
    int first_failure_margin = 0;
    Sequence first_same_color_failure;
    int first_same_color_donor = -1;
    int first_same_color_recipient = -1;
    int first_same_color_margin = 0;
    Coloring first_same_color_opposite_certificate;
    std::uint64_t hard_transfers = 0;
    std::uint64_t hard_no_feasible_same = 0;
    std::map<int, std::uint64_t> hard_same_margin_counts;
    std::map<int, std::uint64_t> hard_opposite_margin_counts;
    std::map<std::pair<int, int>, std::uint64_t> hard_value_pair_counts;
    std::map<std::pair<int, int>, std::uint64_t> hard_cut_counts;
    std::map<int, std::uint64_t> hard_cut_multiplicity_counts;

    AdjacentFiberCensus(int level, std::uint64_t limit, bool exact)
        : k(level), parent(singleton_base(level)), hall(singleton_base(level - 1)),
          state_limit(limit), exact_margins(exact) {
        for (int value : parent) {
            total += value;
            parent_prefix.push_back(total);
        }
        margin_counts.assign(2 * total + 3, 0);
    }

    int parent_H(int count) const {
        return parent_prefix[std::min(count, static_cast<int>(parent.size()))];
    }

    Sequence transferred_state(const Sequence &state, int donor, int recipient) const {
        Sequence result = state;
        if (!remove_one(result, donor)) {
            std::cerr << "ADJACENT_FIBER_INTERNAL_ERROR state donor\n";
            std::exit(1);
        }
        result.push_back(donor - 1);
        if (recipient > 0) {
            if (!remove_one(result, recipient)) {
                std::cerr << "ADJACENT_FIBER_INTERNAL_ERROR state recipient\n";
                std::exit(1);
            }
        }
        result.push_back(recipient + 1);
        std::sort(result.begin(), result.end(), std::greater<int>());
        return result;
    }

    void verify_transferred_state(const Sequence &state, int donor, int recipient) const {
        const Sequence result = transferred_state(state, donor, recipient);
        int prefix = 0;
        int mass = 0;
        for (std::size_t i = 0; i < result.size(); ++i) {
            prefix += result[i];
            mass += result[i];
            if (prefix > parent_H(static_cast<int>(i) + 1)) {
                std::cerr << "ADJACENT_FIBER_INTERNAL_ERROR transferred majorization\n";
                std::exit(1);
            }
        }
        if (mass != total) {
            std::cerr << "ADJACENT_FIBER_INTERNAL_ERROR transferred mass\n";
            std::exit(1);
        }
    }

    void inspect_transfer(const Sequence &state, int donor, int recipient) {
        ++transfers;
        verify_transferred_state(state, donor, recipient);
        int best_margin = std::numeric_limits<int>::min();
        int best_same_margin = std::numeric_limits<int>::min();
        Coloring best_coloring;
        bool best_recipient_in_a = false;
        if (exact_margins) {
            AdjacentFiberSearch search(state, hall, donor, recipient, false);
            search.run();
            nodes += search.nodes;
            complete_colorings += search.complete_colorings;
            best_margin = search.best_margin;
            best_same_margin = search.best_same_margin;
            best_coloring = search.best_coloring;
            best_recipient_in_a = search.best_recipient_in_a;
        } else {
            AdjacentFiberSearch same(state, hall, donor, recipient, true, 1);
            same.run();
            nodes += same.nodes;
            complete_colorings += same.complete_colorings;
            best_same_margin = same.best_margin;
            if (same.best_margin >= 1) {
                best_margin = same.best_margin;
                best_coloring = same.best_coloring;
                best_recipient_in_a = true;
            } else {
                // Same-color failure is rare.  Exhaust the opposite-color fiber so that the
                // census measures its true best separator margin, rather than only existence.
                AdjacentFiberSearch opposite(state, hall, donor, recipient, false, 0);
                opposite.run();
                nodes += opposite.nodes;
                complete_colorings += opposite.complete_colorings;
                if (same.best_margin >= opposite.best_margin) {
                    best_margin = same.best_margin;
                    best_coloring = same.best_coloring;
                    best_recipient_in_a = true;
                } else {
                    best_margin = opposite.best_margin;
                    best_coloring = opposite.best_coloring;
                    best_recipient_in_a = false;
                }
            }
        }
        if (best_margin == std::numeric_limits<int>::min()) {
            std::cerr << "ADJACENT_FIBER_INTERNAL_ERROR no feasible coloring"
                      << " k=" << k << " state=" << show(state) << '\n';
            std::exit(1);
        }

        const int recorded_margin = exact_margins
            ? best_margin
            : std::min(best_margin, 2);
        if (recorded_margin < -total || recorded_margin > total + 1) {
            std::cerr << "ADJACENT_FIBER_INTERNAL_ERROR margin range\n";
            std::exit(1);
        }
        ++margin_counts[recorded_margin + total];
        maximum_best_margin = std::max(maximum_best_margin, recorded_margin);
        if (recorded_margin < minimum_best_margin) {
            minimum_best_margin = recorded_margin;
            worst_state = state;
            worst_donor = donor;
            worst_recipient = recipient;
            worst_coloring = best_coloring;
            worst_recipient_in_a = best_recipient_in_a;
        }
        if (best_same_margin >= 1) {
            ++common_with_same_color;
        } else {
            ++hard_transfers;
            if (best_same_margin == std::numeric_limits<int>::min())
                ++hard_no_feasible_same;
            ++hard_same_margin_counts[best_same_margin];
            ++hard_opposite_margin_counts[best_margin];
            ++hard_value_pair_counts[{donor, recipient}];
            const SeparatorWitness witness = separator_witness(
                hall, best_coloring, donor, recipient, best_recipient_in_a);
            ++hard_cut_counts[{witness.p, witness.q}];
            ++hard_cut_multiplicity_counts[witness.minimizers];
            std::cout << "HARD_ADJACENT_FIBER state=" << show(state)
                      << " donor=" << donor
                      << " recipient=" << recipient
                      << " best_same_margin=";
            if (best_same_margin == std::numeric_limits<int>::min())
                std::cout << "NO_FEASIBLE_SAME_COLORING";
            else
                std::cout << best_same_margin;
            std::cout << " opposite_margin=" << best_margin
                      << " cut_p=" << witness.p
                      << " cut_q=" << witness.q
                      << " cut_demand=" << witness.demand
                      << " cut_capacity=" << witness.capacity
                      << " minimizing_cuts=" << witness.minimizers
                      << " A=" << show(best_coloring.a)
                      << " B=" << show(best_coloring.b) << '\n';
            if (first_same_color_failure.empty()) {
                first_same_color_failure = state;
                first_same_color_donor = donor;
                first_same_color_recipient = recipient;
                first_same_color_margin = best_same_margin;
                if (best_margin >= 1 && !best_recipient_in_a)
                    first_same_color_opposite_certificate = best_coloring;
            }
        }
        if (best_margin >= 1) {
            ++common;
        } else if (first_failure.empty()) {
            first_failure = state;
            first_failure_donor = donor;
            first_failure_recipient = recipient;
            first_failure_margin = best_margin;
        }
    }

    void inspect(const Sequence &state) {
        ++states;
        std::vector<int> values;
        for (int value : state)
            if (values.empty() || values.back() != value) values.push_back(value);
        for (int donor : values) {
            if (donor < 2) continue;
            for (int recipient : values) {
                if (donor >= recipient + 2)
                    inspect_transfer(state, donor, recipient);
            }
            if (static_cast<int>(state.size()) < total)
                inspect_transfer(state, donor, 0);
        }
    }

    bool limit_reached() const {
        return state_limit != 0 && states >= state_limit;
    }

    void enumerate(int remaining, int maximum, Sequence &state) {
        if (limit_reached()) return;
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
            if (limit_reached()) return;
        }
    }

    void run() {
        Sequence state;
        enumerate(total, parent.front(), state);
        std::cout << "ADJACENT_FIBER_CENSUS k=" << k
                  << " mode=" << (exact_margins ? "exact-margin" : "existence")
                  << " complete=" << (state_limit == 0 ? "YES" : "NO")
                  << " states=" << states
                  << " transfers=" << transfers
                  << " common=" << common
                  << " same_color_certificates=" << common_with_same_color
                  << " nodes=" << nodes
                  << " complete_colorings=" << complete_colorings << '\n';
        if (transfers == 0) return;
        std::cout << "MARGIN_RANGE min=" << minimum_best_margin
                  << " max=" << maximum_best_margin
                  << " capped=" << (exact_margins ? "NO" : "YES") << '\n';
        for (int margin = -total; margin <= total + 1; ++margin) {
            const std::uint64_t count = margin_counts[margin + total];
            if (count == 0) continue;
            std::cout << "MARGIN margin=" << margin << " transfers=" << count << '\n';
        }
        std::cout << "WORST_ADJACENT_FIBER state=" << show(worst_state)
                  << " donor=" << worst_donor
                  << " recipient=" << worst_recipient
                  << " margin=" << minimum_best_margin
                  << " A=" << show(worst_coloring.a)
                  << " B=" << show(worst_coloring.b)
                  << " recipient_color=" << (worst_recipient_in_a ? 'A' : 'B') << '\n';
        if (!first_failure.empty())
            std::cout << "ADJACENT_FIBER_COUNTEREXAMPLE state=" << show(first_failure)
                      << " donor=" << first_failure_donor
                      << " recipient=" << first_failure_recipient
                      << " best_margin=" << first_failure_margin << '\n';
        if (!first_same_color_failure.empty())
        {
            std::cout << "SAME_COLOR_COUNTEREXAMPLE state=" << show(first_same_color_failure)
                      << " donor=" << first_same_color_donor
                      << " recipient=" << first_same_color_recipient
                      << " best_same_margin=";
            if (first_same_color_margin == std::numeric_limits<int>::min())
                std::cout << "NO_FEASIBLE_SAME_COLORING";
            else
                std::cout << first_same_color_margin;
            std::cout << " opposite_A=" << show(first_same_color_opposite_certificate.a)
                      << " opposite_B=" << show(first_same_color_opposite_certificate.b) << '\n';
        }
        if (hard_transfers != 0) {
            std::cout << "HARD_SUMMARY transfers=" << hard_transfers
                      << " no_feasible_same=" << hard_no_feasible_same << '\n';
            for (const auto &[margin, count] : hard_same_margin_counts) {
                std::cout << "HARD_SAME_MARGIN margin=";
                if (margin == std::numeric_limits<int>::min())
                    std::cout << "NO_FEASIBLE_SAME_COLORING";
                else
                    std::cout << margin;
                std::cout << " transfers=" << count << '\n';
            }
            for (const auto &[margin, count] : hard_opposite_margin_counts)
                std::cout << "HARD_OPPOSITE_MARGIN margin=" << margin
                          << " transfers=" << count << '\n';
            for (const auto &[values, count] : hard_value_pair_counts)
                std::cout << "HARD_VALUE_PAIR donor=" << values.first
                          << " recipient=" << values.second
                          << " transfers=" << count << '\n';
            for (const auto &[cut, count] : hard_cut_counts)
                std::cout << "HARD_CUT p=" << cut.first
                          << " q=" << cut.second
                          << " certificates=" << count << '\n';
            for (const auto &[multiplicity, count] : hard_cut_multiplicity_counts)
                std::cout << "HARD_CUT_MULTIPLICITY minimizing_cuts=" << multiplicity
                          << " certificates=" << count << '\n';
        }
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

// A compressed, exactly checked K=19 counterexample to strict alternation in the high-support
// padded regime.  It does not refute the Row-Coloring Lemma: only the deterministic choice that
// gives the larger member of every adjacent pair to A.
void check_padded_alternation_counterexample() {
    constexpr int k = 19;
    constexpr int child_level = k - 1;
    constexpr std::int64_t mass = 387420489;  // 3^18
    constexpr std::int64_t support = 2 * mass;
    constexpr int p = 513;
    constexpr int q = 256;
    constexpr int first_index = 2 * q + 1;
    constexpr int second_index = 2 * p - 1;

    const Sequence child = singleton_base(child_level);
    const Hall hall(child);
    const auto parent_H = [&](int count) -> std::int64_t {
        return static_cast<std::int64_t>(hall.H(count)) +
               hall.H((count + 1) / 2) + hall.H(count / 2);
    };
    const auto U = [&](int count) -> std::int64_t {
        return std::min(parent_H(count), mass + count);
    };

    int switch_index = 0;
    const int parent_rows = 2 * static_cast<int>(child.size());
    for (int i = 1; i <= parent_rows; ++i) {
        if (parent_H(i) >= mass + i) {
            switch_index = i;
            break;
        }
    }
    if (switch_index <= second_index) {
        std::cerr << "PADDED_ALTERNATION_INTERNAL_ERROR bad switch\n";
        std::exit(1);
    }

    const std::int64_t segment_mass = U(second_index) - U(first_index);
    const int pair_count = (second_index - first_index) / 2;
    if (segment_mass % 2 != 0) {
        std::cerr << "PADDED_ALTERNATION_INTERNAL_ERROR odd segment\n";
        std::exit(1);
    }
    const std::int64_t pair_mass = segment_mass / 2;
    const std::int64_t low_value = pair_mass / pair_count;
    const int high_pairs = static_cast<int>(pair_mass - low_value * pair_count);
    if (pair_count <= 0 || high_pairs < 0 || high_pairs > pair_count) {
        std::cerr << "PADDED_ALTERNATION_INTERNAL_ERROR bad smoothing\n";
        std::exit(1);
    }

    std::vector<std::int64_t> head;
    head.reserve(switch_index);
    for (int i = 1; i <= switch_index; ++i) {
        if (i <= first_index || i > second_index) {
            head.push_back(U(i) - U(i - 1));
            continue;
        }
        const int offset = i - first_index - 1;
        head.push_back(low_value + (offset < 2 * high_pairs ? 1 : 0));
    }

    std::int64_t prefix = 0;
    for (int i = 1; i <= switch_index; ++i) {
        if (i > 1 && head[i - 2] < head[i - 1]) {
            std::cerr << "PADDED_ALTERNATION_INTERNAL_ERROR unsorted\n";
            std::exit(1);
        }
        prefix += head[i - 1];
        if (prefix > parent_H(i)) {
            std::cerr << "PADDED_ALTERNATION_INTERNAL_ERROR majorization i=" << i << '\n';
            std::exit(1);
        }
    }
    if (head.empty() || head.back() < 1) {
        std::cerr << "PADDED_ALTERNATION_INTERNAL_ERROR nonpositive head\n";
        std::exit(1);
    }
    for (int i = switch_index + 1; i <= parent_rows; ++i) {
        if (mass + i > parent_H(i)) {
            std::cerr << "PADDED_ALTERNATION_INTERNAL_ERROR tail majorization i=" << i << '\n';
            std::exit(1);
        }
    }
    const std::int64_t total = prefix + support - switch_index;
    if (prefix != mass + switch_index || total != 3 * mass) {
        std::cerr << "PADDED_ALTERNATION_INTERNAL_ERROR mass\n";
        std::exit(1);
    }

    std::int64_t a_prefix = 0;
    std::int64_t b_prefix = 0;
    for (int i = 1; i <= second_index; ++i) {
        if (i % 2 == 1 && (i + 1) / 2 <= p) a_prefix += head[i - 1];
        if (i % 2 == 0 && i / 2 <= q) b_prefix += head[i - 1];
    }
    const std::int64_t lhs = a_prefix + b_prefix;
    const std::int64_t rhs = static_cast<std::int64_t>(hall.H(p)) + hall.H(q) +
                             hall.H(p + q);
    if (lhs <= rhs) {
        std::cerr << "PADDED_ALTERNATION_INTERNAL_ERROR no Hall failure\n";
        std::exit(1);
    }

    std::cout << "PADDED_ALTERNATION_COUNTEREXAMPLE k=" << k
              << " child_mass=" << mass
              << " support=" << support
              << " switch=" << switch_index
              << " p=" << p << " q=" << q
              << " lhs=" << lhs << " rhs=" << rhs
              << " excess=" << lhs - rhs << '\n';
    std::cout << "COMPRESSED_HEAD canonical_rows=1.." << first_index
              << " high_value=" << low_value + 1
              << " high_count=" << 2 * high_pairs
              << " low_value=" << low_value
              << " low_count=" << 2 * (pair_count - high_pairs)
              << " canonical_rows=" << second_index + 1 << ".." << switch_index - 1
              << " adjusted_row=" << head.back()
              << " trailing_ones=" << support - switch_index << '\n';
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

// Falsification probe for the Core--Blocker Escape target beyond the exhaustive K=4 range.
// For one feasible labelled coloring, test random opposite-color Robin--Hood transfers.  Whenever
// the current coloring fails after the transfer, exhaust every positive one-row flip and swap.
// A missing common neighbor would refute both Positive Pascal Crossing and Core--Blocker Escape.
void sample_fixed_positive_crossings(int k, std::uint64_t state_count,
                                     std::uint64_t pairs_per_state, std::uint64_t seed) {
    const Sequence parent = singleton_base(k);
    const Hall hall(singleton_base(k - 1));
    int total = 0;
    for (int value : parent) total += value;
    std::mt19937_64 random(seed);
    Sequence padded(total, 0);
    const auto reset = [&] {
        std::fill(padded.begin(), padded.end(), 0);
        std::copy(parent.begin(), parent.end(), padded.begin());
    };
    reset();
    std::uint64_t walk_left = 0;
    std::uint64_t pairs_tested = 0;
    std::uint64_t failed_colorings = 0;
    std::uint64_t positive_common = 0;

    for (std::uint64_t sample = 0; sample < state_count; ++sample) {
        if (walk_left == 0) {
            reset();
            walk_left = 1 + random() % static_cast<std::uint64_t>(2 * total);
        }
        Sequence state = padded;
        while (!state.empty() && state.back() == 0) state.pop_back();
        GeneralSearch exact(state, hall);
        Coloring current;
        if (!exact.dfs(0, current)) {
            std::cerr << "FIXED_POSITIVE_INTERNAL_ERROR uncolorable sampled state\n";
            std::exit(1);
        }

        Sequence values;
        std::vector<unsigned char> base_a;
        for (int value : exact.solution.a) {
            values.push_back(value);
            base_a.push_back(1);
        }
        for (int value : exact.solution.b) {
            values.push_back(value);
            base_a.push_back(0);
        }
        const int n = static_cast<int>(values.size());
        // The first exact coloring is deliberately balanced and often lies well inside every
        // transfer fiber.  Walk on the feasible-coloring graph to probe boundary colorings.
        for (int step = 0; step < 16 * n; ++step) {
            std::vector<unsigned char> candidate = base_a;
            const int first = static_cast<int>(random() % n);
            if ((random() & 1U) == 0) {
                candidate[first] = !candidate[first];
            } else {
                const int second = static_cast<int>(random() % n);
                if (first == second || candidate[first] == candidate[second]) continue;
                candidate[first] = !candidate[first];
                candidate[second] = !candidate[second];
            }
            if (labelled_coloring_holds(hall, values, candidate))
                base_a = std::move(candidate);
        }
        std::vector<std::pair<int, int>> pairs;
        for (int donor = 0; donor < n; ++donor)
            for (int recipient = 0; recipient < n; ++recipient)
                if (base_a[donor] != base_a[recipient] &&
                    values[donor] >= values[recipient] + 2)
                    pairs.emplace_back(donor, recipient);
        std::shuffle(pairs.begin(), pairs.end(), random);
        if (pairs.size() > pairs_per_state) pairs.resize(pairs_per_state);

        for (const auto [raw_donor, raw_recipient] : pairs) {
            std::vector<unsigned char> old_a = base_a;
            if (!old_a[raw_donor])
                for (unsigned char &side : old_a) side = !side;
            const int donor = raw_donor;
            const int recipient = raw_recipient;
            if (!old_a[donor] || old_a[recipient]) std::exit(1);
            ++pairs_tested;
            if (labelled_coloring_holds(hall, values, old_a, donor, recipient)) continue;
            ++failed_colorings;

            bool repaired = false;
            for (int v = 0; v < n && !repaired; ++v) {
                if (old_a[v] || values[v] <= 0) continue;
                std::vector<unsigned char> candidate = old_a;
                candidate[v] = 1;
                if (labelled_coloring_holds(hall, values, candidate) &&
                    labelled_coloring_holds(hall, values, candidate, donor, recipient)) {
                    repaired = true;
                    break;
                }
                for (int u = 0; u < n && !repaired; ++u) {
                    if (!old_a[u] || values[v] <= values[u]) continue;
                    candidate = old_a;
                    candidate[v] = 1;
                    candidate[u] = 0;
                    if (labelled_coloring_holds(hall, values, candidate) &&
                        labelled_coloring_holds(
                            hall, values, candidate, donor, recipient))
                        repaired = true;
                }
            }
            if (repaired) {
                ++positive_common;
                continue;
            }

            Coloring oriented;
            for (int row = 0; row < n; ++row) {
                if (old_a[row]) oriented.a.push_back(values[row]);
                else oriented.b.push_back(values[row]);
            }
            std::sort(oriented.a.begin(), oriented.a.end(), std::greater<int>());
            std::sort(oriented.b.begin(), oriented.b.end(), std::greater<int>());
            std::cout << "FIXED_POSITIVE_COUNTEREXAMPLE k=" << k
                      << " state=" << show(state)
                      << " donor=" << values[donor]
                      << " recipient=" << values[recipient]
                      << " A=" << show(oriented.a)
                      << " B=" << show(oriented.b) << '\n';
            return;
        }

        std::vector<int> donors;
        for (int row = 0; row < total; ++row)
            if (padded[row] >= 2) donors.push_back(row);
        bool moved = false;
        for (int attempt = 0; attempt < 64 && !moved && !donors.empty(); ++attempt) {
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

    std::cout << "FIXED_POSITIVE_SAMPLE k=" << k
              << " states=" << state_count
              << " pairs_per_state=" << pairs_per_state
              << " seed=" << seed
              << " pairs_tested=" << pairs_tested
              << " failed_colorings=" << failed_colorings
              << " positive_common=" << positive_common << '\n';
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

std::uint64_t expected_exact_support_states(int k) {
    static const std::uint64_t expected[] = {
        0ULL, 1ULL, 4ULL, 160ULL, 408776ULL, 1431800647444ULL};
    return k >= 1 && k < static_cast<int>(sizeof(expected) / sizeof(expected[0]))
        ? expected[k] : 0;
}

struct PrefixCylinderCensus {
    int k;
    Sequence parent;
    Hall hall;
    Sequence parent_prefix{0};
    int rows;
    int mass = 0;
    std::uint64_t generation_node_limit;
    std::uint64_t certificate_node_limit;
    int stride;
    int minimum_prefix;
    int maximum_prefix;
    std::uint64_t requested_skip;
    std::uint64_t remaining_skip;
    std::uint64_t state_limit;
    std::map<std::tuple<int, int, int>, std::uint64_t> completion_memo;
    std::unordered_map<std::uint64_t, int> tail_maximum_memo;
    std::uint64_t tail_maximum_calls = 0;
    std::uint64_t total_states = 0;
    std::uint64_t generation_nodes = 0;
    std::uint64_t certificate_attempts = 0;
    std::uint64_t certificate_nodes = 0;
    std::uint64_t certificate_cutoffs = 0;
    std::uint64_t cylinders = 0;
    std::uint64_t covered_states = 0;
    std::uint64_t tested_leaves = 0;
    std::uint64_t leaf_search_nodes = 0;
    std::uint64_t maximum_leaf_search_nodes = 0;
    std::vector<std::uint64_t> cylinders_by_depth;
    std::vector<std::uint64_t> covered_by_depth;
    Sequence first_cylinder_prefix;
    Coloring first_cylinder_coloring;
    int first_cylinder_suffix_start = -1;
    Sequence hole;
    bool stopped = false;
    bool emit_output = true;
    std::chrono::steady_clock::time_point started =
        std::chrono::steady_clock::now();

    PrefixCylinderCensus(int level, std::uint64_t traversal_limit,
                         std::uint64_t search_limit, int depth_stride,
                         int first_depth, int last_depth, std::uint64_t state_skip,
                         std::uint64_t requested_state_limit)
        : k(level), parent(singleton_base(level)), hall(singleton_base(level - 1)),
          rows(static_cast<int>(parent.size())),
          generation_node_limit(traversal_limit),
          certificate_node_limit(search_limit), stride(depth_stride),
          minimum_prefix(first_depth), maximum_prefix(last_depth),
          requested_skip(state_skip),
          remaining_skip(state_skip), state_limit(requested_state_limit),
          cylinders_by_depth(rows + 1, 0),
          covered_by_depth(rows + 1, 0) {
        for (int value : parent) {
            mass += value;
            parent_prefix.push_back(mass);
        }
    }

    std::uint64_t count_completions(int position, int remaining, int maximum) {
        if (position == rows) return remaining == 0 ? 1 : 0;
        const int slots = rows - position;
        if (remaining < slots || remaining > slots * maximum) return 0;
        const auto key = std::tuple{position, remaining, maximum};
        if (const auto found = completion_memo.find(key);
            found != completion_memo.end()) return found->second;

        const int used_mass = mass - remaining;
        const int smallest = std::max(1, remaining - (slots - 1) * maximum);
        const int greatest = std::min(maximum, remaining - (slots - 1));
        std::uint64_t result = 0;
        for (int value = greatest; value >= smallest; --value) {
            if (used_mass + value > parent_prefix[position + 1]) continue;
            const std::uint64_t add =
                count_completions(position + 1, remaining - value, value);
            if (add > std::numeric_limits<std::uint64_t>::max() - result) {
                std::cerr << "PREFIX_CYLINDER_COUNT_OVERFLOW K=" << k << '\n';
                std::exit(2);
            }
            result += add;
        }
        completion_memo.emplace(key, result);
        return result;
    }

    static std::uint64_t tail_maximum_key(int position, int remaining, int maximum,
                                          int start, int take_a, int take_b) {
        return static_cast<std::uint64_t>(position)
            | (static_cast<std::uint64_t>(remaining) << 6)
            | (static_cast<std::uint64_t>(maximum) << 16)
            | (static_cast<std::uint64_t>(start) << 23)
            | (static_cast<std::uint64_t>(take_a) << 24)
            | (static_cast<std::uint64_t>(take_b) << 30);
    }

    int maximum_selected_tail(int position, int remaining, int maximum,
                              int start, int take_a, int take_b) {
        ++tail_maximum_calls;
        if (position == rows)
            return remaining == 0 && take_a == 0 && take_b == 0
                ? 0 : -1000000;
        const int slots = rows - position;
        if (remaining < slots || remaining > slots * maximum) return -1000000;
        const std::uint64_t key = tail_maximum_key(
            position, remaining, maximum, start, take_a, take_b);
        if (const auto found = tail_maximum_memo.find(key);
            found != tail_maximum_memo.end()) return found->second;

        const int used_mass = mass - remaining;
        const int smallest = std::max(1, remaining - (slots - 1) * maximum);
        const int greatest = std::min(maximum, remaining - (slots - 1));
        int best = -1000000;
        for (int value = greatest; value >= smallest; --value) {
            if (used_mass + value > parent_prefix[position + 1]) continue;
            int next_a = take_a;
            int next_b = take_b;
            int selected = 0;
            if (start == 0 && next_a > 0) {
                selected = value;
                --next_a;
            } else if (start == 1 && next_b > 0) {
                selected = value;
                --next_b;
            }
            const int suffix = maximum_selected_tail(
                position + 1, remaining - value, value,
                1 - start, next_a, next_b);
            if (suffix > -1000000) best = std::max(best, selected + suffix);
        }
        tail_maximum_memo.emplace(key, best);
        return best;
    }

    std::array<std::vector<std::vector<int>>, 2> exact_tail_bounds(
        int position, int remaining, int maximum) {
        std::array<std::vector<std::vector<int>>, 2> result;
        const int suffix_rows = rows - position;
        for (int start = 0; start < 2; ++start) {
            const int rows_a = (suffix_rows + (start == 0 ? 1 : 0)) / 2;
            const int rows_b = suffix_rows - rows_a;
            result[start].assign(rows_a + 1, std::vector<int>(rows_b + 1, 0));
            for (int take_a = 0; take_a <= rows_a; ++take_a) {
                for (int take_b = 0; take_b <= rows_b; ++take_b) {
                    const int bound = maximum_selected_tail(
                        position, remaining, maximum, start, take_a, take_b);
                    if (bound < 0) {
                        std::cerr << "PREFIX_CYLINDER_INVALID_TAIL_BOUND K=" << k
                                  << " position=" << position
                                  << " remaining=" << remaining
                                  << " maximum=" << maximum
                                  << " start=" << start
                                  << " take_a=" << take_a
                                  << " take_b=" << take_b << '\n';
                        std::exit(2);
                    }
                    result[start][take_a][take_b] = bound;
                }
            }
        }
        return result;
    }

    bool should_attempt(int position) const {
        return position >= minimum_prefix && position < rows
            && position <= maximum_prefix
            && (position - minimum_prefix) % stride == 0;
    }

    bool certify(const Sequence &state, int remaining, int maximum,
                 std::uint64_t subtree) {
        ++certificate_attempts;
        const auto tail_bounds = exact_tail_bounds(
            static_cast<int>(state.size()), remaining, maximum);
        PrefixCylinderSearch search(
            state, hall, parent_prefix, remaining, maximum, rows,
            certificate_node_limit, &tail_bounds);
        Coloring current;
        const bool found = search.dfs(0, current);
        certificate_nodes += search.nodes;
        certificate_cutoffs += search.cutoff;
        if (!found) return false;
        const int depth = static_cast<int>(state.size());
        ++cylinders;
        ++cylinders_by_depth[depth];
        covered_states += subtree;
        covered_by_depth[depth] += subtree;
        if (first_cylinder_prefix.empty()) {
            first_cylinder_prefix = state;
            first_cylinder_coloring = search.solution;
            first_cylinder_suffix_start = search.solution_suffix_start;
        }
        return true;
    }

    void inspect_leaf(const Sequence &state) {
        ++tested_leaves;
        GeneralSearch exact(state, hall);
        Coloring current;
        const bool found = exact.dfs(0, current);
        leaf_search_nodes += exact.nodes;
        maximum_leaf_search_nodes = std::max(maximum_leaf_search_nodes, exact.nodes);
        if (!found) hole = state;
    }

    void progress() const {
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        std::cout << "PREFIX_CYLINDER_PROGRESS K=" << k
                  << " generation_nodes=" << generation_nodes
                  << " cylinders=" << cylinders
                  << " covered_states=" << covered_states
                  << " tested_leaves=" << tested_leaves
                  << " certificate_nodes=" << certificate_nodes
                  << " seconds=" << seconds << '\n' << std::flush;
    }

    void enumerate(int position, int remaining, int maximum, Sequence &state) {
        if (stopped || !hole.empty()) return;
        if (generation_node_limit != 0
            && generation_nodes >= generation_node_limit) {
            stopped = true;
            return;
        }
        const std::uint64_t subtree =
            count_completions(position, remaining, maximum);
        if (subtree == 0) return;
        if (remaining_skip != 0 && subtree <= remaining_skip) {
            remaining_skip -= subtree;
            return;
        }
        ++generation_nodes;
        if (emit_output && generation_nodes % 1000000 == 0) progress();

        const std::uint64_t processed = covered_states + tested_leaves;
        if (state_limit != 0 && processed >= state_limit) {
            stopped = true;
            return;
        }

        if (position == rows) {
            inspect_leaf(state);
            return;
        }
        if (should_attempt(position)
            && (state_limit == 0 || subtree <= state_limit - processed)
            && certify(state, remaining, maximum, subtree)) return;

        const int slots = rows - position;
        const int used_mass = mass - remaining;
        const int smallest = std::max(1, remaining - (slots - 1) * maximum);
        const int greatest = std::min(maximum, remaining - (slots - 1));
        for (int value = greatest; value >= smallest; --value) {
            if (used_mass + value > parent_prefix[position + 1]) continue;
            state.push_back(value);
            enumerate(position + 1, remaining - value, value, state);
            state.pop_back();
            if (stopped || !hole.empty()) return;
        }
    }

    int run(bool emit = true) {
        emit_output = emit;
        total_states = count_completions(0, mass, parent.front());
        if (requested_skip > total_states) {
            if (emit_output)
                std::cerr << "PREFIX_CYLINDER_SKIP_OUT_OF_RANGE K=" << k << '\n';
            return 2;
        }
        Sequence state;
        enumerate(0, mass, parent.front(), state);
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        const std::uint64_t skipped_states = requested_skip - remaining_skip;
        const std::uint64_t classified =
            skipped_states + covered_states + tested_leaves;
        const std::uint64_t requested_window = state_limit == 0
            ? total_states - requested_skip
            : std::min(state_limit, total_states - requested_skip);
        const bool window_complete = remaining_skip == 0
            && covered_states + tested_leaves == requested_window;
        const bool complete = hole.empty() && classified == total_states;
        const bool verified_count =
            total_states == expected_exact_support_states(k);
        if (emit_output) std::cout << "PREFIX_CYLINDER_CENSUS K=" << k
                  << " complete=" << (complete ? "YES" : "NO")
                  << " verified_count=" << (verified_count ? "YES" : "NO")
                  << " total_states=" << total_states
                  << " classified_states=" << classified
                  << " unresolved_states=" << total_states - classified
                  << " skipped_states=" << skipped_states
                  << " generation_node_limit=" << generation_node_limit
                  << " certificate_node_limit=" << certificate_node_limit
                  << " stride=" << stride
                  << " minimum_prefix=" << minimum_prefix
                  << " maximum_prefix=" << maximum_prefix
                  << " requested_skip=" << requested_skip
                  << " remaining_skip=" << remaining_skip
                  << " state_limit=" << state_limit
                  << " generation_nodes=" << generation_nodes
                  << " completion_memo_states=" << completion_memo.size()
                  << " tail_maximum_memo_states=" << tail_maximum_memo.size()
                  << " tail_maximum_calls=" << tail_maximum_calls
                  << " certificate_attempts=" << certificate_attempts
                  << " certificate_nodes=" << certificate_nodes
                  << " certificate_cutoffs=" << certificate_cutoffs
                  << " cylinders=" << cylinders
                  << " covered_states=" << covered_states
                  << " tested_leaves=" << tested_leaves
                  << " leaf_search_nodes=" << leaf_search_nodes
                  << " max_leaf_search_nodes=" << maximum_leaf_search_nodes
                  << " first_cylinder_prefix=" << show(first_cylinder_prefix)
                  << " first_cylinder_A=" << show(first_cylinder_coloring.a)
                  << " first_cylinder_B=" << show(first_cylinder_coloring.b)
                  << " first_cylinder_suffix_start="
                  << (first_cylinder_suffix_start < 0 ? "NONE"
                      : first_cylinder_suffix_start == 0 ? "A" : "B")
                  << " hole=" << show(hole)
                  << " seconds=" << seconds << '\n';
        if (emit_output) for (int depth = 0; depth <= rows; ++depth) {
            if (cylinders_by_depth[depth] == 0) continue;
            std::cout << "PREFIX_CYLINDER_DEPTH K=" << k
                      << " depth=" << depth
                      << " cylinders=" << cylinders_by_depth[depth]
                      << " covered_states=" << covered_by_depth[depth] << '\n';
        }
        return verified_count && window_complete && hole.empty() ? 0 : 1;
    }
};

int run_parallel_prefix_cylinder(int k, int workers,
                                 std::uint64_t certificate_node_limit,
                                 int stride, int minimum_prefix,
                                 int maximum_prefix) {
    const std::uint64_t expected = expected_exact_support_states(k);
    std::vector<std::unique_ptr<PrefixCylinderCensus>> censuses;
    std::vector<int> codes(workers, 1);
    censuses.reserve(workers);
    for (int worker = 0; worker < workers; ++worker) {
        const std::uint64_t begin = expected * worker / workers;
        const std::uint64_t end = expected * (worker + 1) / workers;
        censuses.push_back(std::make_unique<PrefixCylinderCensus>(
            k, 0, certificate_node_limit, stride, minimum_prefix,
            maximum_prefix, begin, end - begin));
    }
    const auto started = std::chrono::steady_clock::now();
    std::vector<std::thread> threads;
    for (int worker = 0; worker < workers; ++worker) {
        threads.emplace_back([&, worker] {
            codes[worker] = censuses[worker]->run(false);
        });
    }
    for (auto &thread : threads) thread.join();

    bool verified = true;
    std::uint64_t covered = 0;
    std::uint64_t leaves = 0;
    std::uint64_t generation_nodes = 0;
    std::uint64_t certificate_attempts = 0;
    std::uint64_t certificate_nodes = 0;
    std::uint64_t certificate_cutoffs = 0;
    std::uint64_t cylinders = 0;
    std::uint64_t leaf_search_nodes = 0;
    std::uint64_t tail_memo_states = 0;
    std::uint64_t tail_calls = 0;
    std::vector<std::uint64_t> cylinders_by_depth((1 << k) + 1, 0);
    std::vector<std::uint64_t> covered_by_depth((1 << k) + 1, 0);
    Sequence hole;
    for (int worker = 0; worker < workers; ++worker) {
        const auto &census = *censuses[worker];
        const std::uint64_t begin = expected * worker / workers;
        const std::uint64_t end = expected * (worker + 1) / workers;
        verified = verified && codes[worker] == 0
            && census.covered_states + census.tested_leaves == end - begin;
        covered += census.covered_states;
        leaves += census.tested_leaves;
        generation_nodes += census.generation_nodes;
        certificate_attempts += census.certificate_attempts;
        certificate_nodes += census.certificate_nodes;
        certificate_cutoffs += census.certificate_cutoffs;
        cylinders += census.cylinders;
        leaf_search_nodes += census.leaf_search_nodes;
        tail_memo_states += census.tail_maximum_memo.size();
        tail_calls += census.tail_maximum_calls;
        for (int depth = 0; depth <= (1 << k); ++depth) {
            cylinders_by_depth[depth] += census.cylinders_by_depth[depth];
            covered_by_depth[depth] += census.covered_by_depth[depth];
        }
        if (hole.empty() && !census.hole.empty()) hole = census.hole;
    }
    verified = verified && covered + leaves == expected && hole.empty();
    const double seconds = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - started).count();
    std::cout << "PREFIX_CYLINDER_PARALLEL_CENSUS K=" << k
              << " workers=" << workers
              << " complete=" << (verified ? "YES" : "NO")
              << " verified=" << (verified ? "YES" : "NO")
              << " total_states=" << expected
              << " covered_states=" << covered
              << " tested_leaves=" << leaves
              << " generation_nodes=" << generation_nodes
              << " certificate_node_limit=" << certificate_node_limit
              << " stride=" << stride
              << " minimum_prefix=" << minimum_prefix
              << " maximum_prefix=" << maximum_prefix
              << " certificate_attempts=" << certificate_attempts
              << " certificate_nodes=" << certificate_nodes
              << " certificate_cutoffs=" << certificate_cutoffs
              << " cylinders=" << cylinders
              << " leaf_search_nodes=" << leaf_search_nodes
              << " tail_maximum_memo_states=" << tail_memo_states
              << " tail_maximum_calls=" << tail_calls
              << " hole=" << show(hole)
              << " seconds=" << seconds << '\n';
    for (int depth = 0; depth <= (1 << k); ++depth) {
        if (cylinders_by_depth[depth] == 0) continue;
        std::cout << "PREFIX_CYLINDER_PARALLEL_DEPTH K=" << k
                  << " depth=" << depth
                  << " cylinders=" << cylinders_by_depth[depth]
                  << " covered_states=" << covered_by_depth[depth] << '\n';
    }
    return verified ? 0 : 1;
}

// Exact-support shell d consists of the positive, sorted, full-mass parents
// majorized by G_k whose l1 distance from G_k is 2d.  These constants come
// from the independent TransferShellCounter in singleton_tight_band_certificate.cpp;
// count_completions below rederives every requested shell before searching it.
std::uint64_t expected_transfer_shell_states(int k, int distance) {
    static const std::uint64_t k3[] = {
        1, 5, 18, 35, 42, 31, 17, 8, 3};
    static const std::uint64_t k5[] = {
        1ULL, 14ULL, 126ULL, 834ULL, 4477ULL, 20144ULL,
        78781ULL, 273442ULL, 858985ULL, 2469117ULL, 6558361ULL, 16200491ULL,
        37429391ULL, 81188327ULL, 165999532ULL, 320843757ULL, 587942811ULL,
        1023918872ULL, 1699183407ULL, 2692985809ULL, 4086409432ULL,
        5950391300ULL, 8334519115ULL, 11254380239ULL, 14684852257ULL,
        18555352471ULL, 22756429686ULL, 27148398851ULL, 31574112080ULL,
        35878570360ULL, 39916993000ULL, 43571583709ULL, 46744059429ULL,
        49376636593ULL, 51420524194ULL, 52880748963ULL, 53745593203ULL,
        54065429952ULL, 53857973018ULL, 53212218344ULL, 52140274911ULL,
        50753203580ULL, 49060823731ULL, 47173259713ULL, 45078740371ULL,
        42889858325ULL, 40578828821ULL, 38256071777ULL, 35874174720ULL,
        33533057841ULL, 31188274095ULL, 28929327187ULL, 26693928498ULL,
        24573785473ULL, 22500860510ULL, 20558994060ULL, 18677652606ULL,
        16932741205ULL, 15255341716ULL, 13723756614ULL, 12253609502ULL,
        10927611075ULL, 9670811959ULL, 8551563140ULL, 7494049217ULL,
        6565123892ULL, 5695954825ULL, 4947174263ULL, 4242868822ULL,
        3645857210ULL, 3095504680ULL, 2634223688ULL, 2208828512ULL,
        1858871006ULL, 1540051309ULL, 1284845425ULL, 1046487650ULL,
        860972887ULL, 693964335ULL, 564408359ULL, 447749401ULL,
        359030917ULL, 280387849ULL, 223571957ULL, 169330196ULL,
        132147376ULL, 99587258ULL, 76701609ULL, 56541455ULL, 42694910ULL,
        30749769ULL, 23424567ULL, 15689003ULL, 11487092ULL, 7902102ULL,
        5687419ULL, 3787493ULL, 2639954ULL, 1682468ULL, 1247487ULL,
        637940ULL, 431663ULL, 262492ULL, 172914ULL, 100063ULL, 62130ULL,
        32144ULL, 24989ULL, 5460ULL, 3300ULL, 1736ULL, 1005ULL, 492ULL,
        262ULL, 105ULL, 82ULL};
    static const std::uint64_t k6[] = {
        1ULL, 20ULL, 240ULL, 2076ULL, 14449ULL, 84716ULL,
        433710ULL, 1980479ULL, 8219313ULL, 31395729ULL,
        111555624ULL, 371716237ULL, 1169629534ULL,
        3494418291ULL, 9960648265ULL};
    if (k == 3 && distance >= 0
        && distance < static_cast<int>(sizeof(k3) / sizeof(k3[0]))) {
        return k3[distance];
    }
    if (k == 5 && distance >= 0
        && distance < static_cast<int>(sizeof(k5) / sizeof(k5[0]))) {
        return k5[distance];
    }
    if (k == 6 && distance >= 0
        && distance < static_cast<int>(sizeof(k6) / sizeof(k6[0]))) {
        return k6[distance];
    }
    return 0;
}

struct TransferShellCensus {
    int k;
    int distance;
    Sequence parent;
    Hall hall;
    Sequence parent_prefix{0};
    Sequence parent_suffix;
    int rows = 0;
    int mass = 0;
    int target_l1 = 0;
    std::uint64_t limit = 0;
    std::uint64_t skip = 0;
    std::uint64_t remaining_skip = 0;
    std::uint64_t seen = 0;
    std::uint64_t tested = 0;
    std::uint64_t lookahead_ok = 0;
    std::uint64_t exact_fallbacks = 0;
    std::uint64_t exact_nodes = 0;
    std::uint64_t maximum_exact_nodes = 0;
    Sequence first_lookahead_failure;
    Sequence hole;
    std::unordered_map<std::uint64_t, std::uint64_t> completion_memo;
    bool emit_output = true;
    bool emit_oracle_queries = false;
    std::chrono::steady_clock::time_point started =
        std::chrono::steady_clock::now();

    TransferShellCensus(
        int level, int transfer_distance, std::uint64_t state_limit,
        std::uint64_t state_skip)
        : k(level), distance(transfer_distance), parent(singleton_base(level)),
          hall(singleton_base(level - 1)), rows(static_cast<int>(parent.size())),
          target_l1(2 * transfer_distance), limit(state_limit), skip(state_skip),
          remaining_skip(state_skip) {
        for (int value : parent) {
            mass += value;
            parent_prefix.push_back(mass);
        }
        parent_suffix.assign(rows + 1, 0);
        for (int index = rows - 1; index >= 0; --index)
            parent_suffix[index] = parent_suffix[index + 1] + parent[index];
    }

    bool done() const {
        return !hole.empty() || (limit != 0 && tested >= limit);
    }

    std::uint64_t completion_key(
        int position, int remaining, int maximum, int used_l1) const {
        return static_cast<std::uint64_t>(position)
            | (static_cast<std::uint64_t>(remaining) << 7)
            | (static_cast<std::uint64_t>(maximum) << 17)
            | (static_cast<std::uint64_t>(used_l1) << 24);
    }

    std::uint64_t count_completions(
        int position, int remaining, int maximum, int used_l1) {
        if (position == rows)
            return remaining == 0 && used_l1 == target_l1 ? 1 : 0;
        const int slots = rows - position;
        if (remaining < slots || remaining > slots * maximum
            || used_l1 > target_l1) return 0;
        const int remaining_difference = remaining - parent_suffix[position];
        const int available_l1 = target_l1 - used_l1;
        if (remaining_difference < 0 || remaining_difference > available_l1
            || ((available_l1 - remaining_difference) & 1) != 0) return 0;

        // Once the sorted prefix is fixed, these four scalars contain every
        // constraint on the suffix: mass, order, dominance rank and l1 use.
        const std::uint64_t key =
            completion_key(position, remaining, maximum, used_l1);
        if (const auto found = completion_memo.find(key);
            found != completion_memo.end()) return found->second;

        const int canonical_value = parent[position];
        const int smallest = std::max(1, canonical_value - available_l1);
        const int greatest = std::min(
            {maximum, remaining - (slots - 1),
             canonical_value + available_l1});
        const int used_mass = mass - remaining;
        std::uint64_t result = 0;
        for (int value = greatest; value >= smallest; --value) {
            if (used_mass + value > parent_prefix[position + 1]) continue;
            const int new_l1 = used_l1 + std::abs(value - canonical_value);
            if (new_l1 > target_l1) continue;
            const std::uint64_t add = count_completions(
                position + 1, remaining - value, value, new_l1);
            if (add > std::numeric_limits<std::uint64_t>::max() - result) {
                std::cerr << "TRANSFER_SHELL_COUNT_OVERFLOW K=" << k
                          << " distance=" << distance << '\n';
                std::exit(2);
            }
            result += add;
        }
        completion_memo.emplace(key, result);
        return result;
    }

    void inspect(const Sequence &state) {
        ++tested;
        if (emit_oracle_queries) {
            std::cout << k;
            for (int value : state) std::cout << ' ' << value << " 1";
            std::cout << '\n';
            return;
        }
        if (greedy_block_coloring(state, hall, true)) {
            ++lookahead_ok;
        } else {
            ++exact_fallbacks;
            if (first_lookahead_failure.empty()) first_lookahead_failure = state;
            GeneralSearch exact(state, hall);
            Coloring current;
            const bool found = exact.dfs(0, current);
            exact_nodes += exact.nodes;
            maximum_exact_nodes = std::max(maximum_exact_nodes, exact.nodes);
            if (!found) hole = state;
        }
        if (emit_output && tested % 1000000 == 0) {
            const double seconds = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - started).count();
            std::cout << "TRANSFER_SHELL_PROGRESS K=" << k
                      << " distance=" << distance
                      << " seen=" << seen
                      << " tested=" << tested
                      << " lookahead_failures=" << exact_fallbacks
                      << " exact_nodes=" << exact_nodes
                      << " seconds=" << seconds << '\n' << std::flush;
        }
    }

    void enumerate(
        int position, int remaining, int maximum, int used_l1,
        Sequence &state) {
        if (done()) return;
        const std::uint64_t subtree =
            count_completions(position, remaining, maximum, used_l1);
        if (subtree == 0) return;
        if (remaining_skip != 0) {
            if (subtree <= remaining_skip) {
                remaining_skip -= subtree;
                seen += subtree;
                return;
            }
        }
        if (position == rows) {
            if (remaining == 0 && used_l1 == target_l1) {
                ++seen;
                inspect(state);
            }
            return;
        }
        const int slots = rows - position;
        if (remaining < slots || remaining > slots * maximum
            || used_l1 > target_l1) return;
        const int remaining_difference = remaining - parent_suffix[position];
        const int available_l1 = target_l1 - used_l1;
        if (remaining_difference < 0 || remaining_difference > available_l1
            || ((available_l1 - remaining_difference) & 1) != 0) return;

        const int canonical_value = parent[position];
        const int smallest = std::max(1, canonical_value - available_l1);
        const int greatest = std::min(
            {maximum, remaining - (slots - 1),
             canonical_value + available_l1});
        const int used_mass = mass - remaining;
        for (int value = greatest; value >= smallest; --value) {
            if (used_mass + value > parent_prefix[position + 1]) continue;
            const int new_l1 = used_l1 + std::abs(value - canonical_value);
            if (new_l1 > target_l1) continue;
            state.push_back(value);
            enumerate(
                position + 1, remaining - value, value, new_l1, state);
            state.pop_back();
            if (done()) return;
        }
    }

    int run(bool emit = true) {
        emit_output = emit;
        const std::uint64_t counted =
            count_completions(0, mass, parent.front(), 0);
        Sequence state;
        enumerate(0, mass, parent.front(), 0, state);
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        const std::uint64_t expected =
            expected_transfer_shell_states(k, distance);
        const bool complete = limit == 0 && skip == 0 && hole.empty();
        const bool window_complete = remaining_skip == 0
            && (limit == 0 || tested == limit || seen == expected);
        const bool verified = hole.empty() && expected != 0
            && counted == expected && window_complete
            && (!complete || seen == expected);
        if (emit_output)
            std::cout << "TRANSFER_SHELL_COLORING_CENSUS K=" << k
                      << " distance=" << distance
                      << " complete=" << (complete ? "YES" : "NO")
                      << " verified=" << (verified ? "YES" : "NO")
                      << " expected_states=" << expected
                      << " counted_states=" << counted
                      << " count_memo_states=" << completion_memo.size()
                      << " seen=" << seen
                      << " skipped=" << skip
                      << " tested=" << tested
                      << " lookahead_ok=" << lookahead_ok
                      << " exact_fallbacks=" << exact_fallbacks
                      << " exact_nodes=" << exact_nodes
                      << " max_exact_nodes=" << maximum_exact_nodes
                      << " first_lookahead_failure="
                      << show(first_lookahead_failure)
                      << " hole=" << show(hole)
                      << " seconds=" << seconds
                      << " states_per_second="
                      << (seconds == 0.0 ? 0.0 : tested / seconds) << '\n';
        if (!verified) return 1;
        return 0;
    }
};

struct ParallelTransferShellResult {
    bool verified = false;
    std::uint64_t states = 0;
    std::uint64_t lookahead_ok = 0;
    std::uint64_t exact_fallbacks = 0;
    std::uint64_t exact_nodes = 0;
    std::uint64_t maximum_exact_nodes = 0;
    Sequence first_lookahead_failure;
    Sequence hole;
    double seconds = 0.0;
};

ParallelTransferShellResult run_parallel_transfer_shell(
    int k, int distance, int requested_workers, bool emit = true) {
    ParallelTransferShellResult result;
    const std::uint64_t expected =
        expected_transfer_shell_states(k, distance);
    if (expected == 0 || requested_workers <= 0) return result;
    const int worker_count = static_cast<int>(std::min<std::uint64_t>(
        static_cast<std::uint64_t>(requested_workers), expected));

    std::vector<std::unique_ptr<TransferShellCensus>> workers;
    std::vector<int> statuses(worker_count, 1);
    workers.reserve(worker_count);
    // Completion counts rank the deterministic DFS without visiting skipped
    // leaves.  Contiguous rank windows are therefore disjoint and exhaustive;
    // summing tested states back to expected is the final shard-cover check.
    const std::uint64_t base = expected / worker_count;
    const std::uint64_t remainder = expected % worker_count;
    std::uint64_t offset = 0;
    for (int index = 0; index < worker_count; ++index) {
        const std::uint64_t length =
            base + (static_cast<std::uint64_t>(index) < remainder ? 1 : 0);
        workers.push_back(std::make_unique<TransferShellCensus>(
            k, distance, length, offset));
        offset += length;
    }

    const auto started = std::chrono::steady_clock::now();
    std::vector<std::thread> threads;
    threads.reserve(worker_count);
    for (int index = 0; index < worker_count; ++index) {
        threads.emplace_back([&, index] {
            statuses[index] = workers[index]->run(false);
        });
    }
    for (auto &thread : threads) thread.join();
    result.seconds = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - started).count();

    result.verified = offset == expected;
    for (int index = 0; index < worker_count; ++index) {
        const TransferShellCensus &worker = *workers[index];
        result.verified = result.verified && statuses[index] == 0;
        result.states += worker.tested;
        result.lookahead_ok += worker.lookahead_ok;
        result.exact_fallbacks += worker.exact_fallbacks;
        result.exact_nodes += worker.exact_nodes;
        result.maximum_exact_nodes =
            std::max(result.maximum_exact_nodes, worker.maximum_exact_nodes);
        if (result.first_lookahead_failure.empty()
            && !worker.first_lookahead_failure.empty())
            result.first_lookahead_failure = worker.first_lookahead_failure;
        if (result.hole.empty() && !worker.hole.empty())
            result.hole = worker.hole;
    }
    result.verified = result.verified && result.states == expected
        && result.hole.empty();
    if (emit)
        std::cout << "TRANSFER_SHELL_PARALLEL_COLORING_CENSUS K=" << k
                  << " distance=" << distance
                  << " workers=" << worker_count
                  << " complete=" << (result.verified ? "YES" : "NO")
                  << " verified=" << (result.verified ? "YES" : "NO")
                  << " expected_states=" << expected
                  << " tested=" << result.states
                  << " lookahead_ok=" << result.lookahead_ok
                  << " exact_fallbacks=" << result.exact_fallbacks
                  << " exact_nodes=" << result.exact_nodes
                  << " max_exact_nodes=" << result.maximum_exact_nodes
                  << " first_lookahead_failure="
                  << show(result.first_lookahead_failure)
                  << " hole=" << show(result.hole)
                  << " seconds=" << result.seconds
                  << " states_per_second="
                  << (result.seconds == 0.0
                          ? 0.0 : result.states / result.seconds)
                  << '\n' << std::flush;
    return result;
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
    if (argc >= 2 && std::string(argv[1]) == "--boundary-blockers") {
        if (argc < 7) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --boundary-blockers k donor recipient point-index value...\n";
            return 2;
        }
        const int k = std::atoi(argv[2]);
        const int donor = std::atoi(argv[3]);
        const int recipient = std::atoi(argv[4]);
        const int point_index = std::atoi(argv[5]);
        Sequence state;
        for (int i = 6; i < argc; ++i) state.push_back(std::atoi(argv[i]));
        if (k < 1) return 2;
        inspect_boundary_blockers(k, donor, recipient, point_index, std::move(state));
        return 0;
    }
    if (argc >= 2 && std::string(argv[1]) == "--boundary-delta-case") {
        if (argc < 4) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --boundary-delta-case k value...\n";
            return 2;
        }
        const int k = std::atoi(argv[2]);
        Sequence state;
        for (int i = 3; i < argc; ++i) state.push_back(std::atoi(argv[i]));
        if (k < 1) return 2;
        inspect_boundary_delta_case(k, std::move(state));
        return 0;
    }
    if (argc >= 2 && std::string(argv[1]) == "--adjacent-fiber-landscape-census") {
        const int k = argc > 2 ? std::atoi(argv[2]) : 3;
        const std::uint64_t state_limit =
            argc > 3 ? std::strtoull(argv[3], nullptr, 10) : 0;
        const std::uint64_t state_skip =
            argc > 4 ? std::strtoull(argv[4], nullptr, 10) : 0;
        if (k < 1 || k > 4 || (state_limit == 0 && state_skip != 0)) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --adjacent-fiber-landscape-census k [state-limit [state-skip]]\n";
            return 2;
        }
        LandscapeCensus census(k, state_limit, state_skip);
        census.run();
        return 0;
    }
    if (argc >= 2 && std::string(argv[1]) == "--adjacent-fiber-landscape") {
        if (argc < 6) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --adjacent-fiber-landscape k donor recipient value...\n";
            return 2;
        }
        const int k = std::atoi(argv[2]);
        const int donor = std::atoi(argv[3]);
        const int recipient = std::atoi(argv[4]);
        Sequence state;
        for (int i = 5; i < argc; ++i) state.push_back(std::atoi(argv[i]));
        if (k < 1) return 2;
        inspect_adjacent_fiber_landscape(k, donor, recipient, std::move(state));
        return 0;
    }
    if (argc >= 2 && std::string(argv[1]) == "--adjacent-fiber-case") {
        if (argc < 6) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --adjacent-fiber-case k donor recipient value...\n";
            return 2;
        }
        const int k = std::atoi(argv[2]);
        const int donor = std::atoi(argv[3]);
        const int recipient = std::atoi(argv[4]);
        Sequence state;
        for (int i = 5; i < argc; ++i) state.push_back(std::atoi(argv[i]));
        if (k < 1) return 2;
        inspect_adjacent_fiber_case(k, donor, recipient, std::move(state));
        return 0;
    }
    if (argc >= 2 && std::string(argv[1]) == "--adjacent-fiber-census") {
        const int k = argc > 2 ? std::atoi(argv[2]) : 3;
        const std::uint64_t state_limit =
            argc > 3 ? std::strtoull(argv[3], nullptr, 10) : 0;
        const bool exact_margins = argc > 4 && std::string(argv[4]) == "exact";
        if (k < 1 || k > 4 || (argc > 4 && !exact_margins)) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --adjacent-fiber-census k [state-limit] [exact]\n";
            return 2;
        }
        AdjacentFiberCensus census(k, state_limit, exact_margins);
        census.run();
        return 0;
    }
    if (argc >= 2 && std::string(argv[1]) == "--padded-alternation-counterexample") {
        check_padded_alternation_counterexample();
        return 0;
    }
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
    if (argc >= 2 && std::string(argv[1]) == "--general-coloring-case") {
        if (argc < 4) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --general-coloring-case k value...\n";
            return 2;
        }
        const int k = std::atoi(argv[2]);
        Sequence state;
        for (int index = 3; index < argc; ++index)
            state.push_back(std::atoi(argv[index]));
        if (k < 2 || k > 6
            || std::any_of(state.begin(), state.end(),
                           [](int value) { return value <= 0; })) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --general-coloring-case k value...\n";
            return 2;
        }
        std::sort(state.begin(), state.end(), std::greater<int>());
        Hall hall(singleton_base(k - 1));
        const bool lookahead = greedy_block_coloring(state, hall, true);
        GeneralSearch exact(state, hall);
        Coloring current;
        const bool feasible = exact.dfs(0, current);
        std::cout << "GENERAL_COLORING_CASE K=" << k
                  << " state=" << show(state)
                  << " lookahead=" << (lookahead ? "YES" : "NO")
                  << " feasible=" << (feasible ? "YES" : "NO")
                  << " exact_nodes=" << exact.nodes
                  << " A=" << show(exact.solution.a)
                  << " B=" << show(exact.solution.b) << '\n';
        return 0;
    }
    if (argc >= 2 && std::string(argv[1]) == "--prefix-cylinder-parallel") {
        const int k = argc > 2 ? std::atoi(argv[2]) : 5;
        const int workers = argc > 3 ? std::atoi(argv[3]) : 12;
        const std::uint64_t certificate_node_limit =
            argc > 4 ? std::strtoull(argv[4], nullptr, 10) : 256;
        const int stride = argc > 5 ? std::atoi(argv[5]) : 1;
        const int minimum_prefix = argc > 6 ? std::atoi(argv[6]) : 1;
        const int maximum_prefix = argc > 7 ? std::atoi(argv[7]) : 16;
        if (argc > 8 || k < 1 || k > 5 || workers < 1 || workers > 64
            || stride < 1 || minimum_prefix < 0
            || minimum_prefix >= (1 << k)
            || maximum_prefix < minimum_prefix || maximum_prefix >= (1 << k)) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --prefix-cylinder-parallel k workers"
                      << " [certificate-node-limit [stride [minimum-prefix"
                      << " [maximum-prefix]]]]\n";
            return 2;
        }
        return run_parallel_prefix_cylinder(
            k, workers, certificate_node_limit, stride,
            minimum_prefix, maximum_prefix);
    }
    if (argc >= 2 && std::string(argv[1]) == "--prefix-cylinder-census") {
        const int k = argc > 2 ? std::atoi(argv[2]) : 5;
        const std::uint64_t generation_node_limit =
            argc > 3 ? std::strtoull(argv[3], nullptr, 10) : 0;
        const std::uint64_t certificate_node_limit =
            argc > 4 ? std::strtoull(argv[4], nullptr, 10) : 256;
        const int stride = argc > 5 ? std::atoi(argv[5]) : 2;
        const int minimum_prefix = argc > 6 ? std::atoi(argv[6]) : 4;
        const std::uint64_t state_skip =
            argc > 7 ? std::strtoull(argv[7], nullptr, 10) : 0;
        const std::uint64_t state_limit =
            argc > 8 ? std::strtoull(argv[8], nullptr, 10) : 0;
        const int maximum_prefix = argc > 9 ? std::atoi(argv[9]) : (1 << k) - 1;
        if (argc > 10 || k < 1 || k > 5 || stride < 1
            || minimum_prefix < 0 || minimum_prefix >= (1 << k)
            || maximum_prefix < minimum_prefix || maximum_prefix >= (1 << k)) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --prefix-cylinder-census k [generation-node-limit"
                      << " [certificate-node-limit [stride [minimum-prefix"
                      << " [state-skip [state-limit [maximum-prefix]]]]]]]\n";
            return 2;
        }
        PrefixCylinderCensus census(
            k, generation_node_limit, certificate_node_limit,
            stride, minimum_prefix, maximum_prefix, state_skip, state_limit);
        return census.run();
    }
    if (argc >= 2 && std::string(argv[1]) == "--transfer-shell-counts") {
        if (argc != 4) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --transfer-shell-counts k maximum-distance\n";
            return 2;
        }
        const int k = std::atoi(argv[2]);
        const int maximum_distance = std::atoi(argv[3]);
        if (k < 2 || k > 6 || maximum_distance < 0
            || expected_transfer_shell_states(k, maximum_distance) == 0) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --transfer-shell-counts k maximum-distance\n";
            return 2;
        }
        bool verified = true;
        std::uint64_t states = 0;
        std::uint64_t memo_states = 0;
        for (int distance = 0; distance <= maximum_distance; ++distance) {
            TransferShellCensus census(k, distance, 0, 0);
            const std::uint64_t counted = census.count_completions(
                0, census.mass, census.parent.front(), 0);
            verified = verified
                && counted == expected_transfer_shell_states(k, distance);
            if (counted > std::numeric_limits<std::uint64_t>::max() - states) {
                std::cerr << "TRANSFER_SHELL_COUNT_OVERFLOW K=" << k
                          << " maximum_distance=" << maximum_distance << '\n';
                return 2;
            }
            states += counted;
            memo_states += census.completion_memo.size();
        }
        std::cout << "TRANSFER_SHELL_COUNT_CHECK K=" << k
                  << " maximum_distance=" << maximum_distance
                  << " shells=" << maximum_distance + 1
                  << " states=" << states
                  << " memo_states=" << memo_states
                  << " verified=" << (verified ? "YES" : "NO") << '\n';
        return verified ? 0 : 1;
    }
    if (argc >= 2
        && std::string(argv[1]) == "--transfer-shell-oracle-input") {
        if (argc != 7) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --transfer-shell-oracle-input k distance skip limit budget\n";
            return 2;
        }
        const int k = std::atoi(argv[2]);
        const int distance = std::atoi(argv[3]);
        const std::uint64_t skip = std::strtoull(argv[4], nullptr, 10);
        const std::uint64_t limit = std::strtoull(argv[5], nullptr, 10);
        const std::uint64_t budget = std::strtoull(argv[6], nullptr, 10);
        const std::uint64_t expected = expected_transfer_shell_states(k, distance);
        if (k < 2 || k > 6 || distance < 0 || limit == 0 || expected == 0
            || skip >= expected || limit > expected - skip) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --transfer-shell-oracle-input k distance skip limit budget\n";
            return 2;
        }
        std::cout << "budget " << budget << '\n';
        TransferShellCensus census(k, distance, limit, skip);
        census.emit_oracle_queries = true;
        const int status = census.run(false);
        std::cout << "stats\nquit\n";
        return status;
    }
    if (argc >= 2
        && std::string(argv[1]) == "--transfer-shell-parallel") {
        if (argc != 5) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --transfer-shell-parallel k distance workers\n";
            return 2;
        }
        const int k = std::atoi(argv[2]);
        const int distance = std::atoi(argv[3]);
        const int workers = std::atoi(argv[4]);
        if (k < 2 || k > 6 || distance < 0 || workers < 1 || workers > 64
            || expected_transfer_shell_states(k, distance) == 0) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --transfer-shell-parallel k distance workers\n";
            return 2;
        }
        return run_parallel_transfer_shell(k, distance, workers).verified ? 0 : 1;
    }
    if (argc >= 2
        && std::string(argv[1]) == "--transfer-ball-parallel") {
        if (argc != 5) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --transfer-ball-parallel k maximum-distance workers\n";
            return 2;
        }
        const int k = std::atoi(argv[2]);
        const int maximum_distance = std::atoi(argv[3]);
        const int workers = std::atoi(argv[4]);
        if (k < 2 || k > 6 || maximum_distance < 0
            || workers < 1 || workers > 64
            || expected_transfer_shell_states(k, maximum_distance) == 0) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --transfer-ball-parallel k maximum-distance workers\n";
            return 2;
        }
        const auto started = std::chrono::steady_clock::now();
        ParallelTransferShellResult total;
        total.verified = true;
        for (int distance = 0; distance <= maximum_distance; ++distance) {
            const ParallelTransferShellResult shell =
                run_parallel_transfer_shell(k, distance, workers);
            total.verified = total.verified && shell.verified;
            total.states += shell.states;
            total.lookahead_ok += shell.lookahead_ok;
            total.exact_fallbacks += shell.exact_fallbacks;
            total.exact_nodes += shell.exact_nodes;
            total.maximum_exact_nodes = std::max(
                total.maximum_exact_nodes, shell.maximum_exact_nodes);
            if (total.first_lookahead_failure.empty()
                && !shell.first_lookahead_failure.empty())
                total.first_lookahead_failure = shell.first_lookahead_failure;
            if (total.hole.empty() && !shell.hole.empty())
                total.hole = shell.hole;
            if (!shell.verified) break;
        }
        total.seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        std::cout << "TRANSFER_BALL_PARALLEL_COLORING_CENSUS K=" << k
                  << " maximum_distance=" << maximum_distance
                  << " workers=" << workers
                  << " complete=" << (total.verified ? "YES" : "NO")
                  << " verified=" << (total.verified ? "YES" : "NO")
                  << " states=" << total.states
                  << " lookahead_ok=" << total.lookahead_ok
                  << " exact_fallbacks=" << total.exact_fallbacks
                  << " exact_nodes=" << total.exact_nodes
                  << " max_exact_nodes=" << total.maximum_exact_nodes
                  << " first_lookahead_failure="
                  << show(total.first_lookahead_failure)
                  << " hole=" << show(total.hole)
                  << " seconds=" << total.seconds
                  << " states_per_second="
                  << (total.seconds == 0.0
                          ? 0.0 : total.states / total.seconds)
                  << '\n' << std::flush;
        return total.verified ? 0 : 1;
    }
    if (argc >= 2 && std::string(argv[1]) == "--transfer-ball") {
        if (argc != 4) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --transfer-ball k maximum-distance\n";
            return 2;
        }
        const int k = std::atoi(argv[2]);
        const int maximum_distance = std::atoi(argv[3]);
        if (k < 2 || k > 6 || maximum_distance < 0
            || expected_transfer_shell_states(k, maximum_distance) == 0) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --transfer-ball k maximum-distance\n";
            return 2;
        }
        const auto started = std::chrono::steady_clock::now();
        std::uint64_t states = 0;
        std::uint64_t lookahead_ok = 0;
        std::uint64_t exact_fallbacks = 0;
        std::uint64_t exact_nodes = 0;
        std::uint64_t maximum_exact_nodes = 0;
        for (int distance = 0; distance <= maximum_distance; ++distance) {
            TransferShellCensus census(k, distance, 0, 0);
            if (census.run() != 0) return 1;
            states += census.tested;
            lookahead_ok += census.lookahead_ok;
            exact_fallbacks += census.exact_fallbacks;
            exact_nodes += census.exact_nodes;
            maximum_exact_nodes =
                std::max(maximum_exact_nodes, census.maximum_exact_nodes);
        }
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        std::cout << "TRANSFER_BALL_COLORING_CENSUS K=" << k
                  << " maximum_distance=" << maximum_distance
                  << " complete=YES verified=YES"
                  << " states=" << states
                  << " lookahead_ok=" << lookahead_ok
                  << " exact_fallbacks=" << exact_fallbacks
                  << " exact_nodes=" << exact_nodes
                  << " max_exact_nodes=" << maximum_exact_nodes
                  << " seconds=" << seconds
                  << " states_per_second="
                  << (seconds == 0.0 ? 0.0 : states / seconds) << '\n';
        return 0;
    }
    if (argc >= 2 && std::string(argv[1]) == "--transfer-shell") {
        if (argc < 4 || argc > 6) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --transfer-shell k distance [state-limit [state-skip]]\n";
            return 2;
        }
        const int k = std::atoi(argv[2]);
        const int distance = std::atoi(argv[3]);
        const std::uint64_t state_limit =
            argc > 4 ? std::strtoull(argv[4], nullptr, 10) : 0;
        const std::uint64_t state_skip =
            argc > 5 ? std::strtoull(argv[5], nullptr, 10) : 0;
        if (k < 2 || k > 6 || distance < 0
            || (state_limit == 0 && state_skip != 0)) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --transfer-shell k distance [state-limit [state-skip]]\n";
            return 2;
        }
        TransferShellCensus census(k, distance, state_limit, state_skip);
        return census.run();
    }
    if (argc >= 2 && std::string(argv[1]) == "--fixed-positive-sample") {
        const int k = argc > 2 ? std::atoi(argv[2]) : 5;
        const std::uint64_t states =
            argc > 3 ? std::strtoull(argv[3], nullptr, 10) : 100;
        const std::uint64_t pairs =
            argc > 4 ? std::strtoull(argv[4], nullptr, 10) : 32;
        const std::uint64_t seed =
            argc > 5 ? std::strtoull(argv[5], nullptr, 10) : 1;
        if (k < 2 || k > 6 || states == 0 || pairs == 0) {
            std::cerr << "usage: singleton_pair_coloring_census"
                      << " --fixed-positive-sample k states pairs-per-state [seed]\n";
            return 2;
        }
        sample_fixed_positive_crossings(k, states, pairs, seed);
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
