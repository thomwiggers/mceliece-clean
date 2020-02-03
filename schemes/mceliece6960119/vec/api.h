#ifndef MC_API_H
#define MC_API_H

#include <stdint.h>

#define MC_CRYPTO_ALGNAME "Classic McEliece 6960119"
#define MC_CRYPTO_PUBLICKEYBYTES 1047319
#define MC_CRYPTO_SECRETKEYBYTES 13908
#define MC_CRYPTO_CIPHERTEXTBYTES 226
#define MC_CRYPTO_BYTES 32


int MC_crypto_kem_enc(
    uint8_t *c,
    uint8_t *key,
    const uint8_t *pk
);

int MC_crypto_kem_dec(
    uint8_t *key,
    const uint8_t *c,
    const uint8_t *sk
);

int MC_crypto_kem_keypair
(
    uint8_t *pk,
    uint8_t *sk
);

#endif

