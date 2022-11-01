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

pushd crypto_kem
for scheme in *; do
    pushd $scheme
    if [ -d $dst ]; then
        for f in ${src}/*; do
            output="$(diff -u "$f" "${f/$src/$dst}")"

            if [[ "$output" == "" ]]; then
                echo "$scheme/$f is unchanged"
            else
                echo "$scheme/$f was patched"
                patchfile=../../patches/"$scheme"/"$f".patch
                echo "${output}" > $patchfile
                sed -i 's@--- '"$f"'@--- '"${f/$src/a}"'@' $patchfile
                sed -i 's@+++ '"${f/$src/$dst}"'@+++ '"${f/$src/b}"'@' $patchfile
                # delete dates for rmdiff purposes
                sed -i 's@[0-9]\{4\}-[0-9]\{2\}.*@@' $patchfile

            fi
        done
    else
        echo "Scheme $scheme doesn't have $dst dir"
    fi
    popd
done
popd

./generate.py tidy
