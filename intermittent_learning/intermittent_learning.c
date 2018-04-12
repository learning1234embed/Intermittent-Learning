#include <msp430.h>
#include <stdio.h>
#include "driverlib.h"
#include "actions.h"

#define NUM_ACTION              (11)
#define MAX_NUM_EACH_ACTION     (10)
#define PERFORMANCE_THRESHOLD   (100)
#define LEARN_INFER_RATIO_THRESHOLD    (0.3)

struct actions_ {
    int (*last_action_)();
    int last_action;
    int last_num[NUM_ACTION];
    int returns[NUM_ACTION];
    int num[NUM_ACTION];
    int (*action_[NUM_ACTION][MAX_NUM_EACH_ACTION])();
};

#pragma PERSISTENT(actions)
struct actions_ actions = { 0, };

#pragma PERSISTENT(num_learned)
long num_learned = 0;

#pragma PERSISTENT(num_inferred)
long num_inferred = 0;

#pragma PERSISTENT(learn_infer_ratio)
float learn_infer_ratio = 0;

#pragma PERSISTENT(learning_performance)
int learning_performance = 0;


long lr(long *addr)
{
    return __data20_read_long((unsigned long int)addr);
}

void lw(long *addr, long i)
{
    __data20_write_short((unsigned long int)addr, i);
}

int ir(int *addr)
{
    int temp = __data20_read_short((unsigned long int)addr);
    int *i = (int *)&temp;
    return *i;
}

void iw(int *addr, int f)
{
    int *i = (int *)&f;
    __data20_write_short((unsigned long int)addr, *i);
}

float fr(float *addr)
{
    unsigned long temp = __data20_read_long((unsigned long int)addr);
    float *f = (float *)&temp;
    return *f;
}

void fw(float *addr, float f)
{
    uint32_t *i = (uint32_t *)&f;
    __data20_write_long((unsigned long int)addr, *i);
}

int execute(long action_address)
{
    int (*action)() = (int (*)())action_address;
    return (*action)();
}

int (*state_diagram())()
{
    int last_action = ir(&actions.last_action);
    int returns = ir(&actions.returns[last_action]);
    int new_last_action;
    const int new_last_num = 0;
    int infer = 0; /* 1=infer, 0=lern */

    if (ir(&learning_performance) >= PERFORMANCE_THRESHOLD) {
        infer = 1;
    } else if (fr(&learn_infer_ratio) < LEARN_INFER_RATIO_THRESHOLD && lr(&num_learned) > 0) {
        infer = 1;
    }

    switch (last_action) {
    case 0: case 3: case 7:
        new_last_action = last_action + 1;
        break;

    case 1:
        switch (infer) {
        case 0:
            new_last_action = last_action + 1;
            break;
        case 1:
            new_last_action = 6;
            break;
        }
        break;

    case 2: case 4: case 5:
        switch (returns) {
        case 1:
            new_last_action = last_action+1;
            break;
        default:
            new_last_action = 0;
            break;
        }
        break;

    case 6:
        switch (infer) {
        case 0:
            new_last_action = last_action + 1;
            break;
        case 1:
            new_last_action = 10;
            break;
        }
        break;

    default:
        new_last_action = 0;
        break;
    }

    iw(&actions.last_action, new_last_action);
    iw(&actions.last_num[ir(&actions.last_action)], new_last_num);
    lw(&actions.last_action_, lr(&actions.action_[ir(&actions.last_action)][ir(&actions.last_num[ir(&actions.last_action)])]));

    return lr(&actions.last_action_);
}

int (*DAP())()
{
    /* in the beginning, execute sense */
    if (lr(&actions.last_action_) == 0) {
        iw(&actions.last_action, 0);
        iw(&actions.last_num[ir(&actions.last_action)], 0);
        lw(&actions.last_action_, lr(&actions.action_[ir(&actions.last_action)][ir(&actions.last_num[ir(&actions.last_action)])]));
        return lr(&actions.last_action_);
    }

    /* execute the last action if it did not finish yet */
    if (ir(&actions.last_num[ir(&actions.last_action)]) < ir(&actions.num[ir(&actions.last_action)]))
        return lr(&actions.action_[ir(&actions.last_action)][ir(&actions.last_num[ir(&actions.last_action)])]);

    /* last action is done, set last_num = 0 */
    iw(&actions.last_num[ir(&actions.last_action)], 0);

    int last_action = ir(&actions.last_action);
    if (last_action == 7) {
        long num_learned_example = lr(&num_learned) + 1;
        long num_inferred_example = lr(&num_inferred);
        float ratio = (float)num_inferred_example / (float)num_learned_example;
        lw(&num_learned, num_learned_example);
        fw(&learn_infer_ratio, ratio);
        printf("%ld %ld %f\n", num_learned_example, num_inferred_example, ratio);
    } else if (last_action == 8) {
        iw(&learning_performance, ir(&actions.returns[last_action]));
        printf("%d\n", ir(&learning_performance));
    } else if (last_action == 10) {
        long num_learned_example = lr(&num_learned);
        long num_inferred_example = lr(&num_inferred) + 1;
        float ratio = (float)num_inferred_example / (float)num_learned_example;
        lw(&num_inferred, num_learned_example);
        fw(&learn_infer_ratio, ratio);
        printf("%ld %ld %f\n", num_learned_example, num_inferred_example, ratio);
    }

    /* select the next action */
    int (*action)() = state_diagram();

    return action;
}

