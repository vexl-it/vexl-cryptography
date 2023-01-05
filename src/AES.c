//
// Created by Adam Salih on 04.04.2022.
//

#include "AES.h"

char *aes_encrypt(const char *password, const char *message) {
    char *cipher = NULL;
    int cipher_len = 0;

    char *tag = NULL;
    int tag_len = 0;

    _aes_encrypt(password, strlen(password), message, strlen(message), &cipher, &cipher_len, &tag, &tag_len);

    char *encoded = malloc(VERSION_PREFIX_LEN + cipher_len + 1 + tag_len + 1);
    memset(encoded, 0, cipher_len + 1);
    memcpy(encoded, VERSION_PREFIX, VERSION_PREFIX_LEN);
    memcpy(encoded + VERSION_PREFIX_LEN, cipher, cipher_len);
    memcpy(encoded + VERSION_PREFIX_LEN + cipher_len, ".", 1);
    memcpy(encoded + VERSION_PREFIX_LEN + cipher_len + 1, tag, tag_len);

    free(cipher);
    free(tag);

    return encoded;
}

char *aes_decrypt(const char *password, const char *cipher) {
    char *b64_cipher = NULL;
    char *b64_tag = NULL;

    const char s[2] = ".";
    char *token;

    // version
    token = strtok(cipher, ".");
    if(token == NULL) {
        free(b64_cipher);
        free(b64_tag);
        return NULL;
    }

    // cipher
    token = strtok(NULL, s);
    if(token == NULL) {
        free(b64_cipher);
        free(b64_tag);
        return NULL;
    }
    b64_cipher = malloc((char *) (strlen(token) + 1));
    strcpy(b64_cipher,token);

    // tag
    token = strtok(NULL, s);
    if(token == NULL){
        free(b64_cipher);
        free(b64_tag);
        return NULL;
    }
    b64_tag = malloc((char *) (strlen(token) + 1));
    strcpy(b64_tag,token);

    char *message = NULL;
    int message_len = 0;
    _aes_decrypt(password, strlen(password), b64_cipher, strlen(b64_cipher), b64_tag, strlen(b64_tag), &message, &message_len);

    free(b64_cipher);
    free(b64_tag);
    return message;
}

void _aes_encrypt(const char *password, const int password_len, const char *message, const int message_len, char **cipher, int *cipher_len, char **tag, int *tag_len) {
    if (password == NULL || cipher == NULL || cipher_len == NULL, message_len > MAX_DATA_SIZE_LIMIT) {
        return;
    }
    const EVP_MD *md = EVP_sha1();
    const EVP_CIPHER *evp_cipher = EVP_aes_256_gcm();
    size_t ke_len = EVP_CIPHER_key_length(evp_cipher) + EVP_CIPHER_iv_length(evp_cipher);
    size_t km_len = EVP_MD_block_size(md);
    unsigned char ke_km[ke_len+km_len];

    PKCS5_PBKDF2_HMAC(
        // pass
        password,
        // pass lem
        password_len,
        // salt
        SALT,
        // salt len
        SALT_LEN,
        // iterations
        PBKDF2ITER,
        // digest
        md,
        // keylen
        ke_len+km_len,
        // out
        ke_km);

    EVP_CIPHER_CTX *ectx = EVP_CIPHER_CTX_new();
    *cipher_len = 0;

    char *cipher_str = malloc(0);
    int cipher_str_len = 0;
    const int buff_size = 128;
    const int m_size = message_len;
    int m_offset = 0;
    char m_buffer[buff_size];
    memset(m_buffer, 0, buff_size);
    int o_len;
    char o_buffer[buff_size + EVP_MAX_BLOCK_LENGTH];
    memset(o_buffer, 0, buff_size + EVP_MAX_BLOCK_LENGTH);

    EVP_EncryptInit_ex(ectx, evp_cipher, NULL, ke_km, ke_km + EVP_CIPHER_key_length(evp_cipher));

    do {
        int read_len = (m_offset + buff_size > m_size) ? m_size - m_offset : buff_size;
        read_len = (read_len < 0) ? 0 : read_len;

        if (read_len < buff_size) {
            memset(m_buffer, 0, buff_size);
        }

        memcpy(m_buffer, message+m_offset, read_len*sizeof(char));

        EVP_EncryptUpdate(ectx, o_buffer, &o_len, m_buffer, buff_size);

        cipher_str = (char *)realloc(cipher_str, cipher_str_len + o_len);
        memcpy(cipher_str+cipher_str_len, o_buffer, o_len*sizeof(char));
        cipher_str_len += o_len;
        m_offset += read_len;
    } while (m_offset < m_size);

    memset(o_buffer, 0, buff_size);
    EVP_EncryptFinal_ex(ectx, cipher_str, &o_len);
    cipher_str_len += o_len;

    char *tag_buf[16];
    EVP_CIPHER_CTX_ctrl(ectx, EVP_CTRL_GCM_GET_TAG, 16, tag_buf);

    base64_encode(cipher_str, cipher_str_len, cipher_len, cipher);
    base64_encode(tag_buf, 16, tag_len, tag);

    EVP_CIPHER_CTX_free(ectx);
    EVP_CIPHER_free(evp_cipher);
    EVP_MD_free(md);
    free(cipher_str);
}


