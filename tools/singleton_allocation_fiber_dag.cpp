#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// Exact K<=3 survey of literal first-cut transport along the singleton
// Robin--Hood DAG.
//
// A solution is a normalized multiset of legal row allocations (l,m,r),
// modulo permutations of equal parent rows and the genuine l<->r symmetry.
// Along x -> y, one coin is transported from the marked donor row to the
// marked recipient row in the same child coordinate.  All other row
// allocations remain fixed.  The resulting allocation must still give three
// children majorized by G_(K-1).  This is stronger than merely rebuilding a
// cut from a coloring common to x and y.

namespace {

constexpr int MAX_VALUE = 8;
using Sequence = std::vector<int>;
using Counts = std::array<std::uint8_t, MAX_VALUE + 1>;
using Row = std::array<std::uint8_t, 3>;
using Allocation = std::vector<Row>;

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

std::string show_allocation(const Allocation &allocation) {
    std::string out = "(";
    for (std::size_t i = 0; i < allocation.size(); ++i) {
        if (i) out += ',';
        out += '(' + std::to_string(allocation[i][0]) + ',' +
               std::to_string(allocation[i][1]) + ',' +
               std::to_string(allocation[i][2]) + ')';
    }
    return out + ')';
}

struct Child {
    Counts counts{};
    int mass = 0;
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

Allocation allocation_key(const std::vector<Choice> &rows, bool swap_sides) {
    Allocation result;
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

Allocation canonical_allocation(const std::vector<Choice> &rows, int largest) {
    Counts left{};
    Counts right{};
    for (const Choice &row : rows) {
        if (row.left) ++left[row.left];
        if (row.right) ++right[row.right];
    }
    const bool swap = partition_less(right, left, largest);
    Allocation result = allocation_key(rows, swap);
    if (left == right)
        result = std::min(result, allocation_key(rows, !swap));
    return result;
}

Allocation reflected(const Allocation &allocation) {
    Allocation result = allocation;
    for (Row &row : result) std::swap(row[0], row[2]);
    std::sort(result.begin(), result.end(), std::greater<>());
    return result;
}

struct AllocationSearch {
    const Sequence &state;
    Sequence child_prefix{0};
    int child_mass = 0;
    int child_largest = 0;
    std::vector<std::vector<Choice>> choices;
    std::vector<Allocation> allocations;
    std::uint64_t nodes = 0;
    std::uint64_t complete = 0;

    AllocationSearch(const Sequence &parent_state, const Sequence &child_base)
        : state(parent_state) {
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

    static void add(Child &child, int value) {
        if (!value) return;
        ++child.counts[value];
        child.mass += value;
    }

    static void remove(Child &child, int value) {
        if (!value) return;
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

    void dfs(std::size_t row, int minimum_choice, int remaining_mass,
             Child &left, Child &mixed, Child &right,
             std::vector<Choice> &rows) {
        ++nodes;
        if (!deficits_possible(left, mixed, right, remaining_mass)) return;
        if (row == state.size()) {
            if (left.mass != child_mass || mixed.mass != child_mass ||
                right.mass != child_mass)
                return;
            ++complete;
            allocations.push_back(canonical_allocation(rows, child_largest));
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
            if (child_ok(left) && child_ok(mixed) && child_ok(right))
                dfs(row + 1, index, remaining_mass - value,
                    left, mixed, right, rows);
            rows.pop_back();
            remove(right, choice.right);
            remove(mixed, choice.mixed);
            remove(left, choice.left);
        }
    }

    void run() {
        Child left, mixed, right;
        std::vector<Choice> rows;
        rows.reserve(state.size());
        const int total = std::accumulate(state.begin(), state.end(), 0);
        dfs(0, 0, total, left, mixed, right, rows);
        std::sort(allocations.begin(), allocations.end());
        allocations.erase(std::unique(allocations.begin(), allocations.end()),
                          allocations.end());
    }
};

struct StateData {
    Sequence state;
    std::vector<Allocation> allocations;
    std::vector<bool> inherited;
    std::vector<bool> reached;
    std::vector<bool> novel_component;
};

struct UnitEdge {
    int source = -1;
    int target = -1;
    int donor = 0;
    int recipient = 0;
};

struct EdgeSummary {
    UnitEdge edge;
    std::size_t source_count = 0;
    std::size_t target_count = 0;
    std::size_t links = 0;
    std::size_t persistent = 0;
    std::size_t inherited = 0;

    std::size_t deaths() const { return source_count - persistent; }
    std::size_t births() const { return target_count - inherited; }
};

struct Survey {
    int k;
    int examples;
    Sequence profile;
    Sequence child;
    Sequence parent_prefix{0};
    int total = 0;
    int padded_rows = 0;
    std::vector<StateData> states;
    std::map<Sequence, int> state_index;
    std::vector<std::vector<UnitEdge>> outgoing;
    std::vector<int> incoming_edges;
    std::uint64_t allocation_nodes = 0;
    std::uint64_t complete_allocations = 0;

    Survey(int level, int requested_examples)
        : k(level), examples(requested_examples), profile(singleton_base(level)),
          child(singleton_base(level - 1)), padded_rows(1) {
        for (int value : profile) {
            total += value;
            parent_prefix.push_back(total);
        }
        for (int i = 0; i < k; ++i) padded_rows *= 3;
    }

    int parent_H(int count) const {
        return parent_prefix[std::min(count, static_cast<int>(profile.size()))];
    }

    void enumerate_states(int remaining, int maximum, Sequence &state) {
        if (remaining == 0) {
            const int index = static_cast<int>(states.size());
            state_index[state] = index;
            StateData data;
            data.state = state;
            states.push_back(std::move(data));
            return;
        }
        const int used = total - remaining;
        for (int value = std::min(maximum, remaining); value >= 1; --value) {
            if (used + value > parent_H(static_cast<int>(state.size()) + 1)) continue;
            state.push_back(value);
            enumerate_states(remaining - value, value, state);
            state.pop_back();
        }
    }

    void build_states() {
        Sequence state;
        enumerate_states(total, profile.front(), state);
        for (StateData &data : states) {
            AllocationSearch search(data.state, child);
            search.run();
            if (search.allocations.empty()) {
                std::cerr << "ALLOCATION_COUNTEREXAMPLE state=" << show(data.state) << '\n';
                std::exit(1);
            }
            allocation_nodes += search.nodes;
            complete_allocations += search.complete;
            data.allocations = std::move(search.allocations);
            data.inherited.assign(data.allocations.size(), false);
            data.reached.assign(data.allocations.size(), false);
            data.novel_component.assign(data.allocations.size(), false);
        }
    }

    static Sequence transferred_state(const Sequence &state, int donor, int recipient) {
        Sequence target = state;
        target.erase(std::find(target.begin(), target.end(), donor));
        if (donor > 1) target.push_back(donor - 1);
        if (recipient > 0)
            target.erase(std::find(target.begin(), target.end(), recipient));
        target.push_back(recipient + 1);
        std::sort(target.begin(), target.end(), std::greater<int>());
        return target;
    }

    void build_edges() {
        outgoing.resize(states.size());
        incoming_edges.assign(states.size(), 0);
        for (int index = 0; index < static_cast<int>(states.size()); ++index) {
            const Sequence &state = states[index].state;
            Sequence values = state;
            values.erase(std::unique(values.begin(), values.end()), values.end());
            Sequence recipients = values;
            if (static_cast<int>(state.size()) < padded_rows) recipients.push_back(0);
            for (int donor : values) {
                for (int recipient : recipients) {
                    if (donor < recipient + 2) continue;
                    Sequence target = transferred_state(state, donor, recipient);
                    const auto found = state_index.find(target);
                    if (found == state_index.end()) {
                        std::cerr << "TRANSFER_LEFT_CORPUS source=" << show(state)
                                  << " target=" << show(target) << '\n';
                        std::exit(1);
                    }
                    outgoing[index].push_back({index, found->second, donor, recipient});
                    ++incoming_edges[found->second];
                }
            }
        }
    }

    static int row_sum(const Row &row) {
        return row[0] + row[1] + row[2];
    }

    int find_allocation(int state, const Allocation &key) const {
        const auto &values = states[state].allocations;
        const auto found = std::lower_bound(values.begin(), values.end(), key);
        if (found == values.end() || *found != key) return -1;
        return static_cast<int>(found - values.begin());
    }

    std::vector<int> images(const UnitEdge &edge, const Allocation &source) const {
        std::vector<int> result;
        std::array<Allocation, 2> orientations{source, reflected(source)};
        const int orientation_count = orientations[0] == orientations[1] ? 1 : 2;
        for (int orientation = 0; orientation < orientation_count; ++orientation) {
            const Allocation &rows = orientations[orientation];
            for (int donor_index = 0;
                 donor_index < static_cast<int>(rows.size()); ++donor_index) {
                if (row_sum(rows[donor_index]) != edge.donor) continue;
                const int recipient_begin = edge.recipient == 0 ? -1 : 0;
                const int recipient_end = edge.recipient == 0 ? 0 : static_cast<int>(rows.size());
                for (int recipient_index = recipient_begin;
                     recipient_index < recipient_end; ++recipient_index) {
                    if (recipient_index >= 0 &&
                        row_sum(rows[recipient_index]) != edge.recipient)
                        continue;
                    for (int coordinate = 0; coordinate < 3; ++coordinate) {
                        if (!rows[donor_index][coordinate]) continue;
                        Allocation moved = rows;
                        --moved[donor_index][coordinate];
                        if (recipient_index < 0) {
                            Row added{};
                            ++added[coordinate];
                            moved.push_back(added);
                        } else {
                            ++moved[recipient_index][coordinate];
                        }
                        const Row &new_donor = moved[donor_index];
                        const Row &new_recipient = recipient_index < 0
                            ? moved.back() : moved[recipient_index];
                        if ((new_donor[0] && new_donor[2]) ||
                            (new_recipient[0] && new_recipient[2]))
                            continue;

                        std::vector<Choice> choices;
                        choices.reserve(moved.size());
                        for (const Row &row : moved)
                            choices.push_back({row[0], row[1], row[2]});
                        Allocation key = canonical_allocation(choices, child.front());
                        const int target_index = find_allocation(edge.target, key);
                        if (target_index >= 0) result.push_back(target_index);
                    }
                }
            }
        }
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }

    std::string show_edge(const EdgeSummary &summary) const {
        const UnitEdge &edge = summary.edge;
        return "source=" + show(states[edge.source].state) +
               " move=" + std::to_string(edge.donor) + "->" +
               std::to_string(edge.recipient) +
               " target=" + show(states[edge.target].state) +
               " fibers=" + std::to_string(summary.source_count) + "->" +
               std::to_string(summary.target_count) +
               " links=" + std::to_string(summary.links) +
               " persistent=" + std::to_string(summary.persistent) +
               " inherited=" + std::to_string(summary.inherited) +
               " deaths=" + std::to_string(summary.deaths()) +
               " births=" + std::to_string(summary.births());
    }

    void run() {
        const auto started = std::chrono::steady_clock::now();
        build_states();
        build_edges();
        std::uint64_t edge_count = 0;
        for (const auto &edges : outgoing) edge_count += edges.size();
        const std::array<std::uint64_t, 4> expected_states{0, 2, 15, 1206};
        const std::array<std::uint64_t, 4> expected_edges{0, 1, 33, 8916};
        const std::array<std::uint64_t, 4> expected_allocations{0, 2, 57, 1063464};
        std::uint64_t allocation_count = 0;
        for (const StateData &state : states) allocation_count += state.allocations.size();
        if (states.size() != expected_states[k] || edge_count != expected_edges[k] ||
            allocation_count != expected_allocations[k]) {
            std::cerr << "ALLOCATION_DAG_REGRESSION states=" << states.size()
                      << " edges=" << edge_count
                      << " allocations=" << allocation_count << '\n';
            std::exit(1);
        }

        std::vector<int> order(states.size());
        std::iota(order.begin(), order.end(), 0);
        const auto square_sum = [&](int index) {
            int result = 0;
            for (int value : states[index].state) result += value * value;
            return result;
        };
        std::sort(order.begin(), order.end(), [&](int lhs, int rhs) {
            return std::tuple{square_sum(lhs), states[lhs].state} >
                   std::tuple{square_sum(rhs), states[rhs].state};
        });
        const int root = state_index.at(profile);
        std::fill(states[root].reached.begin(), states[root].reached.end(), true);

        std::vector<EdgeSummary> summaries;
        summaries.reserve(edge_count);
        std::uint64_t total_links = 0;
        std::uint64_t nonempty = 0;
        std::uint64_t no_birth = 0;
        std::uint64_t no_death = 0;
        std::uint64_t no_churn = 0;
        std::map<std::pair<std::size_t, std::size_t>, std::uint64_t> classes;

        for (int source_index : order) {
            const StateData &source = states[source_index];
            for (const UnitEdge &edge : outgoing[source_index]) {
                std::vector<bool> target_seen(states[edge.target].allocations.size(), false);
                std::size_t links = 0;
                std::size_t persistent = 0;
                std::size_t inherited = 0;
                for (int allocation_index = 0;
                     allocation_index < static_cast<int>(source.allocations.size());
                     ++allocation_index) {
                    const std::vector<int> targets = images(
                        edge, source.allocations[allocation_index]);
                    if (!targets.empty()) ++persistent;
                    links += targets.size();
                    for (int target_index : targets) {
                        if (!target_seen[target_index]) {
                            target_seen[target_index] = true;
                            ++inherited;
                        }
                        states[edge.target].inherited[target_index] = true;
                        if (source.reached[allocation_index])
                            states[edge.target].reached[target_index] = true;
                    }
                }
                EdgeSummary summary{edge, source.allocations.size(),
                                    states[edge.target].allocations.size(), links,
                                    persistent, inherited};
                summaries.push_back(summary);
                total_links += links;
                nonempty += links != 0;
                no_birth += summary.births() == 0;
                no_death += summary.deaths() == 0;
                no_churn += summary.births() == 0 && summary.deaths() == 0;
                ++classes[{summary.deaths(), summary.births()}];
            }
        }

        std::size_t reached_states = 0;
        std::uint64_t reached_allocations = 0;
        std::size_t states_with_novel = 0;
        std::uint64_t novel_allocations = 0;
        std::vector<std::tuple<std::size_t, int>> unreachable;
        std::vector<std::tuple<std::size_t, int>> novel;
        for (int index = 0; index < static_cast<int>(states.size()); ++index) {
            const StateData &state = states[index];
            const std::size_t reached = std::count(state.reached.begin(), state.reached.end(), true);
            const std::size_t inherited = std::count(
                state.inherited.begin(), state.inherited.end(), true);
            reached_states += reached != 0;
            reached_allocations += reached;
            if (reached < state.allocations.size())
                unreachable.push_back({state.allocations.size() - reached, index});
            if (index != root && inherited < state.allocations.size()) {
                ++states_with_novel;
                novel_allocations += state.allocations.size() - inherited;
                novel.push_back({state.allocations.size() - inherited, index});
                for (int j = 0; j < static_cast<int>(state.allocations.size()); ++j)
                    if (!state.inherited[j]) states[index].novel_component[j] = true;
            }
        }

        for (int source_index : order) {
            const StateData &source = states[source_index];
            for (const UnitEdge &edge : outgoing[source_index]) {
                for (int allocation_index = 0;
                     allocation_index < static_cast<int>(source.allocations.size());
                     ++allocation_index) {
                    if (!source.novel_component[allocation_index]) continue;
                    for (int target_index : images(edge, source.allocations[allocation_index]))
                        states[edge.target].novel_component[target_index] = true;
                }
            }
        }
        std::uint64_t novel_descendants = 0;
        std::uint64_t unreachable_covered = 0;
        std::uint64_t novel_canonical_overlap = 0;
        std::uint64_t total_unreachable = 0;
        for (const StateData &state : states) {
            for (int index = 0; index < static_cast<int>(state.allocations.size()); ++index) {
                const bool in_novel = state.novel_component[index];
                const bool is_unreachable = !state.reached[index];
                novel_descendants += in_novel;
                total_unreachable += is_unreachable;
                unreachable_covered += in_novel && is_unreachable;
                novel_canonical_overlap += in_novel && !is_unreachable;
            }
        }

        const auto elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        std::cout << "ALLOCATION_FIBER_DAG k=" << k
                  << " states=" << states.size()
                  << " transfers=" << edge_count
                  << " allocation_orbits=" << allocation_count
                  << " search_nodes=" << allocation_nodes
                  << " complete_allocations=" << complete_allocations
                  << " seconds=" << elapsed << '\n';
        std::cout << "EDGE_SUMMARY nonempty=" << nonempty
                  << " empty=" << edge_count - nonempty
                  << " no_birth=" << no_birth
                  << " no_death=" << no_death
                  << " no_churn=" << no_churn
                  << " transport_links=" << total_links << '\n';
        std::cout << "INCOMING_UNION states_with_novel_orbits=" << states_with_novel
                  << " novel_orbits=" << novel_allocations << '\n';
        if (novel_allocations <= 10) {
            for (const auto &[count, index] : novel) {
                (void)count;
                for (int allocation = 0;
                     allocation < static_cast<int>(states[index].allocations.size());
                     ++allocation) {
                    if (!states[index].inherited[allocation])
                        std::cout << "NOVEL_SOURCE state=" << show(states[index].state)
                                  << " allocation="
                                  << show_allocation(states[index].allocations[allocation])
                                  << '\n';
                }
            }
        }
        std::cout << "FORWARD_REACHABILITY states=" << reached_states << '/'
                  << states.size() << " allocation_orbits=" << reached_allocations
                  << '/' << allocation_count << '\n';
        std::cout << "NOVEL_COMPONENT descendants=" << novel_descendants
                  << " unreachable_covered=" << unreachable_covered << '/'
                  << total_unreachable
                  << " overlap_with_canonical=" << novel_canonical_overlap << '\n';

        if (examples == 0) return;

        std::cout << "EMPTY_EDGES\n";
        int printed = 0;
        for (const EdgeSummary &summary : summaries) {
            if (summary.links || printed >= examples) continue;
            std::cout << "  " << show_edge(summary) << '\n';
            ++printed;
        }

        std::cout << "MOST_DESTRUCTIVE_EDGES\n";
        std::vector<EdgeSummary> ranked = summaries;
        std::sort(ranked.begin(), ranked.end(), [](const auto &lhs, const auto &rhs) {
            const double left_ratio = static_cast<double>(lhs.deaths()) / lhs.source_count;
            const double right_ratio = static_cast<double>(rhs.deaths()) / rhs.source_count;
            return std::tuple{left_ratio, lhs.deaths(), lhs.births()} >
                   std::tuple{right_ratio, rhs.deaths(), rhs.births()};
        });
        for (int i = 0; i < std::min(examples, static_cast<int>(ranked.size())); ++i)
            std::cout << "  " << show_edge(ranked[i]) << '\n';

        std::cout << "MOST_NOVEL_STATES\n";
        std::sort(novel.begin(), novel.end(), std::greater<>());
        for (int i = 0; i < std::min(examples, static_cast<int>(novel.size())); ++i) {
            const auto [count, index] = novel[i];
            std::cout << "  state=" << show(states[index].state)
                      << " fiber=" << states[index].allocations.size()
                      << " inherited=" << states[index].allocations.size() - count
                      << " novel=" << count
                      << " predecessor_edges=" << incoming_edges[index] << '\n';
            if (count <= 3) {
                for (int j = 0; j < static_cast<int>(states[index].allocations.size()); ++j)
                    if (!states[index].inherited[j])
                        std::cout << "    novel "
                                  << show_allocation(states[index].allocations[j]) << '\n';
            }
        }

        std::cout << "MOST_UNREACHABLE_STATES\n";
        std::sort(unreachable.begin(), unreachable.end(), std::greater<>());
        for (int i = 0; i < std::min(examples, static_cast<int>(unreachable.size())); ++i) {
            const auto [count, index] = unreachable[i];
            std::cout << "  state=" << show(states[index].state)
                      << " fiber=" << states[index].allocations.size()
                      << " reached=" << states[index].allocations.size() - count
                      << " unreachable=" << count << '\n';
        }

        std::map<std::size_t, std::uint64_t> death_histogram;
        std::map<std::size_t, std::uint64_t> birth_histogram;
        for (const auto &[key, count] : classes) {
            death_histogram[key.first] += count;
            birth_histogram[key.second] += count;
        }
        std::cout << "DEATH_HISTOGRAM";
        for (const auto &[count, edges] : death_histogram)
            std::cout << ' ' << count << ':' << edges;
        std::cout << '\n';
        std::cout << "BIRTH_HISTOGRAM";
        for (const auto &[count, edges] : birth_histogram)
            std::cout << ' ' << count << ':' << edges;
        std::cout << '\n';
    }
};

}  // namespace

int main(int argc, char **argv) {
    const int k = argc > 1 ? std::atoi(argv[1]) : 3;
    const int examples = argc > 2 ? std::atoi(argv[2]) : 12;
    if (k < 1 || k > 3 || examples < 0) {
        std::cerr << "usage: singleton_allocation_fiber_dag [k<=3 [examples]]\n";
        return 2;
    }
    Survey survey(k, examples);
    survey.run();
    return 0;
}
