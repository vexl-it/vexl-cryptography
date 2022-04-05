//
//  Tests.c
//  vexl-cryptography
//
//  Created by Adam Salih on 16.03.2022.
//

#include "Tests.h"

char password[] = "Password123";
const char test_message[] = "{\"widget\": {"\
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

void test_performance() {
    log_message("Testing performance");
    log_performance_header();
    for (int i = 0; i < 69; i++) {
        test_curve_performance((Curve) i);
    }
}

void test_curve_performance(Curve curve) {
    int count = 1;
    clock_t key_gen_start, encryption_start, decryption_start, ecdsa_sign_start, ecdsa_verify_start, end;
    KeyPair keyPairs[count];
    char *ciphers[count];
    char *messages[count];
    char *signatures[count];
    bool verifications[count];

    key_gen_start = clock();
    for (int i = 0; i < count; i++) {
        log_progress("Generating keys: %d/%d", i+1,count);
        keyPairs[i] = generate_key_pair(curve);
    }
    encryption_start = clock();
    for (int i = 0; i < count; i++) {
        log_progress("Encrypting: %d/%d", i+1,count);
        ciphers[i] = ecies_encrypt(keyPairs[i], test_message);
    }
    decryption_start = clock();
    for (int i = 0; i < count; i++) {
        log_progress("Decrypting: %d/%d", i+1,count);
        messages[i] = ecies_decrypt(keyPairs[i], ciphers[i]);
    }
    ecdsa_sign_start = clock();
    for (int i = 0; i < count; i++) {
        log_progress("Decrypting: %d/%d", i+1,count);
        signatures[i] = ecdsa_sign(keyPairs[i], ciphers[i], strlen(ciphers[i]));
    }
    ecdsa_verify_start = clock();
    for (int i = 0; i < count; i++) {
        log_progress("Decrypting: %d/%d", i+1,count);
        verifications[i] = ecdsa_verify(keyPairs[i], ciphers[i], strlen(ciphers[i]), signatures[i]);
    }
    end = clock();

    log_progress_end();
    log_performance(_get_group_name(curve), key_gen_start, encryption_start, decryption_start, ecdsa_sign_start, ecdsa_verify_start, end);

    for (int i = 0; i < count; i++) {
        KeyPair_free(keyPairs[i]);
        free(ciphers[i]);
        free(messages[i]);
        free(signatures[i]);
    }
}

void test_aes() {
    log_message("Testing AES symetric encryption");

    char *cipher = aes_encrypt(password, test_message);
    assert_not_null(cipher, "Encrypted message");

    char *message = aes_decrypt(password, cipher);
    assert_equals(message, test_message, "Decrypted content match");

    free(cipher);
    free(message);
}

void test_hmac() {
    log_message("Testing hmac");

    char *mac = hmac_digest(password, test_message);
    assert_not_null(mac, "Created cryptographic checksum");

    bool valid = hmac_verify(password, test_message, mac);
    assert_true(valid, "Checksum matches");

    free(mac);
}

void test_ecies(Curve curve) {
    log_message("Testing encryption");

    KeyPair keys = generate_key_pair(curve);
    KeyPair pubkey = keys;
    pubkey.pemPrivateKey = NULL;

    char *cipher = ecies_encrypt(pubkey, test_message);
    assert_not_null(cipher, "Encrypted test message");

    char *message = ecies_decrypt(keys, cipher);
    assert_not_null(message, "Decrypted test message");

    assert_equals(message, test_message, "Encrypted and decrypted data match");

    KeyPair_free(keys);
    free(cipher);
    free(message);
}

void test_ecdsa(Curve curve) {
    log_message("Testing digital signature");

    KeyPair privkey = generate_key_pair(curve);
    KeyPair pubkey = privkey;
    pubkey.pemPrivateKey = NULL;

    int test_message_len = strlen(test_message);

    char *signature = ecdsa_sign(privkey, test_message, test_message_len);
    assert_not_null(signature, "Created digital signature");

    bool valid = ecdsa_verify(privkey, test_message, test_message_len, signature);
    assert_true(valid, "Successfully validated digital signature");

    KeyPair_free(privkey); // pubkey doesn't have to be freed
    free(signature);
}
