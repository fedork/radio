#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// Test a possible global shortcut to the Singleton Majorization Lemma.
//
// Let Q_k be the transcript graph: two singleton transcripts are adjacent when
// they cannot belong to the same row.  A legal state is the type of a stable
// partition of Q_k.  The coefficient [m_lambda] X_{Q_k} counts proper colorings
// having a fixed set of labelled color-class sizes lambda.  If these
// coefficients are nondecreasing down dominance, Q_k is strongly nice, hence
// its stable-partition support is a dominance ideal.
//
// The recursion Q_k = Q_{k-1} disjoint-union
//                    (Q_{k-1} join Q_{k-1})
// gives the coefficient recurrence used below.  For each color of size w, its
// occurrences have the form (w-x,x,0) or (0,w-x,x) in the three first-test
// branches.  At a leaf, the three child coefficients multiply.

namespace {

struct WideInteger {
    static constexpr std::uint32_t base = 1000000000;
    std::vector<std::uint32_t> digits;

    WideInteger(std::uint64_t value = 0) {
        while (value) {
            digits.push_back(static_cast<std::uint32_t>(value % base));
            value /= base;
        }
    }

    void normalize() {
        while (!digits.empty() && digits.back() == 0) digits.pop_back();
    }

    WideInteger &operator+=(const WideInteger &other) {
        const std::size_t size = std::max(digits.size(), other.digits.size());
        digits.resize(size, 0);
        std::uint64_t carry = 0;
        for (std::size_t i = 0; i < size; ++i) {
            const std::uint64_t sum = digits[i] + carry +
                (i < other.digits.size() ? other.digits[i] : 0);
            digits[i] = static_cast<std::uint32_t>(sum % base);
            carry = sum / base;
        }
        if (carry) digits.push_back(static_cast<std::uint32_t>(carry));
        return *this;
    }

    WideInteger &operator*=(std::uint32_t factor) {
        std::uint64_t carry = 0;
        for (std::uint32_t &digit : digits) {
            const std::uint64_t product =
                static_cast<std::uint64_t>(digit) * factor + carry;
            digit = static_cast<std::uint32_t>(product % base);
            carry = product / base;
        }
        while (carry) {
            digits.push_back(static_cast<std::uint32_t>(carry % base));
            carry /= base;
        }
        normalize();
        return *this;
    }

    WideInteger &operator*=(const WideInteger &other) {
        if (digits.empty() || other.digits.empty()) {
            digits.clear();
            return *this;
        }
        std::vector<std::uint32_t> result(digits.size() + other.digits.size(), 0);
        for (std::size_t i = 0; i < digits.size(); ++i) {
            std::uint64_t carry = 0;
            for (std::size_t j = 0; j < other.digits.size(); ++j) {
                const std::uint64_t product = result[i + j] + carry +
                    static_cast<std::uint64_t>(digits[i]) * other.digits[j];
                result[i + j] = static_cast<std::uint32_t>(product % base);
                carry = product / base;
            }
            std::size_t position = i + other.digits.size();
            while (carry) {
                const std::uint64_t sum = result[position] + carry;
                result[position] = static_cast<std::uint32_t>(sum % base);
                carry = sum / base;
                ++position;
                if (position == result.size() && carry) result.push_back(0);
            }
        }
        digits = std::move(result);
        normalize();
        return *this;
    }

    WideInteger &operator/=(std::uint32_t divisor) {
        std::uint64_t remainder = 0;
        for (std::size_t i = digits.size(); i-- > 0;) {
            const std::uint64_t current = remainder * base + digits[i];
            digits[i] = static_cast<std::uint32_t>(current / divisor);
            remainder = current % divisor;
        }
        assert(remainder == 0);
        normalize();
        return *this;
    }

    friend bool operator==(const WideInteger &value, int scalar) {
        return scalar == 0 && value.digits.empty();
    }

    friend bool operator<(const WideInteger &left, const WideInteger &right) {
        if (left.digits.size() != right.digits.size())
            return left.digits.size() < right.digits.size();
        for (std::size_t i = left.digits.size(); i-- > 0;)
            if (left.digits[i] != right.digits[i])
                return left.digits[i] < right.digits[i];
        return false;
    }
};
using Sequence = std::vector<int>;

std::string show_integer(const WideInteger &value) {
    if (value.digits.empty()) return "0";
    std::ostringstream out;
    out << value.digits.back();
    for (std::size_t i = value.digits.size() - 1; i-- > 0;)
        out << std::setw(9) << std::setfill('0') << value.digits[i];
    return out.str();
}

int power(int base, int exponent) {
    int result = 1;
    while (exponent-- > 0) result *= base;
    return result;
}

Sequence singleton_base(int k) {
    Sequence result{1};
    while (k-- > 0) {
        Sequence next(2 * result.size(), 0);
        for (std::size_t i = 0; i < result.size(); ++i) {
            next[i] += result[i];
            next[2 * i] += result[i];
            next[2 * i + 1] += result[i];
        }
        std::sort(next.begin(), next.end(), std::greater<int>());
        result = std::move(next);
    }
    return result;
}

std::string show(const Sequence &values) {
    std::string result = "(";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i) result += ',';
        result += std::to_string(values[i]);
    }
    return result + ')';
}

