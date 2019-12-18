from collections import defaultdict
import os
import json
import sys
import yaml


SPECIAL_FILES = ["Makefile", "Makefile.Microsoft_nmake", "LICENSE"]


def path_to_impl(path):
    (implpath, filename) = os.path.split(path)
    (z, impl) = os.path.split(implpath)
    scheme = os.path.basename(z)

    return (scheme, impl, filename)


def load_duplicates():
    inodes = defaultdict(list)
    with open("rmlint.json") as f:
        rmlint = json.load(f)

    for item in rmlint:
        if "type" not in item:
            continue
        assert item["type"] == "duplicate_file"
        inodes[item["inode"]].append(path_to_impl(item["path"]))

    return inodes


def get_scheme_files(scheme, impl, dest_impl):
    scheme_path = os.path.join("crypto_kem", scheme, dest_impl)
    files = filter(lambda x: x not in SPECIAL_FILES, os.listdir(scheme_path))
    return list(files)


def duplicates_of_file(inodes, filename, scheme, src_impl):
    src_path = os.path.join("schemes", scheme, src_impl, filename)
    inode = os.stat(src_path).st_ino
    for (dup_scheme, impl, dup_filename) in inodes[inode]:
        if (
            dup_scheme == scheme and src_impl == impl
        ) or dup_filename != filename:
            continue
        yield (dup_scheme, impl)


def render_duplicates_file(duplicates):
    items = []
    for scheme, details in duplicates.items():
        for implementation, files in details.items():
            items.append(
                {
                    "source": {
                        "scheme": scheme,
                        "implementation": implementation,
                    },
                    "files": files,
                }
            )
    return yaml.dump({"consistency_checks": items}, indent=2)


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <scheme> <impl> <dest_impl>")
        sys.exit(1)
    scheme = sys.argv[1]
    impl = sys.argv[2]
    dest_impl = sys.argv[3]

    inodes = load_duplicates()
    scheme_files = get_scheme_files(scheme, impl, dest_impl)
    duplicates = defaultdict(lambda: defaultdict(list))
    for filename in scheme_files:
        dups = duplicates_of_file(inodes, filename, scheme, impl)
        for (dup_scheme, dup_impl) in dups:
            if dup_impl == "ref":
                dup_impl = "clean"
            if dup_impl != "clean":
                continue
            duplicates[dup_scheme][dup_impl].append(filename)

    with open(
        os.path.join(
            "test", "duplicate_consistency", f"{ scheme }_{ dest_impl }.yml"
        ),
        "w",
    ) as f:
        f.write(render_duplicates_file(duplicates))
