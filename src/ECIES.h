//
//  ECIES.h
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#ifndef ECIES_h
#define ECIES_h

#define SALT (const unsigned char*)"vexlvexl"
#define PBKDF2ITER 2000

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
#include "model/Cipher.h"

void ecies_init();

void pbkdf2_encrypt(const unsigned char *password, const int password_len, const char *message, Cipher *cipher);
char *pbkdf2_decrypt(const unsigned char *password, const int password_len, Cipher *cipher);

char *ecies_encrypt(KeyPair keys, const char *message);
char *ecies_decrypt(KeyPair keys, char *encoded_cipher);

#endif
