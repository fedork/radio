#!/usr/bin/env python3
"""A TCP front-end for one long-lived `radio_oracle` subprocess.

Why: the oracle speaks a line protocol over its own stdin/stdout, which is awkward to reach
across separate SSH sessions over time. This wraps it in a small TCP server bound to localhost
(reach it via an SSH local port-forward, never expose it directly) so any future session can
connect, send one request, and get the full response back, while the single oracle subprocess
underneath keeps its warm cache across every connection.

Protocol: connect, send one line (any request the oracle itself accepts: a plain query, an
`enumerate ...` call, `stats`, `budget N`, etc.), read the response until the connection is
closed by the server. For `enumerate`, that means every `WINNER` line followed by `ENUM_END`;
for everything else, one line. The server closes the connection after each request -- open a new
one for the next. Requests are serialized (one at a time) since the oracle is a single process.

Also periodically dumps a cache snapshot to disk (not S3 -- a separate uploader can push it),
so a crash loses at most one interval's worth of accumulated facts, not the whole run.

Usage:
    tools/oracle_server.py <oracle_binary> --port 7777 [--snapshot-every 1800]
                            [--snapshot-dir /path] [caches...]
"""
from __future__ import annotations

import argparse
import socketserver
import sys
import threading
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from oracle_client import Oracle, OracleError


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("binary")
    ap.add_argument("caches", nargs="*")
    ap.add_argument("--port", type=int, default=7777)
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--budget", type=int, default=30)
    ap.add_argument("--snapshot-every", type=int, default=1800,
                     help="seconds between snapshot dumps, 0 to disable")
    ap.add_argument("--snapshot-dir", default=".")
    ap.add_argument("--restore", default=None,
                     help="path to a cache.snap file to restore-any from at startup, if it exists")
    args = ap.parse_args()

    caches = list(args.caches)
    if args.restore and Path(args.restore).exists():
        caches = [f"--restore-any={args.restore}"] + caches
        print(f"[oracle_server] restoring from {args.restore}", file=sys.stderr, flush=True)

    oracle = Oracle(args.binary, caches, budget_seconds=args.budget, echo_ready=True)
    lock = threading.Lock()
    print(f"[oracle_server] {oracle.ready}", file=sys.stderr, flush=True)

    def snapshot_loop():
        n = 0
        while True:
            time.sleep(args.snapshot_every)
            n += 1
            path = str(Path(args.snapshot_dir) / f"oracle.snap")
            tmp = path + ".tmp"
            with lock:
                try:
                    reply = oracle._round_trip(f"snapshot {tmp}")
                except OracleError as e:
                    print(f"[oracle_server] snapshot failed: {e}", file=sys.stderr, flush=True)
                    continue
            if reply.startswith("OK"):
                Path(tmp).replace(path)
                print(f"[oracle_server] snapshot #{n} -> {path}: {reply}", file=sys.stderr, flush=True)
            else:
                print(f"[oracle_server] snapshot #{n} reply: {reply}", file=sys.stderr, flush=True)

    if args.snapshot_every > 0:
        threading.Thread(target=snapshot_loop, daemon=True).start()

    class Handler(socketserver.StreamRequestHandler):
        def handle(self):
            try:
                line = self.rfile.readline().decode().rstrip("\n")
            except Exception:
                return
            if not line:
                return
            with lock:
                try:
                    if oracle.proc.poll() is not None:
                        self.wfile.write(b"ERR oracle process exited\n")
                        return
                    oracle.proc.stdin.write(line + "\n")
                    oracle.proc.stdin.flush()
                    is_enum = line.startswith("enumerate ")
                    while True:
                        reply = oracle.proc.stdout.readline()
                        if not reply:
                            self.wfile.write(b"ERR oracle closed its output\n")
                            return
                        self.wfile.write(reply.encode())
                        if not is_enum or reply.startswith("ENUM_END") or reply.startswith("ERR"):
                            break
                except (BrokenPipeError, OSError) as e:
                    try:
                        self.wfile.write(f"ERR {e}\n".encode())
                    except Exception:
                        pass

    class Server(socketserver.ThreadingTCPServer):
        allow_reuse_address = True
        daemon_threads = True

    with Server((args.host, args.port), Handler) as server:
        print(f"[oracle_server] listening on {args.host}:{args.port}", file=sys.stderr, flush=True)
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass
    oracle.close()


if __name__ == "__main__":
    main()
