//
//  main.c
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#include <stdio.h>

#include "Tests.h"

int main(void) {
    printf("\n\n[VEXL]\n");

    Curve curve = secp224r1;

    for (int i = 0; i < 69; i++) {
        test_performance((Curve) i);
    }

    test_encryption(curve);
    
    return 0;
}
