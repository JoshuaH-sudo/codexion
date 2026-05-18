#!/usr/bin/env bash
set -eu

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT_DIR/codexion"
RUNS="${RUNS:-5}"

# Format: name|group|expectation|args
# expectation: success -> should have no burnout
# expectation: fail -> should show at least one burnout
cases=(
"single-coder-no-cooldown|edge|success|1 500 200 100 100 2 0 fifo"
"single-coder-edf-cooldown|edge|success|1 500 200 100 100 2 50 edf"
"two-coders-tight-fifo|expected-fail|fail|2 500 150 100 100 3 0 fifo"
"two-coders-tight-edf|expected-fail|fail|2 500 150 100 100 3 50 edf"
"balanced-5-fifo|baseline|success|5 1200 150 150 150 3 0 fifo"
"balanced-5-edf|baseline|success|5 1200 150 150 150 3 50 edf"
"contention-5-fifo|expected-fail|fail|5 800 200 200 200 3 50 fifo"
"contention-5-edf|expected-fail|fail|5 800 200 200 200 3 50 edf"
"ten-coders-fast-fifo|edge|success|10 500 80 80 80 3 10 fifo"
"ten-coders-fast-edf|edge|success|10 500 80 80 80 3 10 edf"
"minimal-cooldown-fifo|edge|success|4 700 120 120 120 3 1 fifo"
"minimal-cooldown-edf|edge|success|4 700 120 120 120 3 1 edf"
"five-coders-very-tight-fifo|expected-fail|fail|5 450 200 100 100 2 0 fifo"
"five-coders-very-tight-edf|expected-fail|fail|5 450 200 100 100 2 0 edf"
"long-debug-edf|edge|success|5 900 50 300 300 2 20 edf"
"large-64-feasible-edf|large-coders|success|64 3000 25 25 25 1 5 edf"
"large-120-feasible-edf|large-coders|success|120 5000 15 15 15 1 2 edf"
"large-180-feasible-edf|large-coders|success|180 8000 10 10 10 1 1 edf"
"large-199-feasible-edf|large-coders|success|199 9000 8 8 8 1 1 edf"
"large-120-tight-edf|large-coders|fail|120 700 30 30 30 2 10 edf"
)

if [ ! -x "$BIN" ]; then
	echo "Building codexion..."
	make -C "$ROOT_DIR" re >/dev/null
fi

echo "Case | Group | Expectation | coders | burnout_ms | compile_ms | debug_ms | refactor_ms | required_compiles | cooldown_ms | scheduler | Burnouts/$RUNS | Outcome"
echo "---|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|---"

for entry in "${cases[@]}"; do
	burnouts=0
	name="${entry%%|*}"
	rest="${entry#*|}"
	group="${rest%%|*}"
	rest="${rest#*|}"
	expectation="${rest%%|*}"
	arg_set="${rest#*|}"
	read -r coders burnout_ms compile_ms debug_ms refactor_ms required cooldown scheduler << EOF
$arg_set
EOF
	for _ in $(seq 1 "$RUNS"); do
		if "$BIN" $arg_set 2>&1 | grep -q "burned out"; then
			burnouts=$((burnouts + 1))
		fi
	done
	outcome="mismatch"
	if [ "$expectation" = "success" ] && [ "$burnouts" -eq 0 ]; then
		outcome="ok"
	elif [ "$expectation" = "fail" ] && [ "$burnouts" -gt 0 ]; then
		outcome="ok"
	fi
	echo "$name | $group | $expectation | $coders | $burnout_ms | $compile_ms | $debug_ms | $refactor_ms | $required | $cooldown | $scheduler | $burnouts/$RUNS | $outcome"
done
