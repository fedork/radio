#!/usr/bin/env python3
"""A minimal client for tools/oracle_server.py's TCP protocol (distinct from oracle_client.py,
which drives a `radio_oracle` subprocess directly). Connect over an SSM port-forward to the
persistent oracle-serve instance -- see docs/aws-run.md for the current instance and tunnel
command. One request per connection, matching the server's own per-request accept loop.
"""
from __future__ import annotations

import socket


class TCPOracle:
    def __init__(self, host="127.0.0.1", port=7777, timeout=180):
        self.host = host
        self.port = port
        self.timeout = timeout

    def _request(self, line: str) -> str:
        s = socket.create_connection((self.host, self.port), timeout=self.timeout)
        try:
            s.sendall((line + "\n").encode())
            buf = b""
            while True:
                chunk = s.recv(65536)
                if not chunk:
                    break
                buf += chunk
            return buf.decode()
        finally:
            s.close()

    def ask(self, k, parts):
        """parts: sequence of (n, m). Returns SOLVABLE / UNSOLVABLE / MAYBE."""
        flat = " ".join(f"{n} {m}" for n, m in parts)
        reply = self._request(f"{k} {flat}").strip()
        if reply.startswith("ERR"):
            raise RuntimeError(f"{reply}  (request: k={k} {parts})")
        if not reply.startswith("VERDICT "):
            raise RuntimeError(f"unexpected reply: {reply!r}")
        return reply.split()[1]

    def enumerate(self, k, parts):
        """Returns (winners: list[tuple[(a,b),...]], summary: dict) where summary has
        winners/checked/admissible/inconclusive as ints."""
        flat = " ".join(f"{n} {m}" for n, m in parts)
        reply = self._request(f"enumerate {k} {flat}")
        winners = []
        summary = {}
        for line in reply.splitlines():
            line = line.strip()
            if not line:
                continue
            if line.startswith("WINNER"):
                pairs = tuple(tuple(int(x) for x in tok.split(":")) for tok in line.split()[1:])
                winners.append(pairs)
            elif line.startswith("ENUM_END"):
                for tok in line.split()[1:]:
                    key, _, val = tok.partition("=")
                    summary[key] = int(val)
            elif line.startswith("ERR"):
                raise RuntimeError(f"{line}  (request: enumerate k={k} {parts})")
        return winners, summary

    def stats(self):
        return self._request("stats").strip()


if __name__ == "__main__":
    import sys
    o = TCPOracle()
    print(o.stats())
    print(o.ask(4, [(5, 3), (4, 2)]))
    winners, summary = o.enumerate(6, [(16, 12), (17, 10), (29, 5), (21, 6)])
    print(winners, summary)
