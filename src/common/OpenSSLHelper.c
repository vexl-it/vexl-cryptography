//
//  OpenSSLHelper.c
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#include "OpenSSLHelper.h"

char *_BIO_read_chars(BIO *bio) {
	int keySize = 0;
    
	char *content = malloc(1);

	int readSize = 1;
	char buf[64];

	while (readSize != 0) {
		readSize = BIO_gets(bio, buf, sizeof(buf));
		content = (char *)realloc(content, keySize + readSize);
		for (int i = keySize; i < keySize + readSize; i++) {
			content[i] = buf[i - keySize];
		}
		keySize += readSize;
	}

	return content;
}