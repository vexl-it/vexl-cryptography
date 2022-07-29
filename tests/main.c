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
    test_hmac();
    test_ecies(curve);
    test_incorrect_keys_ecies(curve);
    test_ecdsa(curve);


    int count;
    if ((count = parse_args(argc, argv)) > 0) {
        test_performance(count);
    }

    return 0;
}
