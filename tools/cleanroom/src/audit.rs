//! The audit: verify one negative claim "Sb(...) is unsolvable in k" against a frozen set
//! of level-(k-1) negative facts, from first principles.
//!
//! Obligation (docs/problem.md: "A state is solvable in k if some test exists whose three
//! children are all solvable in k-1"): the claim is verified iff EVERY legal split has at
//! least one child shown unsolvable at k-1. A child is shown unsolvable only by:
//!   INFO   - its mass exceeds 3^(k-1) (docs/problem.md);
//!   STAR   - its star expansion violates weak majorization by G_{k-1} (gk.rs);
//!   DOM    - a stored level-(k-1) fact injects componentwise into it (dom.rs), so by
//!            SUBMON its unsolvability transfers upward.
//! There is no recursion, no budget, and no other rule: a split none of whose children is
//! discharged is a GAP and the audit fails loudly.
//!
//! Sound reductions of the enumeration (each is an application of the lemmas above, not a
//! new assumption):
//!
//!   partial children (SUBMON) - after choosing options for a prefix of the parts, the
//!     prefix's partial child is a sub-multiset of every completion's child, so a refuted
//!     partial child discharges all completions at once.
//!
//!   dead options (SUBMON) - an option whose OWN one-part children already contain a
//!     refuted piece discharges every split using it; such options are removed from the
//!     table up front, once per (part, k).
//!
//!   monotone level retirement (INFO) - options are enumerated in ascending order of one
//!     child's mass, so the first option whose cumulative mass for that child exceeds
//!     3^(k-1) proves every later option in this order fails the same INFO test; the rest
//!     of the level is retired wholesale. Which child's order to use is chosen per level
//!     to minimize the number of enumerable options (a performance choice only).
//!
//!   SYM-E, equal-part quotient - adjacent equal parts share one option table; permuting
//!     their choices permutes children, which are multisets, so option indices may be
//!     required non-decreasing (NEVER strictly increasing - equal choices are distinct
//!     legal splits collapsing to one representative). This is the ONLY split-space
//!     quotient used. The complement/global-flip quotient is deliberately absent: it was
//!     measured at 0.01% of work, and combining two quotients without a compatibility
//!     proof is the documented 2026-08-31 false-negative trap (docs/status.md,
//!     evidence/singleton_direct_split_cleanroom_2026-08-31.md).
//!
//!   SYM-S, square-part shore swap - for a part with n == m the two shores are
//!     interchangeable, so options are generated with b <= a. This is a restriction of one
//!     part's own option DOMAIN (K_{n,n} automorphism), independent of SYM-E, which acts
//!     on choices ACROSS equal parts of the shared restricted table.

use crate::dom::{ClosureIndex, PartUniverse};
use crate::gk::GTables;
use crate::state::{part_key, pow3, split_children, Part, State};
use std::collections::HashMap;

const MAX_QUERY_PARTS: usize = 96;

#[derive(Clone, Copy)]
struct Opt {
    a: u16,
    b: u16,
    m0: u32,
    m1: u32,
    m2: u32,
    /// Position in the shared live table; the SYM-E non-decreasing constraint compares
    /// these. Any fixed order works; this is generation order (a asc, then b asc).
    static_idx: u32,
}

struct OptTable {
    opts: Vec<Opt>,
    /// ord[j] = option positions sorted ascending by child-j mass; keys[j] the masses in
    /// that order (for the INFO retirement and the count-minimizing order choice).
    ord: [Vec<u32>; 3],
    keys: [Vec<u32>; 3],
}

pub struct Auditor {
    pub k: usize,
    pub g: GTables,
    pub uni: PartUniverse,
    pub dom: ClosureIndex,
    tables: HashMap<Part, OptTable>,
    pub dead_options: u64,
    pub total_options: u64,
}

#[derive(Debug)]
pub enum Verdict {
    /// Every split has a discharged child.
    Verified,
    /// The claim state is itself solvable by a base rule (UNIT: units-only within the
    /// information bound) - the certificate asserts a falsehood.
    Contradicted,
    /// A concrete split none of whose children could be discharged.
    Gap { take: Vec<(u16, u16)> },
}

