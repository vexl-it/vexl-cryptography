//
// Created by Adam Salih on 29.03.2022.
//

#include "ECDSA.h"

unsigned char *ecdsa_sign(const KeyPair keys, const void *data, const int data_len) {
    const char *digest = sha256_hash(data, data_len);
    const EC_KEY *eckey = _KeyPair_get_EC_KEY(keys);

    ECDSA_SIG *signature = ECDSA_do_sign(digest, SHA256_DIGEST_LENGTH, eckey);
    if (NULL == signature) {
        printf("Failed to generate EC Signature\n");
        return NULL;
    }

    unsigned char *str_signature_tmp = NULL;
    int sig_len = i2d_ECDSA_SIG(signature, &str_signature_tmp);
    unsigned char *str_signature = malloc(sig_len);
    memcpy(str_signature, str_signature_tmp, sig_len);

    free(digest);
    EC_KEY_free(eckey);
    free(str_signature_tmp);
    ECDSA_SIG_free(signature);

    return str_signature;
}

bool ecdsa_verify(const KeyPair pubkey, const void *data, const int data_len, unsigned char *sig) {
    const char *digest = sha256_hash(data, data_len);
    const EC_KEY *eckey = _KeyPair_get_EC_KEY(pubkey);
    if (digest == NULL)
        return false;

    int sig_len = ECDSA_size(eckey);

    unsigned char *sig_cpy = malloc(sig_len);
    unsigned char *tmp_ptr = sig_cpy;
    memcpy(sig_cpy, sig, sig_len);
    ECDSA_SIG *decoded_signature = d2i_ECDSA_SIG(NULL, &sig_cpy, sig_len);

    int verify_status = ECDSA_do_verify(digest, SHA256_DIGEST_LENGTH, decoded_signature, eckey);
    const int verify_success = 1;

    free(digest);
    EC_KEY_free(eckey);
    free(tmp_ptr);
    ECDSA_SIG_free(decoded_signature);

    return verify_status == verify_success;
}
