#!/usr/bin/env bash
set -eu

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT_DIR/codexion"

run_case() {
	name="$1"
	shift
	echo "== $name =="
	tmp="$(mktemp)"
	if "$BIN" "$@" >"$tmp" 2>&1; then
		head -n 12 "$tmp"
		if grep -q "burned out" "$tmp"; then
			echo "Result: burnout observed"
		else
			echo "Result: completed without burnout"
		fi
		echo ""
		rm -f "$tmp"
	else
		cat "$tmp"
		rm -f "$tmp"
		echo "Case failed: $name"
		exit 1
	fi
}

if [ ! -x "$BIN" ]; then
	echo "Building codexion..."
	make -C "$ROOT_DIR" re >/dev/null
fi

run_case "single coder baseline" 1 500 120 80 80 2 0 fifo
run_case "high contention fifo" 5 800 200 200 200 3 50 fifo
run_case "high contention edf" 5 800 200 200 200 3 50 edf
run_case "cooldown edge" 4 700 120 120 120 3 1 fifo
run_case "edf liveness stress" 5 1000 200 200 200 10 50 edf

echo "All smoke tests passed."
