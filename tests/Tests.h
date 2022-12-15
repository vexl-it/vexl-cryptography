

#ifndef TESTS_h
#define TESTS_h

#include <stdio.h>
#include <time.h>

#include <vc/ECIES.h>
#include <vc/ECDSA.h>

#include "TestConstants.h"
#include "Tools.h"

void test_performance();
void test_curve_performance(Curve curve);
void test_aes();
void test_aes_long_string();
void test_hmac();
void test_ecies(Curve curve);
void test_incorrect_keys_ecies(Curve curve);
void test_ecdsa(Curve curve);
void test_pre_generated_message_ecdsa();
void test_pre_generated_message_depreciated_ecdsa();
void test_depreciated_ecdsa(Curve curve);
void test_sha();

#endif