/// Why a child state is unsolvable at level kk, or None.
#[inline]
fn refuted_reason(
    g: &GTables,
    uni: &PartUniverse,
    dom: &ClosureIndex,
    kk: usize,
    mass_full: u32,
    parts: &[Part], // canonical order (descending part_key), units stripped
    ids: &mut Vec<u32>,
    suffix_mass: &mut Vec<u32>,
) -> bool {
    // INFO
    if mass_full as u64 > 3u64.pow(kk as u32) {
        return true;
    }
    // STAR (rejection-only; sound on the stripped parts by UNIT)
    if g.star_refutes(parts, kk) {
        return true;
    }
    // DOM. Mass prefilter: an injecting fact cannot outweigh the query (dom.rs).
    if parts.is_empty() {
        return false;
    }
    ids.clear();
    suffix_mass.clear();
    suffix_mass.resize(parts.len() + 1, 0);
    let mut acc = 0u32;
    for (i, p) in parts.iter().enumerate().rev() {
        match uni.id(*p) {
            // A part outside the universe has mass beyond 3^kk, but INFO passed above -
            // impossible by construction (universe covers every part with mass <= 3^kk and
            // n <= n_max, and query pieces never exceed claim coordinates).
            None => unreachable!("query part outside DOM universe"),
            Some(_) => {}
        }
        acc += p.mass();
        suffix_mass[i] = acc;
    }
    if acc < dom.min_fact_mass {
        return false;
    }
    for p in parts {
        ids.push(uni.id(*p).unwrap());
    }
    dom.refutes(ids, suffix_mass)
}

/// Canonicalize raw pieces into `out` (descending part_key, units stripped), no allocation
/// beyond `out`'s capacity. Equivalent to State::canon on the same pieces.
#[inline]
fn canon_into(pieces: impl Iterator<Item = (u16, u16)>, out: &mut Vec<Part>) {
    out.clear();
    for (n, m) in pieces {
        if n == 0 || m == 0 {
            continue;
        }
        let p = if n >= m { Part { n, m } } else { Part { n: m, m: n } };
        if p.is_unit() {
            continue;
        }
        // insertion sort, descending part_key
        let key = part_key(p);
        let mut i = out.len();
        out.push(p);
        while i > 0 && part_key(out[i - 1]) < key {
            out[i] = out[i - 1];
            i -= 1;
        }
        out[i] = p;
    }
}

/// Per-worker scratch to keep the hot path allocation-free.
pub struct Scratch {
    child: Vec<Part>,
    ids: Vec<u32>,
    suffix_mass: Vec<u32>,
    take: Vec<(u16, u16)>,
    prev_static: Vec<u32>,
    p0: Vec<u32>,
    p1: Vec<u32>,
    p2: Vec<u32>,
    /// Charged candidate cells (comparable to the reference verifier's prefix count).
    pub cells: u64,
}

impl Scratch {
    pub fn new() -> Scratch {
        Scratch {
            child: Vec::with_capacity(MAX_QUERY_PARTS),
            ids: Vec::with_capacity(MAX_QUERY_PARTS),
            suffix_mass: Vec::with_capacity(MAX_QUERY_PARTS + 1),
            take: Vec::new(),
            prev_static: Vec::new(),
            p0: Vec::new(),
            p1: Vec::new(),
            p2: Vec::new(),
            cells: 0,
        }
    }
}

impl Auditor {
    /// Build the audit context for one level: claims at `k`, facts at `k - 1`.
    /// `claim_parts` must contain every distinct part occurring in any claim.
    pub fn build(k: usize, claim_parts: &[Part], facts: &[State]) -> Auditor {
        assert!(k >= 1);
        let g = GTables::build(k.max(1));
        let n_max = claim_parts.iter().map(|p| p.n).max().unwrap_or(1);
        let cap = pow3(k as u32 - 1);
        let uni = PartUniverse::build(n_max, cap);
        let mut dom = ClosureIndex::new();
        for f in facts {
            // A fact with units stripped is what refutes (UNIT); mass screening of queries
            // is INFO's job, not the index's.
            dom.insert_closure(f, k - 1, &uni, &g);
        }
        dom.seal(&uni);
        let mut aud = Auditor {
            k,
            g,
            uni,
            dom,
            tables: HashMap::new(),
            dead_options: 0,
            total_options: 0,
        };
        for p in claim_parts {
            aud.ensure_table(*p);
        }
        aud
    }

