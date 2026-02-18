#define _GNU_SOURCE
#include "core/rules.h"
#include "core/logger.h"
#include "core/response_engine.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct rule_entry { char condition[256]; char action[128]; };
static struct rule_entry *g_rules = NULL;
static size_t g_rules_count = 0;

int rules_load_from_file(const char *path)
{
    if (!path) return -1;
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    char line[512];
    size_t cap = 0;
    while (fgets(line, sizeof(line), f)) {
        char *p = strchr(line, '\n'); if (p) *p = '\0';
        if (line[0] == '\0') continue;
        char cond[256]; char act[128];
        if (sscanf(line, "if %255[^\t] then %127s", cond, act) == 2) {
            if (g_rules_count + 1 > cap) {
                size_t ncap = cap == 0 ? 8 : cap * 2;
                struct rule_entry *tmp = realloc(g_rules, ncap * sizeof(*tmp));
                if (!tmp) { fclose(f); return -1; }
                g_rules = tmp; cap = ncap;
            }
            strncpy(g_rules[g_rules_count].condition, cond, sizeof(g_rules[g_rules_count].condition) - 1);
            strncpy(g_rules[g_rules_count].action, act, sizeof(g_rules[g_rules_count].action) - 1);
            g_rules_count++;
        }
    }
    fclose(f);
    nulleye_log(NYE_LOG_INFO, "loaded %zu rules from %s", g_rules_count, path);
    return 0;
}

static int evaluate_condition_simple(const char *cond, const void *blob, size_t len)
{
    (void)blob; (void)len;
    if (strstr(cond, "anomaly_score>")) return 1;
    if (strstr(cond, "comm==")) return 1;
    return 0;
}

int rules_evaluate_and_act(const void *event_blob, size_t len)
{
    if (!g_rules) return 0;
    for (size_t i = 0; i < g_rules_count; ++i) {
        if (evaluate_condition_simple(g_rules[i].condition, event_blob, len)) {
            response_engine_perform(g_rules[i].action, "auto", "rule match");
            nulleye_log(NYE_LOG_INFO, "rule triggered: %s -> %s", g_rules[i].condition, g_rules[i].action);
        }
    }
    return 0;
}
