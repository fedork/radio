#!/usr/bin/env bash
# Benchmark the complete raw-log -> normalized -> colored -> replayed certificate pipeline.
set -euo pipefail

usage() {
    cat <<'EOF'
usage: tools/benchmark_verifier_pipeline.sh \
    --label LABEL --input RAW_LOG [--line-limit N] --top-k K \
    --threads N [--cpus CPU_LIST] --output-dir DIR \
    --root 'Sb(...)' [--root 'Sb(...)' ...]

The output directory must not already contain a completed run.  CPU_LIST uses
taskset syntax (for example 0-7); omit it on systems without taskset.
EOF
}

label=
input=
line_limit=0
top_k=
threads=
cpus=
output_dir=
roots=()

while (( $# )); do
    case "$1" in
        --label) label=$2; shift 2 ;;
        --input) input=$2; shift 2 ;;
        --line-limit) line_limit=$2; shift 2 ;;
        --top-k) top_k=$2; shift 2 ;;
        --threads) threads=$2; shift 2 ;;
        --cpus) cpus=$2; shift 2 ;;
        --output-dir) output_dir=$2; shift 2 ;;
        --root) roots+=("$2"); shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) echo "benchmark_verifier_pipeline.sh: unknown option $1" >&2; usage >&2; exit 2 ;;
    esac
done

