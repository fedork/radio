#!/usr/bin/env python3
"""Talk to a warm `radio_oracle` process.

The oracle pays init() and cache replay once and then answers queries for as long as it is kept
alive, which is the only affordable way to get thousands of verdicts -- init() alone is minutes at a
useful MAX_N. Dependency-free stdlib so it can be used from anywhere in the repo.

    from tools.oracle_client import Oracle
    with Oracle("./radio_oracle", caches=["exact.cache"]) as o:
        print(o.ask(5, [(12,5),(11,5),(10,5),(9,5)]))     # 'SOLVABLE'

Verdicts are 'SOLVABLE', 'UNSOLVABLE' or 'MAYBE'. **MAYBE is not a refutation** -- it means the
per-query budget ran out. Raise the budget or accept the state as undecided; never record it as a
negative.
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path


class OracleError(RuntimeError):
    pass


class Oracle:
    def __init__(self, binary, caches=(), budget_seconds=60, stderr_path=None, echo_ready=False):
        self.binary = str(binary)
        cmd = [self.binary, *[str(c) for c in caches]]
        self._err = open(stderr_path, "w") if stderr_path else subprocess.DEVNULL
        self.proc = subprocess.Popen(
            cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=self._err,
            text=True, bufsize=1,
        )
        self.ready = None
        for line in self.proc.stdout:                    # skip banner and cache-load chatter
            line = line.rstrip("\n")
            if echo_ready and line:
                print(line, file=sys.stderr)
            if line.startswith("ORACLE READY"):
                self.ready = line
                break
        if self.ready is None:
            raise OracleError(f"{self.binary} never reported ready (check its stderr)")
        if budget_seconds is not None:
            self.budget(budget_seconds)

    # ---- protocol ---------------------------------------------------------------------------
    def _round_trip(self, request):
        if self.proc.poll() is not None:
            raise OracleError("oracle has exited")
        self.proc.stdin.write(request + "\n")
        self.proc.stdin.flush()
        reply = self.proc.stdout.readline()
        if not reply:
            raise OracleError("oracle closed its output")
        return reply.rstrip("\n")

    def budget(self, seconds):
        return self._round_trip(f"budget {int(seconds)}")

    def load(self, path):
        return self._round_trip(f"load {path}")

    def stats(self):
        return self._round_trip("stats")

    def ask(self, k, parts):
        """parts is a sequence of (n, m). Returns SOLVABLE / UNSOLVABLE / MAYBE."""
        flat = " ".join(f"{n} {m}" for n, m in parts)
        reply = self._round_trip(f"{k} {flat}")
        if reply.startswith("ERR"):
            raise OracleError(f"{reply}  (request: k={k} {parts})")
        if not reply.startswith("VERDICT "):
            raise OracleError(f"unexpected reply: {reply}")
        return reply.split()[1]

    def ask_many(self, items):
        """items is a sequence of (k, parts). Yields verdicts in order."""
        for k, parts in items:
            yield self.ask(k, parts)

    # ---- lifecycle --------------------------------------------------------------------------
    def close(self):
        if self.proc.poll() is None:
            try:
                self.proc.stdin.write("quit\n")
                self.proc.stdin.flush()
                self.proc.wait(timeout=30)
            except Exception:
                self.proc.kill()
        if self._err not in (subprocess.DEVNULL, None):
            self._err.close()

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()


if __name__ == "__main__":
    import argparse
    ap = argparse.ArgumentParser(description="smoke-test a radio_oracle binary")
    ap.add_argument("binary")
    ap.add_argument("caches", nargs="*")
    ap.add_argument("--budget", type=int, default=60)
    args = ap.parse_args()
    with Oracle(args.binary, args.caches, budget_seconds=args.budget, echo_ready=True) as o:
        print(o.ready)
        for k, parts in ((5, [(12, 5), (11, 5), (10, 5), (9, 5)]),
                         (5, [(13, 5), (12, 5), (11, 5), (10, 4)]),
                         (4, [(12, 4), (10, 3), (8, 4), (7, 3)])):
            print(f"k={k} {parts} -> {o.ask(k, parts)}")
        print(o.stats())
