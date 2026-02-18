#include "ai/lstm_predictor.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "core/logger.h"

static float sigmoidf_approx(float x)
{
    return 1.0f / (1.0f + expf(-x));
}

int lstm_init_model(lstm_model_t *m, int input_size, int hidden_size)
{
    if (!m || input_size <= 0 || hidden_size <= 0) return -1;
    m->input_size = input_size;
    m->hidden_size = hidden_size;
    size_t wcount = (size_t)input_size * hidden_size * 4 + (size_t)hidden_size * hidden_size * 4;
    m->weights = calloc(wcount, sizeof(float));
    if (!m->weights) return -1;
    m->biases = calloc((size_t)hidden_size * 4, sizeof(float));
    if (!m->biases) { free(m->weights); m->weights = NULL; return -1; }
    for (size_t i = 0; i < wcount; ++i) m->weights[i] = ((float)((i * 13 + 7) % 97)) / 97.0f * 0.1f;
    for (size_t i = 0; i < (size_t)hidden_size * 4; ++i) m->biases[i] = 0.0f;
    return 0;
}

void lstm_free_model(lstm_model_t *m)
{
    if (!m) return;
    free(m->weights);
    free(m->biases);
    m->weights = NULL;
    m->biases = NULL;
}

int lstm_predict(lstm_model_t *m, const float *input, size_t len, float *out, size_t outlen)
{
    if (!m || !input || !out || outlen == 0) return -1;
    int hs = m->hidden_size;
    float *h = calloc(hs, sizeof(float));
    float *c = calloc(hs, sizeof(float));
    if (!h || !c) { free(h); free(c); return -1; }
    for (size_t t = 0; t < len; ++t) {
        const float *x = &input[t * m->input_size];
        for (int i = 0; i < hs; ++i) {
            float ig = m->biases[i] * 0.0f;
            float fg = m->biases[hs + i] * 0.0f;
            float og = m->biases[2 * hs + i] * 0.0f;
            float gg = m->biases[3 * hs + i] * 0.0f;
            float wx = 0.0f;
            for (int j = 0; j < m->input_size; ++j) wx += x[j] * m->weights[(size_t)j * hs * 4 + i];
            float uh = 0.0f;
            for (int j = 0; j < hs; ++j) uh += h[j] * m->weights[(size_t)m->input_size * hs * 4 + (size_t)j * hs * 4 + i];
            ig += wx + uh;
            fg += wx + uh;
            og += wx + uh;
            gg += wx + uh;
            ig = sigmoidf_approx(ig);
            fg = sigmoidf_approx(fg);
            og = sigmoidf_approx(og);
            gg = tanhf(gg);
            c[i] = fg * c[i] + ig * gg;
            h[i] = og * tanhf(c[i]);
        }
    }
    size_t outcount = outlen < (size_t)hs ? outlen : (size_t)hs;
    for (size_t i = 0; i < outcount; ++i) out[i] = h[i];
    free(h); free(c);
    return 0;
}
