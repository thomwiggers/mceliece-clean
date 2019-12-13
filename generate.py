#!/usr/bin/python

import fileinput
import glob
import re
import shutil
import os
import contextlib


IMPLEMENTATIONS = {
    ('mceliece348864', 'ref', 'clean'),
}


def replace_in_file(path, text_to_search, replacement_text):
    with fileinput.FileInput(path, inplace=True) as file:
        for line in file:
            print(re.sub(text_to_search, replacement_text, line), end='')


for (scheme, impl, dst) in IMPLEMENTATIONS:
    namespace = f'PQCLEAN_{scheme}_{dst}_'.upper()
    src_dir = os.path.join('schemes', scheme, impl)
    dest_dir = os.path.join('crypto_kem', scheme, dst)

    with contextlib.suppress(IOError):
        shutil.rmtree(dest_dir)
    os.makedirs(dest_dir)

    for src_file in glob.glob(os.path.join(src_dir, '*')):
        filename = os.path.basename(src_file)
        dest_file = os.path.join(dest_dir, filename)
        shutil.copy(src_file, dest_dir)
        replace_in_file(dest_file, 'MC_', namespace)

    replace_in_file(os.path.join(dest_dir, 'Makefile'),
                    'libmceliece_clean.a',
                    f'lib{scheme}_{dst}.a')
