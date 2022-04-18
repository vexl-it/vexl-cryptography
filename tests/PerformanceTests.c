//
// Created by Adam Salih on 05.04.2022.
//

#include "PerformanceTests.h"

void test_asymetric_performance(Curve curve, int count) {
    clock_t key_gen_start, encryption_start, decryption_start, ecdsa_sign_start, ecdsa_verify_start, end;
    KeyPair keyPairs[count];
    char *ciphers[count];
    char *messages[count];
    char *signatures[count];
    bool verifications[count];

    key_gen_start = clock();
    for (int i = 0; i < count; i++) {
        keyPairs[i] = generate_key_pair(curve);
    }
    encryption_start = clock();
    for (int i = 0; i < count; i++) {
        ciphers[i] = ecies_encrypt(keyPairs[i], test_message);
    }
    decryption_start = clock();
    for (int i = 0; i < count; i++) {
        messages[i] = ecies_decrypt(keyPairs[i], ciphers[i]);
    }
    ecdsa_sign_start = clock();
    for (int i = 0; i < count; i++) {
        signatures[i] = ecdsa_sign(keyPairs[i], ciphers[i], strlen(ciphers[i]));
    }
    ecdsa_verify_start = clock();
    for (int i = 0; i < count; i++) {
        verifications[i] = ecdsa_verify(keyPairs[i], ciphers[i], strlen(ciphers[i]), signatures[i]);
    }
    end = clock();

    log_asymetric_performance(_get_group_name(curve), key_gen_start, encryption_start, decryption_start, ecdsa_sign_start, ecdsa_verify_start, end);

    for (int i = 0; i < count; i++) {
        KeyPair_free(keyPairs[i]);
        free(ciphers[i]);
        free(messages[i]);
        free(signatures[i]);
    }
}

void test_symetric_performance(int count) {
    clock_t encryption_start, decryption_start, sign_start, verify_start, end;
    char *ciphers[count];
    char *messages[count];
    char *signatures[count];
    bool verifications[count];

    encryption_start = clock();
    for (int i = 0; i < count; i++) {
        ciphers[i] = aes_encrypt(password, test_message);
    }
    decryption_start = clock();
    for (int i = 0; i < count; i++) {
        messages[i] = aes_decrypt(password, ciphers[i]);
    }
    sign_start = clock();
    for (int i = 0; i < count; i++) {
        signatures[i] = hmac_digest(password, ciphers[i]);
    }
    verify_start = clock();
    for (int i = 0; i < count; i++) {
        verifications[i] = hmac_verify(password, ciphers[i], signatures[i]);
    }
    end = clock();

    log_symetric_performance(encryption_start, decryption_start, sign_start, verify_start, end);

    for (int i = 0; i < count; i++) {
        free(ciphers[i]);
        free(messages[i]);
        free(signatures[i]);
    }
}

void test_performance(int count) {
    log_message("Running performance with %d iterations", count);

    log_message("Testing ECIES performance");
    log_asymetric_performance_header();
    for (int i = 0; i < 69; i++) {
        test_asymetric_performance((Curve) i, count);
    }

    log_message("Testing AES performance");
    test_symetric_performance(count);
}