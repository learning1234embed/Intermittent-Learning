#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "memory_util.h"

/* Number of neurons */
#define NUM_INPUT           (14*14)
#define NUM_HIDDEN          (64)
#define NUM_OUT             (3)

/* Number of layers: layer-index starts from 0. So 0, 1, 2 for total three layers */
#define NUM_TOTAL_LAYER     (3)
#define NUM_HL              (NUM_TOTAL_LAYER - 1)

/* neurons */
#pragma PERSISTENT(input)
float input[NUM_INPUT] = { 0 };

#pragma PERSISTENT(target)
float target[NUM_INPUT] = { 0 };

#pragma PERSISTENT(hidden)
float hidden[NUM_HL][NUM_HIDDEN] = { 0 };

#pragma PERSISTENT(output)
float output[NUM_OUT] = { 0 };

/* weight parameters */
#pragma PERSISTENT(weight_input)
float weight_input[NUM_INPUT][NUM_HIDDEN] = { 0 };

#pragma PERSISTENT(weight_input_bias)
float weight_input_bias[NUM_HIDDEN] = { 0 };

#if (NUM_HL > 2)
#pragma PERSISTENT(weight_hidden)
float weight_hidden[NUM_HL-1][NUM_HIDDEN][NUM_HIDDEN] = { 0 };
#pragma PERSISTENT(weight_hidden_bias)
float weight_hidden_bias[NUM_HL-1][NUM_HIDDEN] = { 0 };
#endif

#pragma PERSISTENT(weight_out)
float weight_out[NUM_HIDDEN][NUM_OUT] = { 0 };

#pragma PERSISTENT(weight_out_bias)
float weight_out_bias[NUM_OUT] = { 0 };

#pragma PERSISTENT(is_inited)
int is_inited = 0;

/* learning parameters */
#define MAX_EPOCH   3
#define ERROR       0.08
#define ETA         0.3 // learning rate
#define DELTA_ETA   0.002
#define MIN_ETA     0.1

#pragma PERSISTENT(learning_rate)
float learning_rate = ETA;

#pragma PERSISTENT(epoch)
int epoch = 0;

#pragma PERSISTENT(pattern)
int pattern = 0;

#pragma PERSISTENT(total_error)
float total_error = 0;

/* sensed raw example */
#pragma PERSISTENT(raw_example)
float raw_example[NUM_INPUT] = { 0, };

/* training examples */
#define TOTAL_NUM_EXAMPLE   (6)

#pragma PERSISTENT(input_example)
float input_example[NUM_INPUT] = { { 0, }, };

#pragma PERSISTENT(input_label)
int input_label = 0;

#pragma PERSISTENT(num_example)
int num_example[NUM_OUT] = { 0, };

#pragma PERSISTENT(example)
float example[NUM_OUT][NUM_INPUT] = { { 0, }, };


float random()
{
    int r = rand() - RAND_MAX/2;
    float f = (float)r / RAND_MAX * sqrt((float)2/NUM_INPUT);
    return f;
}

int sense()
{
    int i;
    int l;
    float f;

    /* it's a dummy learning data */
    /* real data needs to be obtained by a user */
    l = rand() % NUM_OUT;
    iw(&input_label, l);

    for (i = 0; i < NUM_INPUT; i++) {
        f = random();
        fw(&input_example[i], f);
    }

    printf("sense %d\n", l);
    return 1;
}

int preprocess()
{
    printf("preprocess\n");
    return 1;
}

int select()
{
    if (ir(&num_example[ir(&input_label)]) == 0) {
        return 1;
    }

    printf("select\n");
    return 0;
}

int store()
{
    int new_label = ir(&input_label);
    int n = ir(&num_example[new_label]);
    int i;

    for (i = 0; i < NUM_INPUT; i++) {
        float data = fr(&input_example[i]);
        fw(&example[new_label][i], data);
    }

    iw(&num_example[new_label], n+1);

    printf("store\n");
    return 1;
}

int pick()
{
    printf("pick\n");
    return 1;
}

int learnable()
{
    int i;

    for (i = 0; i < NUM_OUT; i++) {
        if (ir(&num_example[i]) == 0)
            return 0;
    }

    printf("learnable\n");
    return 1;
}

int extract()
{
    printf("extract\n");
    return 1;
}

void init_weight(void)
{
    uint32_t i, j;
    int is = ir(&is_inited);

    printf("is_inited: %d\n", is);

    if (is == 1) {
        printf("init done.\n");
        return;
    }

    for (i = 0; i < NUM_INPUT; i++) {
        for (j = 0; j < NUM_HIDDEN; j++) {
            fw(&weight_input[i][j], random());
        }
    }

    /* bias */
    for (j = 0; j < NUM_HIDDEN; j++)
        fw(&weight_input_bias[j], random());

#if (NUM_HL > 2)
    int layer;
    for (layer = 1; layer < NUM_HL-1; layer++) {
        for (i = 0; i < NUM_HIDDEN; i++)
            for (j = 0; j < NUM_HIDDEN; j++) {
                fw(&weight_hidden[layer][i][j], random());
            }
    }

    /* bias */
    for (layer = 1; layer < NUM_HL-1; layer++) {
        for (j = 0; j < NUM_HIDDEN; j++)
            fw(&weight_hidden_bias[layer][j], random());
    }
#endif

    for (i = 0; i < NUM_HIDDEN; i++) {
        for (j = 0; j < NUM_OUT; j++) {
            fw(&weight_out[i][j], random());
        }
    }

    /* bias */
    for (j = 0; j < NUM_OUT; j++)
        fw(&weight_out_bias[j], random());

    iw(&is_inited, 1);
}

