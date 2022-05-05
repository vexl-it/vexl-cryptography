//
//  Tests.c
//  vexl-cryptography
//
//  Created by Adam Salih on 16.03.2022.
//

#include "Tests.h"

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
    log_message("Testing ECIES asymetric encryption");

    KeyPair keys = generate_key_pair(curve);
    KeyPair pubkey = keys;
    pubkey.pemPrivateKey = NULL;

    char *cipher = ecies_encrypt(pubkey.pemPublicKey, test_message);
    assert_not_null(cipher, "Encrypted test message");

    char *message = ecies_decrypt(keys.pemPublicKey, keys.pemPrivateKey, cipher);
    assert_not_null(message, "Decrypted test message");

    assert_equals(message, test_message, "Encrypted and decrypted data match");

    KeyPair_free(keys);
    free(cipher);
    free(message);
}

void test_incorrect_keys_ecies(Curve curve) {
    log_message("Testing ECIES asymetric encryption");

    KeyPair keys = generate_key_pair(curve);
    KeyPair keys2 = generate_key_pair(curve);
    KeyPair pubkey = keys;
    pubkey.pemPrivateKey = NULL;

    char *cipher = ecies_encrypt(pubkey.pemPublicKey, test_message);
    assert_not_null(cipher, "Encrypted test message");

    char *message = ecies_decrypt(keys2.pemPublicKey, keys2.pemPrivateKey, cipher);
    assert_not_null(message, "Decrypted test message");

    assert_equals(message, test_message, "Encrypted and decrypted data match");

    KeyPair_free(keys);
    free(cipher);
    free(message);
}

void test_ecdsa(Curve curve) {
    log_message("Testing ECDSA digital signature");

    KeyPair privkey = generate_key_pair(curve);
    KeyPair pubkey = privkey;
    pubkey.pemPrivateKey = NULL;

    int test_message_len = strlen(test_message);

    char *signature = ecdsa_sign(privkey.pemPublicKey, privkey.pemPrivateKey, test_message, test_message_len);
    assert_not_null(signature, "Created digital signature");

    bool valid = ecdsa_verify(privkey.pemPublicKey, test_message, test_message_len, signature);
    assert_true(valid, "Successfully validated digital signature");

    KeyPair_free(privkey);
    free(signature);
}

void test_sha() {
    log_message("Testing SHA");

    char content[] = "content";
    char pre_digest[] = "s3Jw3/orJaGBJtE8OZBkpxx9ZQPc1N27/rgwmQQZ6sk=";

    char *digest = sha256_hash(content, sizeof(content));
    assert_not_null(digest, "Successfully hashed content");

    assert_equals(digest, pre_digest, "Hash digests equals");
}
