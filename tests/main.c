//
//  main.c
//  vexl-cryptography
//
//  Created by Adam Salih on 23.02.2022.
//

#include <stdio.h>
#include <vc/ECIES.h>


int main(void) {
        unsigned char R[512], D[512], c[512], salt[16];
        size_t R_len, D_len, c_len;

        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
        RAND_init();

        char key[] = \
"-----BEGIN PRIVATE KEY-----\n" \
"MIIBAAIBADAQBgcqhkjOPQIBBgUrgQQAJwSB6DCB5QIBAQRIArxG5w0ydYPXKOh8\n" \
"NDD78GSW3yioDSf6a/nVmrLU7uokoqHGh8DhZczsed7PIen1sjJSRFQvpTfXzW6g\n" \
"yvBTiQHmRxmwWgx8oYGVA4GSAAQFFd9vDBbNrTpj4fqijc/r0SsjNsux05RlH35k\n" \
"4iKmOScufwf3qjLdQwlRVb2gxU9xqyf5zzye4cRypgWxuEmMb0/vy/bdvMkGS7HS\n" \
"Tl7dD4tWKGhGAB4oV2roBC6B5tTLFzpQL+SjqabQDjwCIrw9rhsoR5UTrcikJioa\n" \
"nzwv/wzEUsNPrLSUfMq1dYvt3hk=\n" \
"-----END PRIVATE KEY-----\n";

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
