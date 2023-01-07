//
// Created by Adam Salih on 20.03.2022.
//

#include "Cipher.h"

Cipher *cipher_new() {
    Cipher *cipher = malloc(sizeof(Cipher));
    memset(cipher, 0, sizeof(Cipher));

    return cipher;
}

void cipher_free(Cipher *cipher) {
    if(cipher == NULL) return;
    free(cipher->cipher);
    free(cipher->mac);
    free(cipher->public_key);
    free(cipher->tag);

    memset(cipher, 0, sizeof(Cipher));
    free(cipher);
}

char *cipher_encode(Cipher *cipher) {
    char *result = NULL;

    const int number_of_elements = 4;
    unsigned int content_lens[number_of_elements] = { cipher->cipher_len, cipher->mac_len, cipher->public_key_len, cipher->tag_len };
    char *content[] = { cipher->cipher, cipher->mac, cipher->public_key, cipher->tag };

    // this could be written more simply, but I want you to understand what is going on
    unsigned int total_len =
        // version prefix
        VERSION_PREFIX_LEN
        // dots in between
        + number_of_elements - 1
        // terminator
        + 1;

    // add lengths of all elements
    for (int i = 0; i < number_of_elements; i++) {
        total_len += content_lens[i];
    }

    result = malloc(total_len);
    memset(result, 0, total_len);

    // Write without dot
    memcpy(result, VERSION_PREFIX, VERSION_PREFIX_LEN - 1);
    unsigned int position_to_write = VERSION_PREFIX_LEN - 1;

    for(int i = 0; i < number_of_elements; i++) {
        strcpy(result + position_to_write, ".");
        memcpy(result + position_to_write + 1, content[i], content_lens[i]);
        position_to_write += content_lens[i] + 1;
    }

    return result;
}

void token_into_part(const char *token, char **part, unsigned int *part_len) {
    *part_len = strlen(token);
    *part = malloc(*part_len + 1);
    memset(*part, 0, *part_len + 1);
    strcpy(*part, token);
}

Cipher *cipher_decode(char *digest) {
    const char separator[2] = ".";
    Cipher *cipher = cipher_new();

    // We don't want to modify digest todo is this a good idea?
    char *digest_copy = malloc(strlen(digest) + 1);
    strcpy(digest_copy, digest);

    char *token = strtok(digest_copy, separator);
    if (token == NULL) {
        cipher_free(cipher);
        goto cleanup;
        cipher = NULL;
    }

    // cipher
    token = strtok(NULL, separator);
    if(token == NULL) {
        cipher_free(cipher);
        goto cleanup;
        cipher = NULL;
    }
    token_into_part(token, &cipher->cipher, &cipher->cipher_len);


    // mac
    token = strtok(NULL, separator);

    if(token == NULL) {
        cipher_free(cipher);
        goto cleanup;
        cipher = NULL;
    }
    token_into_part(token, &cipher->mac, &cipher->mac_len);

    // public_key
    token = strtok(NULL, separator);
    if(token == NULL) {
        cipher_free(cipher);
        goto cleanup;
        cipher = NULL;
    }
    token_into_part(token, &cipher->public_key, &cipher->public_key_len);

    // tag
    token = strtok(NULL, separator);
    if(token == NULL) {
        cipher_free(cipher);
        goto cleanup;
        cipher = NULL;
    }
    token_into_part(token, &cipher->tag, &cipher->tag_len);

    cleanup:
    free(digest_copy);

    return cipher;
}
