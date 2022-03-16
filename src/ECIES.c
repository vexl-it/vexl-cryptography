//
//  ECIES.c
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#include "ECIES.h"

void _log(char *message, ...) {
    va_list argptr;
    va_start(argptr, message);
    vfprintf(stderr, message, argptr);
    va_end(argptr);
    printf("\n");
}

void _error(int code, char *message, ...) {
    printf("\n");
    va_list argptr;
    va_start(argptr, message);
    vfprintf(stderr, message, argptr);
    va_end(argptr);
    printf("\n");
    exit(code);
}

char *_get_group_name(Curve curve) {
    switch (curve)
    {
    case secp224r1:
        return "secp224r1";
    case secp384r1:
        return "secp384r1";
    case secp521r1:
        return "secp521r1";
    default:
        return NULL;
    }
}

char *_BIO_read_chars(BIO *bio) {
	int keySize = 0;
    
	char *content = malloc(1);

	int readSize = 1;
	char buf[64];

	while (readSize != 0) {
		readSize = BIO_gets(bio, buf, sizeof(buf));
		content = (char *)realloc(content, keySize + readSize);
		for (int i = keySize; i < keySize + readSize; i++) {
			content[i] = buf[i - keySize];
		}
		keySize += readSize;
	}

	return content;
}

KeyPair _EVP_PKEY_get_KeyPair(const EVP_PKEY *pkey) {
	KeyPair keys;

    BIO *privBIO = BIO_new(BIO_s_mem());
    if (!PEM_write_bio_PrivateKey(privBIO, pkey, NULL, NULL, 0, 0, NULL)) {
        _error(1, "Error writing private key data in PEM format");
	}
	keys.pemPrivateKey = _BIO_read_chars(privBIO);
    BIO_free_all(privBIO);

	BIO *pubBIO = BIO_new(BIO_s_mem());
	if (!PEM_write_bio_PUBKEY(pubBIO, pkey)) {
        _error(2, "Error writing public key data in PEM format");
	}
	keys.pemPublicKey = _BIO_read_chars(pubBIO);
	BIO_free_all(pubBIO);

    _log("Generated new keypair");
    _log("private key: \n%s", keys.pemPrivateKey);
    _log("public key: \n%s", keys.pemPublicKey);

	return keys;
}

void KeyPair_free(KeyPair KeyPair) {
	free(KeyPair.pemPrivateKey);
	free(KeyPair.pemPublicKey);
}

KeyPair generate_key_pair(const Curve curve) {
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    
    int eccgrp = OBJ_txt2nid(_get_group_name(curve));
    EC_KEY *myecc = EC_KEY_new_by_curve_name(eccgrp);

    EC_KEY_set_asn1_flag(myecc, OPENSSL_EC_NAMED_CURVE);
    
    if (!EC_KEY_generate_key(myecc)) {
        _error(3, "Error generating the ECC key.");
    }
    
    EVP_PKEY *pkey=EVP_PKEY_new();
    if (!EVP_PKEY_assign_EC_KEY(pkey,myecc)) {
        _error(4, "Error assigning ECC key to EVP_PKEY structure.");
    }
    
    myecc = EVP_PKEY_get1_EC_KEY(pkey);
    const EC_GROUP *ecgrp = EC_KEY_get0_group(myecc);

    KeyPair keys = _EVP_PKEY_get_KeyPair(pkey);
	
    EVP_PKEY_free(pkey);
    EC_KEY_free(myecc);

    return keys;
}

char *encrypt(KeyPair keys, char *message) {
    return message;
}

