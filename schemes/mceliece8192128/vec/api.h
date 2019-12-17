#ifndef MC_API_H
#define MC_API_H

#define MC_CRYPTO_ALGNAME "Classic McEliece 8192128"
#define MC_CRYPTO_PUBLICKEYBYTES 1357824
#define MC_CRYPTO_SECRETKEYBYTES 14080
#define MC_CRYPTO_CIPHERTEXTBYTES 240
#define MC_CRYPTO_BYTES 32


int MC_crypto_kem_enc(
    unsigned char *c,
    unsigned char *key,
    const unsigned char *pk
);

int MC_crypto_kem_dec(
    unsigned char *key,
    const unsigned char *c,
    const unsigned char *sk
);

int MC_crypto_kem_keypair
(
    unsigned char *pk,
    unsigned char *sk
);

#endif

