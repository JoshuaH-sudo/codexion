#!/usr/bin/env bash
set -eu

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT_DIR/codexion"

BATCHES="${1:-20}"
RUNS_PER_BATCH="${2:-10}"

ARGS=(10 500 80 80 80 3 10 edf)

if [ ! -x "$BIN" ]; then
	echo "Building codexion..."
	make -C "$ROOT_DIR" re >/dev/null
fi

total_runs=$((BATCHES * RUNS_PER_BATCH))
total_burnouts=0
batches_with_burnout=0

declare -A burnout_by_coder

printf "Batch consistency test\n"
printf "Batches: %d\n" "$BATCHES"
printf "Runs per batch: %d\n" "$RUNS_PER_BATCH"
printf "Total runs: %d\n" "$total_runs"
printf "Command: %s\n\n" "$BIN ${ARGS[*]}"

batch=1
while [ "$batch" -le "$BATCHES" ]; do
	batch_burnouts=0
	run=1
	while [ "$run" -le "$RUNS_PER_BATCH" ]; do
		tmp="$(mktemp)"
		if "$BIN" "${ARGS[@]}" >"$tmp" 2>&1; then
			if grep -q "burned out" "$tmp"; then
				batch_burnouts=$((batch_burnouts + 1))
				total_burnouts=$((total_burnouts + 1))
				line="$(grep "burned out" "$tmp" | tail -n 1)"
				coder_id="$(printf "%s\n" "$line" | awk '{print $2}')"
				if [ -n "$coder_id" ]; then
					if [ -z "${burnout_by_coder[$coder_id]+x}" ]; then
						burnout_by_coder[$coder_id]=0
					fi
					burnout_by_coder[$coder_id]=$((burnout_by_coder[$coder_id] + 1))
				fi
			fi
		else
			echo "Run failed at batch $batch run $run"
			cat "$tmp"
			rm -f "$tmp"
			exit 1
		fi
		rm -f "$tmp"
		run=$((run + 1))
	done
	if [ "$batch_burnouts" -gt 0 ]; then
		batches_with_burnout=$((batches_with_burnout + 1))
	fi
	printf "Batch %2d: %d/%d runs had burnout\n" "$batch" "$batch_burnouts" "$RUNS_PER_BATCH"
	batch=$((batch + 1))
done

burnout_percent="$(awk -v b="$total_burnouts" -v t="$total_runs" 'BEGIN { printf "%.2f", (b * 100.0) / t }')"

printf "\nSummary\n"
printf "Total burnouts: %d/%d (%s%%)\n" "$total_burnouts" "$total_runs" "$burnout_percent"
printf "Batches with burnout: %d/%d\n" "$batches_with_burnout" "$BATCHES"

if [ "$total_burnouts" -eq 0 ]; then
	echo "Consistency: HIGH (no burnout observed)"
else
	echo "Burnout distribution by coder id:"
	for coder in "${!burnout_by_coder[@]}"; do
		printf "  coder %s: %d\n" "$coder" "${burnout_by_coder[$coder]}"
	done | sort -n -k2
	echo "Consistency: VARIABLE (burnout behavior not deterministic)"
fi
