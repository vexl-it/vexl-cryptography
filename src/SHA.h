//
// Created by Adam Salih on 31.03.2022.
//

#ifndef SHA_H
#define SHA_H

#include <stdio.h>

#ifdef BUILD_FOR_LIBRARY
#include <openssl/sha.h>
#endif

char *sha256_hash(const void *data, const int data_len);

#endif
