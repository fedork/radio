//! CLI: audit negative certificates (level-v2 or flat v1), or run the built-in selftest.
//!
//!   radio_cleanroom audit [--threads N] [--stride S] [--offset O] [--progress SECONDS] <cert>...
//!   radio_cleanroom selftest
//!
//! Exit status: 0 = every selected claim verified; 1 = gaps or contradictions; 2 = usage,
//! parse, or structural error. Output is line-oriented and mirrors the reference verifier
//! where that helps side-by-side comparison (TOTAL verified N, gaps M).

use radio_cleanroom::audit::{Auditor, Scratch, Verdict};
use radio_cleanroom::naive::Naive;
use radio_cleanroom::parse::{parse, Cert};
use radio_cleanroom::state::{Part, State};
use std::sync::atomic::{AtomicBool, AtomicU64, AtomicUsize, Ordering};
use std::sync::Mutex;
use std::time::Instant;

struct Args {
    threads: usize,
    stride: usize,
    offset: usize,
    /// Seconds between PROGRESS lines during a level; 0 disables reporting.
    progress: u64,
    paths: Vec<String>,
}

fn usage() -> ! {
    eprintln!(
        "usage: radio_cleanroom audit [--threads N] [--stride S] [--offset O] [--progress SECONDS] <cert>...\n       radio_cleanroom selftest"
    );
    std::process::exit(2);
}

fn main() {
    let mut argv = std::env::args().skip(1);
    match argv.next().as_deref() {
        Some("audit") => {
            let mut args =
                Args { threads: 1, stride: 1, offset: 0, progress: 0, paths: Vec::new() };
            let rest: Vec<String> = argv.collect();
            let mut i = 0;
            while i < rest.len() {
                match rest[i].as_str() {
                    "--threads" | "--stride" | "--offset" | "--progress" => {
                        let v: usize = rest
                            .get(i + 1)
                            .and_then(|s| s.parse().ok())
                            .unwrap_or_else(|| usage());
                        match rest[i].as_str() {
                            "--threads" => args.threads = v.clamp(1, 256),
                            "--stride" => args.stride = v.max(1),
                            "--progress" => args.progress = v as u64,
                            _ => args.offset = v,
                        }
                        i += 2;
                    }
                    p => {
                        args.paths.push(p.to_string());
                        i += 1;
                    }
                }
            }
            if args.paths.is_empty() || args.offset >= args.stride {
                usage();
            }
            let mut total_verified = 0u64;
            let mut total_gaps = 0u64;
            let mut total_cells = 0u64;
            for path in &args.paths {
                let (v, gp, c) = audit_file(path, &args);
                total_verified += v;
                total_gaps += gp;
                total_cells += c;
            }
            println!("TOTAL verified {total_verified}, gaps {total_gaps}, cells {total_cells}");
            std::process::exit(if total_gaps == 0 { 0 } else { 1 });
        }
        Some("selftest") => selftest(),
        _ => usage(),
    }
}

fn audit_file(path: &str, args: &Args) -> (u64, u64, u64) {
    let text = std::fs::read_to_string(path).unwrap_or_else(|e| {
        eprintln!("cannot read {path}: {e}");
        std::process::exit(2);
    });
    match parse(path, &text) {
        Cert::Level(lc) => {
            println!(
                "INPUT certificate={path} format=level-v2 level={} support={} claims={}",
                lc.level,
                lc.support.len(),
                lc.claims.len()
            );
            audit_level(lc.level, &lc.support, &lc.claims, args)
        }
        Cert::Flat(fc) => {
            println!("INPUT certificate={path} format=full-v1");
            let mut totals = (0u64, 0u64, 0u64);
            let empty: Vec<State> = Vec::new();
            for k in 1..fc.claims_by_k.len() {
                if fc.claims_by_k[k].is_empty() {
                    continue;
                }
                let support = if k >= 2 { &fc.claims_by_k[k - 1] } else { &empty };
                let (v, g, c) = audit_level(k, support, &fc.claims_by_k[k], args);
                totals.0 += v;
                totals.1 += g;
                totals.2 += c;
            }
            totals
        }
    }
}

