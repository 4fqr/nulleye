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
        char *s = line;
        while (*s == ' ' || *s == '\t') s++;
        if (s[0] == '\0' || s[0] == '#') continue;
        /* expect: if <condition> then <action> */
        if (strncmp(s, "if ", 3) != 0) continue;
        char *then = strstr(s, " then ");
        if (!then) continue;
        *then = '\0';
        const char *cond = s + 3;
        const char *act = then + 6;
        if (!cond || !act || cond[0] == '\0' || act[0] == '\0') continue;
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
    fclose(f);
    nulleye_log(NYE_LOG_INFO, "loaded %zu rules from %s", g_rules_count, path);
    return 0;
}

static int evaluate_condition_simple(const char *cond, const void *blob, size_t len)
{
    if (!cond) return 0;

    /* simple contains() operator: contains('text') */
    const char *cpos = strstr(cond, "contains(");
    if (cpos) {
        const char *start = strchr(cpos, '\'');
        if (!start) start = strchr(cpos, '"');
        if (start) {
            start++;
            const char *end = strchr(start, start[-1]);
            if (end) {
                size_t slen = end - start;
                if (blob && len > 0) {
                    const char *s = (const char *)blob;
                    if (memmem(s, len, start, slen)) return 1;
                }
            }
        }
    }

    /* numeric inequality for anomaly_score (if payload contains score=NN) */
    cpos = strstr(cond, "anomaly_score>");
    if (cpos && blob && len > 0) {
        int threshold = atoi(cpos + strlen("anomaly_score>"));
        const char *s = (const char *)blob;
        const char *p = strstr(s, "score=");
        if (p) {
            int score = atoi(p + 6);
            return score > threshold;
        }
    }

    /* fall-back textual checks */
    if (strstr(cond, "comm==") && blob && len > 0) {
        const char *eq = strstr(cond, "comm==");
        if (eq) {
            const char *val = eq + strlen("comm==");
            if (*val == '\'' || *val == '"') val++;
            if (blob && len > 0) {
                const char *s = (const char *)blob;
                if (strstr(s, val)) return 1;
            }
        }
    }

    return 0;
}

int rules_evaluate_and_act(const void *event_blob, size_t len)
{
    if (!g_rules) return 0;
    int triggers = 0;
    for (size_t i = 0; i < g_rules_count; ++i) {
        if (evaluate_condition_simple(g_rules[i].condition, event_blob, len)) {
            response_engine_perform(g_rules[i].action, "auto", "rule match");
            nulleye_log(NYE_LOG_INFO, "rule triggered: %s -> %s", g_rules[i].condition, g_rules[i].action);
            triggers++;
        }
    }
    return triggers;
}