std::string sequence_key(const Sequence &values) {
    std::string result;
    result.reserve(values.size());
    for (int value : values) result.push_back(static_cast<char>(value));
    return result;
}

struct LocalOption {
    std::array<int, 3> piece{};
};

struct CoefficientCounter;

struct SplitCounter {
    CoefficientCounter &owner;
    const Sequence &parent;
    int k;
    Sequence child_base;
    Sequence child_prefix{0};
    int child_mass;
    int child_max_width;
    std::vector<int> suffix_mass;
    std::vector<std::vector<LocalOption>> options_by_width;
    std::array<Sequence, 3> frequency;
    std::array<int, 3> mass{};
    std::unordered_map<std::string, WideInteger> memo;
    std::uint64_t nodes = 0;

    SplitCounter(CoefficientCounter &counter, const Sequence &state, int level);

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

    std::string memo_key(int index) const {
        std::string key;
        key.reserve(1 + 3 * child_max_width);
        key.push_back(static_cast<char>(index));
        // The two pure transcript branches are interchangeable.  This is safe
        // for memoization: swapping them preserves the number of completions.
        int first = 0;
        int third = 2;
        if (frequency[first] > frequency[third]) std::swap(first, third);
        for (int child : {first, 1, third})
            for (int value = 1; value <= child_max_width; ++value)
                key.push_back(static_cast<char>(frequency[child][value]));
        return key;
    }

    Sequence child_profile(int child, int added_units = 0) const {
        Sequence result;
        for (int value = child_max_width; value >= 1; --value) {
            int copies = frequency[child][value];
            if (value == 1) copies += added_units;
            result.insert(result.end(), copies, value);
        }
        return result;
    }

    WideInteger finish_unit_suffix(int index);
    WideInteger dfs(int index);
};

struct CoefficientCounter {
    std::vector<std::unordered_map<std::string, WideInteger>> cache;
    std::vector<std::uint64_t> nodes_by_level;

    explicit CoefficientCounter(int maximum_k)
        : cache(maximum_k + 1), nodes_by_level(maximum_k + 1, 0) {}

    WideInteger count(int k, const Sequence &profile) {
        const std::string key = sequence_key(profile);
        const auto old = cache[k].find(key);
        if (old != cache[k].end()) return old->second;
        WideInteger result = 0;
        if (k == 0) {
            if (profile == Sequence{1}) result = 1;
        } else {
            SplitCounter search(*this, profile, k);
            result = search.dfs(0);
            nodes_by_level[k] += search.nodes;
        }
        cache[k].emplace(key, result);
        return result;
    }
};

SplitCounter::SplitCounter(CoefficientCounter &counter, const Sequence &state,
                           int level)
    : owner(counter), parent(state), k(level),
      child_base(singleton_base(level - 1)), child_mass(power(3, level - 1)),
      child_max_width(child_base.front()), suffix_mass(parent.size() + 1, 0),
      options_by_width(parent.empty() ? 1 : parent.front() + 1) {
    for (int value : child_base) child_prefix.push_back(child_prefix.back() + value);
    for (int i = static_cast<int>(parent.size()) - 1; i >= 0; --i)
        suffix_mass[i] = suffix_mass[i + 1] + parent[i];
    for (auto &f : frequency) f.assign(child_max_width + 1, 0);

    for (int width = 1; width < static_cast<int>(options_by_width.size()); ++width) {
        auto &choices = options_by_width[width];
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
                choices.push_back({pieces});
            }
        }
    }
}

WideInteger SplitCounter::finish_unit_suffix(int index) {
    const int units = static_cast<int>(parent.size()) - index;
    std::array<int, 3> deficit{};
    int required = 0;
    for (int child = 0; child < 3; ++child) {
        deficit[child] = child_mass - mass[child];
        if (deficit[child] < 0) return 0;
        required += deficit[child];
    }
    if (required != units) return 0;

    WideInteger assignments = 1;
    int remaining = units;
    for (int child = 0; child < 2; ++child) {
        for (int i = 1; i <= deficit[child]; ++i) {
            assignments *= remaining - deficit[child] + i;
            assignments /= i;
        }
        remaining -= deficit[child];
    }

    WideInteger result = assignments;
    for (int child = 0; child < 3; ++child)
        result *= owner.count(k - 1, child_profile(child, deficit[child]));
    return result;
}

