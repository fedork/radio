//! Strict parsers for the two certificate formats.
//!
//! Grammars from docs/certificate.md (level-v2) and docs/tools.md (v1). Both are parsed
//! fail-closed: any structural surprise is a hard error, never a skipped record. Masses
//! are always derived from the parts, never read from the file.

use crate::state::{part_key, Part, State};
use std::fmt::Write as _;

#[derive(Debug)]
pub struct LevelCert {
    pub level: usize,
    pub support: Vec<State>,
    pub claims: Vec<State>,
}

#[derive(Debug)]
pub struct FlatCert {
    /// claims_by_k[k] = negative claims asserted at level k (roots and facts alike; in a
    /// full audit every record is a claim, and level k's support is level k-1's claims).
    pub claims_by_k: Vec<Vec<State>>,
}

pub enum Cert {
    Level(LevelCert),
    Flat(FlatCert),
}

const V1: &str = "radio-negative-certificate-v1";
const V2: &str = "radio-negative-level-certificate-v2";

fn fail(path: &str, lineno: usize, msg: &str) -> ! {
    eprintln!("{path}:{lineno}: {msg}");
    std::process::exit(2);
}

fn data_lines(text: &str) -> impl Iterator<Item = (usize, &str)> {
    text.lines()
        .enumerate()
        .map(|(i, l)| (i + 1, l.trim()))
        .filter(|(_, l)| !l.is_empty() && !l.starts_with('#'))
}

pub fn parse(path: &str, text: &str) -> Cert {
    let first = data_lines(text).next();
    match first {
        Some((_, l)) if l == V2 => Cert::Level(parse_v2(path, text)),
        Some((_, l)) if l.starts_with(V1) => Cert::Flat(parse_v1(path, text)),
        _ => fail(path, 1, "missing certificate header"),
    }
}

fn parse_u64(path: &str, lineno: usize, tok: Option<&str>, what: &str) -> u64 {
    match tok.and_then(|t| t.parse::<u64>().ok()) {
        Some(v) => v,
        None => fail(path, lineno, &format!("invalid {what}")),
    }
}

/// `Sb(n1:m1,n2:m2,...)` -> raw pairs.
fn parse_sb(path: &str, lineno: usize, tok: &str) -> Vec<(u16, u16)> {
    let inner = tok
        .strip_prefix("Sb(")
        .and_then(|t| t.strip_suffix(')'))
        .unwrap_or_else(|| fail(path, lineno, "expected Sb(...)"));
    let mut out = Vec::new();
    for part in inner.split(',') {
        let (n, m) = part
            .split_once(':')
            .unwrap_or_else(|| fail(path, lineno, "malformed part"));
        let n: u16 = n.parse().unwrap_or_else(|_| fail(path, lineno, "malformed n"));
        let m: u16 = m.parse().unwrap_or_else(|_| fail(path, lineno, "malformed m"));
        if n == 0 || m == 0 {
            fail(path, lineno, "zero-sided part");
        }
        out.push((n, m));
    }
    if out.is_empty() {
        fail(path, lineno, "empty Sb state");
    }
    out
}

fn parse_v1(path: &str, text: &str) -> FlatCert {
    let mut claims_by_k: Vec<Vec<State>> = Vec::new();
    let mut header_seen = false;
    for (lineno, line) in data_lines(text) {
        if line.starts_with(V1) {
            header_seen = true;
            continue;
        }
        if line.starts_with("meta ") {
            continue;
        }
        if !header_seen {
            fail(path, lineno, "record before header");
        }
        let mut it = line.split_whitespace();
        let tag = it.next().unwrap();
        if tag != "root" && tag != "fact" {
            fail(path, lineno, "unknown record");
        }
        let k = parse_u64(path, lineno, it.next(), "k") as usize;
        if k == 0 || k > 12 {
            fail(path, lineno, "k out of range");
        }
        let sb = it.next().unwrap_or_else(|| fail(path, lineno, "missing state"));
        if it.next().is_some() {
            fail(path, lineno, "trailing data");
        }
        let state = State::canon(parse_sb(path, lineno, sb));
        if claims_by_k.len() <= k {
            claims_by_k.resize_with(k + 1, Vec::new);
        }
        claims_by_k[k].push(state);
    }
    FlatCert { claims_by_k }
}

