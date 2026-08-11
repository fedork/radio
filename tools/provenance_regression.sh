#!/usr/bin/env bash
# End-to-end check of embedded build metadata, exact run arguments and the incomplete direct-build
# fallback.  Uses tiny table bounds so radiobase initialization takes a fraction of a second.
set -euo pipefail

cd "$(dirname "$0")/.."
tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-provenance.XXXXXX")
trap 'rm -rf "$tmp"' EXIT

# Any explicit runtime knob in a C/C++ program must be in both safe allow-lists. Otherwise an output
# can look reproducible while omitting a flag that actually changed program behaviour.
while IFS= read -r env_name; do
    [ "$env_name" = RADIO_PROVENANCE_WRAPPER_EMITTED ] && continue
    grep -F "\"$env_name\"" radiobase.c >/dev/null || {
        echo "runtime environment knob missing from radiobase provenance: $env_name" >&2; exit 1;
    }
    grep -F "\"$env_name\"" tools/run_with_provenance.py >/dev/null || {
        echo "runtime environment knob missing from standalone provenance: $env_name" >&2; exit 1;
    }
done < <(rg -o --no-filename 'getenv\("[A-Z0-9_]+"\)' --glob='*.c' --glob='*.cpp' . \
          | sed 's/getenv("//; s/")//' | sort -u)

if RADIO_SOURCE_COMMIT=deadbee python3 tools/build_radio.py -O0 -DMAX_K=2 -DMAX_N=4 \
        tools/provenance_probe.c -o "$tmp/bad-short" >/dev/null 2>&1; then
    echo "abbreviated source commit unexpectedly produced complete provenance" >&2
    exit 1
fi
if RADIO_SOURCE_COMMIT=0000000000000000000000000000000000000000 \
        python3 tools/build_radio.py -O0 -DMAX_K=2 -DMAX_N=4 \
        tools/provenance_probe.c -o "$tmp/bad-conflict" >/dev/null 2>&1; then
    echo "source commit conflicting with checkout HEAD was accepted" >&2
    exit 1
fi

SOURCE_DATE_EPOCH=1722470400 python3 tools/build_radio.py -O0 -DMAX_K=2 -DMAX_N=4 \
    '-DPROVENANCE_BUILD_LABEL="argument with spaces"' \
    tools/provenance_probe.c -o "$tmp/probe"
RADIO_PROBE_INIT=1 RADIO_RUNNER=provenance-regression RADIO_LIMIT_WALL_SECONDS=17 \
    "$tmp/probe" "argument with spaces" 'backslash\argument' > "$tmp/out"

python3 tools/check_provenance.py "$tmp/probe.provenance" "$tmp/out"
grep -F '# run_arg[1]=argument with spaces' "$tmp/out" >/dev/null
grep -F '# run_arg[2]=backslash\\argument' "$tmp/out" >/dev/null
grep -F '# runtime_env.RADIO_LIMIT_WALL_SECONDS=17' "$tmp/out" >/dev/null
grep -F '# runtime_env.RADIO_PROBE_INIT=1' "$tmp/out" >/dev/null
grep -F -- '-DPROVENANCE_BUILD_LABEL="argument with spaces"' "$tmp/out" >/dev/null
grep -F '.name=SOURCE_DATE_EPOCH' "$tmp/out" >/dev/null
grep -F '.value=1722470400' "$tmp/out" >/dev/null
for field in git_identity_source compiler_executable_sha256 build_tool_sha256 \
             provenance_injection build_env_count runtime_cpu_model \
             runtime_rlimit.CPU_seconds.soft runtime_rlimit.address_space_bytes.soft; do
    grep -F "# $field=" "$tmp/out" >/dev/null || {
        echo "canonical output omitted $field" >&2; exit 1;
    }
done
./parse_out.sh < "$tmp/out" > "$tmp/parsed"
python3 tools/check_provenance.py "$tmp/parsed"

# The pre-main constructor covers focused tools and usage failures which never call init().
"$tmp/probe" > "$tmp/no-init.out"
python3 tools/check_provenance.py "$tmp/no-init.out"
[ "$(grep -c '^# radio-provenance-v1 begin$' "$tmp/no-init.out")" -eq 1 ]

"${CC:-clang}" -O0 -DMAX_K=2 -DMAX_N=4 tools/provenance_probe.c -o "$tmp/direct"
"$tmp/direct" > "$tmp/direct.out"
if python3 tools/check_provenance.py "$tmp/direct.out" >/dev/null 2>&1; then
    echo "direct compiler build unexpectedly passed the strict provenance gate" >&2
    exit 1
fi
python3 tools/check_provenance.py --allow-incomplete "$tmp/direct.out"

# Standalone programs have no radiobase init hook, so the generic launcher supplies the same raw
# output contract from the verified build sidecar.
python3 tools/run_with_provenance.py "$tmp/probe" standalone > "$tmp/wrapped.out"
python3 tools/check_provenance.py "$tmp/wrapped.out"
[ "$(grep -c '^# radio-provenance-v1 begin$' "$tmp/wrapped.out")" -eq 1 ]

echo "provenance regression passed"
