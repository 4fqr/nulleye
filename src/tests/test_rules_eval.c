#include "../core/rules.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *tmp = "/tmp/nulleye_rules_eval_test.conf";
    FILE *f = fopen(tmp, "w");
    assert(f);
    fputs("if anomaly_score>80 then kill\n", f);
    fputs("if contains('hello') then notify\n", f);
    fclose(f);

    assert(rules_load_from_file(tmp) == 0);

    const char *event1 = "score=85 comm=badproc";
    int t = rules_evaluate_and_act(event1, strlen(event1));
    assert(t >= 1);

    const char *event2 = "note: hello world";
    t = rules_evaluate_and_act(event2, strlen(event2));
    assert(t >= 1);

    remove(tmp);
    printf("rules evaluation tests passed\n");
    return 0;
}
