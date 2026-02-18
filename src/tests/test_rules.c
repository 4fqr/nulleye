#include "../core/rules.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    const char *tmp = "/tmp/nulleye_rules_test.conf";
    FILE *f = fopen(tmp, "w");
    assert(f);
    fputs("if anomaly_score>80 then kill\n", f);
    fclose(f);
    assert(rules_load_from_file(tmp) == 0);
    remove(tmp);
    printf("rules tests passed\n");
    return 0;
}
