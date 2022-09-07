//
// Created by Adam Salih on 04.04.2022.
//

#include "HMAC.h"

char *hmac_digest(const char *password, const char *message) {
    char *digest = NULL;
    int digest_len = 0;
    _hmac_digest(password, strlen(password), message, strlen(message), &digest, &digest_len);
    return digest;
}

bool hmac_verify(const char *password, const char *message, const char *digest) {
    return _hmac_verify(password, strlen(password), message, strlen(message), digest, strlen(digest));
}

void _hmac_digest(const char *password, const int password_len, const char *message, const int message_len, char **mac, int *mac_len) {
    if (password == NULL || message == NULL || mac == NULL || mac_len == NULL) {
        return;
    }
    const EVP_MD *md = EVP_sha256();
    const EVP_CIPHER *evp_cipher = EVP_aes_256_gcm();
    size_t ke_len = EVP_CIPHER_key_length(evp_cipher) + EVP_CIPHER_iv_length(evp_cipher);
    size_t km_len = EVP_MD_block_size(md);
    unsigned char ke_km[ke_len+km_len];
    unsigned char tmp_mac[message_len];
    int tmp_mac_len;

    PKCS5_PBKDF2_HMAC(password, password_len, SALT, SALT_LEN, PBKDF2ITER, md, ke_len+km_len, ke_km);
    HMAC(md, ke_km + ke_len, km_len, message, message_len, tmp_mac, &tmp_mac_len);

    EVP_CIPHER_free(evp_cipher);
    EVP_MD_free(md);

    base64_encode(tmp_mac, tmp_mac_len, mac_len, mac);
}

bool _hmac_verify(const char *password, const int password_len, const char *message, const int message_len, const char *base64_mac, const int base64_mac_len) {
    if (password == NULL || message == NULL || base64_mac == NULL) {
        return false;
    }
    unsigned char *mac = NULL;
    int mac_len = 0;
    base64_decode(base64_mac, base64_mac_len, &mac_len, &mac);

    const EVP_MD *md = EVP_sha256();
    const EVP_CIPHER *evp_cipher = EVP_aes_256_gcm();
    size_t ke_len = EVP_CIPHER_key_length(evp_cipher) + EVP_CIPHER_iv_length(evp_cipher);
    size_t km_len = EVP_MD_block_size(md);
    unsigned char ke_km[ke_len + km_len];

    unsigned char dv_out[EVP_MAX_MD_SIZE];
    memset(dv_out, 0, EVP_MAX_MD_SIZE);
    unsigned int dv_len;

    PKCS5_PBKDF2_HMAC((const char *) password, password_len, SALT, SALT_LEN, PBKDF2ITER, md, ke_len + km_len, ke_km);

    HMAC(md, ke_km + ke_len, km_len, message, message_len, dv_out, &dv_len);

    bool verified = mac_len == dv_len;
    if (verified)
        verified = verified && memcmp(dv_out, mac, dv_len) == 0;

    EVP_CIPHER_free(evp_cipher);
    EVP_MD_free(md);

    return verified;
}
