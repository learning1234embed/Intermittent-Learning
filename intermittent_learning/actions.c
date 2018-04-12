#include <stdio.h>

int sense()
{
    printf("sense\n");
    /* needs to be programmed by a user */
    return 1;
}

int preprocess()
{
    printf("preprocess\n");
    /* needs to be programmed by a user */
    return 1;
}

int select()
{
    printf("select\n");
    /* needs to be programmed by a user */
    return 1;
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

int learn1()
{
    printf("learn1\n");
    /* needs to be programmed by a user */
    return 1;
}

int learn2()
{
    printf("learn2\n");
    /* needs to be programmed by a user */
    return 1;
}

int learn3()
{
    printf("learn3\n");
    /* needs to be programmed by a user */
    return 1;
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
    /* needs to be programmed by a user */
    return 1;
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
