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

void test_aes_decrypt_static_message() {
    log_message("Testing AES symetric decryption with static message");

    const char encrypted[] = "000.MIjTH/q+BU3aOjE=.R17vayzHmO50dNhHG7B+Bw==";
    const char expected_message[] = "hello world";
    const char key[] = "somePass";

    char *message = aes_decrypt(key, encrypted);
    assert_equals(message, expected_message, "Decrypted content match");

    free(message);
}

void test_aes_encrypt_static_message() {
    log_message("Testing AES symetric encryption with static message");

    const char expected_encrypted[] = "000.MIjTH/q+BU3aOjE=.R17vayzHmO50dNhHG7B+Bw==";
    const char message[] = "hello world";
    const char key[] = "somePass";

    char *encrypted = aes_encrypt(key, message);
    assert_equals(encrypted, expected_encrypted, "Decrypted content match");
    free(encrypted);
}

void test_aes_bad_tag() {
    log_message("Testing AES encryption with bad tag");

    const char bad_encrypted[] = "000.MIjTH/q+BU3aOjE=.R17vayzHmO50dnhHG7B+Bw==";
    const char key[] = "somePass";

    char *decrypted = aes_decrypt(key, bad_encrypted);
    assert_null(decrypted, "Decrypted content is null");
    free(decrypted);
}

void test_hmac() {
    log_message("Testing hmac");

    char *mac = hmac_digest(password, test_message);
    assert_not_null(mac, "Created cryptographic checksum");

    bool valid = hmac_verify(password, test_message, mac);
    assert_true(valid, "Checksum matches");

    free(mac);
}

