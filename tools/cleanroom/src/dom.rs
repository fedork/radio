//! DOM: componentwise-injective part dominance, from Subgraph Monotonicity.
//!
//! SUBMON (docs/theorems/subgraph-monotonicity.md): if G' is a subgraph of G and G is
//! solvable in k, so is G'. Corollaries used here, quoted from that document:
//!   1. Sub-multiset: Sb(S') is solvable in k whenever S' <= S as a multiset of parts.
//!   2. Componentwise part dominance: K_{n',m'} embeds in K_{n,m} when n' <= n, m' <= m.
//! Contrapositive, the DOM rule this module answers: a stored UNSOLVABLE fact F refutes a
//! query state Q iff there is an injection from F's parts into Q's parts that is
//! componentwise <= (then F is a subgraph of Q, so Q solvable would make F solvable).
//!
//! Only componentwise dominance is sound. In particular "move a coin to the larger side"
//! is NOT (docs/status.md trap: Sb(15:2,5:4) is solvable in 4, Sb(15:2,6:3) is not).
//!
//! Two implementations:
//!   * `dominates_naive` - bipartite matching (Kuhn augmenting paths). The reference; used
//!     in tests and for differential validation of the index.
//!   * `ClosureIndex` - insert-time materialization of each fact's upward closure into a
//!     trie of canonically sorted part-id sequences, so a query is an exact sub-multiset
//!     walk with no matching. Same idea as the production dominance cache; implementation
//!     written from the lemma. Soundness: every stored sequence W satisfies F ⊑ W for its
//!     source fact (built by chains of single-part growth); a query Q containing W as a
//!     sub-multiset then has F ⊑ W ⊆ Q. Completeness: if F ⊑ Q via injection φ, the sorted
//!     tuple of φ-images is in the closure (each image grows one part) and is a
//!     sub-multiset of Q, and both being sorted by the same total order, the walk finds it.
//!
//! The closure is bounded by the necessary region, per the documented rule "bound negative
//! cache closure by the same necessary region as search, and quotient equal parts"
//! (docs/status.md): a replacement state that INFO- or STAR-fails at the fact level never
//! serves a query, because the audit rejects such children before consulting DOM. STAR is
//! monotone for this purpose: growing a part or adding a part only raises every prefix sum
//! of the star expansion, so a STAR-refuted state has all its dominators STAR-refuted.

use crate::gk::GTables;
use crate::state::{part_key, Part, State};

/// Reference matcher: does `fact` inject into `query` componentwise (DOM)?
pub fn dominates_naive(fact: &[Part], query: &[Part]) -> bool {
    if fact.len() > query.len() {
        return false;
    }
    // Kuhn's augmenting-path bipartite matching, fact parts -> query slots.
    let mut match_of_query: Vec<Option<usize>> = vec![None; query.len()];
    fn try_augment(
        fi: usize,
        fact: &[Part],
        query: &[Part],
        seen: &mut [bool],
        match_of_query: &mut [Option<usize>],
    ) -> bool {
        for (qi, q) in query.iter().enumerate() {
            if !seen[qi] && fact[fi].dominated_by(*q) {
                seen[qi] = true;
                let free = match match_of_query[qi] {
                    None => true,
                    Some(other) => try_augment(other, fact, query, seen, match_of_query),
                };
                if free {
                    match_of_query[qi] = Some(fi);
                    return true;
                }
            }
        }
        false
    }
    for fi in 0..fact.len() {
        let mut seen = vec![false; query.len()];
        if !try_augment(fi, fact, query, &mut seen, &mut match_of_query) {
            return false;
        }
    }
    true
}

/// Dense part-id table over the universe of parts that can appear in queries at one level:
/// n >= m >= 1, n <= n_max, mass <= mass_cap (INFO at the fact level). Ids are assigned in
/// descending part_key order, so id order == canonical state order.
pub struct PartUniverse {
    pub parts: Vec<Part>,            // id -> part, descending part_key
    id_of: Vec<u32>,                 // (n-1) * m_stride + (m-1) -> id + 1, 0 = absent
    m_stride: usize,
    /// For each id, ids of all parts >= it componentwise (its up-set), ascending id.
    pub up: Vec<Vec<u32>>,
}

pub const NO_ID: u32 = 0;