if [[ -z "$label" || -z "$input" || -z "$top_k" || -z "$threads" ||
      -z "$output_dir" || ${#roots[@]} -eq 0 ]]; then
    usage >&2
    exit 2
fi
if [[ ! "$line_limit" =~ ^[0-9]+$ || ! "$top_k" =~ ^[0-9]+$ ||
      ! "$threads" =~ ^[1-9][0-9]*$ ]]; then
    echo "benchmark_verifier_pipeline.sh: numeric arguments are invalid" >&2
    exit 2
fi
for root in "${roots[@]}"; do
    if [[ ! "$root" =~ ^Sb\([0-9]+:[0-9]+(,[[:space:]]*[0-9]+:[0-9]+)*\)$ ]]; then
        echo "benchmark_verifier_pipeline.sh: invalid root: $root" >&2
        exit 2
    fi
done

repo_dir=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
case "$input" in /*) ;; *) input=$PWD/$input ;; esac
case "$output_dir" in /*) ;; *) output_dir=$PWD/$output_dir ;; esac
if [[ ! -f "$input" ]]; then
    echo "benchmark_verifier_pipeline.sh: input not found: $input" >&2
    exit 2
fi
if [[ -e "$output_dir/COMPLETE" ]]; then
    echo "benchmark_verifier_pipeline.sh: completed output already exists: $output_dir" >&2
    exit 2
fi
mkdir -p "$output_dir"

if [[ -n "$cpus" ]] && ! command -v taskset >/dev/null; then
    echo "benchmark_verifier_pipeline.sh: --cpus requires taskset" >&2
    exit 2
fi
if /usr/bin/time --version 2>&1 | grep -qi 'GNU time'; then
    time_flavor=gnu
elif [[ $(uname -s) == Darwin ]]; then
    time_flavor=bsd
else
    echo "benchmark_verifier_pipeline.sh: unsupported /usr/bin/time implementation" >&2
    exit 2
fi

sha256_file() {
    if command -v sha256sum >/dev/null; then
        sha256sum "$1" | awk '{print $1}'
    else
        shasum -a 256 "$1" | awk '{print $1}'
    fi
}

run_stage() {
    local stage=$1
    shift
    set +e
    if [[ $time_flavor == gnu ]]; then
        /usr/bin/time -f \
            'wall_seconds=%e\nuser_seconds=%U\nsystem_seconds=%S\ncpu_percent=%P\nmax_rss_kib=%M\nexit_status=%x' \
            -o "$output_dir/$stage.time" \
            "$@" >"$output_dir/$stage.log" 2>"$output_dir/$stage.stderr"
        local status=$?
    else
        /usr/bin/time -l -p -o "$output_dir/$stage.time.raw" \
            "$@" >"$output_dir/$stage.log" 2>"$output_dir/$stage.stderr"
        local status=$?
        awk -v status="$status" '
            $1 == "real" { print "wall_seconds=" $2 }
            $1 == "user" { print "user_seconds=" $2 }
            $1 == "sys" { print "system_seconds=" $2 }
            /maximum resident set size/ { printf "max_rss_kib=%.0f\n", $1 / 1024 }
            END { print "exit_status=" status }
        ' "$output_dir/$stage.time.raw" > "$output_dir/$stage.time"
    fi
    set -e
    if (( status != 0 )); then
        echo "benchmark_verifier_pipeline.sh: $stage failed with status $status" >&2
        tail -40 "$output_dir/$stage.stderr" >&2 || true
        exit "$status"
    fi
}

corpus=$output_dir/corpus.raw
if (( line_limit )); then
    head -n "$line_limit" "$input" > "$corpus"
    actual_lines=$(wc -l < "$corpus" | tr -d ' ')
    if (( actual_lines != line_limit )); then
        echo "benchmark_verifier_pipeline.sh: requested $line_limit lines, found $actual_lines" >&2
        exit 2
    fi
else
    cp "$input" "$corpus"
fi

roots_file=$output_dir/roots.cert
{
    echo 'radio-negative-certificate-v1'
    echo '# Explicit roots for an Sa first-test reduction.'
    for root in "${roots[@]}"; do
        printf 'root %d %s\n' "$top_k" "$root"
    done
} > "$roots_file"

for root in "${roots[@]}"; do
    raw=${root//[(),:]/ }
    read -r _ n1 m1 <<<"$raw"
    if ! grep -Eq "^can't solve Sb\\($n1:$m1\\).* in $top_k " "$corpus"; then
        echo "benchmark_verifier_pipeline.sh: root is absent from the selected corpus: $root" >&2
        exit 2
    fi
done

binary=$output_dir/radio_verify
(cd "$repo_dir" && tools/build_radio.py -O3 -pthread radio_verify.c -o "$binary") \
    >"$output_dir/build.log" 2>"$output_dir/build.stderr"

run_context="verifier_pipeline_label=$label"
if [[ -n "$cpus" ]]; then
    run_context="$run_context; cpu_affinity=$cpus"
    launcher=(taskset -c "$cpus" "$repo_dir/tools/run_with_provenance.py" "$binary")
else
    launcher=("$repo_dir/tools/run_with_provenance.py" "$binary")
fi

normalized=$output_dir/normalized.cert
roundtrip=$output_dir/normalized-roundtrip.cert
colored=$output_dir/colored.cert

run_stage sanitize env \
    RADIO_RUN_CONTEXT="$run_context; stage=sanitize" \
    CERT_ONLY=1 CERT_OUT="$normalized" \
    "${launcher[@]}" "$corpus" "$top_k"
run_stage roundtrip env \
    RADIO_RUN_CONTEXT="$run_context; stage=roundtrip" \
    CERT_ONLY=1 CERT_OUT="$roundtrip" \
    "${launcher[@]}" "$normalized" "$top_k"
cmp "$normalized" "$roundtrip"

run_stage color env \
    RADIO_RUN_CONTEXT="$run_context; stage=color" \
    VERIFY_THREADS="$threads" TOPDOWN="$top_k" ROOTS="$roots_file" \
    MINIMIZE_BEFORE_COLOR=1 CERT_OUT="$colored" \
    "${launcher[@]}" "$normalized" "$top_k"
run_stage replay env \
    RADIO_RUN_CONTEXT="$run_context; stage=replay" \
    VERIFY_THREADS="$threads" \
    "${launcher[@]}" "$colored" "$top_k"

for stage in sanitize roundtrip color replay; do
    "$repo_dir/tools/check_provenance.py" "$output_dir/$stage.log" \
        >"$output_dir/$stage.provenance-check"
done
if ! grep -Eq 'TOTAL verified [0-9]+, unverified 0, budget 0,' "$output_dir/replay.log"; then
    echo 'benchmark_verifier_pipeline.sh: replay did not close with zero unresolved facts' >&2
    exit 1
fi

root_count=$(grep -c '^root ' "$colored")
fact_count=$(grep -c '^fact ' "$colored")
expected=$(( root_count + fact_count ))
verified=$(sed -nE 's/^TOTAL verified ([0-9]+), unverified 0, budget 0,.*/\1/p' \
    "$output_dir/replay.log")
if [[ "$verified" != "$expected" ]]; then
    echo "benchmark_verifier_pipeline.sh: replay verified $verified records, expected $expected" >&2
    exit 1
fi

{
    echo "label=$label"
    echo "source_path=$input"
    echo "source_sha256=$(sha256_file "$input")"
    echo "line_limit=$line_limit"
    echo "corpus_lines=$(wc -l < "$corpus" | tr -d ' ')"
    echo "corpus_sha256=$(sha256_file "$corpus")"
    echo "top_k=$top_k"
    echo "threads=$threads"
    echo "cpu_affinity=${cpus:-unrestricted}"
    echo "roots=$root_count"
    echo "colored_facts=$fact_count"
    echo "normalized_sha256=$(sha256_file "$normalized")"
    echo "colored_sha256=$(sha256_file "$colored")"
    echo "replay_verified=$verified"
    for stage in sanitize roundtrip color replay; do
        while IFS= read -r line; do echo "$stage.$line"; done < "$output_dir/$stage.time"
    done
} > "$output_dir/summary.txt"

touch "$output_dir/COMPLETE"
cat "$output_dir/summary.txt"
