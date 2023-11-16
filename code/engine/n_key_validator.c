#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#elif defined(__unix__)
#define EXPORT __attribute__((visibility("default")))
#endif

#define KEY_LENGTH 2048
#define XOR 0x3ffad
#define LEVEL_IDENT (('M'<<24)+('F'<<16)+('F'<<8)+'B')

static const unsigned int __key_validator[KEY_LENGTH] = {
    0x0000
};

static void xor_decrypt(unsigned char *key)
{
    for (size_t i = 0; i < KEY_LENGTH; i++) {
        ((unsigned int *)key)[i] = ((unsigned int *)key)[i] ^ XOR;
    }
}

EXPORT int ValidateLevelKey(unsigned char *buffer, unsigned long int bufferLength)
{
    unsigned int *p;

    p = buffer;

    if (p[40] != LEVEL_IDENT) {
        return -1;
    }

    xor_decrypt(buffer);

    return 1;
}