impl PartUniverse {
    pub fn build(n_max: u16, mass_cap: u32) -> PartUniverse {
        let mut parts = Vec::new();
        for n in 1..=n_max {
            for m in 1..=n {
                let p = Part { n, m };
                if p.mass() <= mass_cap {
                    parts.push(p);
                }
            }
        }
        parts.sort_unstable_by(|a, b| part_key(*b).cmp(&part_key(*a)));
        let m_stride = n_max as usize;
        let mut id_of = vec![NO_ID; n_max as usize * m_stride];
        for (i, p) in parts.iter().enumerate() {
            id_of[(p.n as usize - 1) * m_stride + (p.m as usize - 1)] = i as u32 + 1;
        }
        let mut up: Vec<Vec<u32>> = vec![Vec::new(); parts.len()];
        for (i, p) in parts.iter().enumerate() {
            for (j, q) in parts.iter().enumerate() {
                if p.dominated_by(*q) {
                    up[i].push(j as u32);
                }
            }
        }
        PartUniverse { parts, id_of, m_stride, up }
    }

    /// id of a canonical part, or None when outside the universe (such a part can never
    /// appear in a query this index serves, see module comment).
    pub fn id(&self, p: Part) -> Option<u32> {
        if p.n == 0 || p.m == 0 || p.n as usize > self.m_stride {
            return None;
        }
        let v = self.id_of[(p.n as usize - 1) * self.m_stride + (p.m as usize - 1)];
        if v == NO_ID {
            None
        } else {
            Some(v - 1)
        }
    }
}

#[derive(Clone, Default)]
struct TrieNode {
    /// Sorted ascending by part id. Small: linear/binary search.
    edges: Vec<(u32, u32)>, // (part id, child node index)
    terminal: bool,
    /// Filled by seal(): minimum tuple length and minimum tuple mass from here to any
    /// terminal. A walk whose remaining query cannot meet either bound must fail; this
    /// prunes failure exploration (a stored tuple is a sub-multiset of the query, so the
    /// query remainder needs at least that many parts and at least that much mass).
    min_len: u8,
    min_mass: u32,
}

/// Insert-time up-closure dominance index (see module comment).
pub struct ClosureIndex {
    nodes: Vec<TrieNode>,
    pub inserted_tuples: u64,
    pub revisits: u64,
    /// Minimum stripped mass over the ORIGINAL facts inserted. A fact F refutes Q only
    /// with mass(F) <= mass(Q) (componentwise injection can only grow mass), so queries
    /// lighter than every fact can skip the walk outright.
    pub min_fact_mass: u32,
}

impl ClosureIndex {
    pub fn new() -> ClosureIndex {
        ClosureIndex {
            nodes: vec![TrieNode::default()],
            inserted_tuples: 0,
            revisits: 0,
            min_fact_mass: u32::MAX,
        }
    }

    fn child(&mut self, node: usize, id: u32) -> (usize, bool) {
        match self.nodes[node].edges.binary_search_by_key(&id, |e| e.0) {
            Ok(pos) => (self.nodes[node].edges[pos].1 as usize, true),
            Err(pos) => {
                let idx = self.nodes.len();
                self.nodes.push(TrieNode::default());
                self.nodes[node].edges.insert(pos, (id, idx as u32));
                (idx, false)
            }
        }
    }

    /// Insert one exact sorted tuple (ascending ids = canonical descending parts).
    /// Returns true if the terminal was already present (dedup hit).
    fn insert_tuple(&mut self, ids: &[u32]) -> bool {
        let mut node = 0usize;
        let mut all_existing = true;
        for &id in ids {
            let (next, existed) = self.child(node, id);
            all_existing &= existed;
            node = next;
        }
        let was = self.nodes[node].terminal;
        self.nodes[node].terminal = true;
        was && all_existing
    }

    /// Insert a fact together with its upward closure inside the necessary region.
    /// `k` is the fact's level (queries are states tested for refutation at this level).
    /// Closure = transitive single-part growth, deduped against the trie itself; a
    /// replacement state is pruned when INFO (mass > 3^k) or STAR refutes it at level k,
    /// since the audit rejects such a query before consulting DOM. Equal-part choices are
    /// quotiented: growing either of two equal parts yields the same multiset.
    /// Is `ids` already refuted by what has been inserted so far? Used to drop redundant
    /// facts before their closure is expanded: if a stored F' injects into F, then every
    /// query F would refute is already refuted by F', so F contributes nothing. This is
    /// Subgraph Monotonicity corollary 3 - "certificates may store antichains" - applied at
    /// build time. Same walk as FrozenIndex::refutes, without the sealed bounds.
    pub fn refuted_by_existing(&self, ids: &[u32]) -> bool {
        self.walk_builder(0, ids)
    }

