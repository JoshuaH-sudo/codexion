#!/bin/bash
args=(
"1 500 200 100 100 2 0 fifo"
"1 500 200 100 100 2 50 edf"
"2 500 150 100 100 3 0 fifo"
"2 500 150 100 100 3 50 edf"
"5 1200 150 150 150 3 0 fifo"
"5 1200 150 150 150 3 50 edf"
"5 800 200 200 200 3 50 fifo"
"5 800 200 200 200 3 50 edf"
"10 500 80 80 80 3 10 fifo"
"10 500 80 80 80 3 10 edf"
"4 700 120 120 120 3 1 fifo"
"4 700 120 120 120 3 1 edf"
"5 450 200 100 100 2 0 fifo"
"5 450 200 100 100 2 0 edf"
"5 900 50 300 300 2 20 edf"
)

echo "Args | Scheduler | Burnouts/5"
echo "---|---|---"

for arg_set in "${args[@]}"; do
    burnouts=0
    sched=$(echo $arg_set | awk '{print $NF}')
    params=$(echo $arg_set | rev | cut -d' ' -f2- | rev)
    for i in {1..5}; do
        output=$(./codexion $arg_set 2>&1)
        if echo "$output" | grep -q "burned out"; then
            ((burnouts++))
        fi
    done
    echo "$params | $sched | $burnouts/5"
done
