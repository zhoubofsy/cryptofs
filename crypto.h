#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#include <cstddef>

class Crypto {
	public:
		virtual size_t encrypto_buffer(char *buf, size_t s, char key);
		virtual size_t decrypto_buffer(char *buf, size_t s, char key);
};

#endif
