//
// Created by Adam Salih on 29.03.2022.
//

#include "ECDSA.h"

char *ecdsa_sign(const KeyPair keys, const void *data, const int data_len) {
    const char *digest = sha256_hash(data, data_len);
    const EC_KEY *eckey = _KeyPair_get_EC_KEY(keys);

    ECDSA_SIG *signature = ECDSA_do_sign(digest, SHA256_DIGEST_LENGTH, eckey);
    if (NULL == signature) {
        printf("Failed to generate EC Signature\n");
        return NULL;
    }

    unsigned char *der_signature_tmp = NULL;
    int der_signature_len = i2d_ECDSA_SIG(signature, &der_signature_tmp);
    unsigned char *der_signature = malloc(der_signature_len);
    memcpy(der_signature, der_signature_tmp, der_signature_len);

    free(digest);
    EC_KEY_free(eckey);

    size_t tmp;
    char *base64_signature = NULL;
    base64_encode(der_signature, der_signature_len, &tmp, &base64_signature);

    free(der_signature);
    free(der_signature_tmp);
    ECDSA_SIG_free(signature);

    return base64_signature;
}

bool ecdsa_verify(const KeyPair pubkey, const void *data, const int data_len, char *base64_signature) {
    const char *digest = sha256_hash(data, data_len);
    const EC_KEY *eckey = _KeyPair_get_EC_KEY(pubkey);
    if (digest == NULL)
        return false;

    int signature_len = ECDSA_size(eckey) - 1;
    size_t base64_signature_len = base64_calculate_encoding_lenght(signature_len);
    size_t tmp;
    unsigned char *der_signature;
    base64_decode(base64_signature, base64_signature_len, &tmp, &der_signature);

    ECDSA_SIG *decoded_signature = d2i_ECDSA_SIG(NULL, &der_signature, signature_len);

    int verify_status = ECDSA_do_verify(digest, SHA256_DIGEST_LENGTH, decoded_signature, eckey);
    const int verify_success = 1;

    free(digest);
    EC_KEY_free(eckey);
    free(tmp_ptr);
    ECDSA_SIG_free(decoded_signature);

    return verify_status == verify_success;
}