    fn ensure_table(&mut self, p: Part) {
        if p.is_unit() || self.tables.contains_key(&p) {
            return;
        }
        let kk = self.k - 1;
        let mut opts = Vec::new();
        let mut scratch_ids = Vec::new();
        let mut scratch_sm = Vec::new();
        for a in 0..=p.n {
            // SYM-S: for a square part the two shores are interchangeable.
            let b_hi = if p.n == p.m { a.min(p.m) } else { p.m };
            for b in 0..=b_hi {
                self.total_options += 1;
                let (c0, c1, c2) = split_children(&[p], &[(a, b)]);
                // Dead option (SUBMON): any refuted one-part child kills every completion.
                let dead = [&c0, &c1, &c2].iter().any(|c| {
                    refuted_reason(
                        &self.g,
                        &self.uni,
                        &self.dom,
                        kk,
                        c.mass_full,
                        &c.parts,
                        &mut scratch_ids,
                        &mut scratch_sm,
                    )
                });
                if dead {
                    self.dead_options += 1;
                    continue;
                }
                opts.push(Opt {
                    a,
                    b,
                    m0: c0.mass_full,
                    m1: c1.mass_full,
                    m2: c2.mass_full,
                    static_idx: 0,
                });
            }
        }
        for (i, o) in opts.iter_mut().enumerate() {
            o.static_idx = i as u32;
        }
        let mut ord = [Vec::new(), Vec::new(), Vec::new()];
        let mut keys = [Vec::new(), Vec::new(), Vec::new()];
        for j in 0..3 {
            let mass_of = |o: &Opt| match j {
                0 => o.m0,
                1 => o.m1,
                _ => o.m2,
            };
            let mut idx: Vec<u32> = (0..opts.len() as u32).collect();
            idx.sort_by_key(|&i| mass_of(&opts[i as usize]));
            keys[j] = idx.iter().map(|&i| mass_of(&opts[i as usize])).collect();
            ord[j] = idx;
        }
        self.tables.insert(p, OptTable { opts, ord, keys });
    }

    /// Audit one claim. `parts` must be canonical (State::canon), `mass_full` its full
    /// mass including units.
    pub fn audit_claim(&self, parts: &[Part], mass_full: u32, s: &mut Scratch) -> Verdict {
        let cap_k = 3u64.pow(self.k as u32);
        // INFO refutes the claim state itself: the claim is true outright.
        if mass_full as u64 > cap_k {
            return Verdict::Verified;
        }
        // UNIT: a state that is units-only within the information bound is solvable, so
        // the claim is false. (mass 0/1 is likewise solvable; canon leaves those empty or
        // unit-only... a single pair IS one unit part.)
        if parts.is_empty() {
            return Verdict::Contradicted;
        }
        // STAR refutes the claim state itself at level k: verified outright.
        if self.g.star_refutes(parts, self.k) {
            return Verdict::Verified;
        }
        s.take.clear();
        s.prev_static.clear();
        s.p0.clear();
        s.p1.clear();
        s.p2.clear();
        match self.descend(parts, 0, s) {
            None => Verdict::Verified,
            Some(()) => Verdict::Gap { take: s.take.clone() },
        }
    }

    /// Enumerate options for `parts[depth..]` given the prefix recorded in the scratch
    /// stacks. Returns Some(()) when an uncovered split was found (its take vector is left
    /// in s.take).
    fn descend(&self, parts: &[Part], depth: usize, s: &mut Scratch) -> Option<()> {
        let cap = pow3(self.k as u32 - 1);
        let part = parts[depth];
        let table = &self.tables[&part];
        let (pp0, pp1, pp2) = if depth == 0 {
            (0, 0, 0)
        } else {
            (s.p0[depth - 1], s.p1[depth - 1], s.p2[depth - 1])
        };
        // Choose the enumeration order minimizing enumerable options under the remaining
        // caps (performance only; every order is exhaustive up to its own retirement).
        let mut best_j = 0usize;
        let mut best_count = usize::MAX;
        for j in 0..3 {
            let budget = cap.saturating_sub([pp0, pp1, pp2][j]);
            let count = table.keys[j].partition_point(|&m| m <= budget);
            if count < best_count {
                best_count = count;
                best_j = j;
            }
        }
        let ord = &table.ord[best_j];
        let keys = &table.keys[best_j];
        // SYM-E: adjacent equal parts require non-decreasing static option index.
        let min_static = if depth > 0 && parts[depth - 1] == part {
            s.prev_static[depth - 1]
        } else {
            0
        };
        let prefix_key = [pp0, pp1, pp2][best_j];

        for (pos, &oi) in ord.iter().enumerate() {
            // INFO retirement: keys ascend in this order; the first violation retires the
            // whole rest of the level.
            if prefix_key + keys[pos] > cap {
                break;
            }
            let o = &table.opts[oi as usize];
            if o.static_idx < min_static {
                continue; // SYM-E representative filter; not a charged cell
            }
            s.cells += 1;
            let (q0, q1, q2) = (pp0 + o.m0, pp1 + o.m1, pp2 + o.m2);
            // INFO on the other two partial children (the ordered one is within cap here).
            if q0 > cap || q1 > cap || q2 > cap {
                continue;
            }
            s.take.truncate(depth);
            s.take.push((o.a, o.b));
            // Partial children (SUBMON): outcome 2, then 0, then 1.
            let covered = self.child_refuted(parts, s, 2, q2)
                || self.child_refuted(parts, s, 0, q0)
                || self.child_refuted(parts, s, 1, q1);
            if covered {
                continue;
            }
            if depth + 1 == parts.len() {
                // Complete split, no child discharged: the audit fails with this witness.
                return Some(());
            }
            s.prev_static.truncate(depth);
            s.prev_static.push(o.static_idx);
            s.p0.truncate(depth);
            s.p1.truncate(depth);
            s.p2.truncate(depth);
            s.p0.push(q0);
            s.p1.push(q1);
            s.p2.push(q2);
            if self.descend(parts, depth + 1, s).is_some() {
                return Some(());
            }
        }
        None
    }

