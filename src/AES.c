//
// Created by Adam Salih on 04.04.2022.
//

#include "AES.h"

char *aes_encrypt(const char *password, const char *message) {
    char *cipher = NULL;
    int cipher_len = 0;
    _aes_encrypt(password, strlen(password), message, strlen(message), &cipher, &cipher_len);
    return cipher;
}

char *aes_decrypt(const char *password, const char *cipher) {
    char *message = NULL;
    int message_len = 0;
    _aes_decrypt(password, strlen(password), cipher, strlen(cipher), &message, &message_len);
    return message;
}

void _aes_encrypt(const char *password, const int password_len, const char *message, const int message_len, char **cipher, int *cipher_len) {
    const EVP_MD *md = EVP_sha1();
    const EVP_CIPHER *evp_cipher = EVP_aes_256_cbc();
    size_t ke_len = EVP_CIPHER_key_length(evp_cipher) + EVP_CIPHER_iv_length(evp_cipher);
    size_t km_len = EVP_MD_block_size(md);
    unsigned char ke_km[ke_len+km_len];

    PKCS5_PBKDF2_HMAC(password, password_len, SALT, sizeof(SALT), PBKDF2ITER, md, ke_len+km_len, ke_km);

    EVP_CIPHER_CTX *ectx = EVP_CIPHER_CTX_new();
    *cipher_len = 0;

    char *cipher_str = malloc(0);
    int cipher_str_len = 0;
    const int buff_size = 1024;
    const int m_size = message_len;
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

    base64_encode(cipher_str, cipher_str_len, cipher_len, cipher);

    EVP_CIPHER_CTX_free(ectx);
    EVP_CIPHER_free(evp_cipher);
    EVP_MD_free(md);
    free(cipher_str);
}

void _aes_decrypt(const char *password, const int password_len, const char *base64_cipher, const int base64_cipher_len, char **message, int *message_len) {
    char *cipher;
    int cipher_len;
    base64_decode(base64_cipher, base64_cipher_len, &cipher_len, &cipher);


    const EVP_MD *md = EVP_sha1();
    const EVP_CIPHER *evp_cipher = EVP_aes_256_cbc();
    size_t ke_len = EVP_CIPHER_key_length(evp_cipher) + EVP_CIPHER_iv_length(evp_cipher);
    size_t km_len = EVP_MD_block_size(md);
    unsigned char ke_km[ke_len + km_len];


    unsigned char dc_out[cipher_len+km_len];
    memset(dc_out, 0, cipher_len+km_len);
    size_t dc_len = 0;
    int outl = 0;

    PKCS5_PBKDF2_HMAC(password, password_len, SALT, sizeof(SALT), PBKDF2ITER, md, ke_len + km_len, ke_km);

    EVP_CIPHER_CTX *ectx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ectx, evp_cipher, NULL, ke_km, ke_km + EVP_CIPHER_key_length(evp_cipher));
    EVP_DecryptUpdate(ectx, dc_out, &outl, cipher, cipher_len);
    dc_len += outl;
    EVP_DecryptFinal_ex(ectx, dc_out, &outl);
    dc_len += outl;
    dc_out[dc_len] = 0;

    *message = malloc(dc_len);
    memcpy(*message, dc_out, dc_len);
    *message_len = dc_len;
    
    EVP_CIPHER_CTX_free(ectx);
    EVP_CIPHER_free(evp_cipher);
    EVP_MD_free(md);
}
