//
//  ECIES.c
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#include "ECIES.h"

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
	int bits,ret=-1;
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

void pbkdf2_encrypt(const unsigned char *password, const int password_len, const char *message, Cipher *cipher) {
	const EVP_MD *md = EVP_sha1();
	const EVP_CIPHER *evp_cipher = EVP_aes_256_cbc();
	size_t ke_len = EVP_CIPHER_key_length(evp_cipher) + EVP_CIPHER_iv_length(evp_cipher);
	size_t km_len = EVP_MD_block_size(md);
	unsigned char ke_km[ke_len+km_len];

	PKCS5_PBKDF2_HMAC((const char*)password, password_len, SALT, sizeof(SALT), PBKDF2ITER, md, ke_len+km_len, ke_km);

	EVP_CIPHER_CTX *ectx = EVP_CIPHER_CTX_new();
	cipher->cipherLen = 0;
	
    char *cipher_str = malloc(0);
    int cipher_str_len = 0;
    const int buff_size = 1024;
    const int m_size = strlen(message)*sizeof(unsigned char);
    int m_offset = 0;
    char m_buffer[buff_size];
    memset(m_buffer, 0, buff_size);
    int o_len;
    char o_buffer[buff_size + EVP_MAX_BLOCK_LENGTH];
    memset(o_buffer, 0, buff_size + EVP_MAX_BLOCK_LENGTH);

	EVP_EncryptInit_ex(ectx, evp_cipher, NULL, ke_km, ke_km + EVP_CIPHER_key_length(evp_cipher));

    do {
        int readlen = (m_offset + buff_size > m_size) ? m_size - m_offset : buff_size;
        readlen = (readlen < 0) ? 0 : readlen;

        memcpy(m_buffer, message+m_offset, readlen*sizeof(char));

        EVP_EncryptUpdate(ectx, o_buffer, &o_len, m_buffer, buff_size);

        cipher_str = (char *)realloc(cipher_str, cipher_str_len + o_len);
        memcpy(cipher_str+cipher_str_len, o_buffer, o_len*sizeof(char));
        cipher_str_len += o_len;
        m_offset += readlen;
    } while (m_offset < m_size);

    memset(o_buffer, 0, buff_size);
    EVP_EncryptFinal_ex(ectx, o_buffer, &o_len);

    cipher_str = (char *)realloc(cipher_str, cipher_str_len + o_len);
    memcpy(cipher_str+cipher_str_len, o_buffer, o_len*sizeof(char));

    cipher->cipher = cipher_str;
	cipher->cipherLen = cipher_str_len;

    unsigned char D[cipher->cipherLen];
	unsigned int D_len;

	/* calculate MAC */
	HMAC(md, ke_km + ke_len, km_len, cipher->cipher, cipher->cipherLen, D, &D_len);

	cipher->D_len = D_len;
    cipher->D = malloc(D_len);
    strcpy(cipher->D, D);
}

char *pbkdf2_decrypt(const unsigned char *password, const int password_len, Cipher *cipher) {
    const EVP_MD *md = EVP_sha1();
    const EVP_CIPHER *evp_cipher = EVP_aes_256_cbc();
    size_t ke_len = EVP_CIPHER_key_length(evp_cipher) + EVP_CIPHER_iv_length(evp_cipher);
    size_t km_len = EVP_MD_block_size(md);
    unsigned char ke_km[ke_len + km_len];

    unsigned char dc_out[cipher->cipherLen];
    memset(dc_out, 0, cipher->cipherLen);
    size_t dc_len = 0;
    int outl = 0;

    PKCS5_PBKDF2_HMAC((const char *) password, password_len, SALT, sizeof(SALT), PBKDF2ITER, md, ke_len + km_len, ke_km);

    unsigned char dv_out[km_len];
    unsigned int dv_len;
    HMAC(md, ke_km + ke_len, km_len, cipher->cipher, cipher->cipherLen, dv_out, &dv_len);

    if (cipher->D_len != dv_len || memcmp(dv_out, cipher->D, dv_len) != 0) {
        _error(6, "MAC verification failed\n");
        return NULL;
    }

    EVP_CIPHER_CTX *ectx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ectx, evp_cipher, NULL, ke_km, ke_km + EVP_CIPHER_key_length(evp_cipher));
    EVP_DecryptUpdate(ectx, dc_out, &outl, cipher->cipher, cipher->cipherLen);
    dc_len += outl;
    EVP_DecryptFinal_ex(ectx, dc_out, &outl);
    dc_len += outl;
    dc_out[dc_len] = 0;

    char *message = malloc(dc_len);
    memcpy(message, dc_out,dc_len);
    return message;
}

char *ecies_encrypt(KeyPair keys, const char *message) {
    Cipher *cipher = cipher_new();
    EC_KEY *key = _KeyPair_get_EC_KEY(keys);
    
    BIGNUM *R = BN_new();
	BIGNUM *S = BN_new();

    //	Generates shared secret and corresponding public key
	while(EC_KEY_public_derive_S(key, POINT_CONVERSION_COMPRESSED, S, R) != 0);

    cipher->R_len = BN_num_bytes(R);
    cipher->R = malloc(cipher->R_len);
    BN_bn2bin(R, cipher->R);

	size_t S_len = BN_num_bytes(S);
	unsigned char password[S_len];
	BN_bn2bin(S, password);
    
    pbkdf2_encrypt(password, S_len, message, cipher);

    char *encoded_cipher = cipher_encode(cipher);

    BN_free(R);
    BN_free(S);
    EC_KEY_free(key);
    cipher_free(cipher);

    return encoded_cipher;
}

char *ecies_decrypt(KeyPair keys, char *encoded_cipher) {
    Cipher *cipher = cipher_decode(encoded_cipher);

    EC_KEY *key = _KeyPair_get_EC_KEY(keys);
    BIGNUM *R = BN_bin2bn(cipher->R, cipher->R_len, BN_new());
    BIGNUM *S = BN_new();

    if (EC_KEY_private_derive_S(key, R, S) != 0) {
        _error(5, "Key derivation failed\n");
        return NULL;
    }

    size_t S_len = BN_num_bytes(S);
    unsigned char password[S_len];
    BN_bn2bin(S, password);

    char *encrypted = pbkdf2_decrypt(password, S_len, cipher);

    BN_free(R);
    BN_free(S);
    EC_KEY_free(key);
    cipher_free(cipher);

    return encrypted;
}

