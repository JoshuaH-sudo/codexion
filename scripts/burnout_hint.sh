#!/usr/bin/env bash
set -eu

usage() {
	echo "Usage: $0 <coders> <compile_ms> <debug_ms> <refactor_ms> <cooldown_ms> [margin_ms]"
	echo "Example: $0 5 100 100 100 10 15"
}

is_non_negative_int() {
	case "$1" in
		""|*[!0-9]*) return 1 ;;
		*) return 0 ;;
	esac
}

if [ "$#" -lt 5 ] || [ "$#" -gt 6 ]; then
	usage
	exit 1
fi

CODERS="$1"
COMPILE_MS="$2"
DEBUG_MS="$3"
REFACTOR_MS="$4"
COOLDOWN_MS="$5"
MARGIN_MS="${6:-15}"

for value in "$CODERS" "$COMPILE_MS" "$DEBUG_MS" "$REFACTOR_MS" "$COOLDOWN_MS" "$MARGIN_MS"; do
	if ! is_non_negative_int "$value"; then
		echo "Error: all parameters must be non-negative integers (milliseconds for times)."
		usage
		exit 1
	fi
done

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

LOW_TEST="$MINIMAL_BURNOUT"
if [ "$MINIMAL_BURNOUT" -gt "$MARGIN_MS" ]; then
	LOW_TEST=$((MINIMAL_BURNOUT - MARGIN_MS))
fi
HIGH_TEST=$((MINIMAL_BURNOUT + MARGIN_MS))

echo "Burnout timing recommendation"
echo "============================"
echo "Coders:              $CODERS ($CASE_LABEL case)"
echo "Full work cycle:     $FULL_WORK_CYCLE ms (compile + debug + refactor)"
echo "Resource-turn bound: $RESOURCE_TURNS ms"
echo "Minimal burnout:     $MINIMAL_BURNOUT ms"
echo "Limiting factor:     $LIMITING_FACTOR"
echo "Margin:              +/-$MARGIN_MS ms"
echo ""
echo "Suggested burnout values to test:"
echo "- near-fail (below): $LOW_TEST ms"
echo "- threshold:         $MINIMAL_BURNOUT ms"
echo "- near-pass (above): $HIGH_TEST ms"