    fn walk_builder(&self, node: usize, ids: &[u32]) -> bool {
        let n = &self.nodes[node];
        if n.terminal {
            return true;
        }
        let mut qi = 0;
        while qi < ids.len() {
            let id = ids[qi];
            if qi == 0 || ids[qi - 1] != id {
                if let Ok(pos) = n.edges.binary_search_by_key(&id, |e| e.0) {
                    if self.walk_builder(n.edges[pos].1 as usize, &ids[qi + 1..]) {
                        return true;
                    }
                }
            }
            qi += 1;
        }
        false
    }

    /// Returns false when the fact was dropped as redundant (see refuted_by_existing) or
    /// as unusable, true when its closure was expanded.
    pub fn insert_closure(
        &mut self,
        fact: &State,
        k: usize,
        uni: &PartUniverse,
        g: &GTables,
    ) -> bool {
        // Fact parts as ids; a fact part outside the universe cannot be grown into any
        // query part either, but the fact itself may still refute queries containing it
        // exactly... a part outside the universe has mass > 3^k, and any query containing
        // a part >= it also INFO-fails, so the whole fact is useless for DOM: skip it.
        let mut ids: Vec<u32> = Vec::with_capacity(fact.parts.len());
        for p in &fact.parts {
            match uni.id(*p) {
                Some(i) => ids.push(i),
                None => return false,
            }
        }
        ids.sort_unstable();
        // Antichain reduction. Callers insert in nondecreasing (mass, parts) order, so any
        // F' that injects into this fact has already been inserted; if one has, this fact's
        // whole closure is redundant.
        if self.refuted_by_existing(&ids) {
            return false;
        }
        self.min_fact_mass = self.min_fact_mass.min(fact.mass_stripped());
        let cap = 3u64.pow(k as u32);
        self.grow(&mut ids, k, cap, uni, g);
        true
    }

    fn grow(&mut self, ids: &mut Vec<u32>, k: usize, cap: u64, uni: &PartUniverse, g: &GTables) {
        let mut next = Vec::with_capacity(ids.len() + 1);
        let mut runs = Vec::with_capacity(ids.len() + 1);
        self.grow_inner(ids, k, cap, uni, g, &mut next, &mut runs);
    }

    /// Scratch-carrying body. The two buffers are reused down the whole recursion: cloning
    /// the id vector and collecting a Vec<Part> per CANDIDATE replacement - before the STAR
    /// test rejected most of them - was the bulk of this build's cost.
    fn grow_inner(
        &mut self,
        ids: &mut Vec<u32>,
        k: usize,
        cap: u64,
        uni: &PartUniverse,
        g: &GTables,
        next: &mut Vec<u32>,
        runs: &mut Vec<(u64, u64)>,
    ) {
        if self.insert_tuple(ids) {
            self.revisits += 1;
            return; // already present: its closure was already expanded
        }
        self.inserted_tuples += 1;
        for i in 0..ids.len() {
            // SYM-E analog for closure: growing either of two equal parts produces the
            // same multiset; expand only the first of an equal run.
            if i > 0 && ids[i] == ids[i - 1] {
                continue;
            }
            let orig = ids[i];
            let base_mass: u64 = ids
                .iter()
                .enumerate()
                .filter(|&(j, _)| j != i)
                .map(|(_, &id)| uni.parts[id as usize].mass() as u64)
                .sum();
            // Clone the up-set list handle; iterate replacements.
            let ups = &uni.up[orig as usize];
            for &w in ups {
                if w == orig {
                    continue;
                }
                // Necessary-region bound: prune replacement states that INFO- or
                // STAR-fail at level k (docs/status.md: "Bound negative cache closure by
                // the same necessary region as search").
                if base_mass + uni.parts[w as usize].mass() as u64 > cap {
                    continue;
                }
                next.clear();
                next.extend_from_slice(ids);
                next[i] = w;
                next.sort_unstable();
                if g.star_refutes_ids(next, &uni.parts, k, runs) {
                    continue;
                }
                // The recursion needs its own buffers; `next` is this level's.
                let mut child = std::mem::take(next);
                self.grow(&mut child, k, cap, uni, g);
                *next = child;
            }
        }
    }

