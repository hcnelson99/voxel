#!/bin/bash

rm -f benchmark

if [ $# -ne 1 ]; then
    echo "Usage: ./benchmark.sh [world file]"
    exit 1
fi

world="$1"

if [ -f "$world" ]; then
    size=$(wc -c "$world" | awk '{print $1}')
    ws=$(python -c "print(int(round($size ** (1.0 / 3))))")

    make benchmark CFLAGS="-DWORLD_SIZE=$ws -DWORLD=\"$world\""
else
    echo "World file not found: $world"
fi
