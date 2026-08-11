#!/bin/bash
# Convert verdicts to cache facts while retaining all comment metadata (provenance, certificate and
# segment-chain headers). POSIX awk's leftmost-longest match duplicates grep's former greedy
# `^can.* in [0-9]+`.
awk '
    /^#/ {
        print
        next
    }
    match($0, /^can.* in [0-9]+/) {
        line = substr($0, RSTART, RLENGTH)
        sub(/^can\047t solve /, "- ", line)
        sub(/^can solve /, "+ ", line)
        sub(/ size=[0-9\/]* /, " ", line)
        sub(/Sa\(/, "a ", line)
        sub(/\) in /, " ", line)
        sub(/Sb\(/, "b ", line)
        sub(/\)\[/, " t ", line)
        sub(/] in /, " ", line)
        sub(/]/, "", line)
        gsub(/[:,]/, " ", line)
        print line
    }
'
