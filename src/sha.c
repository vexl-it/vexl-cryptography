//
// Created by Adam Salih on 31.03.2022.
//

#include "SHA.h"

char *sha256_hash(const void *data, const int data_len) {
    SHA256_CTX context;
    char *hash = malloc(SHA256_DIGEST_LENGTH);

    if(!SHA256_Init(&context))
        return NULL;

    if(!SHA256_Update(&context, (unsigned char*)data, data_len))
        return NULL;

    if(!SHA256_Final(hash, &context))
        return NULL;

    return hash;
}