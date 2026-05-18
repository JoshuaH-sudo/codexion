#!/usr/bin/env bash
set -eu

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT_DIR/codexion"
RUNS=10
ARGS=(10 500 80 80 80 3 10 edf)

if [ ! -x "$BIN" ]; then
	echo "Building codexion..."
	make -C "$ROOT_DIR" re >/dev/null
fi

burnout_runs=0
burned_ids=()

printf "Running consistency test: %d runs of: %s\n\n" "$RUNS" "$BIN ${ARGS[*]}"

for i in $(seq 1 "$RUNS"); do
	tmp="$(mktemp)"
	if "$BIN" "${ARGS[@]}" >"$tmp" 2>&1; then
		if grep -q "burned out" "$tmp"; then
			burnout_runs=$((burnout_runs + 1))
			line="$(grep "burned out" "$tmp" | tail -n 1)"
			burned_id="$(printf "%s\n" "$line" | awk '{print $2}')"
			burned_ids+=("$burned_id")
			printf "Run %2d: burnout (coder %s)\n" "$i" "$burned_id"
		else
			printf "Run %2d: no burnout\n" "$i"
		fi
	else
		echo "Run $i failed to execute"
		cat "$tmp"
		rm -f "$tmp"
		exit 1
	fi
	rm -f "$tmp"
done

echo ""
printf "Summary: %d/%d runs had burnout\n" "$burnout_runs" "$RUNS"

if [ "$burnout_runs" -gt 0 ]; then
	unique_count="$(printf "%s\n" "${burned_ids[@]}" | sort -u | wc -l | tr -d ' ')"
	echo "Burned coder ids by run: ${burned_ids[*]}"
	if [ "$burnout_runs" -eq "$RUNS" ] && [ "$unique_count" -eq 1 ]; then
		echo "Consistency: CONSISTENT (same burnout pattern every run)"
	else
		echo "Consistency: INCONSISTENT (burnout varies by run)"
	fi
else
	echo "Consistency: CONSISTENT (no burnout in any run)"
fi
