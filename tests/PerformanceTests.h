//
// Created by Adam Salih on 05.04.2022.
//

#ifndef PERFOMRANCETESTS_H
#define PERFOMRANCETESTS_H

#include <stdio.h>
#include <stdbool.h>

#include <vc/AES.h>
#include <vc/ECDSA.h>
#include <vc/ECIES.h>
#include <vc/HMAC.h>
#include <vc/SHA.h>

#include "TestConstants.h"
#include "Tools.h"

void test_performance(int count);

#endif
