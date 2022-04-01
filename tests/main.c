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

    test_performance();

    Curve curve = secp224r1;
    test_encryption(curve);
    test_digital_signature(curve);

    return 0;
}
