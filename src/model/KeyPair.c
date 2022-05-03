//
//  KeyPair.c
//  vexl-cryptography
//
//  Created by Adam Salih on 16.03.2022.
//

#include "KeyPair.h"

KeyPair _EVP_PKEY_get_KeyPair(const EVP_PKEY *pkey) {
    int tmp_len = 0;
    KeyPair keys;
    keys.pemPrivateKey = NULL;
    keys.pemPublicKey = NULL;

    BIO *privBIO = BIO_new(BIO_s_mem());
    if (!PEM_write_bio_PrivateKey(privBIO, pkey, NULL, NULL, 0, 0, NULL)) {
        _error(1, "Error writing private key data in PEM format");
    }
    char *pem_private_key = _BIO_read_chars(privBIO);
    BIO_free_all(privBIO);

    BIO *pubBIO = BIO_new(BIO_s_mem());
    if (!PEM_write_bio_PUBKEY(pubBIO, pkey)) {
        _error(2, "Error writing public key data in PEM format");
    }
    char *pem_public_key = _BIO_read_chars(pubBIO);
    BIO_free_all(pubBIO);

    char *base64_public_key;
    char *base64_private_key;

    base64_encode(pem_private_key, strlen(pem_private_key), &tmp_len, &base64_private_key);
    base64_encode(pem_public_key, strlen(pem_public_key), &tmp_len, &base64_public_key);

    keys.pemPublicKey = base64_public_key;
    keys.pemPrivateKey = base64_private_key;

    free(pem_private_key);
    free(pem_public_key);

    return keys;
}



void _base64_keys_get_EC_KEY(const char *base64_public_key, const char *base64_private_key, EC_KEY **eckey) {
    EVP_PKEY *pkey = NULL;

    char *public_key;
    int public_key_len;
    base64_decode(base64_public_key, strlen(base64_public_key), &public_key_len, &public_key);

    BIO *pub_bio = BIO_new_mem_buf((void*) public_key, public_key_len);
    PEM_read_bio_PUBKEY(pub_bio, &pkey, NULL, NULL);

    if (base64_private_key != NULL) {
        char *private_key;
        int private_key_len;
        base64_decode(base64_private_key, strlen(base64_private_key), &private_key_len, &private_key);
        BIO *priv_bio = BIO_new_mem_buf((void*) private_key, private_key_len);
        PEM_read_bio_PrivateKey(priv_bio, &pkey, NULL, NULL);
        BIO_free_all(priv_bio);
    }

    *eckey = EVP_PKEY_get1_EC_KEY(pkey);

    BIO_free_all(pub_bio);
    EVP_PKEY_free(pkey);
}

char *generate_key_public_key(const int curve) {
    KeyPair keys = generate_key_pair((Curve) curve);
    char *pub_key = keys.pemPublicKey;
    keys.pemPublicKey = NULL;
    KeyPair_free(keys);
    return pub_key;
}

KeyPair generate_key_pair(const Curve curve) {
    int eccgrp = OBJ_txt2nid(_get_group_name(curve));
    EC_KEY *myecc = EC_KEY_new_by_curve_name(eccgrp);

    EC_KEY_set_asn1_flag(myecc, OPENSSL_EC_NAMED_CURVE);

    if (!EC_KEY_generate_key(myecc)) {
        _error(3, "Error generating the ECC key.");
    }
    
    EVP_PKEY *pkey = EVP_PKEY_new();
    if (!EVP_PKEY_assign_EC_KEY(pkey, myecc)) {
        _error(4, "Error assigning ECC key to EVP_PKEY structure.");
    }

    myecc = EVP_PKEY_get1_EC_KEY(pkey);

    KeyPair keys = _EVP_PKEY_get_KeyPair(pkey);

    _log("Generated new keypair");
    _log("private key: \n%s", keys.pemPrivateKey);
    _log("public key: \n%s", keys.pemPublicKey);
    
    EVP_PKEY_free(pkey);
    EC_KEY_free(myecc);

    return keys;
}

void KeyPair_free(KeyPair keyPair) {
    if (keyPair.pemPrivateKey != NULL)
        free(keyPair.pemPrivateKey);
    if (keyPair.pemPublicKey)
        free(keyPair.pemPublicKey);
}