void init_actions()
{
    int actions_num;
    unsigned int i;

    iw(&actions.num[0], get_num_sense());
    iw(&actions.num[1], get_num_preprocess());
    iw(&actions.num[2], get_num_select());
    iw(&actions.num[3], get_num_store());
    iw(&actions.num[4], get_num_pick());
    iw(&actions.num[5], get_num_learnable());
    iw(&actions.num[6], get_num_extract());
    iw(&actions.num[7], get_num_learn());
    iw(&actions.num[8], get_num_evaluate());
    iw(&actions.num[9], get_num_expect());
    iw(&actions.num[10], get_num_infer());

    actions_num = ir(&actions.num[0]);
    for (i = 0; i < actions_num; i++) {
        lw(&actions.action_[0][i], (long)get_sense(i));
    }

    actions_num = ir(&actions.num[1]);
    for (i = 0; i < actions_num; i++) {
        lw(&actions.action_[1][i], (long)get_preprocess(i));
    }

    actions_num = ir(&actions.num[2]);
    for (i = 0; i < actions_num; i++) {
        lw(&actions.action_[2][i], (long)get_select(i));
    }

    actions_num = ir(&actions.num[3]);
    for (i = 0; i < actions_num; i++) {
        lw(&actions.action_[3][i], (long)get_store(i));
    }

    actions_num = ir(&actions.num[4]);
    for (i = 0; i < actions_num; i++) {
        lw(&actions.action_[4][i], (long)get_pick(i));
    }

    actions_num = ir(&actions.num[5]);
    for (i = 0; i < actions_num; i++) {
        lw(&actions.action_[5][i], (long)get_learnable(i));
    }

    actions_num = ir(&actions.num[6]);
    for (i = 0; i < actions_num; i++) {
        lw(&actions.action_[6][i], (long)get_extract(i));
    }

    actions_num = ir(&actions.num[7]);
    for (i = 0; i < actions_num; i++) {
        lw(&actions.action_[7][i], (long)get_learn(i));
    }

    actions_num = ir(&actions.num[8]);
    for (i = 0; i < actions_num; i++) {
        lw(&actions.action_[8][i], (long)get_evaluate(i));
    }

    actions_num = ir(&actions.num[9]);
    for (i = 0; i < actions_num; i++) {
        lw(&actions.action_[9][i], (long)get_expect(i));
    }

    actions_num = ir(&actions.num[10]);
    for (i = 0; i < actions_num; i++) {
        lw(&actions.action_[10][i], (long)get_infer(i));
    }
}

void init_MCU()
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR = 0xFF;
    P1OUT = 0x00;
    P2DIR = 0xFF;
    P2OUT = 0x00;
    P3DIR = 0xFF;
    P3OUT = 0x00;
    P4DIR = 0xFF;
    P4OUT = 0x00;
    P5DIR = 0xFF;
    P5OUT = 0x00;
    P6DIR = 0xFF;
    P6OUT = 0x00;
    P7DIR = 0xFF;
    P7OUT = 0x00;
    P8DIR = 0xFF;
    P8OUT = 0x00;
    PADIR = 0xFF;
    PAOUT = 0x00;
    PBDIR = 0xFF;
    PBOUT = 0x00;
    PCDIR = 0xFF;
    PCOUT = 0x00;
    PDDIR = 0xFF;
    PDOUT = 0x00;

#if 1
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;

    // Clock System Setup
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz
    // Set SMCLK = MCLK = DCO, ACLK = VLOCLK
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    // Per Device Errata set divider to 4 before changing frequency to
    // prevent out of spec operation from overshoot transient
    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4;   // Set all corresponding clk sources to divide by 4 for errata
    CSCTL1 = DCOFSEL_4 | DCORSEL;           // Set DCO to 16MHz
    // Delay by ~10us to let DCO settle. 60 cycles = 20 cycles buffer + (10us / (1/4MHz))
    __delay_cycles(60);
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers to 1 for 16MHz operation
    CSCTL0_H = 0;
#endif

    PMM_unlockLPM5();
}

int main(void)
{
    init_MCU();
    init_actions();

    /* LED for test */
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN1);

    /* setup interrupt */
    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_P1, GPIO_PIN3);
    GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN3, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN3);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN3);

    _BIS_SR(LPM4_bits + GIE);
}

void intermittent_learn()
{
    int (*action_)() = DAP();
    iw(&actions.returns[ir(&actions.last_action)], (*action_)());

    /* 1=yes, 0=no, -1=functional error */
    if (ir(&actions.returns[ir(&actions.last_action)]) <= 0)
        printf("ret = %d\n", ir(&actions.returns[ir(&actions.last_action)]));

    if (ir(&actions.returns[ir(&actions.last_action)]) >= 0) {
        iw(&actions.last_num[ir(&actions.last_action)], ir(&actions.last_num[ir(&actions.last_action)])+1);
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    /* LED for test */
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);

    while (true) {
        uint8_t status = GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN3);
        if (status == 0)
            break;

        intermittent_learn();
    }

    /* LED for test */
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN1);

    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN3);
}
