//
//  ECIES.c
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#include "ECIES.h"

char *encrypt(KeyPair keys, char *message) {
    EC_KEY *eckey = _KeyPair_get_EC_KEY(keys);

    return message;
}

char *decrypt(KeyPair keys, char *cipher) {
    EC_KEY *eckey = _KeyPair_get_EC_KEY(keys);

    return cipher;
}

