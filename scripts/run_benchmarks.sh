#!/bin/bash

mkdir output

function run() {
  echo $1
  OMP_NUM_THREADS=1 ./$1 > output/$1-1
  sleep 1
  OMP_NUM_THREADS=2 ./$1 > output/$1-2
  sleep 1
  OMP_NUM_THREADS=4 ./$1 > output/$1-4
  sleep 1
  OMP_NUM_THREADS=8 ./$1 > output/$1-8
  sleep 1
  OMP_NUM_THREADS=16 ./$1 > output/$1-16
  sleep 1
}

run stress_256
run stress_256_no_delays
run stress_256_no_delays_transposed
run stress_256_transposed
