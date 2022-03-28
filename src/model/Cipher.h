//
// Created by Adam Salih on 20.03.2022.
//

#ifndef CIPHER_H
#define CIPHER_H

#include <stdio.h>
#include <math.h>

#include "../common/Base64.h"

typedef struct {
    char *cipher;
    unsigned int cipherLen;
    char *R;
    unsigned int R_len;
    char *D;
    unsigned int D_len;
} Cipher;

Cipher *cipher_new();
void cipher_free(Cipher *cipher);

char *cipher_encode(Cipher *cipher);
Cipher *cipher_decode(char *digest);

#endif
