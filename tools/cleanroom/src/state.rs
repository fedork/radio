//! States and split semantics, from docs/problem.md alone.
//!
//! Lemma tags used across this crate (statements and proofs in the cited documents):
//!   SPLIT  - split semantics, docs/problem.md ("A test on an Sb state ...")
//!   INFO   - information bound mass(S) <= 3^k, docs/problem.md
//!   CANON  - canonical form: orient n>=m, drop empty, sort descending, docs/problem.md
//!   UNIT   - Unit-Group Elimination, docs/theorems/unit-group-elimination.md:
//!            S = R + units is solvable in k  <=>  mass(S) <= 3^k and R solvable in k
//!   SUBMON - Subgraph Monotonicity, docs/theorems/subgraph-monotonicity.md
//!   DOM    - its corollary: componentwise-injective part dominance (see dom.rs)
//!   STAR   - full star-expansion majorization necessity (see gk.rs)
//!   SYM-S  - square-part shore swap (K_{n,n} is symmetric in its two shores)
//!   SYM-E  - equal-part permutation quotient (see audit.rs; the composition trap is
//!            documented in docs/status.md and evidence/singleton_direct_split_cleanroom_2026-08-31.md)

/// One component K_{n,m}, oriented n >= m >= 1 (CANON).
#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Debug)]
pub struct Part {
    pub n: u16,
    pub m: u16,
}

impl Part {
    /// CANON orientation. Returns None for an empty part (a zero shore kills every pair).
    pub fn new(n: u16, m: u16) -> Option<Part> {
        if n == 0 || m == 0 {
            return None;
        }
        Some(if n >= m { Part { n, m } } else { Part { n: m, m: n } })
    }

    pub fn mass(self) -> u32 {
        self.n as u32 * self.m as u32
    }

    pub fn is_unit(self) -> bool {
        self.n == 1 && self.m == 1
    }

    /// DOM on single parts: self embeds in other iff both coordinates fit (after CANON
    /// orientation; K_{a,b} embeds in K_{n,m} with a<=n, b<=m, and orientation is free
    /// because K_{n,m} = K_{m,n}).
    pub fn dominated_by(self, other: Part) -> bool {
        self.n <= other.n && self.m <= other.m
    }
}

/// Canonical total order for storing states: descending (mass, n, m).
/// Any fixed total order works for CANON; the choice matters only for enumeration cost
/// (it decides which part the audit splits first). Measured on the Sa(113) corpus:
/// mass-descending 2.45e9 cells vs long-side-first 8.78e9 - keep mass-descending.
pub fn part_key(p: Part) -> (u32, u16, u16) {
    (p.mass(), p.n, p.m)
}

/// A state: canonical multiset of parts. `mass_full` includes unit parts (UNIT keeps their
/// information cost even though they are structurally inert), `parts` has units stripped.
#[derive(Clone, PartialEq, Eq, Hash, Debug)]
pub struct State {
    pub parts: Vec<Part>,
    pub mass_full: u32,
}

impl State {
    /// CANON + UNIT: orient every part, drop empty parts, count all mass, strip units,
    /// sort descending by `part_key`.
    pub fn canon(raw: impl IntoIterator<Item = (u16, u16)>) -> State {
        let mut parts = Vec::new();
        let mut mass_full = 0u32;
        for (n, m) in raw {
            if let Some(p) = Part::new(n, m) {
                mass_full += p.mass();
                if !p.is_unit() {
                    parts.push(p);
                }
            }
        }
        parts.sort_unstable_by(|a, b| part_key(*b).cmp(&part_key(*a)));
        State { parts, mass_full }
    }

    /// Mass of the stripped parts only (mass_full minus one per unit part).
    pub fn mass_stripped(&self) -> u32 {
        self.parts.iter().map(|p| p.mass()).sum()
    }
}

pub fn pow3(k: u32) -> u32 {
    3u32.pow(k)
}

/// SPLIT: the three children of one split vector, as raw (n, m) pairs before CANON.
/// take[i] = (a_i, b_i) with 0 <= a_i <= n_i, 0 <= b_i <= m_i.
///   outcome 2 (both defectives taken):    (a_i : b_i) per part
///   outcome 0 (neither taken):            (n_i - a_i : m_i - b_i) per part
///   outcome 1 (exactly one taken):        (a_i : m_i - b_i) and (n_i - a_i : b_i) per part
pub fn split_children(parts: &[Part], take: &[(u16, u16)]) -> (State, State, State) {
    debug_assert_eq!(parts.len(), take.len());
    let c2 = State::canon(parts.iter().zip(take).map(|(_, &(a, b))| (a, b)));
    let c0 = State::canon(parts.iter().zip(take).map(|(p, &(a, b))| (p.n - a, p.m - b)));
    let c1 = State::canon(
        parts
            .iter()
            .zip(take)
            .flat_map(|(p, &(a, b))| [(a, p.m - b), (p.n - a, b)]),
    );
    (c0, c1, c2)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn canon_orients_strips_and_sorts() {
        let s = State::canon([(2, 5), (1, 1), (3, 1), (0, 7), (1, 1)]);
        assert_eq!(s.mass_full, 10 + 1 + 3 + 1);
        assert_eq!(
            s.parts,
            vec![Part { n: 5, m: 2 }, Part { n: 3, m: 1 }]
        );
        assert_eq!(s.mass_stripped(), 13);
    }

    #[test]
    fn split_children_match_problem_md_example() {
        // Sb(5:3), take (2,1): outcome2 = (2:1), outcome0 = (3:2), outcome1 = (2:2),(3:1).
        let parts = [Part { n: 5, m: 3 }];
        let (c0, c1, c2) = split_children(&parts, &[(2, 1)]);
        assert_eq!(c2.parts, vec![Part { n: 2, m: 1 }]);
        assert_eq!(c0.parts, vec![Part { n: 3, m: 2 }]);
        assert_eq!(c1.parts, vec![Part { n: 2, m: 2 }, Part { n: 3, m: 1 }]);
        // Mass is conserved across each outcome's candidate-pair partition:
        assert_eq!(c2.mass_full + c0.mass_full + c1.mass_full, 15);
    }

    #[test]
    fn unit_children_keep_mass() {
        // take (1,1) of (1:1)-to-be children still count toward mass (UNIT).
        let parts = [Part { n: 2, m: 2 }];
        let (c0, c1, c2) = split_children(&parts, &[(1, 1)]);
        assert_eq!(c2.parts, vec![]); // (1:1) stripped
        assert_eq!(c2.mass_full, 1); // but its pair is counted
        assert_eq!(c0.mass_full, 1);
        assert_eq!(c1.parts, vec![]); // two (1:1) parts stripped
        assert_eq!(c1.mass_full, 2);
    }
}
