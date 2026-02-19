#include "../core/db.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void)
{
    assert(db_init(":memory:") == 0);
    const char *key = "model-test";
    unsigned char blob[] = {1,2,3,4,5,6,7,8};
    assert(db_store_model(key, blob, sizeof(blob)) == 0);
    void *out = NULL; int len = 0;
    assert(db_load_model(key, &out, &len) == 0);
    assert(len == (int)sizeof(blob));
    assert(memcmp(out, blob, len) == 0);
    free(out);
    db_close();
    printf("db_model tests passed\n");
    return 0;
}
