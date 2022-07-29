//
// Created by Adam Salih on 29.03.2022.
//

#ifndef TOOLS_H
#define TOOLS_H

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

void log_asymetric_performance_header();
void log_asymetric_performance(char *curveName, clock_t key_gen_start, clock_t encryption_start, clock_t decryption_start, clock_t ecdsa_sign_start, clock_t ecdsa_verify_start, clock_t end);
void log_symetric_performance(clock_t encryption_start, clock_t decryption_start, clock_t sign_start, clock_t verify_start, clock_t end);
void log_message(const char *message, ...);
void log_progress(const char *message, ...);
void log_progress_end();
void log_success(const char *message, ...);
void log_error(const char *message, ...);

bool assert_not_equals(const char *str1, const char *str2, const char *message);
bool assert_equals(const char *str1, const char *str2, const char *message);
bool assert_true(int val, const char *message);
bool assert_not_null(const void *ptr, const char *message);
bool assert_null(const void *ptr, const char *message);

#endif
