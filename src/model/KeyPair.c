//
//  KeyPair.c
//  vexl-cryptography
//
//  Created by Adam Salih on 16.03.2022.
//

#include "KeyPair.h"

KeyPair _EVP_PKEY_get_KeyPair(const EVP_PKEY *pkey) {
    KeyPair keys;
    keys.pemPrivateKey = NULL;
    keys.pemPublicKey = NULL;

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

    return keys;
}

EC_KEY *_KeyPair_get_EC_KEY(const KeyPair keys) {
    EVP_PKEY *pkey = NULL;
    EC_KEY *eckey = NULL;

    BIO *pub_bio = BIO_new_mem_buf((void*)keys.pemPublicKey, strlen(keys.pemPublicKey));
    PEM_read_bio_PUBKEY(pub_bio, &pkey, NULL, NULL);

    if (keys.pemPrivateKey != NULL) {
        BIO *priv_bio = BIO_new_mem_buf((void*)keys.pemPrivateKey, strlen(keys.pemPrivateKey));
        PEM_read_bio_PrivateKey(priv_bio, &pkey, NULL, NULL);
        BIO_free_all(priv_bio);
    }

    eckey = EVP_PKEY_get1_EC_KEY(pkey);
    
    BIO_free_all(pub_bio);
    EVP_PKEY_free(pkey);

    return eckey;
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

    _log("Generated new keypair");
    _log("private key: \n%s", keys.pemPrivateKey);
    _log("public key: \n%s", keys.pemPublicKey);
    
    EVP_PKEY_free(pkey);
    EC_KEY_free(myecc);

    return keys;
}

void KeyPair_free(KeyPair KeyPair) {
    free(KeyPair.pemPrivateKey);
    free(KeyPair.pemPublicKey);
}