fn audit_level(k: usize, support: &[State], claims: &[State], args: &Args) -> (u64, u64, u64) {
    let build_start = Instant::now();
    let mut claim_parts: Vec<Part> = claims.iter().flat_map(|c| c.parts.iter().copied()).collect();
    claim_parts.sort_unstable();
    claim_parts.dedup();
    let aud = Auditor::build(k, &claim_parts, support);
    println!(
        "BUILD k={k} facts={} redundant={} closure_tuples={} closure_nodes={} dead_options={}/{} wall_s={:.3}",
        support.len(),
        aud.redundant_facts,
        aud.dom.inserted_tuples,
        aud.dom.node_count,
        aud.dead_options,
        aud.total_options,
        build_start.elapsed().as_secs_f64()
    );

    let selected: Vec<usize> = (0..claims.len())
        .filter(|i| i % args.stride == args.offset)
        .collect();
    let next = AtomicUsize::new(0);
    let verified = AtomicUsize::new(0);
    let gaps = AtomicUsize::new(0);
    let contradicted = AtomicUsize::new(0);
    let cells = AtomicUsize::new(0);
    let done = AtomicUsize::new(0);
    let report = Mutex::new(0usize);
    let np_stats = Mutex::new(([0u64; 9], [0u64; 9]));
    let rules = Mutex::new((0u64, 0u64, 0u64, 0u64));
    // Per-worker liveness for the reporter: which claim each worker holds and since when.
    // A single hard claim can occupy a worker for minutes, and without this a long level is
    // indistinguishable from a hang.
    let live_claim: Vec<AtomicUsize> =
        (0..args.threads).map(|_| AtomicUsize::new(usize::MAX)).collect();
    let live_since: Vec<AtomicU64> = (0..args.threads).map(|_| AtomicU64::new(0)).collect();
    let live_cells: Vec<AtomicU64> = (0..args.threads).map(|_| AtomicU64::new(0)).collect();
    let stop = AtomicBool::new(false);
    let start = Instant::now();

    // Every shared binding is turned into a reference HERE, outside thread::scope: the
    // spawned closures must be `move` (they capture the Copy worker index), and a borrow
    // taken inside the scope body does not live for 'scope.
    let (a_next, a_sel, a_aud, a_claims) = (&next, &selected, &aud, claims);
    let (a_verified, a_gaps, a_contra, a_cells, a_done) =
        (&verified, &gaps, &contradicted, &cells, &done);
    let (a_report, a_np, a_rules) = (&report, &np_stats, &rules);
    let (a_live, a_since, a_lcells, a_stop, a_start) =
        (&live_claim, &live_since, &live_cells, &stop, &start);

    std::thread::scope(|scope| {
        // The reporter runs until `stop`, so the workers are joined explicitly before it is
        // set; letting the scope join everything at once would deadlock on the reporter.
        let reporter = if args.progress > 0 {
            Some(scope.spawn(move || {
                let mut last_done = 0usize;
                let mut last_at = 0f64;
                loop {
                    // Sleep in short ticks so the level ends promptly rather than after a
                    // full reporting interval.
                    let mut waited = 0f64;
                    while waited < args.progress as f64 {
                        if a_stop.load(Ordering::Relaxed) {
                            return;
                        }
                        std::thread::sleep(std::time::Duration::from_millis(200));
                        waited += 0.2;
                    }
                    let now = a_start.elapsed().as_secs_f64();
                    let d = a_done.load(Ordering::Relaxed);
                    let c: u64 = a_lcells.iter().map(|x| x.load(Ordering::Relaxed)).sum();
                    let window = now - last_at;
                    let recent = if window > 0.0 {
                        (d - last_done) as f64 / window
                    } else {
                        0.0
                    };
                    let eta = if recent > 0.0 {
                        a_sel.len().saturating_sub(d) as f64 / recent
                    } else {
                        -1.0
                    };
                    println!(
                        "PROGRESS k={k} elapsed_s={now:.1} completed={d}/{} percent={:.4} \
verified={} gaps={} cells={c} rate_total={:.2}/s rate_window={recent:.2}/s eta_s={eta:.0}",
                        a_sel.len(),
                        100.0 * d as f64 / a_sel.len().max(1) as f64,
                        a_verified.load(Ordering::Relaxed),
                        a_gaps.load(Ordering::Relaxed),
                        d as f64 / now.max(1e-9),
                    );
                    // Name the oldest in-flight claim: what to blame if throughput collapses.
                    let mut oldest: Option<(f64, usize)> = None;
                    for w in 0..args.threads {
                        let ci = a_live[w].load(Ordering::Acquire);
                        if ci == usize::MAX {
                            continue;
                        }
                        let age = now - a_since[w].load(Ordering::Relaxed) as f64 / 1000.0;
                        if oldest.map_or(true, |(a, _)| age > a) {
                            oldest = Some((age, ci));
                        }
                    }
                    if let Some((age, ci)) = oldest {
                        if age >= args.progress as f64 {
                            println!("PROGRESS_ACTIVE k={k} age_s={age:.1} {}", show(&a_claims[ci]));
                        }
                    }
                    last_done = d;
                    last_at = now;
                }
            }))
        } else {
            None
        };

        let mut workers = Vec::with_capacity(args.threads);
        for slot in 0..args.threads {
            workers.push(scope.spawn(move || {
                let mut s = Scratch::new();
                loop {
                    let q = a_next.fetch_add(1, Ordering::Relaxed);
                    if q >= a_sel.len() {
                        break;
                    }
                    let idx = a_sel[q];
                    let claim = &a_claims[idx];
                    a_since[slot].store(
                        (a_start.elapsed().as_secs_f64() * 1000.0) as u64,
                        Ordering::Relaxed,
                    );
                    a_live[slot].store(idx, Ordering::Release);
                    match a_aud.audit_claim(&claim.parts, claim.mass_full, &mut s) {
                        Verdict::Verified => {
                            a_verified.fetch_add(1, Ordering::Relaxed);
                        }
                        Verdict::Contradicted => {
                            a_contra.fetch_add(1, Ordering::Relaxed);
                            a_gaps.fetch_add(1, Ordering::Relaxed);
                            let mut printed = a_report.lock().unwrap();
                            if *printed < 100 {
                                println!("GAP verdict=contradicted k={k} {}", show(claim));
                                *printed += 1;
                            }
                        }
                        Verdict::Gap { take } => {
                            a_gaps.fetch_add(1, Ordering::Relaxed);
                            let mut printed = a_report.lock().unwrap();
                            if *printed < 100 {
                                println!(
                                    "GAP verdict=uncovered k={k} {} take={:?}",
                                    show(claim),
                                    take
                                );
                                *printed += 1;
                            }
                        }
                    }
                    a_live[slot].store(usize::MAX, Ordering::Release);
                    a_done.fetch_add(1, Ordering::Relaxed);
                    a_lcells[slot].store(s.cells, Ordering::Relaxed);
                }
                a_cells.fetch_add(s.cells as usize, Ordering::Relaxed);
                let mut agg = a_np.lock().unwrap();
                for np in 0..9 {
                    agg.0[np] += s.cells_by_np[np];
                    agg.1[np] += s.descends_by_np[np];
                }
                let mut r = a_rules.lock().unwrap();
                r.0 += s.fired_info;
                r.1 += s.fired_star;
                r.2 += s.fired_dom;
                r.3 += s.fired_none;
            }));
        }
        for w in workers {
            let _ = w.join();
        }
        a_stop.store(true, Ordering::Relaxed);
        if let Some(r) = reporter {
            let _ = r.join();
        }
    });

    let wall = start.elapsed().as_secs_f64();
    {
        let agg = np_stats.lock().unwrap();
        for np in 1..9 {
            if agg.0[np] > 0 || agg.1[np] > 0 {
                println!(
                    "STATS_NP k={k} np={np} cells={} descends={}",
                    agg.0[np], agg.1[np]
                );
            }
        }
    }
    {
        let r = rules.lock().unwrap();
        if r.0 + r.1 + r.2 + r.3 > 0 {
            println!(
                "STATS_RULES k={k} info={} star={} dom={} none={}",
                r.0, r.1, r.2, r.3
            );
        }
    }
    let v = verified.load(Ordering::Relaxed) as u64;
    let g = gaps.load(Ordering::Relaxed) as u64;
    let c = cells.load(Ordering::Relaxed) as u64;
    println!(
        "RESULT_LEVEL k={k} completed={} verified={v} gaps={g} contradicted={} cells={c} wall_s={wall:.3} rate={:.0}/s cell_rate={:.0}/s",
        selected.len(),
        contradicted.load(Ordering::Relaxed),
        selected.len() as f64 / wall.max(1e-9),
        c as f64 / wall.max(1e-9),
    );
    (v, g, c)
}