fn parse_v2(path: &str, text: &str) -> LevelCert {
    let mut lines = data_lines(text);
    let expect = |lines: &mut dyn Iterator<Item = (usize, &str)>, what: &str| -> (usize, String) {
        match lines.next() {
            Some((ln, l)) => (ln, l.to_string()),
            None => fail(path, 0, &format!("truncated before {what}")),
        }
    };

    let (ln, l) = expect(&mut lines, "header");
    if l != V2 {
        fail(path, ln, "missing level-v2 header");
    }
    let (ln, l) = expect(&mut lines, "level");
    let level = match l.strip_prefix("level ") {
        Some(v) => parse_u64(path, ln, Some(v.trim()), "level") as usize,
        None => fail(path, ln, "expected level record"),
    };
    if level < 1 || level > 12 {
        fail(path, ln, "level out of range");
    }

    let (ln, l) = expect(&mut lines, "parts");
    let parts_count = match l.strip_prefix("parts ") {
        Some(v) => parse_u64(path, ln, Some(v.trim()), "parts count") as usize,
        None => fail(path, ln, "expected parts section"),
    };
    let mut dict: Vec<Part> = Vec::with_capacity(parts_count + 1);
    dict.push(Part { n: 0, m: 0 }); // id 0 unused
    let mut prev_key = (0u32, 0u16, 0u16);
    for expected in 1..=parts_count {
        let (ln, l) = expect(&mut lines, "part definition");
        let mut it = l.split_whitespace();
        if it.next() != Some("part") {
            fail(path, ln, "expected part record");
        }
        let id = parse_u64(path, ln, it.next(), "part id") as usize;
        if id != expected {
            fail(path, ln, "part ids must be dense ascending");
        }
        let nm = it.next().unwrap_or_else(|| fail(path, ln, "missing n:m"));
        if it.next().is_some() {
            fail(path, ln, "trailing part data");
        }
        let (n, m) = nm
            .split_once(':')
            .unwrap_or_else(|| fail(path, ln, "malformed n:m"));
        let n: u16 = n.parse().unwrap_or_else(|_| fail(path, ln, "malformed n"));
        let m: u16 = m.parse().unwrap_or_else(|_| fail(path, ln, "malformed m"));
        if n < m || m < 1 {
            fail(path, ln, "part not canonical (need n >= m >= 1)");
        }
        let p = Part { n, m };
        // Dictionary order: ascending mass then long side (docs/certificate.md). Enforced
        // so descending ids in records mean canonical descending states.
        let key = (p.mass(), p.n, p.m);
        if key <= prev_key {
            fail(path, ln, "part definitions not strictly ascending");
        }
        prev_key = key;
        dict.push(p);
    }

    // Section reader shared by support and claims.
    let read_section = |lines: &mut dyn Iterator<Item = (usize, &str)>,
                        tag: &str,
                        want_level: usize,
                        used: &mut [bool]|
     -> Vec<State> {
        let (ln, l) = match lines.next() {
            Some(x) => x,
            None => fail(path, 0, &format!("missing {tag} section")),
        };
        let rest = l
            .strip_prefix(tag)
            .unwrap_or_else(|| fail(path, ln, &format!("expected {tag} section")));
        let mut it = rest.split_whitespace();
        let sec_level = parse_u64(path, ln, it.next(), "section level") as usize;
        let records = parse_u64(path, ln, it.next(), "record count") as usize;
        let refs = parse_u64(path, ln, it.next(), "part-reference count") as usize;
        if it.next().is_some() {
            fail(path, ln, "trailing section header data");
        }
        if sec_level != want_level {
            fail(path, ln, &format!("{tag} level must be {want_level}"));
        }
        let rec_tag = if tag == "support" { "fact" } else { "claim" };
        let mut out = Vec::with_capacity(records);
        let mut seen_refs = 0usize;
        for _ in 0..records {
            let (ln, l) = match lines.next() {
                Some(x) => x,
                None => fail(path, 0, &format!("truncated {tag} section")),
            };
            let rest = l
                .strip_prefix(rec_tag)
                .unwrap_or_else(|| fail(path, ln, &format!("expected {rec_tag} record")));
            let mut prev_id = usize::MAX;
            let mut raw: Vec<(u16, u16)> = Vec::new();
            for tok in rest.split_whitespace() {
                let id: usize = tok
                    .parse()
                    .unwrap_or_else(|_| fail(path, ln, "invalid part id"));
                if id == 0 || id >= dict.len() {
                    fail(path, ln, "part id out of range");
                }
                if id > prev_id {
                    fail(path, ln, "part ids not in canonical descending order");
                }
                prev_id = id;
                used[id] = true;
                raw.push((dict[id].n, dict[id].m));
                seen_refs += 1;
            }
            if raw.is_empty() {
                fail(path, ln, &format!("empty {rec_tag} state"));
            }
            out.push(State::canon(raw));
        }
        if seen_refs != refs {
            fail(
                path,
                0,
                &format!("{tag} part-reference count {seen_refs} != declared {refs}"),
            );
        }
        out
    };

    let mut used = vec![false; dict.len()];
    let support = read_section(&mut lines, "support", level - 1, &mut used);

    // split-hints: a preparation-order performance hint in the reference verifier; here it
    // is validated (exactly the nonunit parts used by claims, with exact use counts) so a
    // malformed file fails loudly, then ignored.
    let (ln, l) = expect(&mut lines, "split-hints");
    let hint_count = match l.strip_prefix("split-hints ") {
        Some(v) => parse_u64(path, ln, Some(v.trim()), "split-hints count") as usize,
        None => fail(path, ln, "expected split-hints section"),
    };
    let mut hints: Vec<(usize, u64)> = Vec::with_capacity(hint_count);
    for _ in 0..hint_count {
        let (ln, l) = expect(&mut lines, "split hint");
        let mut it = l.split_whitespace();
        if it.next() != Some("split") {
            fail(path, ln, "expected split hint");
        }
        let id = parse_u64(path, ln, it.next(), "hint part id") as usize;
        if it.next() != Some("uses") {
            fail(path, ln, "expected uses");
        }
        let uses = parse_u64(path, ln, it.next(), "hint uses");
        if it.next().is_some() {
            fail(path, ln, "trailing hint data");
        }
        if id == 0 || id >= dict.len() || dict[id].is_unit() || uses == 0 {
            fail(path, ln, "malformed split hint");
        }
        hints.push((id, uses));
    }

    let claims = read_section(&mut lines, "claims", level, &mut used);
    if claims.is_empty() {
        fail(path, 0, "claims section must be nonempty");
    }
    if let Some((ln, _)) = lines.next() {
        fail(path, ln, "trailing certificate data");
    }
    for (id, u) in used.iter().enumerate().skip(1) {
        if !u {
            fail(path, 0, &format!("unused part id {id}"));
        }
    }

    // Validate hints against actual claim part usage (nonunit parts only; State::canon
    // already stripped units, which hints never cover).
    let id_of: std::collections::HashMap<Part, usize> = dict
        .iter()
        .enumerate()
        .skip(1)
        .map(|(i, p)| (*p, i))
        .collect();
    let mut uses_of = vec![0u64; dict.len()];
    for c in &claims {
        for p in &c.parts {
            uses_of[id_of[p]] += 1;
        }
    }
    let mut expected: Vec<(usize, u64)> = uses_of
        .iter()
        .enumerate()
        .skip(1)
        .filter(|&(_, &u)| u > 0)
        .map(|(i, &u)| (i, u))
        .collect();
    expected.retain(|&(id, _)| !dict[id].is_unit());
    hints.sort_unstable();
    expected.sort_unstable();
    if hints != expected {
        let mut msg = String::from("split-hint section does not match level claims");
        let _ = write!(msg, " (hints {}, expected {})", hints.len(), expected.len());
        fail(path, 0, &msg);
    }

    LevelCert { level, support, claims }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn v1_roundtrip() {
        let text = "radio-negative-certificate-v1\n# c\nmeta source-sha256 xx\nroot 2 Sb(5:1)\nfact 1 Sb(3:1)\n";
        match parse("t", text) {
            Cert::Flat(f) => {
                assert_eq!(f.claims_by_k[2], vec![State::canon([(5, 1)])]);
                assert_eq!(f.claims_by_k[1], vec![State::canon([(3, 1)])]);
            }
            _ => panic!("expected flat"),
        }
    }

    #[test]
    fn v2_minimal() {
        let text = "radio-negative-level-certificate-v2\nlevel 2\nparts 2\npart 1 3:1\npart 2 5:1\nsupport 1 1 1\nfact 1\nsplit-hints 1\nsplit 2 uses 1\nclaims 2 1 1\nclaim 2\n";
        match parse("t", text) {
            Cert::Level(lc) => {
                assert_eq!(lc.level, 2);
                assert_eq!(lc.support, vec![State::canon([(3, 1)])]);
                assert_eq!(lc.claims, vec![State::canon([(5, 1)])]);
            }
            _ => panic!("expected level"),
        }
    }
}
