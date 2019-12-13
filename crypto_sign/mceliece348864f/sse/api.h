#ifndef API_H
#define API_H

#define CRYPTO_PUBLICKEYBYTES 261120
#define CRYPTO_SECRETKEYBYTES 6452
#define CRYPTO_CIPHERTEXTBYTES 128
#define CRYPTO_BYTES 32



int crypto_kem_enc(
       unsigned char *c,
       unsigned char *key,
       const unsigned char *pk
);

int crypto_kem_dec(
       unsigned char *key,
       const unsigned char *c,
       const unsigned char *sk
);

int crypto_kem_keypair
(
       unsigned char *pk,
       unsigned char *sk 
);

#endif

