#!/usr/bin/python

import fileinput
import glob
import re
import shutil
import os
import contextlib
import subprocess
import sys
import itertools


IMPLEMENTATIONS = itertools.product(
        ["mceliece348864", "mceliece460896", "mceliece6688128",
         "mceliece6960119", "mceliece8192128"],
        ('', 'f'),
        [('ref', 'clean'), ('vec', 'vec'), ('sse', 'sse'), ('avx', 'avx')])


def replace_in_file(path, text_to_search, replacement_text):
    with fileinput.FileInput(path, inplace=True) as file:
        for line in file:
            print(re.sub(text_to_search, replacement_text, line), end="")


def astyle(*paths):
    subprocess.check_output(["astyle", f"--options=.astylerc", *paths])


for (scheme, variant, (impl, dst)) in IMPLEMENTATIONS:
    scheme = f"{scheme}{variant}"
    print(f"{scheme} {impl} -> {dst}")
    namespace = f"PQCLEAN_{scheme}_{dst}_".upper()
    src_dir = os.path.join("schemes", scheme, impl)
    dest_dir = os.path.join("crypto_kem", scheme, dst)

    with contextlib.suppress(IOError):
        shutil.rmtree(dest_dir)
    os.makedirs(dest_dir)
    sourcefiles = []

    subprocess.run(
        ['make', 'clean'],
        cwd=src_dir,
        check=False,
        capture_output=True)

    for src_file in glob.glob(os.path.join(src_dir, "*")):
        filename = os.path.basename(src_file)
        dest_file = os.path.join(dest_dir, filename)
        shutil.copy(src_file, dest_dir)
        if filename[-2:] in (".c", ".h"):
            sourcefiles.append(dest_file)
        replace_in_file(dest_file, "MC_", namespace)

    astyle(*sourcefiles)
    while len(sys.argv) > 1 and sys.argv[1] == 'tidy':
        result = subprocess.run(
            [
                "clang-tidy",
                "-fix-errors",
                "-quiet",
                "-header-filter=.*",
                *[file for file in sourcefiles if file[-2:] in (".c", "*.h")],
                "--",
                "-iquote",
                os.path.join("common"),
                "-iquote",
                dest_dir,
            ],
            check=False, capture_output=True,
        )
        if result.returncode == -11:
            print("Segfaulted while running clang-tidy, trying again")
            # segfault, try again
            continue
        break

    astyle(*sourcefiles)

    replace_in_file(
        os.path.join(dest_dir, "Makefile"),
        r"libmceliece_.*\.a",
        f"lib{scheme}_{dst}.a",
    )
    if impl not in ('sse', 'avx'):
        replace_in_file(
            os.path.join(dest_dir, "Makefile.Microsoft_nmake"),
            r"libmceliece_.*\.lib",
            f"lib{scheme}_{dst}.lib",
        )