    /// Freeze into the flat query-time form. Consumes the builder.
    pub fn seal(mut self, uni: &PartUniverse) -> FrozenIndex {
        // Post-order without recursion: children always have larger indices than their
        // parent (nodes are appended on first descent), so a reverse index sweep works.
        for i in (0..self.nodes.len()).rev() {
            let mut min_len = if self.nodes[i].terminal { 0u32 } else { u32::MAX };
            let mut min_mass = if self.nodes[i].terminal { 0u32 } else { u32::MAX };
            for e in 0..self.nodes[i].edges.len() {
                let (id, child) = self.nodes[i].edges[e];
                let c = &self.nodes[child as usize];
                if c.min_len as u32 + 1 < min_len {
                    min_len = c.min_len as u32 + 1;
                }
                let m = c.min_mass.saturating_add(uni.parts[id as usize].mass());
                if m < min_mass {
                    min_mass = m;
                }
            }
            self.nodes[i].min_len = min_len.min(200) as u8;
            self.nodes[i].min_mass = min_mass;
        }
        let n = self.nodes.len();
        let mut f = FrozenIndex {
            terminal: Vec::with_capacity(n),
            min_len: Vec::with_capacity(n),
            min_mass: Vec::with_capacity(n),
            edge_start: Vec::with_capacity(n + 1),
            edge_id: Vec::new(),
            edge_child: Vec::new(),
            root_child: vec![u32::MAX; uni.parts.len()],
            inserted_tuples: self.inserted_tuples,
            node_count: n,
            min_fact_mass: self.min_fact_mass,
        };
        for node in &self.nodes {
            f.terminal.push(node.terminal);
            f.min_len.push(node.min_len);
            f.min_mass.push(node.min_mass);
            f.edge_start.push(f.edge_id.len() as u32);
            for &(id, child) in &node.edges {
                f.edge_id.push(id);
                f.edge_child.push(child);
            }
        }
        f.edge_start.push(f.edge_id.len() as u32);
        // Direct root dispatch: the root's edge list is the largest by far and is probed
        // once per query part; an array lookup replaces its binary search.
        for &(id, child) in &self.nodes[0].edges {
            f.root_child[id as usize] = child;
        }
        f
    }
}

/// Flat, read-only form of the closure trie (see ClosureIndex): contiguous edge arrays,
/// per-node minimum-requirement bounds, direct root dispatch. Semantics identical.
pub struct FrozenIndex {
    terminal: Vec<bool>,
    min_len: Vec<u8>,
    min_mass: Vec<u32>,
    edge_start: Vec<u32>,
    edge_id: Vec<u32>,
    edge_child: Vec<u32>,
    root_child: Vec<u32>,
    pub inserted_tuples: u64,
    pub node_count: usize,
    pub min_fact_mass: u32,
}

impl FrozenIndex {
    /// DOM query: is some stored tuple a sub-multiset of the query? `ids` ascending
    /// (canonical order), `suffix_mass[qi]` = total mass of parts qi.. (so
    /// suffix_mass[len] = 0); both prepared by the caller.
    pub fn refutes(&self, ids: &[u32], suffix_mass: &[u32]) -> bool {
        // Root level inlined with direct dispatch; terminal root would mean an empty
        // stored tuple, which insert_closure never produces.
        let mut qi = 0;
        while qi < ids.len() {
            let id = ids[qi];
            if qi == 0 || ids[qi - 1] != id {
                let child = self.root_child[id as usize];
                if child != u32::MAX && self.walk(child, &ids[qi + 1..], &suffix_mass[qi + 1..])
                {
                    return true;
                }
            }
            qi += 1;
        }
        false
    }

