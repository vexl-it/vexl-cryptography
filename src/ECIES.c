//
//  ECIES.c
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#include "ECIES.h"

void ecies_init() {
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    base64_build_decoding_table();
}

EC_POINT *EC_POINT_mult_BN(const EC_GROUP *group, EC_POINT *P, const EC_POINT *a, const BIGNUM *b, BN_CTX *ctx) {
	EC_POINT *O = EC_POINT_new(group);
	if (P == NULL) P = EC_POINT_new(group);

	for(int i = BN_num_bits(b); i >= 0; i--) {
		EC_POINT_dbl(group, P, P, ctx);
		if (BN_is_bit_set(b, i))
			EC_POINT_add(group, P, P, a, ctx);
		else
			EC_POINT_add(group, P, P, O, ctx);
	}

    free(O);

	return P;
}

int EC_KEY_public_derive_S(const EC_KEY *key, point_conversion_form_t fmt, BIGNUM *S, BIGNUM *R) {
	BN_CTX *ctx = BN_CTX_new();
	const EC_GROUP *group = EC_KEY_get0_group(key);
	const EC_POINT *Kb = EC_KEY_get0_public_key(key);
	BIGNUM *n = BN_new();
	BIGNUM *r = BN_new();
	EC_POINT *P = NULL;
	EC_POINT *Rp = EC_POINT_new(group);
	BIGNUM *Py = BN_new();
	const EC_POINT *G = EC_GROUP_get0_generator(group);
	int bits, ret=-1;
	EC_GROUP_get_order(group, n, ctx);
	bits = BN_num_bits(n);
	BN_rand(r, bits, -1, 0);
	/* calculate R = rG */
	Rp = EC_POINT_mult_BN(group, Rp, G, r, ctx);
	/* calculate S = Px, P = (Px,Py) = Kb R */
	P = EC_POINT_mult_BN(group, P, Kb, r, ctx);
	if (!EC_POINT_is_at_infinity(group, P)) {
		EC_POINT_get_affine_coordinates_GF2m(group, P, S, Py, ctx);
		EC_POINT_point2bn(group, Rp, fmt, R, ctx);
		ret = 0;
	}
	BN_free(r);
	BN_free(n);
	BN_free(Py);
	EC_POINT_free(P);
	EC_POINT_free(Rp);
	BN_CTX_free(ctx);
	return ret;
}

int EC_KEY_private_derive_S(const EC_KEY *key, const BIGNUM *R, BIGNUM *S) {
	int ret = -1;
	BN_CTX *ctx = BN_CTX_new();
	BIGNUM *n = BN_new();
	BIGNUM *Py = BN_new();
	const EC_GROUP *group = EC_KEY_get0_group(key);
	EC_POINT *Rp = EC_POINT_bn2point(group, R, NULL, ctx);
	const BIGNUM *kB = EC_KEY_get0_private_key(key);
	EC_GROUP_get_order(group, n, ctx);
	/* Calculate S = Px, P = (Px, Py) = R kB */
	EC_POINT *P = EC_POINT_mult_BN(group, NULL, Rp, kB, ctx);
	if (!EC_POINT_is_at_infinity(group, P)) {
		EC_POINT_get_affine_coordinates_GF2m(group, P, S, Py, ctx);
		ret = 0;
	}
	BN_free(n);
    BN_free(Py);
	EC_POINT_free(Rp);
	EC_POINT_free(P);
	BN_CTX_free(ctx);
	return ret;
}

char *ecies_encrypt(const char *base64_public_key, const char *message) {
    char *cipher = NULL;
    _ecies_encrypt(base64_public_key, message, strlen(message), &cipher);
    return cipher;
}

char *ecies_decrypt(const char *base64_public_key, const char *base64_private_key, const char *encoded_cipher) {
    char *message = NULL;
    int message_len = 0;
    _ecies_decrypt(base64_public_key, base64_private_key, encoded_cipher, &message, &message_len);
    return message;
}

void _ecies_encrypt(const char *base64_public_key, const char *message, const int message_len, char **encoded_cipher) {
	if (base64_public_key == NULL || message == NULL || encoded_cipher == NULL || message_len > MAX_DATA_SIZE_LIMIT) {
        *encoded_cipher = NULL;
        return;
    }
    Cipher *cipher = cipher_new();
    EC_KEY *key;
    _base64_keys_get_EC_KEY(base64_public_key, NULL, &key);

    BIGNUM *R = BN_new();
    BIGNUM *S = BN_new();

	while(EC_KEY_public_derive_S(key, POINT_CONVERSION_COMPRESSED, S, R) != 0);

    int pk_len = BN_num_bytes(R);
    char pk[pk_len];
    BN_bn2bin(R, pk);

    base64_encode(pk, pk_len, &(cipher->public_key_len), &(cipher->public_key));

	size_t S_len = BN_num_bytes(S);
	unsigned char shared_secret[S_len];
	BN_bn2bin(S, shared_secret);

    // TODO fix and uncomment
    _aes_encrypt(shared_secret, S_len, message, message_len, &(cipher->cipher), &(cipher->cipher_len), &(cipher->tag), &(cipher->tag_len));
    _hmac_digest(shared_secret, S_len, cipher->cipher, cipher->cipher_len, &(cipher->mac), &(cipher->mac_len));

    *encoded_cipher = cipher_encode(cipher);

    BN_free(R);
    BN_free(S);
    EC_KEY_free(key);
    cipher_free(cipher);
}

void _ecies_decrypt(const char *base64_public_key, const char *base64_private_key, const char *encoded_cipher, char **message, int *message_len) {
	if (base64_public_key == NULL || base64_private_key == NULL || encoded_cipher == NULL, message == NULL || message_len == NULL) {
        return;
    }
    int encoded_cipher_len = strlen(encoded_cipher);
    if (encoded_cipher_len == 0 || encoded_cipher_len > MAX_CIPHER_SIZE_LIMIT) {
        *message = NULL;
        *message_len = 0;
        return;
    }

    Cipher *cipher = cipher_decode(encoded_cipher);

    unsigned char *pub_key;
    int pub_key_len;
    base64_decode(cipher->public_key, cipher->public_key_len, &pub_key_len, &pub_key);

    EC_KEY *key;
    _base64_keys_get_EC_KEY(base64_public_key, base64_private_key, &key);
    BIGNUM *R = BN_bin2bn(pub_key, pub_key_len, BN_new());
    BIGNUM *S = BN_new();

    if (EC_KEY_private_derive_S(key, R, S) != 0) {
        _error(5, "Key derivation failed\n");
        *message = NULL;
        *message_len = 0;
        return;
    }

    size_t S_len = BN_num_bytes(S);
    unsigned char shared_secret[S_len];
    BN_bn2bin(S, shared_secret);

    if (!_hmac_verify(shared_secret, S_len, cipher->cipher, cipher->cipher_len, cipher->mac, cipher->mac_len)) {
        _error(5, "MAC verification failed\n");
        *message = NULL;
        *message_len = 0;
        return;
    }

    char *decrypted;
    int decrypted_len;

    _aes_decrypt(shared_secret, S_len, cipher->cipher, cipher->cipher_len, cipher->tag, cipher->tag_len, &decrypted, &decrypted_len);

    *message = decrypted;
    *message_len = decrypted_len;

    BN_free(R);
    BN_free(S);
    EC_KEY_free(key);
    cipher_free(cipher);
}

