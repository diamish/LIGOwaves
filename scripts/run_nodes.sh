#!/usr/bin/env bash
# Example orchestration script for time-slicing jobs across nodes.
# Usage: ./run_nodes.sh T0 T1 CHUNK_SECONDS nodes.txt input.h5 outdir
# nodes.txt contains one hostname per line (e.g., pi@node01)
set -euo pipefail

if [ "$#" -ne 6 ]; then
  echo "Usage: $0 T0 T1 CHUNK_SECONDS nodes.txt input.h5 outdir"
  exit 1
fi

T0=$1
T1=$2
CHUNK=$3
NODES_FILE=$4
INPUT=$5
OUTDIR=$6

mkdir -p "${OUTDIR}"

mapfile -t NODES < "${NODES_FILE}"
NUM_NODES=${#NODES[@]}
echo "Using ${NUM_NODES} nodes."

# small overlap to avoid cutting events
OVERLAP=2

start=${T0}
idx=0
while [ "${start}" -lt "${T1}" ]; do
  end=$(( start + CHUNK ))
  if [ "${end}" -gt "${T1}" ]; then
    end=${T1}
  fi
  node=${NODES[$((idx % NUM_NODES))]}
  out="${OUTDIR}/candidates_${start}_${end}.csv"
  echo "Dispatching ${node} -> ${start} .. ${end} -> ${out}"
  ssh "${node}" "cd ~/ligo_scan_build && ./ligo_scan --input ${INPUT} --start ${start} --end ${end} --fs 4096 --out ${out}" &
  start=$(( end - OVERLAP ))
  idx=$(( idx + 1 ))
done

wait
echo "All jobs dispatched/completed."

