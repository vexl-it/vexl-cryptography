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

void test_aes_long_string() {
    log_message("Testing AES symetric encryption with long string");

    int counts = 1000;

    int test_message_len = strlen(test_message);
    char long_message[counts*test_message_len];
    for (int i = 0; i < counts; ++i) {
        memcpy(long_message+(i*test_message_len), test_message, test_message_len);
    }

    char *cipher = aes_encrypt(password, long_message);
    assert_not_null(cipher, "Encrypted message");

    char *message = aes_decrypt(password, cipher);
    assert_equals(message, long_message, "Decrypted content match");

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
    log_message("Testing ECIES encryption with incorrect keys");

    KeyPair keys = generate_key_pair(curve);
    KeyPair keys2 = generate_key_pair(curve);
    KeyPair pubkey = keys;
    pubkey.pemPrivateKey = NULL;

    char *cipher = ecies_encrypt(pubkey.pemPublicKey, test_message);
    assert_not_null(cipher, "Encrypted test message");

    char *message = ecies_decrypt(keys2.pemPublicKey, keys2.pemPrivateKey, cipher);
    assert_null(message, "Did not decrypt test message successfully ");

    assert_not_equals(message, test_message, "Encrypted and decrypted data do not match");

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
    assert_not_null(signature, "Successfully created digital signature");


    bool pre_generated_message_valid = ecdsa_verify(privkey.pemPublicKey, test_message, test_message_len, signature);
    assert_true(pre_generated_message_valid, "Successfully validated generated digital signature");

    free(signature);
    KeyPair_free(privkey);
}

void test_depreciated_ecdsa(Curve curve) {
    log_message("Testing ECDSA digital signature - Depreciated method");

    KeyPair privkey = generate_key_pair(curve);
    KeyPair pubkey = privkey;
    pubkey.pemPrivateKey = NULL;

    int test_message_len = strlen(test_message);

    char *signature = ecdsa_sign_depr(privkey.pemPublicKey, privkey.pemPrivateKey, test_message, test_message_len);
    assert_not_null(signature, "Created digital signature");

    bool valid = ecdsa_verify_depr(privkey.pemPublicKey, test_message, test_message_len, signature);
    assert_true(valid, "Successfully validated digital signature");

    KeyPair_free(privkey);
    free(signature);
}

void test_pre_generated_message_ecdsa() {
    log_message("Testing pre generated ECDSA digital signature");

    char publicKey[] = "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUU0d0VBWUhLb1pJemowQ0FRWUZLNEVFQUNFRE9nQUVSZGtIN1hHM1VRaGZIR1RzQmJ5alVXRmU2SFNycmxZWQpYcm95b0cvdGszMDlxaEprbGtCcGN0eWV2OUJIQUE0VlVPWi9GSytpNzZFPQotLS0tLUVORCBQVUJMSUMgS0VZLS0tLS0K";
//    char privateKey[] = "LS0tLS1CRUdJTiBQUklWQVRFIEtFWS0tLS0tCk1IZ0NBUUF3RUFZSEtvWkl6ajBDQVFZRks0RUVBQ0VFWVRCZkFnRUJCQndsOUhvMDd0VTZaUW1kSGhRV01OUUUKR1N3Tm9McldmMVVvaFhkY29Ud0RPZ0FFU0RGWnFSRzBRb291TFpsV09KTFBSVlJqYUxLQXJZdldDRG94ZnRyUAppSVdQNGh6RlRNVDlhZHg5R24vcWpsNlNXWlVFVXp0REdEZz0KLS0tLS1FTkQgUFJJVkFURSBLRVktLS0tLQo=";
    char message[] = "ftaafxneekyrmnfzwyxmathnbvbxjdjt";
    /*
        generated with:
        openssl dgst -sha256 -sign <(base64 -d -i <(echo $PRIVATE_KEY)) <( echo -n "$MESSAGE" ) | openssl base64 -A
    */
    char generatedSignature[] = "MD0CHHRSQVISVq0Ji5wsX4rfas/3dOe9NRcxzsz80c0CHQDtbejFi31EjorwY8ReHgprkhSdKVJfHZbzx7NC";

    int message_len = strlen(message);
    bool pre_generated_message_valid = ecdsa_verify(publicKey, message, message_len, generatedSignature);
    assert_true(pre_generated_message_valid, "Successfully validated pre generated digital signature");
}

void test_pre_generated_message_depreciated_ecdsa() {
    log_message("Testing pre generated ECDSA digital signature - Depreciated method");

    char publicKey[] = "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUU0d0VBWUhLb1pJemowQ0FRWUZLNEVFQUNFRE9nQUVSZGtIN1hHM1VRaGZIR1RzQmJ5alVXRmU2SFNycmxZWQpYcm95b0cvdGszMDlxaEprbGtCcGN0eWV2OUJIQUE0VlVPWi9GSytpNzZFPQotLS0tLUVORCBQVUJMSUMgS0VZLS0tLS0K";
    char privateKey[] = "LS0tLS1CRUdJTiBQUklWQVRFIEtFWS0tLS0tCk1IZ0NBUUF3RUFZSEtvWkl6ajBDQVFZRks0RUVBQ0VFWVRCZkFnRUJCQnhtK3VwbUxUdjFrdURscDVjVlpxNDYKTWxlN1lFNW4wZWl3UzFZbG9Ud0RPZ0FFUmRrSDdYRzNVUWhmSEdUc0JieWpVV0ZlNkhTcnJsWVlYcm95b0cvdAprMzA5cWhKa2xrQnBjdHlldjlCSEFBNFZVT1ovRksraTc2RT0KLS0tLS1FTkQgUFJJVkFURSBLRVktLS0tLQo=";
    char message[] = "ftaafxneekyrmnfzwyxmathnbvbxjdjt";
    // generated with the depreciated API
    char signature[] = "MD4CHQCSV80cM67nZxTudVj1+pPyWvpyxZH/HAsUIqvgAh0AyxJydw0Suv3sD+H0fkrcEYCikJDGEJjR2KLYzA==";
    int message_len = strlen(message);

    bool pre_generated_message_valid = ecdsa_verify_depr(publicKey, message, message_len, signature);
    assert_true(pre_generated_message_valid, "Successfully validated pre generated digital signature for depreciated API");
}

void test_sha() {
    log_message("Testing SHA");

    char content[] = "content";
    char pre_digest[] = "s3Jw3/orJaGBJtE8OZBkpxx9ZQPc1N27/rgwmQQZ6sk=";

    char *digest = sha256_hash(content, sizeof(content));
    assert_not_null(digest, "Successfully hashed content");

    assert_equals(digest, pre_digest, "Hash digests equals");
}