void test_hmac_static() {
    log_message("Testing hmac static values");

    const char pass[] = "pass";
    const char message[] = "message";
    const char hmac[] = "Ze5clxkEFAc/xm96+mX0XYE6ZB1wtoa5LMCP+Oj4zRM=";

    char *generated_hmac = hmac_digest(pass, message);
    assert_equals(hmac, generated_hmac, "Generated hmac matches");

    bool valid = hmac_verify(pass, message, hmac);
    assert_true(valid, "Checksum matches");
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

void test_ecies_static_message() {
    log_message("Testing ECIES decription with static data");

    char pubKey[] = "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUU0d0VBWUhLb1pJemowQ0FRWUZLNEVFQUNFRE9nQUVXRGFXNkVud2xVVStiblFWSU9JY2tnSUQ2djBsU0xFMQowczRjZVNsYjVSbHlRcHl3eVpwR2Y0a0RiWGVRYUxSSitGS1d6clRFT2ZNPQotLS0tLUVORCBQVUJMSUMgS0VZLS0tLS0K";
    char privKey[] = "LS0tLS1CRUdJTiBQUklWQVRFIEtFWS0tLS0tCk1JR0JBZ0VBTUJBR0J5cUdTTTQ5QWdFR0JTdUJCQUFoQkdvd2FBSUJBUVFjMXFPQWpUakduR1FpQmxHdVp1OWIKOFFnbTNMaTQvVlF6T2k5YjhhQUhCZ1VyZ1FRQUlhRThBem9BQkZnMmx1aEo4SlZGUG01MEZTRGlISklDQStyOQpKVWl4TmRMT0hIa3BXK1VaY2tLY3NNbWFSbitKQTIxM2tHaTBTZmhTbHM2MHhEbnoKLS0tLS1FTkQgUFJJVkFURSBLRVktLS0tLQ==";

    char message[] = "Something to encrypt";
    char encrypted[] = "000.Xog6f+i5MUMliwngp7YNTXEddJU=.z1N0EuaKgktjHF96d6aB8ii4D0lgMuJytzYaUDq22Rc=.BOiAShyfTVPIPp1YIuDuarwPt4dM1JRTxOdWabLSLm6Sqi97sRDvzGTa6+CA3NHoUVPjqP/IG2LY.NvnBhi32EivPoncNK70O4w==";

    char *decrypted = ecies_decrypt((const char *) &pubKey, (const char *) &privKey, (const char *) &encrypted);
    assert_equals(decrypted, message, "Decrypts cipher as expected");

    free(decrypted);
}

void test_ecies_fail() {
    log_message("Testing ECIES decription fail with bad cipher");

    char pubKey[] = "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUU0d0VBWUhLb1pJemowQ0FRWUZLNEVFQUNFRE9nQUVXRGFXNkVud2xVVStiblFWSU9JY2tnSUQ2djBsU0xFMQowczRjZVNsYjVSbHlRcHl3eVpwR2Y0a0RiWGVRYUxSSitGS1d6clRFT2ZNPQotLS0tLUVORCBQVUJMSUMgS0VZLS0tLS0K";
    char privKey[] = "LS0tLS1CRUdJTiBQUklWQVRFIEtFWS0tLS0tCk1JR0JBZ0VBTUJBR0J5cUdTTTQ5QWdFR0JTdUJCQUFoQkdvd2FBSUJBUVFjMXFPQWpUakduR1FpQmxHdVp1OWIKOFFnbTNMaTQvVlF6T2k5YjhhQUhCZ1VyZ1FRQUlhRThBem9BQkZnMmx1aEo4SlZGUG01MEZTRGlISklDQStyOQpKVWl4TmRMT0hIa3BXK1VaY2tLY3NNbWFSbitKQTIxM2tHaTBTZmhTbHM2MHhEbnoKLS0tLS1FTkQgUFJJVkFURSBLRVktLS0tLQ==";

    char encrypted_no_dots[] = "000.Xog6f+i5MUMliwngp7YNTXEddJU=.z1N0EuaKgktjHF96d6aB8ii4D0lgMuJytzYaUDq22Rc=BOiAShyfTVPIPp1YIuDuarwPt4dM1JRTxOdWabLSLm6Sqi97sRDvzGTa6+CA3NHoUVPjqP/IG2LY.NvnBhi32EivPoncNK70O4w==";

    char *decrypted = ecies_decrypt((const char *) &pubKey, (const char *) &privKey, (const char *) &encrypted_no_dots);
    assert_null(decrypted, "Did not decrypt test message successfully");

    char encrypted_bad_tag[] = "000.Xog6f+i5MUMliwngp7YNTXEddJU=.z1N0EuaKgktjHF96d6aB8ii4D0lgMuJytzYaUDq22Rc=.BOiAShyfTVPIPp1YIuDuarwPt4dM1JRTxOdWabLSLm6Sqi97sRDvzGTa6+CA3NHoUVPjqP/IG2LY.NvnBhi32EivPoncNK71O4w==";
    decrypted = ecies_decrypt((const char *) &pubKey, (const char *) &privKey, (const char *) &encrypted_bad_tag);
    assert_null(decrypted, "Did not decrypt test message successfully");

    char encrypted_bad_mac[] = "000.Xog6f+i5MUMliwngp7YNTXEddJU=.z1N0EuaKgktjHF96d6aB8ii4D1lgMuJytzYaUDq22Rc=.BOiAShyfTVPIPp1YIuDuarwPt4dM1JRTxOdWabLSLm6Sqi97sRDvzGTa6+CA3NHoUVPjqP/IG2LY.NvnBhi32EivPoncNK70O4w==";
    decrypted = ecies_decrypt((const char *) &pubKey, (const char *) &privKey, (const char *) &encrypted_bad_mac);
    assert_null(decrypted, "Did not decrypt test message successfully");

    free(decrypted);
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

void test_ecdsa_v2(Curve curve) {
    log_message("Testing ECDSA digital signature - V2");

    KeyPair privkey = generate_key_pair(curve);
    KeyPair pubkey = privkey;
    pubkey.pemPrivateKey = NULL;

    int test_message_len = strlen(test_message);

    char *signature = ecdsa_sign_v2(privkey.pemPublicKey, privkey.pemPrivateKey, test_message, test_message_len);
    assert_not_null(signature, "Successfully created digital signature");


    bool pre_generated_message_valid = ecdsa_verify_v2(privkey.pemPublicKey, test_message, test_message_len, signature);
    assert_true(pre_generated_message_valid, "Successfully validated generated digital signature");

    free(signature);
    KeyPair_free(privkey);
}

void test_ecdsa_v1(Curve curve) {
    log_message("Testing ECDSA digital signature - V1");

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

void test_pre_generated_message_ecdsa_v2() {
    log_message("Testing pre generated ECDSA digital signature - V2");

    char publicKey[] = "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUU0d0VBWUhLb1pJemowQ0FRWUZLNEVFQUNFRE9nQUVSZGtIN1hHM1VRaGZIR1RzQmJ5alVXRmU2SFNycmxZWQpYcm95b0cvdGszMDlxaEprbGtCcGN0eWV2OUJIQUE0VlVPWi9GSytpNzZFPQotLS0tLUVORCBQVUJMSUMgS0VZLS0tLS0K";
//    char privateKey[] = "LS0tLS1CRUdJTiBQUklWQVRFIEtFWS0tLS0tCk1IZ0NBUUF3RUFZSEtvWkl6ajBDQVFZRks0RUVBQ0VFWVRCZkFnRUJCQndsOUhvMDd0VTZaUW1kSGhRV01OUUUKR1N3Tm9McldmMVVvaFhkY29Ud0RPZ0FFU0RGWnFSRzBRb291TFpsV09KTFBSVlJqYUxLQXJZdldDRG94ZnRyUAppSVdQNGh6RlRNVDlhZHg5R24vcWpsNlNXWlVFVXp0REdEZz0KLS0tLS1FTkQgUFJJVkFURSBLRVktLS0tLQo=";
    char message[] = "ftaafxneekyrmnfzwyxmathnbvbxjdjt";
    /*
        generated with:
        openssl dgst -sha256 -sign <(base64 -d -i <(echo $PRIVATE_KEY)) <( echo -n "$MESSAGE" ) | openssl base64 -A
    */
    char generatedSignature[] = "MD0CHHRSQVISVq0Ji5wsX4rfas/3dOe9NRcxzsz80c0CHQDtbejFi31EjorwY8ReHgprkhSdKVJfHZbzx7NC";

    int message_len = strlen(message);
    bool pre_generated_message_valid = ecdsa_verify_v2(publicKey, message, message_len, generatedSignature);
    assert_true(pre_generated_message_valid, "Successfully validated pre generated digital signature");
}

void test_pre_generated_message_ecdsa_v1() {
    log_message("Testing pre generated ECDSA digital signature - V1");

    char publicKey[] = "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUU0d0VBWUhLb1pJemowQ0FRWUZLNEVFQUNFRE9nQUVSZGtIN1hHM1VRaGZIR1RzQmJ5alVXRmU2SFNycmxZWQpYcm95b0cvdGszMDlxaEprbGtCcGN0eWV2OUJIQUE0VlVPWi9GSytpNzZFPQotLS0tLUVORCBQVUJMSUMgS0VZLS0tLS0K";
//    char privateKey[] = "LS0tLS1CRUdJTiBQUklWQVRFIEtFWS0tLS0tCk1IZ0NBUUF3RUFZSEtvWkl6ajBDQVFZRks0RUVBQ0VFWVRCZkFnRUJCQnhtK3VwbUxUdjFrdURscDVjVlpxNDYKTWxlN1lFNW4wZWl3UzFZbG9Ud0RPZ0FFUmRrSDdYRzNVUWhmSEdUc0JieWpVV0ZlNkhTcnJsWVlYcm95b0cvdAprMzA5cWhKa2xrQnBjdHlldjlCSEFBNFZVT1ovRksraTc2RT0KLS0tLS1FTkQgUFJJVkFURSBLRVktLS0tLQo=";
    char message[] = "ftaafxneekyrmnfzwyxmathnbvbxjdjt";
    // generated with the depreciated API
    char signature[] = "MD4CHQCSV80cM67nZxTudVj1+pPyWvpyxZH/HAsUIqvgAh0AyxJydw0Suv3sD+H0fkrcEYCikJDGEJjR2KLYzA==";
    int message_len = strlen(message);

    bool pre_generated_message_valid = ecdsa_verify(publicKey, message, message_len, signature);
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
