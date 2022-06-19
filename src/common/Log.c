//
//  Log.c
//  vexl-cryptography
//
//  Created by Adam Salih on 16.03.2022.
//

#include "Log.h"

void _log(char *message, ...) {
    #ifdef VERBOSE
    va_list argptr;
    va_start(argptr, message);
    vfprintf(stderr, message, argptr);
    va_end(argptr);
    printf("\n");
    #endif
}

void _error(int code, char *message, ...) {
    printf("\n");
    va_list argptr;
    va_start(argptr, message);
    vfprintf(stderr, message, argptr);
    va_end(argptr);
    printf("\n");
}