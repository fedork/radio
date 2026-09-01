//! Unquotiented reference solver, for tests and the built-in selftest ONLY.
//!
//! Written from docs/problem.md alone: exhaustive recursion over the FULL Cartesian split
//! space - no symmetry quotient of any kind, no dominance, no majorization, no orderings.
//! Deliberately so: the 2026-08-31 trap (docs/status.md) was caught by exactly such an
//! unquotiented oracle after two individually-sound reductions composed unsoundly. This
//! module is the yardstick the audit engine is measured against; it must stay dumb.

use crate::state::{pow3, split_children, State};
use std::collections::HashMap;

pub struct Naive {
    memo: HashMap<(Vec<(u16, u16)>, usize), bool>,
}

impl Naive {
    pub fn new() -> Naive {
        Naive { memo: HashMap::new() }
    }

    /// Is `state` solvable in `k` tests? (docs/problem.md: base = at most one candidate
    /// pair; step = some split vector with all three children solvable in k-1.)
    pub fn solvable(&mut self, state: &State, k: usize) -> bool {
        if state.mass_full > pow3(k as u32) {
            return false; // INFO
        }
        if state.parts.is_empty() {
            return true; // UNIT: only unit parts (or nothing) within the bound
        }
        if k == 0 {
            return false; // a nonunit part has >= 2 candidate pairs
        }
        let key = (
            state.parts.iter().map(|p| (p.n, p.m)).collect::<Vec<_>>(),
            k,
        );
        if let Some(&v) = self.memo.get(&key) {
            return v;
        }
        let parts = state.parts.clone();
        let mut take = vec![(0u16, 0u16); parts.len()];
        let v = self.any_split(&parts, &mut take, 0, k);
        self.memo.insert(key, v);
        v
    }

    fn any_split(
        &mut self,
        parts: &[crate::state::Part],
        take: &mut Vec<(u16, u16)>,
        i: usize,
        k: usize,
    ) -> bool {
        if i == parts.len() {
            let (c0, c1, c2) = split_children(parts, take);
            return self.solvable(&c2, k - 1)
                && self.solvable(&c0, k - 1)
                && self.solvable(&c1, k - 1);
        }
        for a in 0..=parts[i].n {
            for b in 0..=parts[i].m {
                take[i] = (a, b);
                if self.any_split(parts, take, i + 1, k) {
                    return true;
                }
            }
        }
        false
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Known exact single-part maxima (data/pareto_sb.csv, status exact):
    /// n(k,1) = 2^k; n(2,2) = 3; n(3,3) = 8 - 3 = ... use only tiny, hand-checkable ones.
    #[test]
    fn known_small_values() {
        let mut nv = Naive::new();
        // n(1,1) = 2: Sb(2:1) solvable in 1, Sb(3:1) not.
        assert!(nv.solvable(&State::canon([(2, 1)]), 1));
        assert!(!nv.solvable(&State::canon([(3, 1)]), 1));
        // n(2,1) = 4.
        assert!(nv.solvable(&State::canon([(4, 1)]), 2));
        assert!(!nv.solvable(&State::canon([(5, 1)]), 2));
        // n(2,2) = 3.
        assert!(nv.solvable(&State::canon([(3, 2)]), 2));
        assert!(!nv.solvable(&State::canon([(4, 2)]), 2));
        // Sa-side sanity through Sb: Sb(2:2) solvable in 2.
        assert!(nv.solvable(&State::canon([(2, 2)]), 2));
        // The all-unit-solution shape from the 2026-08-31 trap: states whose only winning
        // split takes one coin from each side everywhere.
        assert!(nv.solvable(&State::canon([(1, 1), (1, 1), (1, 1)]), 2));
        // n(3,1) = 8.
        assert!(nv.solvable(&State::canon([(8, 1)]), 3));
        assert!(!nv.solvable(&State::canon([(9, 1)]), 3));
    }
}
