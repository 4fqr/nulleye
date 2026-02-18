#include "ai/isolation_forest.h"
#include <stdlib.h>
#include <time.h>

void iforest_init(iforest_t *f, int n_trees, int max_depth)
{
    if (!f) return;
    f->n_trees = n_trees;
    f->max_depth = max_depth;
    f->trees = calloc(n_trees, sizeof(if_node_t));
    srand((unsigned)time(NULL));
    for (int i = 0; i < n_trees; ++i) {
        f->trees[i].feature_idx = rand() % 6;
        f->trees[i].threshold = (rand() % 256);
    }
}

void iforest_free(iforest_t *f)
{
    if (!f) return;
    free(f->trees);
    f->trees = NULL;
}

int iforest_score(iforest_t *f, const int features[], int n_features)
{
    if (!f || !features) return 0;
    int score_acc = 0;
    for (int i = 0; i < f->n_trees; ++i) {
        int fi = f->trees[i].feature_idx;
        int thr = f->trees[i].threshold;
        int val = (fi < n_features) ? features[fi] : 0;
        score_acc += (val > thr) ? 1 : 5;
    }
    int avg = score_acc / f->n_trees;
    int score = 100 - (avg * 10) / (f->max_depth > 0 ? f->max_depth : 1);
    if (score < 0) score = 0;
    if (score > 100) score = 100;
    return score;
}
