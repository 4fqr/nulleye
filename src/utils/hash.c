#include "utils/hash.h"
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sha256_file_hex(const char *path, char outhex[65])
{
    unsigned char buf[4096];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    size_t r;
    while ((r = fread(buf, 1, sizeof(buf), f)) > 0) SHA256_Update(&ctx, buf, r);
    SHA256_Final(hash, &ctx);
    fclose(f);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) sprintf(&outhex[i * 2], "%02x", hash[i]);
    outhex[64] = '\0';
    return 0;
}
