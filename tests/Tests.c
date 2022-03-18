//
//  Tests.c
//  vexl-cryptography
//
//  Created by Adam Salih on 16.03.2022.
//

#include "Tests.h"

void test_performance(Curve curve) {
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
//    KeyPair pubkey = keys;
//    pubkey.pemPrivateKey = NULL;

//    printf("Generated key pairs\n");
//    printf("private key:\n%s\n", keys.pemPrivateKey);
//    printf("public key:\n%s\n", keys.pemPublicKey);

    const char sourceMessage[] = "{\"widget\": {"\
"    \"debug\": \"on\","\
"    \"window\": {"\
"        \"title\": \"sourceMessage\","\
"        \"name\": \"main_window\","\
"        \"width\": 500,"\
"        \"height\": 500"\
"    },"\
"    \"image\": { "\
"        \"src\": \"Images/Sun.png\","\
"        \"name\": \"sun1\","\
"        \"hOffset\": 250,"\
"        \"vOffset\": 250,"\
"        \"alignment\": \"center\""\
"    },"\
"    \"text\": {"\
"        \"data\": \"Click Here\","\
"        \"size\": 36,"\
"        \"style\": \"bold\","\
"        \"name\": \"text1\","\
"        \"hOffset\": 250,"\
"        \"vOffset\": 100,"\
"        \"alignment\": \"center\","\
"        \"onMouseUp\": \"sun1.opacity = (sun1.opacity / 100) * 90;\""\
"    }"\
"}}";

    Cipher *cipher = ecies_encrypt(keys, sourceMessage);

    char *message = ecies_decrypt(keys, cipher);

    printf("decrypted messaage: %s\n", message);

    if (strcmp(message, sourceMessage) == 0) {
        printf("Bingo!\n");
    }

    KeyPair_free(keys);
}
