//
//  ECIES.h
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#ifndef ECIES_h
#define ECIES_h

#include <stdio.h>
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

typedef enum {
	secp224r1,
  	secp384r1,
  	secp521r1,
} Curve;

typedef struct {
	char *pemPrivateKey;
	char *pemPublicKey;
	Curve curve;
} KeyPair;

KeyPair generate_key_pair(Curve curve);
void free_KeyPair(KeyPair KeyPair);
char *encrypt(KeyPair keys, char *message);

char *_get_group_name(Curve curve);

#endif
