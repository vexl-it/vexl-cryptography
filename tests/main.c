//
//  main.c
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#include <stdio.h>
#include <time.h>

#include <vc/ECIES.h>

void test_perfomance(Curve curve) {
    int count = 1000;
    clock_t begin = clock();
    KeyPair keyPairs[count];

    for (int i = 0; i < count; i++) {
        printf("\r%d/%d", i+1, count);
        keyPairs[i] = generate_key_pair(curve);
    }
    printf("\n");

    clock_t end = clock();
    double time_in_ms = ((double)(end - begin) / CLOCKS_PER_SEC) * 1000;

    printf("Generated %d %s keypair in ", count, _get_group_name(curve));
    if (time_in_ms < 1000) {
        printf("%f ms\n", time_in_ms);
    } else {
        printf("%f s\n", time_in_ms/1000);
    }

    for (int i = 0; i < count; i++) {
        KeyPair_free(keyPairs[i]);
    }
}

void test_encryption(Curve curve) {
    KeyPair keys = generate_key_pair(curve);

    printf("Generated key pairs\n");
    printf("private key:\n%s\n", keys.pemPrivateKey);
    printf("public key:\n%s\n", keys.pemPublicKey);

    encrypt(keys, "secret");

    KeyPair_free(keys);
}

int main(void) {
    printf("\n\n[VEXL]\n");

    Curve curve = secp224r1;
    
    test_perfomance(curve);
    test_encryption(curve);
    
    return 0;
}
