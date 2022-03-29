//
// Created by user2859193 and ryyst https://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c
//

#ifndef BASE64_H
#define BASE64_H

#ifdef BUILD_FOR_LIBRARY

#include <stdint.h>
#include <stdlib.h>

void base64_build_decoding_table();
char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length);
unsigned char *base64_decode(const char *data, size_t input_length, size_t *output_length);

#endif

#endif
