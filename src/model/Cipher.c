//
// Created by Adam Salih on 20.03.2022.
//

#include "Cipher.h"

static const int decimal_base = 10;
static const char decimals[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
static int isSetup = 0;

Cipher *cipher_new() {
    Cipher *cipher = malloc(sizeof(Cipher));
    cipher->cipher = NULL;
    cipher->cipher_len = 0;
    cipher->mac = NULL;
    cipher->mac_len = 0;
    cipher->public_key = NULL;
    cipher->public_key_len = 0;
    return cipher;
}

void cipher_free(Cipher *cipher) {
    if (cipher->cipher != NULL) {
        free(cipher->cipher);
    }
    if (cipher->mac != NULL) {
        free(cipher->mac);
    }
    if (cipher->public_key != NULL) {
        free(cipher->public_key);
    }
    free(cipher);
}

char *_cipher_encode_len(const int len, int *out_len_len) {
    if (len == 0) {
        *out_len_len = 0;
        char *digits = malloc(1);
        digits[0] = '0';
        return digits;
    }
    int tmp = len;
    int len_len = 0;
    int encode_len = 0;
    while (tmp > 0) {
        tmp = tmp / decimal_base;
        encode_len++;
    }
    tmp = len;
    char *digest = malloc(encode_len);
    while(tmp > 0) {
        digest[encode_len - (len_len * sizeof(char)) - 1] = decimals[tmp % decimal_base];
        tmp = tmp / decimal_base;
        len_len += 1;
    }
    *out_len_len = len_len;
    return digest;
}

char *cipher_encode(Cipher *cipher) {
    int content_lens[3] = { cipher->cipher_len, cipher->mac_len, cipher->public_key_len };
    char *content[] = { cipher->cipher, cipher->mac, cipher->public_key };

    int spacer_count = 0;
    int lens_lens[3] = { 0, 0, 0 };
    char *lens[3];
    int digest_size = 0;
    for (int i = 0; i < 3; i++) {
        lens[i] = _cipher_encode_len(content_lens[i], &lens_lens[i]);
        digest_size += content_lens[i] + lens_lens[i];
        spacer_count += (content[i] == NULL) ? 0 : 1;
    }

    char *digest = malloc(digest_size + spacer_count + 1);
    int offset = 0;
    for (int i = 0; i < 3; i++) {
        if (content[i] == NULL) continue;
        memcpy(&digest[offset], lens[i], lens_lens[i]);
        offset += lens_lens[i];
        digest[offset] = 'A';
        offset += 1;
        memcpy(&digest[offset], content[i], content_lens[i]);
        offset += content_lens[i];
    }
    digest[offset] = 0;

    return digest;
}

void _get_part_size(const char *digest, const int offset, int *size_len, int *base_len) {
    *size_len = 0;
    *base_len = 0;
    while (digest[offset + *size_len] != 'A')
        *size_len += 1;

    char num_char[*size_len+1];
    memcpy(num_char, digest + offset, *size_len*sizeof(char));
    num_char[*size_len] = 0;
    *base_len = atoi(num_char);
}

Cipher *cipher_decode(char *digest) {
    Cipher *cipher = cipher_new();
    int offset = 0, sizeLen = 0, baseLen = 0,  part_offset = 0;

    _get_part_size(digest, offset, &sizeLen, &baseLen);
    part_offset = offset+sizeLen+1;
    cipher->cipher = malloc(baseLen);
    cipher->cipher_len = baseLen;
    memcpy(cipher->cipher, digest+part_offset, baseLen);
    offset = part_offset+baseLen;

    _get_part_size(digest, offset, &sizeLen, &baseLen);
    part_offset = offset+sizeLen+1;
    cipher->mac = malloc(baseLen);
    cipher->mac_len = baseLen;
    memcpy(cipher->mac, digest+part_offset, baseLen);
    offset = part_offset+baseLen;

    _get_part_size(digest, offset, &sizeLen, &baseLen);
    part_offset = offset+sizeLen+1;
    cipher->public_key = malloc(baseLen);
    cipher->public_key_len = baseLen;
    memcpy(cipher->public_key, digest+part_offset, baseLen);

    return cipher;
}
