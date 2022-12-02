//
// Created by Adam Salih on 31.03.2022.
//

#include "SHA.h"

char *sha256_hash(const void *data, const int data_len) {
    if (data == NULL) {
        return NULL;
    }
    SHA256_CTX context;
    char digest[SHA256_DIGEST_LENGTH];

    if(!SHA256_Init(&context))
        return NULL;

    if(!SHA256_Update(&context, (unsigned char*)data, data_len))
        return NULL;

    if(!SHA256_Final(digest, &context))
        return NULL;

    char *output;
    int output_len = 0;
    base64_encode(digest, SHA256_DIGEST_LENGTH, &output_len, &output);

    return output;
}