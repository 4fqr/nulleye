#ifndef NYE_IFOREST_H
#define NYE_IFOREST_H

#include <stdint.h>

typedef struct {
    int feature_idx;
    int threshold;
} if_node_t;

typedef struct {
    int n_trees;
    int max_depth;
    if_node_t *trees;
} iforest_t;

void iforest_init(iforest_t *f, int n_trees, int max_depth);
void iforest_free(iforest_t *f);
int iforest_score(iforest_t *f, const int features[], int n_features);

#endif