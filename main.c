//
//  main.c
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#include <stdio.h>
#include "crypto/ECIES.h"


int main(void) {
	unsigned char R[512], D[512], c[512], salt[16];
	size_t R_len, D_len, c_len;

        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
        RAND_init();

        BIO *b = BIO_new_mem_buf((void*)key, sizeof(key));
        EVP_PKEY *pkey = NULL;
        EC_KEY *eckey = NULL;

        PEM_read_bio_PrivateKey(b, &pkey, NULL, NULL);

        eckey = EVP_PKEY_get1_EC_KEY(pkey);

	RAND_bytes(salt, sizeof(salt));

	encipher(eckey, R, &R_len, c, &c_len, D, &D_len, salt, sizeof(salt));
	decipher(eckey, R, R_len, c, c_len, D, D_len, salt, sizeof(salt));

	return 0;
}
