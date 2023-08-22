#!/bin/sh

set -e

wget https://csrc.nist.gov/csrc/media/Projects/post-quantum-cryptography/documents/round-4/submissions/mceliece-kat-Round4.tar.gz
mkdir KAT
tar -vxz --strip-components=3 -f mceliece-kat-Round4.tar.gz -C KAT
rm mceliece-kat-Round4.tar.gz
