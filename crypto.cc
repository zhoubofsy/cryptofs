
#include "crypto.h"

size_t Crypto::encrypto_buffer(char *buf, size_t s, char key) {
	if (buf == NULL || s == 0) {
		// 输入参数错误
		return 0;
	}

	// 遍历缓冲区中的每个字节，对其进行与 key 的异或操作
	for (size_t i = 0; i < s; ++i) {
		buf[i] = buf[i] ^ key;
	}

	// 返回加密数据的大小
	return s;
}

size_t Crypto::decrypto_buffer(char *buf, size_t s, char key) {
	return this->encrypto_buffer(buf, s, key);
}