WideInteger SplitCounter::dfs(int index) {
    ++nodes;
    if (index == static_cast<int>(parent.size())) {
        for (int child = 0; child < 3; ++child)
            if (mass[child] != child_mass) return 0;
        WideInteger result = 1;
        for (int child = 0; child < 3; ++child)
            result *= owner.count(k - 1, child_profile(child));
        return result;
    }
    if (parent[index] == 1) return finish_unit_suffix(index);

    for (int child = 0; child < 3; ++child)
        if (mass[child] > child_mass ||
            mass[child] + suffix_mass[index] < child_mass)
            return 0;

    const std::string key = memo_key(index);
    const auto old = memo.find(key);
    if (old != memo.end()) return old->second;

    WideInteger result = 0;
    for (const LocalOption &option : options_by_width[parent[index]]) {
        bool legal = true;
        for (int child = 0; child < 3; ++child) {
            const int piece = option.piece[child];
            if (!piece) continue;
            ++frequency[child][piece];
            mass[child] += piece;
            if (mass[child] > child_mass || !child_majorized(child)) legal = false;
        }
        if (legal) result += dfs(index + 1);
        for (int child = 0; child < 3; ++child) {
            const int piece = option.piece[child];
            if (!piece) continue;
            --frequency[child][piece];
            mass[child] -= piece;
        }
    }
    memo.emplace(key, result);
    return result;
}

struct PartitionEnumerator {
    Sequence base;
    Sequence prefix{0};
    int total = 0;
    std::vector<Sequence> partitions;

    explicit PartitionEnumerator(int k) : base(singleton_base(k)) {
        for (int value : base) {
            total += value;
            prefix.push_back(total);
        }
    }

    int H(int count) const {
        return prefix[std::min(count, static_cast<int>(base.size()))];
    }

    void enumerate(int remaining, int maximum, Sequence &current) {
        if (remaining == 0) {
            partitions.push_back(current);
            return;
        }
        const int used = total - remaining;
        for (int value = std::min(maximum, remaining); value >= 1; --value) {
            if (used + value > H(static_cast<int>(current.size()) + 1)) continue;
            current.push_back(value);
            enumerate(remaining - value, value, current);
            current.pop_back();
        }
    }

    void run() {
        Sequence current;
        enumerate(total, base.front(), current);
    }
};

bool dominates(const Sequence &upper, const Sequence &lower) {
    int upper_sum = 0;
    int lower_sum = 0;
    const std::size_t size = std::max(upper.size(), lower.size());
    for (std::size_t i = 0; i < size; ++i) {
        if (i < upper.size()) upper_sum += upper[i];
        if (i < lower.size()) lower_sum += lower[i];
        if (upper_sum < lower_sum) return false;
    }
    return upper_sum == lower_sum;
}

}  // namespace

