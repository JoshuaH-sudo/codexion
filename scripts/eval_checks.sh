#!/usr/bin/env bash
set -eu

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT_DIR/codexion"

LOG_ARGS="${LOG_ARGS:-5 476 60 60 60 3 60 edf}"
VALGRIND_ARGS="${VALGRIND_ARGS:-5 300 40 40 40 1 20 edf}"

pass() {
	echo "PASS: $1"
}

fail() {
	echo "FAIL: $1"
	exit 1
}

build_if_needed() {
	if [ ! -x "$BIN" ]; then
		echo "Building codexion..."
		make -C "$ROOT_DIR" re >/dev/null
	fi
}

check_duplicate_logs() {
	local tmp

	tmp="$(mktemp)"
	set +e
	# This run may end with burnout depending on timing; we still validate log consistency.
	$BIN $LOG_ARGS >"$tmp" 2>&1
	set -e
	if ! awk '
	/^[0-9]+ [0-9]+ / {
		msg = $0
		sub(/^[0-9]+ [0-9]+ /, "", msg)
		id = $2
		if (msg == "has taken a dongle.") {
			taken[id]++
			if (taken[id] > 2) {
				print "coder " id " exceeded 2 dongle-take logs without compile"
				bad = 1
			}
		}
		else if (msg == "is compiling with dongles.") {
			if (taken[id] != 2) {
				print "coder " id " started compile with " taken[id] " take logs"
				bad = 1
			}
			taken[id] = 0
		}
		else if (msg == "has released a dongle.") {
			if (taken[id] > 0)
				taken[id]--
		}
	}
	END {
		exit bad
	}
	' "$tmp"; then
		echo "Duplicate-log check output excerpt:"
		head -n 80 "$tmp"
		rm -f "$tmp"
		fail "duplicate dongle-take log pattern detected"
	fi
	rm -f "$tmp"
	pass "duplicate-log check"
}

expect_invalid() {
	local name
	local tmp

	name="$1"
	shift
	tmp="$(mktemp)"
	if "$BIN" "$@" >"$tmp" 2>&1; then
		echo "Unexpected success for case: $name"
		cat "$tmp"
		rm -f "$tmp"
		fail "invalid-input check failed"
	fi
	rm -f "$tmp"
	pass "invalid case: $name"
}

check_invalid_inputs() {
	local tmp

	tmp="$(mktemp)"
	if "$BIN" >"$tmp" 2>&1; then
		echo "Unexpected success for missing-arguments case"
		cat "$tmp"
		rm -f "$tmp"
		fail "invalid-input check failed"
	fi
	rm -f "$tmp"
	pass "invalid case: missing arguments"

	expect_invalid "zero coders" 0 800 200 200 200 3 10 fifo
	expect_invalid "negative burnout" 5 -1 200 200 200 3 10 fifo
	expect_invalid "non-integer" five 800 200 200 200 3 10 fifo
	expect_invalid "bad scheduler" 5 800 200 200 200 3 10 rr
}

check_helgrind() {
	local tmp

	if ! command -v valgrind >/dev/null 2>&1; then
		fail "valgrind is not installed"
	fi
	tmp="$(mktemp)"
	if ! valgrind --tool=helgrind --error-exitcode=42 \
		"$BIN" $VALGRIND_ARGS >"$tmp" 2>&1; then
		echo "Helgrind output excerpt:"
		tail -n 60 "$tmp"
		rm -f "$tmp"
		fail "helgrind reported threading errors"
	fi
	rm -f "$tmp"
	pass "helgrind thread check"
}

check_memcheck_leaks() {
	local tmp

	tmp="$(mktemp)"
	if ! valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
		--errors-for-leak-kinds=definite,indirect,possible --error-exitcode=43 \
		"$BIN" $VALGRIND_ARGS >"$tmp" 2>&1; then
		echo "Memcheck output excerpt:"
		tail -n 80 "$tmp"
		rm -f "$tmp"
		fail "memcheck reported errors or leaks"
	fi
	if ! grep -Eq "definitely lost:[[:space:]]+0 bytes|no leaks are possible" "$tmp"; then
		echo "Memcheck output excerpt:"
		tail -n 80 "$tmp"
		rm -f "$tmp"
		fail "memcheck leak summary is inconclusive"
	fi
	rm -f "$tmp"
	pass "memcheck leak check"
}

echo "Running evaluation checks..."
build_if_needed
check_duplicate_logs
check_invalid_inputs
check_helgrind
check_memcheck_leaks

echo "All evaluation checks passed."