    #[inline]
    fn child_refuted(&self, parts: &[Part], s: &mut Scratch, outcome: u8, mass: u32) -> bool {
        let depth = s.take.len();
        // Borrow-splitting: temporarily move the scratch buffers out.
        let mut child = std::mem::take(&mut s.child);
        match outcome {
            2 => canon_into(
                s.take.iter().map(|&(a, b)| (a, b)),
                &mut child,
            ),
            0 => canon_into(
                parts[..depth]
                    .iter()
                    .zip(&s.take)
                    .map(|(p, &(a, b))| (p.n - a, p.m - b)),
                &mut child,
            ),
            _ => canon_into(
                parts[..depth]
                    .iter()
                    .zip(&s.take)
                    .flat_map(|(p, &(a, b))| [(a, p.m - b), (p.n - a, b)]),
                &mut child,
            ),
        }
        let mut ids = std::mem::take(&mut s.ids);
        let mut sm = std::mem::take(&mut s.suffix_mass);
        let r = refuted_reason(
            &self.g,
            &self.uni,
            &self.dom,
            self.k - 1,
            mass,
            &child,
            &mut ids,
            &mut sm,
        );
        s.child = child;
        s.ids = ids;
        s.suffix_mass = sm;
        r
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Smallest end-to-end case: Sb(2:1) is solvable in 1 (test one coin of the 2-side),
    /// Sb(3:1) is not (mass 3 = 3^1 but no split works... actually mass 3 <= 3, check the
    /// enumeration finds the gap-free refutation given the right support).
    #[test]
    fn single_part_k1() {
        // Claim: Sb(3:1) unsolvable in 1. At k-1 = 0, a child is refuted iff mass > 1.
        // Support: empty (INFO alone must do it... a (1:1) child has mass 1 <= 1 and is
        // solvable; children (2:1)... let's just check the verdicts are sane).
        let claim = State::canon([(3, 1)]);
        let aud = Auditor::build(1, &claim.parts, &[]);
        let mut s = Scratch::new();
        match aud.audit_claim(&claim.parts, claim.mass_full, &mut s) {
            Verdict::Verified => {}
            v => panic!("Sb(3:1)@1 should verify: {v:?}"),
        }
        // Claim Sb(2:1) unsolvable in 1 is FALSE; with no support the audit must expose a
        // gap (it cannot prove solvability, but it must refuse to verify).
        let bad = State::canon([(2, 1)]);
        let aud = Auditor::build(1, &bad.parts, &[]);
        match aud.audit_claim(&bad.parts, bad.mass_full, &mut s) {
            Verdict::Gap { .. } => {}
            v => panic!("Sb(2:1)@1 must not verify: {v:?}"),
        }
    }

    #[test]
    fn units_only_claim_is_contradicted() {
        let c = State::canon([(1, 1), (1, 1)]);
        let aud = Auditor::build(2, &c.parts, &[]);
        let mut s = Scratch::new();
        match aud.audit_claim(&c.parts, c.mass_full, &mut s) {
            Verdict::Contradicted => {}
            v => panic!("units-only claim must contradict: {v:?}"),
        }
    }
}
