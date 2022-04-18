//
//  OpenSSLHelper.c
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#include "OpenSSLHelper.h"

char *_BIO_read_chars(BIO *bio) {
    int contentSize = 0;
    
    char *content = malloc(1);

    int readSize = 1;
    int buff_len = 1024;
    char buff[buff_len];

    while (readSize != 0) {
        readSize = BIO_gets(bio, buff, buff_len);
        content = (char *)realloc(content, contentSize + readSize);
        memcpy(&content[contentSize], buff, readSize);
        contentSize += readSize;
    }

    return content;
}