int learn1()
{
    /* initialize weights randomly: this is done only once */
    init_weight();
    printf("learn1\n");
    return 1;
}

float objective(float target, float neuron)
{
    /* cross entropy */
    return -target * log(neuron);
}

float objective_diff(float target, float neuron)
{
    /* derivative of cross entropy */
    return -target/neuron;
}

float inner_activate(float neuron)
{
    /* sigmoid function */
    return 1.0/(1.0 + exp(-neuron));
}

float inner_activate_diff(float neuron)
{
    /* derivative of sigmoid function */
    return neuron * (1 - neuron);
}

float outer_activate_diff(float target, float neuron)
{
    /* derivative of softmax function */
    return neuron * (1 - neuron);
}

void feed_forward()
{
    int i, j;
    float sum = 0;

    for (i = 0; i < NUM_HIDDEN; i++) {
        float sum = 0;
        sum = fr(&weight_input_bias[i]);
        for (j = 0; j < NUM_INPUT; j++) {
            sum += fr(&input[j]) * fr(&weight_input[j][i]);
        }
        fw(&hidden[1][i], inner_activate(sum));
    }

    float total_sum = 0;
    float max = -999999;

    for (i = 0; i < NUM_OUT; i++) {
      sum = fr(&weight_out_bias[i]);
      for (j = 0; j < NUM_HIDDEN; j++) {
          sum += fr(&hidden[1][j]) * fr(&weight_out[j][i]);
      }

      fw(&output[i], sum);
      if (fr(&output[i]) > max)
          max = fr(&output[i]);
    }

    for (i = 0; i < NUM_OUT; i++)
        total_sum += exp(fr(&output[i])-max);

    for (i = 0; i < NUM_OUT; i++) {
        float f = fr(&output[i]) - max;
        f = exp(f) / total_sum;
        fw(&output[i], f);
    }
}

int learn2()
{
    /* feed forward */
    int e = ir(&epoch);

    if (e >= MAX_EPOCH)
        return 1;

    int p = ir(&pattern);
    float te = fr(&total_error);
    int k = 0;
    float error;

    // load input
    for (k = 0; k < NUM_INPUT; k++) {
        fw(&input[k], fr(&example[p][k]));
    }

    // load target
    for (k = 0; k < NUM_OUT; k++)
        fw(&target[k],  0);

    fw(&target[p], 1);

    feed_forward();
    error = objective(fr(&target[p]), fr(&output[p]));

    printf("%d: %d: ", e, p);
    for (k = 0; k < NUM_OUT; k++) {
        printf("%f ", fr(&target[k]));
    }
    printf(": ");
    for (k = 0; k < NUM_OUT; k++) {
        printf("%f ", fr(&output[k]));
    }
    printf(": %f\n", error);

    if (p == 0) {
        te = 0;
    }

    te += error;
    fw(&total_error, te);

    printf("learn2\n");
    return 1;
}

void back_propagate()
{
    int i, j;
    float eta = fr(&learning_rate);
    float sum_delta_and_weight[NUM_TOTAL_LAYER-1][NUM_HIDDEN] = { 0, };

    /* output layer -> last hidden layer */
    float deltas[NUM_OUT] = { 0, };
    for (i = 0; i < NUM_HIDDEN; i++) {
        sum_delta_and_weight[NUM_TOTAL_LAYER-2][i] = 0;

        for (j = 0; j < NUM_OUT; j++) {
            if (i == 0) {
                deltas[j] = objective_diff(fr(&target[j]), fr(&output[j])) * outer_activate_diff(fr(&target[j]), fr(&output[j]));
                /* bias */
                fw(&weight_out_bias[j], fr(&weight_out_bias[j]) - eta * deltas[j]);
            }

            sum_delta_and_weight[NUM_TOTAL_LAYER-2][i] += deltas[j] * fr(&weight_out[i][j]);
            fw(&weight_out[i][j], fr(&weight_out[i][j]) - eta * fr(&hidden[NUM_TOTAL_LAYER-2][i]) * deltas[j]);
        }
    }

#if (NUM_HL > 2)
    /* between hidden layers */
    int layer;
    for (layer = NUM_HL-2; layer >= 1; layer--) {
        float deltas3[NUM_HIDDEN] = { 0, };
        for (i = 0; i < NUM_HIDDEN; i++) {
            for (j = 0; j < NUM_HIDDEN; j++) {
                if (i == 0) {
                    delta3[j] = sum_delta_and_weight[layer+1][j] * inner_activate_diff(fr(&hidden[layer+1][j]));
                    /* bias */
                    w(&weight_hidden_bias[layer][j], fr(&weight_hidden[layer][j]) - eta * deltas3[j]);

                }

                sum_delta_and_weight[layer][i] += deltas3[j] * fr(&weight_hidden[layer][i][j]);
                fw(&weight_hidden[layer][i][j], fr(&weight_hidden[layer][i][j]) - eta * fr(&hidden[layer][i]) * deltas3[j]);
            }
        }
    }
#endif

    /* first hidden layer -> input layer */
    float deltas2[NUM_HIDDEN] = { 0, };
    for (i = 0; i < NUM_INPUT; i++) {
        for (j = 0; j < NUM_HIDDEN; j++) {
            if (i == 0) {
                deltas2[j] = sum_delta_and_weight[1][j] * inner_activate_diff(fr(&hidden[1][j]));
                /* bias */
                fw(&weight_input_bias[j], fr(&weight_input_bias[j]) - eta * deltas2[j]);
            }

            fw(&weight_input[i][j], fr(&weight_input[i][j]) - eta * fr(&input[i]) * deltas2[j]);
        }
    }
}

