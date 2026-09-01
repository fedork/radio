//! GBASE + STAR: the singleton base sequence G_k and full star-expansion majorization.
//!
//! GBASE - docs/theorems/singleton-majorization.md, "the recursive base": G_0 = (1); if
//!   G_{K-1} = (h_1 >= ... >= h_m) then G_K = sort(L + M + R) where L = (h_1,0,h_2,0,...),
//!   M = (h_1,...,h_m,0,...,0), R = (0,h_1,0,h_2,...), all of length 2m, summed
//!   coordinatewise. Computed from the recurrence deliberately, so this crate does not
//!   depend on the closed form; the closed form is the cross-check in tests.
//!
//! STAR - the general necessary condition (Singleton Majorization Necessity lifted through
//!   the Vertex-Splitting Pullback Lemma, both in docs/theorems/singleton-majorization.md):
//!   if Sb(n_1:m_1,...,n_p:m_p) is solvable in k, then the full star expansion
//!   Phi = (n_i repeated m_i times, sorted desc) is weakly majorized by G_k:
//!   sum_{i<=t} Phi_i <= sum_{i<=t} G_k[i] for every t. A violation REFUTES the state.
//!   This is rejection-only: the converse is false from K=6 (documented counterexample).
//!
//!   Tail clamp (documented defect class in singleton-majorization.md, "code that reports
//!   one there over-refutes"): for t > len(G_k) = 2^k the right side is the constant 3^k,
//!   and the left side is at most the state's mass, which INFO has already bounded by 3^k,
//!   so no violation can arise past len(G_k). We therefore never test past 2^k.
//!
//!   Endpoint-run evaluation (equivalence documented in singleton-majorization.md and
//!   locked in C by tools/star_majorization_regression.c): within a run of equal star
//!   widths w, the prefix difference sum(Phi)-sum(G_k) has non-decreasing increments
//!   (w - G_k[t] with G_k sorted desc), so its maximum over the run is at the run's end.
//!   Checking each run's endpoint (clamped to len(G_k)) is exactly equivalent to checking
//!   every expanded prefix.

use crate::state::Part;

pub const MAX_K: usize = 12;

pub struct GTables {
    /// g[k] = G_k, descending, length 2^k.
    pub g: Vec<Vec<u64>>,
    /// prefix[k][t] = sum of the first t entries of G_k (prefix[k][0] = 0).
    pub prefix: Vec<Vec<u64>>,
}

impl GTables {
    pub fn build(max_k: usize) -> GTables {
        assert!(max_k <= MAX_K);
        let mut g: Vec<Vec<u64>> = Vec::with_capacity(max_k + 1);
        g.push(vec![1]);
        for _ in 1..=max_k {
            let h = g.last().unwrap();
            let m = h.len();
            let mut next = vec![0u64; 2 * m];
            for (i, &v) in h.iter().enumerate() {
                next[2 * i] += v; // L
                next[i] += v; // M
                next[2 * i + 1] += v; // R
            }
            next.sort_unstable_by(|a, b| b.cmp(a));
            g.push(next);
        }
        let prefix = g
            .iter()
            .map(|gk| {
                let mut p = Vec::with_capacity(gk.len() + 1);
                let mut s = 0u64;
                p.push(0);
                for &v in gk {
                    s += v;
                    p.push(s);
                }
                p
            })
            .collect();
        GTables { g, prefix }
    }

