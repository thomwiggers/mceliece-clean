#!/bin/zsh

# Using this script:
# 1. ./generate.py, do not specify tidy!
# 2. Copy the to-be-fixed scheme/$impl folder to scheme/$fixed
# 3. Fix the files in scheme/$fixed
# 4. ./generate_patches.sh $impl $fixed
# 5. run tests, etc
# 6. if not done yet, goto 3.

if [[ "$2" == "" ]]; then
    echo "Usage $0 orig patched"
    exit 1;
fi

src=$1
dst=$2

./generate.py unpatch

echo "Unsymlinking patch files"
find patches -type l -exec sed -i '' \{\} \;

pushd crypto_kem
for scheme in *; do
    pushd $scheme
    if [ -d $dst ]; then
        for f in ${src}/*; do
            output="$(diff -Zu "$f" "${f/$src/$dst}")"

            if [[ "$output" == "" ]]; then
                echo "$scheme/$f is unchanged"
            else
                echo "$scheme/$f was patched"
                patchfile=../../patches/"$scheme"/"$f".patch
                echo "${output}" > $patchfile
                # delete dates for rmdiff purposes
                sed -i 's@ \+[0-9]\{4\}-[0-9]\{2\}.*@@' $patchfile
            fi
        done
    else
        echo "Scheme $scheme doesn't have $dst dir"
    fi
    popd
done
popd

echo "Cleaning up patch files"
sed -i 's@--- '${src}'/@--- a/@'                      patches/*/$src/*
sed -i 's@\+\+\+ '${dst}'/@+++ b/@'                   patches/*/$src/*
sed -i 's@[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\} [0-9].*@@' patches/*/$src/*

set -e

echo "Symlinking patch files"
find patches -type l -exec sed -i '' \{\} \;
rmlint -o sh:rmlint.sh -c sh:cmd='echo "symlinking to original $2" && rm -rf "$1" && ln -s -r  "$2" "$1"' --rank-by=p patches/*/clean patches/*/avx2 > /dev/null
./rmlint.sh -d > /dev/null
rm rmlint.json

./generate.py tidy
