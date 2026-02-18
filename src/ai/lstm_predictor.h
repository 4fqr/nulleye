#ifndef NYE_LSTM_PREDICTOR_H
#define NYE_LSTM_PREDICTOR_H

#include <stddef.h>

typedef struct {
    int input_size;
    int hidden_size;
    float *weights;
    float *biases;
} lstm_model_t;

int lstm_init_model(lstm_model_t *m, int input_size, int hidden_size);
void lstm_free_model(lstm_model_t *m);
int lstm_predict(lstm_model_t *m, const float *input, size_t len, float *out, size_t outlen);

#endif