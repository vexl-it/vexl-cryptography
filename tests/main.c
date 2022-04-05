//
//  main.c
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#include <stdio.h>

#include "Tests.h"

int main(void) {
    printf("[VEXL TESTS]\n");
    ecies_init();
    Curve curve = secp224r1;

    test_performance();
    test_aes();
    test_hmac();
    test_ecies(curve);
    test_ecdsa(curve);

    return 0;
}
