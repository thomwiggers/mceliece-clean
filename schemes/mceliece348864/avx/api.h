#ifndef MC_API_H
#define MC_API_H

#define MC_CRYPTO_ALGNAME "Classic McEliece 348856f"
#define MC_CRYPTO_PUBLICKEYBYTES 261120
#define MC_CRYPTO_SECRETKEYBYTES 6452
#define MC_CRYPTO_CIPHERTEXTBYTES 128
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