int main(int argc, char **argv) {
    if (argc == 5 && std::string(argv[1]) == "--walk") {
        const int k = std::stoi(argv[2]);
        const int requested_steps = std::stoi(argv[3]);
        const std::uint64_t seed = std::strtoull(argv[4], nullptr, 10);
        if (k < 1 || k > 4 || requested_steps < 0) {
            std::cerr << "walk level must be 1..4 and steps must be nonnegative\n";
            return 2;
        }
        const int slots = power(3, k);
        CoefficientCounter counter(k);
        std::mt19937_64 random(seed);
        Sequence current = singleton_base(k);
        WideInteger current_coefficient = counter.count(k, current);
        int completed = 0;
        for (; completed < requested_steps; ++completed) {
            Sequence padded = current;
            padded.resize(slots, 0);
            std::vector<Sequence> neighbors;
            for (int donor = 0; donor < slots; ++donor)
                for (int recipient = 0; recipient < slots; ++recipient) {
                    if (donor == recipient ||
                        padded[donor] < padded[recipient] + 2)
                        continue;
                    Sequence next = padded;
                    --next[donor];
                    ++next[recipient];
                    next.erase(std::remove(next.begin(), next.end(), 0), next.end());
                    std::sort(next.begin(), next.end(), std::greater<int>());
                    if (std::find(neighbors.begin(), neighbors.end(), next) == neighbors.end())
                        neighbors.push_back(std::move(next));
                }
            if (neighbors.empty()) break;
            Sequence next = neighbors[random() % neighbors.size()];
            WideInteger next_coefficient = counter.count(k, next);
            if (next_coefficient < current_coefficient) {
                std::cout << "TRANSFER_MONOTONICITY_FAIL k=" << k
                          << " step=" << completed
                          << " upper=" << show(current)
                          << " upper_coefficient=" << show_integer(current_coefficient)
                          << " lower=" << show(next)
                          << " lower_coefficient=" << show_integer(next_coefficient) << '\n';
                return 1;
            }
            current = std::move(next);
            current_coefficient = std::move(next_coefficient);
            if ((completed + 1) % 10 == 0)
                std::cerr << "walked " << (completed + 1) << '/' << requested_steps << '\n';
        }
        std::cout << "TRANSFER_MONOTONICITY_WALK_PASS k=" << k
                  << " requested_steps=" << requested_steps
                  << " completed_steps=" << completed
                  << " seed=" << seed
                  << " final_state=" << show(current) << '\n';
        for (int level = 1; level <= k; ++level)
            std::cout << "NODES k=" << level
                      << " value=" << counter.nodes_by_level[level] << '\n';
        return 0;
    }
    if (argc >= 4 && std::string(argv[1]) == "--coefficient") {
        const int k = std::stoi(argv[2]);
        if (k < 0 || k > 4) {
            std::cerr << "coefficient level must be between 0 and 4\n";
            return 2;
        }
        Sequence profile;
        for (int i = 3; i < argc; ++i) profile.push_back(std::stoi(argv[i]));
        std::sort(profile.begin(), profile.end(), std::greater<int>());
        CoefficientCounter counter(k);
        const WideInteger coefficient = counter.count(k, profile);
        std::cout << "COEFFICIENT k=" << k << " state=" << show(profile)
                  << " value=" << show_integer(coefficient) << '\n';
        for (int level = 1; level <= k; ++level)
            std::cout << "NODES k=" << level
                      << " value=" << counter.nodes_by_level[level] << '\n';
        return coefficient == 0 ? 1 : 0;
    }
    if (argc != 2) {
        std::cerr << "usage: singleton_strong_niceness k\n"
                  << "       singleton_strong_niceness --walk k steps seed\n"
                  << "       singleton_strong_niceness --coefficient k widths...\n";
        return 2;
    }
    const int k = std::stoi(argv[1]);
    if (k < 0 || k > 3) {
        std::cerr << "k must be between 0 and 3\n";
        return 2;
    }

    PartitionEnumerator enumerator(k);
    enumerator.run();
    CoefficientCounter counter(k);
    std::vector<WideInteger> coefficients;
    coefficients.reserve(enumerator.partitions.size());
    for (std::size_t i = 0; i < enumerator.partitions.size(); ++i) {
        const Sequence &profile = enumerator.partitions[i];
        const WideInteger coefficient = counter.count(k, profile);
        if (coefficient == 0) {
            std::cout << "COEFFICIENT_FAILURE k=" << k
                      << " state=" << show(profile) << '\n';
            return 1;
        }
        coefficients.push_back(coefficient);
        if ((i + 1) % 100 == 0)
            std::cerr << "counted " << (i + 1) << '/'
                      << enumerator.partitions.size() << '\n';
    }

    std::uint64_t comparable_pairs = 0;
    for (std::size_t i = 0; i < enumerator.partitions.size(); ++i)
        for (std::size_t j = 0; j < enumerator.partitions.size(); ++j) {
            if (i == j || !dominates(enumerator.partitions[i],
                                     enumerator.partitions[j]))
                continue;
            ++comparable_pairs;
            if (coefficients[j] < coefficients[i]) {
                std::cout << "STRONG_NICENESS_FAIL k=" << k
                          << " upper=" << show(enumerator.partitions[i])
                          << " upper_coefficient=" << show_integer(coefficients[i])
                          << " lower=" << show(enumerator.partitions[j])
                          << " lower_coefficient=" << show_integer(coefficients[j])
                          << " partitions=" << enumerator.partitions.size()
                          << " comparable_pairs_checked=" << comparable_pairs << '\n';
                for (int level = 1; level <= k; ++level)
                    std::cout << "NODES k=" << level
                              << " value=" << counter.nodes_by_level[level] << '\n';
                return 1;
            }
        }

    std::cout << "STRONG_NICENESS_PASS k=" << k
              << " partitions=" << enumerator.partitions.size()
              << " comparable_pairs=" << comparable_pairs << '\n';
    for (int level = 1; level <= k; ++level)
        std::cout << "NODES k=" << level
                  << " value=" << counter.nodes_by_level[level] << '\n';
    return 0;
}