int learn3()
{
    /* back propagate */
    int e = ir(&epoch);
    if (e >= MAX_EPOCH)
        return 1;

    int p = ir(&pattern);
    float te = fr(&total_error);

    back_propagate();

    if (p >= NUM_OUT - 1) {
        printf("total error: %f lr: %f\n", fr(&total_error), fr(&learning_rate));

        if (te < ERROR || e >= MAX_EPOCH-1) {
            iw(&epoch, 0);
            iw(&pattern, 0);
            fw(&learning_rate, ETA);

            int i;
            for (i = 0; i < NUM_OUT; i++)
                iw(&num_example[i], 0);

            return 1;
        }

        iw(&epoch, e+1);
        iw(&pattern, 0);

        float new_lr = fr(&learning_rate) - DELTA_ETA;
        if (new_lr <= MIN_ETA)
            new_lr = MIN_ETA;

        fw(&learning_rate, new_lr);
    } else {
        iw(&pattern, p+1);
    }

    printf("learn3\n");
    return 0;
}

int evaluate()
{
    printf("evaluate\n");
    /* needs to be programmed by a user */
    return 1;
}

int expect()
{
    printf("expect\n");
    /* needs to be programmed by a user */
    return 1;
}

int infer()
{
    int i;

    for (i = 0; i < NUM_INPUT; i++) {
        fw(&input[i], fr(&input_example[i]));
    }

    feed_forward();

    float max = -9999;
    int max_idx = 0;

    for (i = 0; i < NUM_OUT; i++) {
        float softmax = fr(&output[i]);
        printf("s[%d] = %f\n", i, softmax);
        if (softmax > max) {
            max = softmax;
            max_idx = i;
        }
    }

    printf("infer\n");
    return max_idx;
}

int (*sense_[])() = { sense };
int (*preprocess_[])() = { preprocess };
int (*select_[])() = { select };
int (*store_[])() = { store };
int (*pick_[])() = { pick };
int (*learnable_[])() = { learnable };
int (*extract_[])() = { extract };
int (*learn_[])() = { learn1, learn2, learn3 };
int (*evaluate_[])() = { evaluate };
int (*expect_[])() = { expect };
int (*infer_[])() = { infer };


int get_num_sense()
{
    return sizeof(sense_) / sizeof(sense_[0]);
}

int get_num_preprocess()
{
    return sizeof(preprocess_) / sizeof(preprocess_[0]);
}

int get_num_select()
{
    return sizeof(select_) / sizeof(select_[0]);
}

int get_num_store()
{
    return sizeof(store_) / sizeof(store_[0]);
}

int get_num_pick()
{
    return sizeof(pick_) / sizeof(pick_[0]);
}

int get_num_learnable()
{
    return sizeof(learnable_) / sizeof(learnable_[0]);
}

int get_num_extract()
{
    return sizeof(extract_) / sizeof(extract_[0]);
}

int get_num_learn()
{
    return sizeof(learn_) / sizeof(learn_[0]);
}

int get_num_evaluate()
{
    return sizeof(evaluate_) / sizeof(evaluate_[0]);
}

int get_num_expect()
{
    return sizeof(expect_) / sizeof(expect_[0]);
}

int get_num_infer()
{
    return sizeof(infer_) / sizeof(infer_[0]);
}

int (*get_sense(int num))()
{
    return sense_[num];
}

int (*get_preprocess(int num))()
{
    return preprocess_[num];
}

int (*get_store(int num))()
{
    return store_[num];
}

int (*get_select(int num))()
{
    return select_[num];
}

int (*get_pick(int num))()
{
    return pick_[num];
}

int (*get_learnable(int num))()
{
    return learnable_[num];
}

int (*get_extract(int num))()
{
    return extract_[num];
}

int (*get_learn(int num))()
{
    return learn_[num];
}

int (*get_evaluate(int num))()
{
    return evaluate_[num];
}

int (*get_expect(int num))()
{
    return expect_[num];
}

int (*get_infer(int num))()
{
    return infer_[num];
}
