#include "ai/model.h"
#include "core/db.h"
#include "core/logger.h"
#include <stdlib.h>
#include <string.h>

int ai_model_save(const char *key, iforest_t *f)
{
    if (!key || !f) return -1;
    int bytes = sizeof(int) * 2 + sizeof(if_node_t) * f->n_trees;
    void *blob = malloc(bytes);
    if (!blob) return -1;
    unsigned char *p = blob;
    memcpy(p, &f->n_trees, sizeof(int)); p += sizeof(int);
    memcpy(p, &f->max_depth, sizeof(int)); p += sizeof(int);
    memcpy(p, f->trees, sizeof(if_node_t) * f->n_trees);
    int rc = db_store_model(key, blob, bytes);
    free(blob);
    if (rc == 0) nulleye_log(NYE_LOG_INFO, "AI model saved (%s)", key);
    return rc;
}

int ai_model_load(const char *key, iforest_t *f)
{
    if (!key || !f) return -1;
    void *blob = NULL; int len = 0;
    if (db_load_model(key, &blob, &len) != 0) return -1;
    unsigned char *p = blob;
    memcpy(&f->n_trees, p, sizeof(int)); p += sizeof(int);
    memcpy(&f->max_depth, p, sizeof(int)); p += sizeof(int);
    f->trees = malloc(sizeof(if_node_t) * f->n_trees);
    memcpy(f->trees, p, sizeof(if_node_t) * f->n_trees);
    free(blob);
    nulleye_log(NYE_LOG_INFO, "AI model loaded (%s)", key);
    return 0;
}
