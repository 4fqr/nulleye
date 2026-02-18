#include "../utils/hash.h"
#include "../core/db.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    char tmp[] = "/tmp/nulleye_test_fileXXXXXX";
    int fd = mkstemp(tmp);
    assert(fd >= 0);
    const char *data = "hello nulleye";
    write(fd, data, strlen(data));
    close(fd);

    char hex[65];
    assert(sha256_file_hex(tmp, hex) == 0);
    assert(strlen(hex) == 64);

    const char *dbpath = "/tmp/nulleye_test.db";
    remove(dbpath);
    assert(db_init(dbpath) == 0);
    assert(db_update_file_entry(tmp, hex, hex, 0, 0, 0644, 1000, 1000, "") == 0);
    db_close();
    remove(tmp);
    remove(dbpath);
    printf("file_integrity tests passed\n");
    return 0;
}
