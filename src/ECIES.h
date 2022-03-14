//
//  ECIES.h
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#ifndef ECIES_h
#define ECIES_h

#include <stdio.h>
#include <stdio.h>
#include <string.h>

#ifdef BUILD_FOR_LIBRARY

#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/hmac.h>

void RAND_init(void);
#endif

EC_POINT *EC_POINT_mult_BN(const EC_GROUP *group, EC_POINT *P, const EC_POINT *a, const BIGNUM *b, BN_CTX *ctx);

int EC_KEY_public_derive_S(const EC_KEY *key, point_conversion_form_t fmt, BIGNUM *S, BIGNUM *R);

int EC_KEY_private_derive_S(const EC_KEY *key, const BIGNUM *R, BIGNUM *S);

int decipher(const EC_KEY *key,
	const unsigned char *R_in, size_t R_len, const unsigned char *c_in, size_t c_len, 
	const unsigned char *d_in, size_t d_len, const unsigned char *salt, size_t salt_len);

int encipher(const EC_KEY *key,
	unsigned char *R_out, size_t *R_len, unsigned char *c_out, size_t *c_len,
	unsigned char *d_out, size_t *d_len, const unsigned char *salt, size_t salt_len);

#endif /* ECIES_h */
