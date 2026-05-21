#!/usr/bin/env bash
set -eu

usage() {
	echo "Usage:"
	echo "  $0 <coders> <compile_ms> <debug_ms> <refactor_ms> <cooldown_ms> [margin_ms]"
	echo "  $0 <coders> <burnout_ms> <compile_ms> <debug_ms> <refactor_ms> <required_compiles> <cooldown_ms> <scheduler>"
	echo ""
	echo "If margin is not provided, an automatic margin is used (60% of minimal burnout, floor 100 ms)."
	echo ""
	echo "Examples:"
	echo "  $0 5 100 100 100 10 15"
}

is_non_negative_int() {
	case "$1" in
		""|*[!0-9]*) return 1 ;;
		*) return 0 ;;
	esac
}

MARGIN_MS=""

if [ "$#" -eq 5 ] || [ "$#" -eq 6 ]; then
	CODERS="$1"
	COMPILE_MS="$2"
	DEBUG_MS="$3"
	REFACTOR_MS="$4"
	COOLDOWN_MS="$5"
	INPUT_MODE="hint"
	REQUIRED_COMPILES="<required_compiles>"
	SCHEDULER="<scheduler>"
elif [ "$#" -eq 8 ]; then
	# Project ARGS format: coders burnout compile debug refactor required cooldown scheduler
	CODERS="$1"
	COMPILE_MS="$3"
	DEBUG_MS="$4"
	REFACTOR_MS="$5"
	REQUIRED_COMPILES="$6"
	COOLDOWN_MS="$7"
	SCHEDULER="$8"
	INPUT_MODE="project"
else
	usage
	exit 1
fi

for value in "$CODERS" "$COMPILE_MS" "$DEBUG_MS" "$REFACTOR_MS" "$COOLDOWN_MS"; do
	if ! is_non_negative_int "$value"; then
		echo "Error: all parameters must be non-negative integers (milliseconds for times)."
		usage
		exit 1
	fi
done

if [ -n "$MARGIN_MS" ] && ! is_non_negative_int "$MARGIN_MS"; then
	echo "Error: margin must be a non-negative integer (milliseconds)."
	usage
	exit 1
fi

if [ "$CODERS" -eq 0 ]; then
	echo "Error: <coders> must be greater than 0."
	exit 1
fi

FULL_WORK_CYCLE=$((COMPILE_MS + DEBUG_MS + REFACTOR_MS))

if [ $((CODERS % 2)) -eq 0 ]; then
	CASE_LABEL="even"
	RESOURCE_TURNS=$((2 * COMPILE_MS + 2 * COOLDOWN_MS))
else
	CASE_LABEL="odd"
	RESOURCE_TURNS=$((3 * COMPILE_MS + 3 * COOLDOWN_MS))
fi

if [ "$FULL_WORK_CYCLE" -gt "$RESOURCE_TURNS" ]; then
	MINIMAL_BURNOUT="$FULL_WORK_CYCLE"
	LIMITING_FACTOR="full work cycle"
else
	MINIMAL_BURNOUT="$RESOURCE_TURNS"
	LIMITING_FACTOR="resource turns"
fi

# Empirical default: contention-heavy runs need a broad buffer, not timer-level jitter.
MARGIN_MS=$((MINIMAL_BURNOUT * 60 / 100))
if [ "$MARGIN_MS" -lt 100 ]; then
	MARGIN_MS=100
fi

LOW_TEST="$MINIMAL_BURNOUT"
if [ "$MINIMAL_BURNOUT" -gt "$MARGIN_MS" ]; then
	LOW_TEST=$((MINIMAL_BURNOUT - MARGIN_MS))
fi
HIGH_TEST=$((MINIMAL_BURNOUT + MARGIN_MS))

echo "Burnout timing recommendation"
echo "============================"
echo "Input mode:          $INPUT_MODE"
echo "Coders:              $CODERS ($CASE_LABEL case)"
echo "Full work cycle:     $FULL_WORK_CYCLE ms (compile + debug + refactor)"
echo "Resource-turn bound: $RESOURCE_TURNS ms"
echo "Minimal burnout:     $MINIMAL_BURNOUT ms"
echo "Limiting factor:     $LIMITING_FACTOR"
echo "Margin:              +/-$MARGIN_MS ms"
echo ""
echo "Suggested burnout values to test:"
echo "- minimal:             $MINIMAL_BURNOUT ms"
echo "- likely-pass (above): $HIGH_TEST ms"
echo ""
echo "Copy/paste commands:"
echo "- Suggested threshold: make run ARGS=\"$CODERS $HIGH_TEST $COMPILE_MS $DEBUG_MS $REFACTOR_MS $REQUIRED_COMPILES $COOLDOWN_MS $SCHEDULER\""
