#!/bin/bash
rmlint -c sh:hardlink schemes/*/* >/dev/null
./rmlint.sh -d > /dev/null

for scheme in schemes/*; do
    scheme=$(basename $scheme)
    python3 duplicate_file_generator.py $scheme ref clean
    python3 duplicate_file_generator.py $scheme vec vec
done

rm rmlint*
