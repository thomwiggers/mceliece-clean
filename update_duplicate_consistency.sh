#!/bin/bash
set -e
rmlint -c sh:hardlink schemes/*/{ref,avx} >/dev/null
./rmlint.sh -d > /dev/null
rmlint -c sh:hardlink schemes/*/{ref,avx} >/dev/null

for scheme in schemes/*; do
    scheme=$(basename $scheme)
    python3 duplicate_file_generator.py $scheme ref clean
    python3 duplicate_file_generator.py $scheme avx avx2
done

rm rmlint*