fn show(s: &State) -> String {
    let mut out = String::from("Sb(");
    for (i, p) in s.parts.iter().enumerate() {
        if i > 0 {
            out.push(',');
        }
        out.push_str(&format!("{}:{}", p.n, p.m));
    }
    out.push(')');
    if s.mass_full != s.mass_stripped() {
        out.push_str(&format!("+units{}", s.mass_full - s.mass_stripped()));
    }
    out
}

/// Exhaustive small-space soundness battery: for every state in a small slice, the naive
/// unquotiented solver decides solvability from docs/problem.md alone; the audit engine
/// must then verify exactly the unsolvable claims (given the complete lower level as
/// support) and must refuse to verify every solvable claim even with full support. The
/// second half is the regression for the 2026-08-31 quotient-composition trap.
fn selftest() {
    let n_cap: u16 = 6;
    let max_k = 4usize;
    let mut nv = Naive::new();

    // All canonical part shapes with n <= n_cap.
    let mut shapes: Vec<Part> = Vec::new();
    for n in 1..=n_cap {
        for m in 1..=n {
            shapes.push(Part { n, m });
        }
    }

    // All canonical states (as multisets, parts non-increasing by index in `shapes`) with
    // up to `max_parts` parts and mass_full <= cap.
    fn gen(
        shapes: &[Part],
        from: usize,
        left: usize,
        mass_left: i64,
        cur: &mut Vec<(u16, u16)>,
        out: &mut Vec<State>,
    ) {
        if !cur.is_empty() {
            out.push(State::canon(cur.iter().copied()));
        }
        if left == 0 {
            return;
        }
        for i in from..shapes.len() {
            let p = shapes[i];
            if (p.mass() as i64) > mass_left {
                continue;
            }
            cur.push((p.n, p.m));
            gen(shapes, i, left - 1, mass_left - p.mass() as i64, cur, out);
            cur.pop();
        }
    }

    let mut failures = 0u64;
    for k in 1..=max_k {
        let cap_k = 3i64.pow(k as u32);
        // Claims: <= 3 parts within the level's information bound.
        let mut claims = Vec::new();
        gen(&shapes, 0, 3, cap_k, &mut Vec::new(), &mut claims);
        claims.sort_by(|a, b| format!("{a:?}").cmp(&format!("{b:?}")));
        claims.dedup();
        // Support: the COMPLETE set of unsolvable states at k-1 that any child of any
        // claim could be, i.e. up to 6 parts (outcome 1 doubles parts) within the child
        // information bound.
        let mut child_space = Vec::new();
        gen(&shapes, 0, 6, cap_k / 3, &mut Vec::new(), &mut child_space);
        child_space.sort_by(|a, b| format!("{a:?}").cmp(&format!("{b:?}")));
        child_space.dedup();
        let support: Vec<State> = child_space
            .iter()
            .filter(|s| !nv.solvable(s, k - 1))
            .cloned()
            .collect();

        let mut claim_parts: Vec<Part> =
            claims.iter().flat_map(|c| c.parts.iter().copied()).collect();
        claim_parts.sort_unstable();
        claim_parts.dedup();
        let aud = Auditor::build(k, &claim_parts, &support);
        let mut s = Scratch::new();
        let mut n_claims = 0u64;
        for claim in &claims {
            n_claims += 1;
            let truth_solvable = nv.solvable(claim, k);
            let verdict = aud.audit_claim(&claim.parts, claim.mass_full, &mut s);
            match (truth_solvable, &verdict) {
                (false, Verdict::Verified) => {}
                (true, Verdict::Gap { .. }) | (true, Verdict::Contradicted) => {}
                _ => {
                    failures += 1;
                    if failures <= 20 {
                        println!(
                            "SELFTEST_FAIL k={k} {} solvable={truth_solvable} verdict={verdict:?}",
                            show(claim)
                        );
                    }
                }
            }
        }
        println!(
            "SELFTEST k={k} claims={n_claims} support={} cells={} ok",
            support.len(),
            s.cells
        );
    }
    if failures > 0 {
        println!("SELFTEST FAILED: {failures} disagreements with the naive oracle");
        std::process::exit(1);
    }
    println!("SELFTEST PASSED: audit agrees with the unquotiented oracle on every state");
}
