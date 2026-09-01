//! Cleanroom verifier for radio negative certificates.
//!
//! Trust base: this crate and the Rust standard library. It shares no code with the
//! solver, reads no caches, and recomputes every mathematical object from its defining
//! statement. Inputs it was written from: docs/problem.md, docs/theorems/*.md, and the
//! certificate grammars in docs/certificate.md. See each module header for the lemma
//! inventory; the audit rules are exactly INFO, STAR, DOM (+ UNIT in canonicalization),
//! applied to full or partial children under Subgraph Monotonicity.

pub mod audit;
pub mod dom;
pub mod gk;
pub mod naive;
pub mod parse;
pub mod state;