void _aes_decrypt(const char *password, const int password_len, const char *base64_cipher, const int base64_cipher_len, const char *base64_tag, const int base64_tag_len, char **message, int *message_len) {
    if (password == NULL || base64_cipher == NULL || base64_tag == NULL || message == NULL || message_len == NULL) {
        return;
    }
    if (base64_cipher_len == 0 || base64_cipher_len > MAX_CIPHER_SIZE_LIMIT) {
        *message = NULL;
        *message_len = 0;
        return;
    }

    char *cipher;
    int cipher_len;
    base64_decode(base64_cipher, base64_cipher_len, &cipher_len, &cipher);

    char *tag;
    int tag_len;
    base64_decode(base64_tag, base64_tag_len, &tag_len, &tag);

    const EVP_MD *md = EVP_sha1();
    const EVP_CIPHER *evp_cipher = EVP_aes_256_gcm();
    size_t ke_len = EVP_CIPHER_key_length(evp_cipher) + EVP_CIPHER_iv_length(evp_cipher);
    size_t km_len = EVP_MD_block_size(md);
    unsigned char ke_km[ke_len + km_len];


    unsigned char *dc_out = malloc(cipher_len+km_len);
    memset(dc_out, 0, cipher_len+km_len);
    size_t dc_len = 0;
    int outl = 0;

    PKCS5_PBKDF2_HMAC(
        // pass
        password,
        // pass lem
        password_len,
        // salt
        SALT,
        // salt len
        SALT_LEN,
        // iterations
        PBKDF2ITER,
        // digest
        md,
        // keylen
        ke_len+km_len,
        // out
        ke_km
    );

    EVP_CIPHER_CTX *ectx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(
        // ctx
        ectx,
        // type
        evp_cipher,
        // impl
        NULL,
        // key
        ke_km,
        // iv
        ke_km + EVP_CIPHER_key_length(evp_cipher)
    );

    EVP_DecryptUpdate(ectx, dc_out, &outl, cipher, cipher_len);
    dc_len += outl;

    EVP_CIPHER_CTX_ctrl(ectx, EVP_CTRL_GCM_SET_TAG, tag_len, tag);

    int rv = EVP_DecryptFinal_ex(ectx, dc_out, &outl);

    dc_len += outl;
    dc_out[dc_len] = 0;

    // If tag is valid
    if(rv > 0){
        *message = malloc(dc_len+1);
        memcpy(*message, dc_out, dc_len);
        (*message)[dc_len] = 0;
        *message_len = dc_len;
    } else {
        _error(5, "Invalid security tag");
        *message = NULL;
        *message_len = 0;
        return;
    }

    EVP_CIPHER_CTX_free(ectx);
    EVP_CIPHER_free(evp_cipher);
    EVP_MD_free(md);
    free(dc_out);
}
