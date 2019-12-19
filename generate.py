#!/usr/bin/python

import fileinput
import glob
import re
import shutil
import os
import contextlib
import subprocess
import sys


IMPLEMENTATIONS = [
    ("mceliece460896", "vec", "vec"),
    ("mceliece460896f", "vec", "vec"),
    ("mceliece348864f", "vec", "vec"),
    ("mceliece6688128", "vec", "vec"),
    ("mceliece6688128f", "vec", "vec"),
    ("mceliece348864", "ref", "clean"),
    ("mceliece348864", "vec", "vec"),
    ("mceliece348864f", "ref", "clean"),
    ("mceliece460896", "ref", "clean"),
    ("mceliece460896f", "ref", "clean"),
    ("mceliece6688128", "ref", "clean"),
    ("mceliece6688128f", "ref", "clean"),
    ("mceliece6960119", "ref", "clean"),
    ("mceliece6960119f", "ref", "clean"),
    ("mceliece8192128", "ref", "clean"),
    ("mceliece8192128f", "ref", "clean"),
]


def replace_in_file(path, text_to_search, replacement_text):
    with fileinput.FileInput(path, inplace=True) as file:
        for line in file:
            print(re.sub(text_to_search, replacement_text, line), end="")


def astyle(*paths):
    subprocess.check_output(["astyle", f"--options=.astylerc", *paths])


for (scheme, impl, dst) in IMPLEMENTATIONS:
    print(f"{scheme} {impl} -> {dst}")
    namespace = f"PQCLEAN_{scheme}_{dst}_".upper()
    src_dir = os.path.join("schemes", scheme, impl)
    dest_dir = os.path.join("crypto_kem", scheme, dst)

    with contextlib.suppress(IOError):
        shutil.rmtree(dest_dir)
    os.makedirs(dest_dir)
    sourcefiles = []

    for src_file in glob.glob(os.path.join(src_dir, "*")):
        filename = os.path.basename(src_file)
        dest_file = os.path.join(dest_dir, filename)
        shutil.copy(src_file, dest_dir)
        if filename[-2:] in (".c", ".h"):
            sourcefiles.append(dest_file)
        replace_in_file(dest_file, "MC_", namespace)

    if len(sys.argv) > 1 and sys.argv[1] == 'tidy':
        subprocess.run(
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
            capture_output=True,
        )

    astyle(*sourcefiles)

    replace_in_file(
        os.path.join(dest_dir, "Makefile"),
        r"libmceliece_.*\.a",
        f"lib{scheme}_{dst}.a",
    )
    replace_in_file(
        os.path.join(dest_dir, "Makefile.Microsoft_nmake"),
        r"libmceliece_.*\.lib",
        f"lib{scheme}_{dst}.a",
    )
