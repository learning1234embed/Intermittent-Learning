#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "memory_util.h"

#define NUM_INPUT   14 * 14
#define NUM_OUT     3
#define ETA         0.001
#define MAX_ITER    3
#define K           3

#pragma PERSISTENT(input)
float input[NUM_INPUT] = { 0, };

#pragma PERSISTENT(weights)
float weights[NUM_INPUT][NUM_OUT] = { 0, };

#pragma PERSISTENT(activation)
float activation[NUM_OUT] = { 0, };

#pragma PERSISTENT(is_inited)
int is_inited = 0;

#pragma PERSISTENT(iteration)
int iteration = 0;

#pragma PERSISTENT(selected_input)
float selected_input[K][NUM_INPUT] = { 0, };

#pragma PERSISTENT(not_selected_input)
float not_selected_input[K][NUM_INPUT] = { 0, };

#pragma PERSISTENT(num_selected_input)
int num_selected_input = 0;

#pragma PERSISTENT(num_not_selected_input)
int num_not_selected_input = 0;


float random()
{
    int r = rand() - RAND_MAX/2;
    float f = (float)r / RAND_MAX * sqrt((float)2/NUM_INPUT);
    return f;
}

int sense()
{
    int i;
    float f;

    /* it's a dummy learning data */
    /* real data needs to be obtained by a user */
    for (i = 0; i < NUM_INPUT; i++) {
        f = random();
        fw(&input[i], f);
    }

    printf("sense\n");
    return 1;
}

int preprocess()
{
    printf("preprocess\n");
    /* needs to be programmed by a user */
    return 1;
}

float distance(float a[NUM_INPUT], float b[NUM_INPUT])
{
    float sum = 0;
    int i = 0;

    for (i = 0; i < NUM_INPUT; i++) {
        sum += (fr(&a[i]) - fr(&b[i])) * (fr(&a[i]) - fr(&b[i]));
    }

    return sqrt(sum);
}

float diversity()
{
    float sum = 0;
    int i, j;

    for (i = 0; i < K; i++) {
        for (j = 0; j < K; j++) {
            sum += distance(selected_input[i], selected_input[j]);
        }
    }

    return (float)sum/((float)i*(float)j);
}

float representative()
{
    float sum = 0;
    int i, j;

    for (i = 0; i < K; i++) {
        for (j = 0; j < K; j++) {
            sum += distance(selected_input[i], not_selected_input[j]);
            }
    }

    return (float)sum/((float)i*(float)j);
}

int to_be_replace_by_div()
{
    int i, j;
    int max_idx = 0;
    float temp[NUM_INPUT];
    float max_div = -999999;

    for (i = 0; i < K; i++) {
        for (j = 0; j < NUM_INPUT; j++) {
            temp[j] = fr(&selected_input[i][j]);
            fw(&selected_input[i][j], input[j]);
        }

        float new_div = diversity();
        if (max_div < new_div) {
            max_div = new_div;
            max_idx = i;
        }

        for (j = 0; j < NUM_INPUT; j++) {
            fw(&selected_input[i][j], temp[j]);
        }
    }

    float div = diversity();
    int ret = -1;
    if (max_div > div) {
        ret = max_idx;
    }

    //printf("idx: %d %f\n", ret, max_div);
    return ret;
}

int to_be_replace_by_rep()
{
    int i, j;
    int min_idx = 0;
    float temp[NUM_INPUT];
    float min_rep = 999999;
    float rep = representative();

    for (i = 0; i < K; i++) {
        for (j = 0; j < NUM_INPUT; j++) {
            temp[j] = fr(&selected_input[i][j]);
            fw(&selected_input[i][j], fr(&input[j]));
        }

        float new_rep = representative();
        if (min_rep > new_rep) {
            min_rep = new_rep;
            min_idx = i;
        }

        for (j = 0; j < NUM_INPUT; j++) {
            fw(&selected_input[i][j], temp[j]);
        }
    }

    int ret = -1;
    if (min_rep < rep) {
        ret = min_idx;
    }

    //printf("idx: %d %f\n", ret, min_rep);
    return ret;
}