    /// STAR: true iff the state's full star expansion violates weak majorization by G_k,
    /// i.e. the state is refuted (unsolvable in k). `parts` must be sorted descending by
    /// (mass, n, m) as State::canon produces; the star widths n_i then need a re-sort by
    /// width alone, done here on the stack.
    pub fn star_refutes(&self, parts: &[Part], k: usize) -> bool {
        if parts.is_empty() {
            return false;
        }
        // Runs of the star expansion: width n_i occurring m_i times, widths sorted desc.
        // Checking each run's endpoint is exact (module comment); checking additional
        // interior points would also be sound (every prefix inequality is necessary), so
        // runs of equal width from different parts need no merging. Stack buffer: states
        // never exceed the certificate's 40-part bound by construction of the callers.
        let mut buf = [(0u64, 0u64); 96];
        assert!(parts.len() <= buf.len());
        let runs = &mut buf[..parts.len()];
        for (i, p) in parts.iter().enumerate() {
            // insertion sort by width descending
            let it = (p.n as u64, p.m as u64);
            let mut j = i;
            while j > 0 && runs[j - 1].0 < it.0 {
                runs[j] = runs[j - 1];
                j -= 1;
            }
            runs[j] = it;
        }
        let runs = &buf[..parts.len()];

        let gp = &self.prefix[k];
        let glen = (gp.len() - 1) as u64; // 2^k
        let mut t = 0u64; // expanded prefix length so far
        let mut phi_sum = 0u64;
        for &(w, c) in runs {
            let end = t + c;
            let checked_end = end.min(glen);
            if checked_end > t {
                let steps = checked_end - t;
                phi_sum += w * steps;
                if phi_sum > gp[checked_end as usize] {
                    return true;
                }
            }
            // Tail clamp: entries past len(G_k) are never tested (see module comment).
            t = end;
            if t >= glen {
                break;
            }
        }
        false
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::state::State;

    fn binom(n: u64, k: u64) -> u64 {
        if k > n {
            return 0;
        }
        let mut r = 1u64;
        for i in 0..k {
            r = r * (n - i) / (i + 1);
        }
        r
    }

    /// Closed-form cross-check (singleton-majorization.md "dyadic blocks"): entries of G_k
    /// come in blocks of sizes 1, 1, 2, 4, ..., 2^{k-1}, and every entry in block r equals
    /// sum_{i=0}^{k-r} C(k, i).
    #[test]
    fn recurrence_matches_closed_form_up_to_k12() {
        let t = GTables::build(12);
        for k in 0..=12u64 {
            let mut expect = Vec::new();
            for r in 0..=k {
                let entry: u64 = (0..=(k - r)).map(|i| binom(k, i)).sum();
                let size = if r == 0 { 1 } else { 1u64 << (r - 1) };
                for _ in 0..size {
                    expect.push(entry);
                }
            }
            assert_eq!(t.g[k as usize], expect, "G_{k}");
            assert_eq!(t.g[k as usize].len(), 1 << k);
            assert_eq!(*t.prefix[k as usize].last().unwrap(), 3u64.pow(k as u32));
        }
    }

    #[test]
    fn known_small_tables() {
        let t = GTables::build(5);
        assert_eq!(t.g[1], vec![2, 1]);
        assert_eq!(t.g[2], vec![4, 3, 1, 1]);
        assert_eq!(t.g[3], vec![8, 7, 4, 4, 1, 1, 1, 1]);
        assert_eq!(t.g[4], vec![16, 15, 11, 11, 5, 5, 5, 5, 1, 1, 1, 1, 1, 1, 1, 1]);
    }

    #[test]
    fn star_boundary_on_single_rows() {
        // (2^k : 1) is exactly G_k[1]; (2^k + 1 : 1) violates the first prefix.
        let t = GTables::build(8);
        for k in 1..=8usize {
            let ok = State::canon([((1u16 << k) as u16, 1)]);
            let bad = State::canon([((1u16 << k) as u16 + 1, 1)]);
            assert!(!t.star_refutes(&ok.parts, k), "k={k} ok row refuted");
            assert!(t.star_refutes(&bad.parts, k), "k={k} bad row not refuted");
        }
    }

    #[test]
    fn star_g_k_itself_never_refuted() {
        let t = GTables::build(6);
        for k in 1..=6usize {
            let parts: Vec<(u16, u16)> = t.g[k].iter().map(|&w| (w as u16, 1)).collect();
            let s = State::canon(parts);
            assert!(!t.star_refutes(&s.parts, k), "G_{k} refuted by its own bound");
        }
    }

    /// The tail clamp: a state whose star expansion is longer than 2^k but whose every
    /// clamped prefix is fine must NOT be refuted. Build one from G_k by splitting the last
    /// width-1 row into... rows of width 1 are already minimal, so instead append extra
    /// (1:1)-free small rows only up to the mass bound: take G_2 = (4,3,1,1) at k=2 and
    /// replace nothing; use (2:2) whose star is (2,2) - fits in G_2 prefixes (4, 7) and
    /// len 2 < 4: fine either way. The load-bearing clamp case: star longer than 2^k.
    #[test]
    fn star_tail_clamp_does_not_over_refute() {
        let t = GTables::build(2);
        // Star of (2:2),(1:2)... canon strips nothing; widths (2,2),(1,1)? (1:2)->(2:1).
        // Use (2:2) + 5 x (2:1): star = (2,2,2,2,2,2,2) length 7 > len(G_2)=4.
        // mass = 4 + 5*2 = 14 > 9 = 3^2, so INFO would reject first; STAR must not be
        // consulted past the clamp anyway. Clamped prefixes: t=1: 2<=4, t=4: 8<=9 -> no
        // STAR refutation even though unclamped t=7 would give 14 > 9.
        let s = State::canon([(2, 2), (2, 1), (2, 1), (2, 1), (2, 1), (2, 1)]);
        assert!(!t.star_refutes(&s.parts, 2));
    }
}
