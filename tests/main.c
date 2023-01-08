//
//  main.c
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#include <stdio.h>
#include <unistd.h>

#include "Tests.h"

int parse_args(int argc, char **argv) {
    int c;
    int count = 0;
    opterr = 0;

    while ((c = getopt (argc, argv, "p:")) != -1) {
        switch (c) {
            case 'p':
                count = atoi(optarg);
                break;
        }
    }
    return count;
}

int main(int argc, char **argv) {
    printf("[VEXL TESTS]\n");
    ecies_init();
    Curve curve = secp224r1;

    test_sha();
    test_aes();
    test_aes_long_string();
    test_aes_decrypt_static_message();
    test_aes_encrypt_static_message();
    test_aes_bad_tag();
    test_hmac();
    test_hmac_static();
    test_ecies(curve);
    test_ecies_static_message();
    test_ecies_fail();
    test_incorrect_keys_ecies(curve);

    test_ecdsa_v2(curve);
    test_ecdsa_v1(curve);
    test_pre_generated_message_ecdsa_v2();
    test_pre_generated_message_ecdsa_v1();

    int count;
    if ((count = parse_args(argc, argv)) > 0) {
        test_performance(count);
    }

    return 0;
}
