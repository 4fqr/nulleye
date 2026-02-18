#include "../ai/lstm_predictor.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    lstm_model_t m;
    assert(lstm_init_model(&m, 4, 8) == 0);
    float input[4 * 3] = {0};
    float out[8];
    assert(lstm_predict(&m, input, 3, out, 8) == 0);
    lstm_free_model(&m);
    printf("lstm tests passed\n");
    return 0;
}
