

#ifndef TESTS_h
#define TESTS_h

#include <stdio.h>
#include <time.h>

#include <vc/ECIES.h>
#include <vc/ECDSA.h>

#include "Tools.h"

void test_performance();
void test_curve_performance(Curve curve);
void test_encryption(Curve curve);
void test_digital_signature(Curve curve);

#endif