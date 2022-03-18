//
//  ECIES.h
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#ifndef ECIES_h
#define ECIES_h

#define SALT (const unsigned char*)"vexlvexl"

#include <stdio.h>
#include <string.h>

#ifdef BUILD_FOR_LIBRARY

#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/hmac.h>

#endif

#include "common/Log.h"
#include "common/OpenSSLHelper.h"
#include "model/Curve.h"
#include "model/KeyPair.h"

typedef struct {
    char *cipher;
    unsigned int cipherLen;
    char *R;
    unsigned int R_len;
    char *D;
    unsigned int D_len;
} Cipher;

Cipher *ecies_encrypt(KeyPair keys, const char *message);
char *decrypt(KeyPair keys, Cipher *cipher);

#endif