    fn walk(&self, node: u32, ids: &[u32], suffix_mass: &[u32]) -> bool {
        let ni = node as usize;
        if self.terminal[ni] {
            return true;
        }
        // Minimum-requirement pruning (computed in seal()).
        if (ids.len() as u32) < self.min_len[ni] as u32 || suffix_mass[0] < self.min_mass[ni] {
            return false;
        }
        let lo = self.edge_start[ni] as usize;
        let hi = self.edge_start[ni + 1] as usize;
        let edge_ids = &self.edge_id[lo..hi];
        // For each DISTINCT remaining query part, descend along a matching edge if one
        // exists. Descending at a part's first occurrence subsumes its later duplicates
        // (the suffix only shrinks), so duplicates are skipped.
        let mut qi = 0;
        while qi < ids.len() {
            let id = ids[qi];
            if qi > 0 && ids[qi - 1] == id {
                qi += 1;
                continue;
            }
            if let Ok(pos) = edge_ids.binary_search(&id) {
                if self.walk(
                    self.edge_child[lo + pos],
                    &ids[qi + 1..],
                    &suffix_mass[qi + 1..],
                ) {
                    return true;
                }
            }
            qi += 1;
        }
        false
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn naive_matcher_needs_crossing() {
        // F = {(4:4),(10:1)}, Q = {(10:2),(5:4)}: only the crossing injection works.
        let f = [Part { n: 4, m: 4 }, Part { n: 10, m: 1 }];
        let q = [Part { n: 10, m: 2 }, Part { n: 5, m: 4 }];
        assert!(dominates_naive(&f, &q));
        // And a near-miss does not:
        let q2 = [Part { n: 10, m: 2 }, Part { n: 5, m: 3 }];
        assert!(!dominates_naive(&f, &q2));
    }

    #[test]
    fn naive_matcher_basics() {
        let f = [Part { n: 3, m: 2 }];
        assert!(dominates_naive(&f, &[Part { n: 3, m: 2 }]));
        assert!(dominates_naive(&f, &[Part { n: 4, m: 2 }, Part { n: 1, m: 1 }]));
        assert!(!dominates_naive(&f, &[Part { n: 2, m: 2 }, Part { n: 3, m: 1 }]));
        assert!(!dominates_naive(
            &[Part { n: 2, m: 1 }, Part { n: 2, m: 1 }],
            &[Part { n: 5, m: 4 }]
        ));
    }

    /// Differential: the closure index agrees with the naive matcher on an exhaustive
    /// small universe. This is the load-bearing test for the index.
    #[test]
    fn closure_index_matches_naive_exhaustively() {
        let k = 4usize; // cap 81
        let g = GTables::build(k);
        let uni = PartUniverse::build(14, 81);
        // Facts: a deliberately awkward set, including equal parts and the crossing shape.
        let facts: Vec<State> = vec![
            State::canon([(4, 4), (5, 1)]),
            State::canon([(3, 2), (3, 2)]),
            State::canon([(9, 1)]),
            State::canon([(2, 2), (2, 1), (2, 1)]),
            State::canon([(14, 1), (4, 4)]),
            State::canon([(5, 3)]),
        ];
        let mut builder = ClosureIndex::new();
        for f in &facts {
            builder.insert_closure(f, k, &uni, &g);
        }
        let idx = builder.seal(&uni);
        // All queries with up to 3 parts drawn from a small pool, within INFO and STAR at
        // level k (the region DOM is consulted in).
        let pool: Vec<Part> = uni.parts.clone();
        let mut checked = 0u64;
        let mut against = |parts: &[Part]| {
            let s = State::canon(parts.iter().map(|p| (p.n, p.m)));
            if s.mass_full > 27 || g.star_refutes(&s.parts, k) {
                return;
            }
            let expect = facts.iter().any(|f| dominates_naive(&f.parts, &s.parts));
            let ids: Vec<u32> = s.parts.iter().map(|p| uni.id(*p).unwrap()).collect();
            // ids ascending: canon sorts descending part_key = ascending id.
            let mut sm = vec![0u32; s.parts.len() + 1];
            for i in (0..s.parts.len()).rev() {
                sm[i] = sm[i + 1] + s.parts[i].mass();
            }
            let got = idx.refutes(&ids, &sm);
            assert_eq!(expect, got, "state {:?}", s.parts);
            checked += 1;
        };
        for i in 0..pool.len() {
            against(&[pool[i]]);
            for j in i..pool.len() {
                against(&[pool[i], pool[j]]);
                for l in j..pool.len() {
                    against(&[pool[i], pool[j], pool[l]]);
                }
            }
        }
        assert!(checked > 1_500, "only {checked} queries in the differential");
    }
}
