#!/usr/bin/env bash
set -eu

usage() {
	echo "Usage:"
	echo "  $0 <coders> <compile_ms> <debug_ms> <refactor_ms> <number_of_compiles> <cooldown_ms>"
	echo ""
	echo "Equation used:"
	echo "  burnout_ms = coders * (refactor + debug + compile + cooldown) * number_of_compiles"
	echo ""
	echo "Example:"
	echo "  $0 199 60 60 60 3 60"
}

is_non_negative_int() {
	case "$1" in
		""|*[!0-9]*) return 1 ;;
		*) return 0 ;;
	esac
}

if [ "$#" -ne 6 ]; then
	usage
	exit 1
fi

CODERS="$1"
COMPILE_MS="$2"
DEBUG_MS="$3"
REFACTOR_MS="$4"
NUMBER_OF_COMPILES="$5"
COOLDOWN_MS="$6"

for value in "$CODERS" "$COMPILE_MS" "$DEBUG_MS" "$REFACTOR_MS" "$NUMBER_OF_COMPILES" "$COOLDOWN_MS"; do
	if ! is_non_negative_int "$value"; then
		echo "Error: all numeric arguments must be non-negative integers."
		usage
		exit 1
	fi
done

if [ "$CODERS" -eq 0 ]; then
	echo "Error: <coders> must be greater than 0."
	exit 1
fi

if [ "$NUMBER_OF_COMPILES" -eq 0 ]; then
	echo "Error: <number_of_compiles> must be greater than 0."
	exit 1
fi

CYCLE_MS=$((REFACTOR_MS + DEBUG_MS + COMPILE_MS + COOLDOWN_MS))
BURNOUT_MS=$((CODERS * CYCLE_MS * NUMBER_OF_COMPILES))
BURNOUT_S=$(awk "BEGIN { printf \"%.3f\", $BURNOUT_MS / 1000 }")

echo "Burnout calculator"
echo "=================="
echo "Coders:             $CODERS"
echo "Per-compile cycle:  $CYCLE_MS ms (refactor + debug + compile + cooldown)"
echo "Number of compiles: $NUMBER_OF_COMPILES"
echo "Calculated burnout: $BURNOUT_MS ms ($BURNOUT_S s)"
echo ""
echo "make command:"
echo "make run ARGS=\"$CODERS $BURNOUT_MS $COMPILE_MS $DEBUG_MS $REFACTOR_MS $NUMBER_OF_COMPILES $COOLDOWN_MS fifo\""