int select()
{
    int j;
    int idx;
    int sn = ir(&num_selected_input);
    int nn = ir(&num_not_selected_input);

    printf("select %d %d\n", sn, nn);

    if (sn < K) {
        int j;
        for (j = 0; j < NUM_INPUT; j++) {
            fw(&selected_input[sn][j], fr(&input[j]));
        }
        iw(&num_selected_input, sn+1);
        return 1;
    }

    /* selection by representative */
    idx = to_be_replace_by_rep();
    if (idx >= 0 && idx < K) {
        for (j = 0; j < NUM_INPUT; j++) {
            fw(&selected_input[idx][j], fr(&input[j]));
        }
        iw(&num_selected_input, sn+1);
        return 1;
    } else {
        for (j = 0; j < NUM_INPUT; j++) {
            fw(&not_selected_input[nn%K][j], fr(&input[j]));
        }
        iw(&num_not_selected_input, nn+1);
        return 0;
    }
}

int store()
{
    printf("store\n");
    /* needs to be programmed by a user */
    return 1;
}

int pick()
{
    printf("pick\n");
    /* needs to be programmed by a user */
    return 1;
}

int learnable()
{
    printf("learnable\n");
    /* needs to be programmed by a user */
    return 1;
}

int extract()
{
    printf("extract\n");
    /* needs to be programmed by a user */
    return 1;
}

void init_weight()
{
    int is = ir(&is_inited);

    printf("is_inited: %d\n", is);

    if (is == 1) {
        printf("init done.\n");
        return;
    }

    float sum = 0;
    float normalizer = 0;
    int i, j;

    for (j = 0; j <= NUM_OUT ; j++) {
        for (i = 0 ; i <= NUM_INPUT; i++) {
            float weight = ((float)rand()) / (float)RAND_MAX * sqrt((float)2/NUM_INPUT);
            fw(&weights[i][j], weight);
            sum += weight*weight;
        }
    }

    normalizer = sqrt(sum);

    for (j = 0; j <= NUM_OUT ; j++) {
        for (i = 0 ; i <= NUM_INPUT; i++) {
            float weight = fr(&weights[i][j]);
            weight /= normalizer;
            fw(&weights[i][j], weight);
        }
    }

    iw(&is_inited, 1);
}

int feed_forward()
{
    float sum = 0;
    float normalizer = 0;
    float max = -9999999999;
    int winner = -1;
    int i, j;

    for (i = 0; i < NUM_INPUT; i++) {
        float in = fr(&input[i]);
        sum += in * in;
    }

    normalizer = sqrt(sum);

    for (j = 0; j < NUM_OUT; j++) {
        fw(&activation[j], 0);
        for (i = 0; i < NUM_INPUT; i++) {
            float in = fr(&input[i]);
            float normalized_input = in / normalizer;
            float weight = fr(&weights[i][j]);
            float act = fr(&activation[j]);
            act += weight * normalized_input;
            fw(&activation[j], act);
        }
    }

    for (i = 0; i < NUM_OUT; i++) {
        float act = fr(&activation[i]);
        if (act > max) {
            max = act;
            winner = i;
        }
    }

    printf("winner = %d %f\n", winner, max);
    return winner;
}

void update_weight(int winner)
{
    float sum = 0;
    float normalizer = 0;
    float delta[NUM_INPUT+1] = { 0, };
    int i;

    for (i = 0; i < NUM_INPUT; i++) {
        delta[i] = (fr(&input[i]) - fr(&weights[i][winner]));
        sum += delta[i]*delta[i];
    }

    normalizer = sqrt(sum);
    for (i = 0; i < NUM_INPUT; i++) {
        delta[i] /= normalizer;
        float weight = fr(&weights[i][winner]);
        weight += ETA * delta[i];
        fw(&weights[i][winner], weight);
    }
}

int learn1()
{
    /* initialize weights randomly: this is done only once */
    init_weight();
    printf("learn1\n");
    return 1;
}

int learn2()
{
    int iter = ir(&iteration);

    int winner = feed_forward();
    update_weight(winner);

    iter++;
    if (iter >= MAX_ITER) {
        iw(&iteration, 0);
        //int sn = ir(&num_selected_input);
        //iw(&num_selected_input, sn-1);
        return 1;
    } else {
        iw(&iteration, iter);
    }

    printf("learn2\n");
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
    printf("infer\n");
    int winner = feed_forward();
    return winner;
}

int (*sense_[])() = { sense };
int (*preprocess_[])() = { preprocess };
int (*select_[])() = { select };
int (*store_[])() = { store };
int (*pick_[])() = { pick };
int (*learnable_[])() = { learnable };
int (*extract_[])() = { extract };
int (*learn_[])() = { learn1, learn2 };
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
