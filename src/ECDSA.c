//
// Created by Adam Salih on 29.03.2022.
//

#include "ECDSA.h"

char *ecdsa_sign_v2(const char* base64_public_key, const char* base64_private_key, const void *data, const int data_len) {
    if (base64_public_key == NULL || base64_private_key == NULL || data == NULL) {
        return NULL;
    }

    const char *digest = sha256_hash(data, data_len);

    // TODO how to move hash generation into function?
    SHA256_CTX sha256;
    char hash[SHA256_DIGEST_LENGTH];
    if(!SHA256_Init(&sha256)) {
        return false;
    }
    if(!SHA256_Update(&sha256, data, data_len)) {
        return false;
    }
    if(!SHA256_Final(hash, &sha256)) {
        return false;
    }

    EC_KEY *eckey;
    _base64_keys_get_EC_KEY(base64_public_key, base64_private_key, &eckey);

    ECDSA_SIG *signature = ECDSA_do_sign(hash, 32, eckey);
    if (NULL == signature) {
        _error(7, "Failed to generate EC Signature\n");
        return NULL;
    }

    int der_signature_len = i2d_ECDSA_SIG(signature, NULL);
    uint8_t* derSig = (uint8_t*)malloc(der_signature_len);
    uint8_t* p = derSig;    //memset(sig_bytes, 6, sig_size);
    der_signature_len = i2d_ECDSA_SIG(signature, &p);


    size_t tmp;
    char *base64_signature = NULL;
    base64_encode(derSig, der_signature_len, &tmp, &base64_signature);

    free(digest);
    EC_KEY_free(eckey);
    ECDSA_SIG_free(signature);
    free(derSig);

    return base64_signature;
}

bool ecdsa_verify_v2(const char* base64_public_key, const char *data, const int data_len, const char *base64_signature) {
    if (base64_public_key == NULL || data == NULL || base64_signature == NULL) {
        return false;
    }

    // TODO how to move hash generation into function?
    SHA256_CTX sha256;
    char hash[SHA256_DIGEST_LENGTH];
    if(!SHA256_Init(&sha256)) {
        return false;
    }
    if(!SHA256_Update(&sha256, data, data_len)) {
        return false;
    }
    if(!SHA256_Final(hash, &sha256)) {
        return false;
    }

    EC_KEY *eckey;
    _base64_keys_get_EC_KEY(base64_public_key, NULL, &eckey);

    int signature_len = ECDSA_size(eckey);
    size_t base64_signature_len = base64_calculate_encoding_lenght(signature_len);
    size_t tmp;

    unsigned char *der_signature;
    base64_decode(base64_signature, base64_signature_len, &tmp, &der_signature);

    ECDSA_SIG *decoded_signature = d2i_ECDSA_SIG(NULL, &der_signature, signature_len);

    if (decoded_signature == NULL) {
        return false;
    }

    int verify_status = ECDSA_do_verify(hash, SHA256_DIGEST_LENGTH, decoded_signature, eckey);
    const int verify_success = 1;

    EC_KEY_free(eckey);
    ECDSA_SIG_free(decoded_signature);

    return verify_status == verify_success;
}


char *ecdsa_sign(const char* base64_public_key, const char* base64_private_key, const void *data, const int data_len) {
    if (base64_public_key == NULL || base64_private_key == NULL || data == NULL) {
        return NULL;
    }
    const char *digest = sha256_hash(data, data_len);
    EC_KEY *eckey;
    _base64_keys_get_EC_KEY(base64_public_key, base64_private_key, &eckey);

    ECDSA_SIG *signature = ECDSA_do_sign(digest, strlen(digest), eckey);
    if (NULL == signature) {
        _error(7, "Failed to generate EC Signature\n");
        return NULL;
    }

    unsigned char *der_signature = NULL;
    int der_signature_len = ECDSA_size(eckey);
    i2d_ECDSA_SIG(signature, &der_signature);

    size_t tmp;
    char *base64_signature = NULL;
    base64_encode(der_signature, der_signature_len, &tmp, &base64_signature);

    free(digest);
    EC_KEY_free(eckey);
    free(der_signature);
    ECDSA_SIG_free(signature);

    return base64_signature;
}

bool ecdsa_verify(const char* base64_public_key, const char *data, const int data_len, const char *base64_signature) {
    if (base64_public_key == NULL || data == NULL || base64_signature == NULL) {
        return false;
    }
    const char *digest = sha256_hash(data, data_len);

    EC_KEY *eckey;
    _base64_keys_get_EC_KEY(base64_public_key, NULL, &eckey);
    if (digest == NULL)
        return false;

    int signature_len = ECDSA_size(eckey);
    size_t base64_signature_len = base64_calculate_encoding_lenght(signature_len);
    size_t tmp;
    unsigned char *der_signature;
    base64_decode(base64_signature, base64_signature_len, &tmp, &der_signature);

    ECDSA_SIG *decoded_signature = d2i_ECDSA_SIG(NULL, &der_signature, signature_len);

    if (decoded_signature == NULL)
        return false;

    int verify_status = ECDSA_do_verify(digest, strlen(digest), decoded_signature, eckey);
    const int verify_success = 1;

    free(digest);
    EC_KEY_free(eckey);
    ECDSA_SIG_free(decoded_signature);

    return verify_status == verify_success;
}
