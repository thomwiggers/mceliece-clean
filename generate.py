#!/usr/bin/python

import fileinput
import io
import glob
import re
import shutil
import os
import contextlib
import subprocess
import sys
import functools
import itertools
import hashlib
from pathlib import Path

from jinja2 import Environment, FileSystemLoader, select_autoescape

JINJAENV = Environment(
    loader=FileSystemLoader("templates"), autoescape=select_autoescape()
)

IMPLEMENTATIONS = itertools.product(
    [
        "348864",
        # "460896",
        # "6688128",
        # "6960119",
        # "8192128",
    ],
    ("", "f"),
    [("ref", "clean"), ("avx", "avx2")],
)


def generate_sources(sourcefiles):
    sources = sorted([
        str(Path(s).name) for s in sourcefiles
        if s.endswith(".c") or s.endswith(".S")
    ])
    headers = sorted([
        str(Path(s).name) for s in sourcefiles if re.match(r".*\.([h])", s)
    ])
    objects = [s.replace(".c", ".o") for s in sources]
    objects = [obj.replace(".S", ".o") for obj in objects]
    result = {"sources": sources, "headers": headers, "objects": sorted(objects)}
    assert all(x.endswith(".o") for x in objects)
    assert all(x.endswith(".h") for x in headers)
    return result


def get_sizes(scheme):
    with (Path("schemes") / scheme / "ref" / "api.h").open() as apifh:
        contents = apifh.readlines()

    result = {}

    for line in contents:
        if not line.startswith("#define CRYPTO_"):
            continue
        _, macro, size = line.split(" ")
        size = int(size)
        if "PUBLIC" in macro:
            result["pk_bytes"] = size
        elif "SECRET" in macro:
            result["sk_bytes"] = size
        elif "CIPHERTEXT" in macro:
            result["ct_bytes"] = size
        elif "CRYPTO_BYTES" == macro:
            result["ss_bytes"] = size

    return result


def get_nist_level(scheme):
    if "348864" in scheme:
        return 1
    elif "460896" in scheme:
        return 3
    else:
        return 5


def split_list(predicate, lst):
    head = itertools.takewhile(predicate, lst)
    tail = list(itertools.dropwhile(predicate, lst))
    return head, tail


def cleanup_header(filename):
    with open(filename, "r") as fh:
        contents = fh.readlines()
    comments, remainder = split_list(lambda x: not x.startswith("#ifndef"), contents)

    with open(filename, "w") as fh:
        fh.writelines(remainder[:2])
        fh.writelines(comments)
        fh.write('\n#include "namespace.h"\n\n')
        fh.writelines(remainder[2:])


def unifdef(filename):
    subprocess.run(["unifdef", "-UKAT", f"-o{filename}", filename])


@functools.lru_cache
def get_nistkat(scheme):
    with (Path("KAT") / scheme / "kat_kem.rsp").open("rb") as katfh:
        kat = katfh.readlines()
        buf = io.BytesIO()
        buf.writelines(kat[2:8])
        sha = hashlib.sha256()
        sha.update(buf.getvalue())
        return sha.hexdigest()


def cleanup_file(filename, schemename, impl):
    if filename.endswith(".h"):
        cleanup_header(filename)

    unifdef(filename)

    if len(sys.argv) == 1 or (len(sys.argv) > 1 and sys.argv[1] != "unpatch"):
        apply_patches(filename, schemename, impl)


def apply_patches(filename, schemename, impl):
    filename = Path(filename)
    patchdir = Path("patches") / schemename / impl

    if (patchfile := (patchdir / f"{filename.name}.patch")).exists():
        with patchfile.open() as fh:
            subprocess.run(["patch", "-p1"], cwd=filename.parent, stdin=fh, check=True)


def replace_in_file(path, text_to_search, replacement_text):
    with fileinput.FileInput(path, inplace=True) as file:
        for line in file:
            print(re.sub(re.escape(text_to_search), replacement_text, line), end="")


def astyle(*paths):
    if len(sys.argv) > 1 and sys.argv[1] == "tidy":
        subprocess.check_output(["astyle", f"--options=.astylerc", *paths])


def is_blacklisted(filename):
    if "goal-" in filename:
        return True
    if filename.endswith(".q"):
        return True

    return False


for (schemesize, variant, (impl, dst)) in IMPLEMENTATIONS:
    scheme = f"mceliece{schemesize}{variant}"
    print(f"{scheme} {impl} -> {dst}")
    namespace = f"PQCLEAN_{scheme}_{dst}".upper()
    src_dir = os.path.join("schemes", scheme, impl)
    dest_dir = os.path.join("crypto_kem", scheme, dst)

    template_vars = {
        "name": f"Classic McEliece {schemesize}",
        "size": str(schemesize),
        "scheme": scheme,
        "NS": namespace,
        "kat_hash": get_nistkat(scheme),
        "version": "SUPERCOP-20221025",
        "nist_level": get_nist_level(scheme),
        **get_sizes(scheme),
    }

    with contextlib.suppress(IOError):
        shutil.rmtree(dest_dir)
    os.makedirs(dest_dir)
    sourcefiles = []

    subprocess.run(["make", "clean"], cwd=src_dir, check=False, capture_output=True)

    for src_file in glob.glob(os.path.join(src_dir, "*")):
        if is_blacklisted(src_file):
            continue
        filename = os.path.basename(src_file)
        dest_file = os.path.join(dest_dir, filename)
        shutil.copy2(src_file, dest_dir)
        if filename[-2:] in (".c", ".h", ".S") or filename.endswith(".data"):
            sourcefiles.append(dest_file)
        cleanup_file(dest_file, scheme, dst)

    # Copy additional files
    for file in [
        "aes256ctr.c",
        "aes256ctr.h",
        "crypto_hash.h",
        "crypto_int16.h",
        "crypto_int16.c",
        "crypto_int32.h",
        "crypto_int32.c",
        "crypto_uint16.h",
        "crypto_uint16.c",
        "crypto_uint32.h",
        "crypto_uint32.c",
        "crypto_uint64.h",
        "crypto_uint64.c",
        "int32_sort.h",
        "LICENSE",
    ]:
        src_file = f"extra_files/{file}"
        dest_file = os.path.join(dest_dir, file)
        shutil.copy(src_file, dest_dir)
        sourcefiles.append(dest_file)
        replace_in_file(dest_file, "MC_", f"{namespace}_")

    astyle(*sourcefiles)
    while len(sys.argv) > 1 and sys.argv[1] == "tidy":
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
            check=False,
            capture_output=True,
        )
        if result.returncode == -11:
            print("Segfaulted while running clang-tidy, trying again")
            # segfault, try again
            continue
        break

    astyle(*sourcefiles)

    template_vars.update(generate_sources(sourcefiles))

    # Render templates
    for tmpl in [f"{dst}/Makefile", "api.h", "crypto_kem.h", "namespace.h"]:
        basefilename = os.path.basename(tmpl)
        JINJAENV.get_template(f"{tmpl}.j2").stream(**template_vars).dump(
            f"{ dest_dir }/{basefilename}"
        )

    # Update META file
    JINJAENV.get_template(f"META.yml.j2").stream(**template_vars).dump(
        f"{dest_dir}/../META.yml"
    )

    if impl not in ("avx",):
        JINJAENV.get_template(f"{dst}/Makefile.Microsoft_nmake.j2").stream(
            **template_vars
        ).dump(f"{dest_dir}/Makefile.Microsoft_nmake")
