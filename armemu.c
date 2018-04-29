#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int sum_array_a(int *array, int n);
int find_max_a(int *array, int n);
int fib_iter_a(int n);
int fib_rec_a(int n);
int find_str_a(char *s, char *sub);

void emulate_sum_array();
void emulate_find_max();
void emulate_fib_iter();
void emulate_fib_rec();
void emulate_find_str();

#define NREGS 16
#define SP 13
#define LR 14 
#define PC 15

struct arm_state 
    {
    unsigned int regs[NREGS];
    unsigned int cpsr;
    unsigned int stack_size;
    unsigned char *stack;
    unsigned int condField[4];
        /* conField[0] = eq */
        /* conField[1] = ne */
        /* conField[2] = lt */
        /* conField[3] = gt */
    unsigned int instr_comput;
    unsigned int instr_memory;
    unsigned int instr_branch;
    };

struct arm_state *arm_state_new(unsigned int stack_size, unsigned int *func,
                                unsigned int arg0, unsigned int arg1,
                                unsigned int arg2, unsigned int arg3)
    {
    struct arm_state *as;
    int i;

    as = (struct arm_state *) malloc(sizeof(struct arm_state));
    if(as == NULL) 
        {
        printf("malloc() failed, exiting.\n");
        exit(-1);
        }

    as->stack = (unsigned char *) malloc(stack_size);
    if(as->stack == NULL) 
        {
        printf("malloc() failed, exiting.\n");
        exit(-1);
        }
    
    as->stack_size = stack_size;

    /* Initialize all registers to zero. */
    for(i = 0; i < NREGS; i++) 
        {
        as->regs[i] = 0;
        }

    as->regs[PC] = (unsigned int) func;
    as->regs[SP] = (unsigned int) as->stack + as->stack_size;
    as->regs[LR] = 0;

    as->regs[0] = arg0;
    as->regs[1] = arg1;
    as->regs[2] = arg2;
    as->regs[3] = arg3;

    as->condField[0] = 0;        /* eq */
    as->condField[1] = 0;        /* ne */
    as->condField[2] = 0;        /* lt */
    as->condField[3] = 0;        /* gt */

    as->instr_comput = 0;
    as->instr_memory = 0;
    as->instr_branch = 0;

    return as;
    }

void arm_state_free(struct arm_state *as)
    {
    free(as->stack);
    free(as);
    }

void arm_state_print(struct arm_state *as)
    {
    int i;
    
    printf("stack size = %d\n", as->stack_size);
    for(i = 0; i < NREGS; i++) 
        {
        printf("regs[%d] = (%X) %d\n", i, as->regs[i], (int) as->regs[i]);
        }
    }

bool iw_is_mov_instruction(unsigned int iw)
    {
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 26) & 0b11;
    opcode = (iw >> 21) & 0b1111;

    return (op == 0) && (opcode == 13);
    }

void execute_mov_instruction(struct arm_state *as, unsigned int iw)
    {
    as->instr_comput++;

    unsigned int rd;
    unsigned int rn;
    unsigned int rm;
    unsigned int i;                   /* immediate */
    unsigned int cond;                /* condition field */
    bool execute = false;

    rd = (iw >> 12) & 0b1111;
    rn = (iw >> 16) & 0b1111;
    i = (iw >> 25) & 0b1;
    cond = (iw >> 28) & 0b1111;

    if(cond == 0b1110)
        {
        execute = true;
        }
    else if((cond == 0b0000) && (as->condField[0] == 1))
        {
        execute = true;
        }
    else if((cond == 0b1011) && (as->condField[2] == 1))
        {
        execute = true;
        }

    if(execute)
        {
        if(i == 0)
            {
            rm = (iw & 0b11111111);
            as->regs[rd] = as->regs[rm];
            }
        else if(i == 1)
            {
            rm = (iw & 0b1111);
            as->regs[rd] = rm;
            }
        }

    as->regs[PC] = as->regs[PC] + 4;
    }

bool iw_is_add_instruction(unsigned int iw)
    {
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 26) & 0b11;
    opcode = (iw >> 21) & 0b1111;

    return (op == 0) && (opcode == 4);
    }

void execute_add_instruction(struct arm_state *as, unsigned int iw)
    {
    as->instr_comput++;

    unsigned int rd;
    unsigned int rn;
    unsigned int rm;
    unsigned int i;                   /* immediate */

    rd = (iw >> 12) & 0b1111;
    rn = (iw >> 16) & 0b1111;
    i = (iw >> 25) & 0b1;

    if(i == 0)
        {
        rm = iw & 0b1111;
        as->regs[rd] = as->regs[rn] + as->regs[rm];
        }
    else if(i == 1)
        {
        rm = iw & 0b11111111;
        as->regs[rd] = as->regs[rn] + rm;
        }

    as->regs[PC] = as->regs[PC] + 4;
    }

bool iw_is_sub_instruction(unsigned int iw)
    {
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 26) & 0b11;
    opcode = (iw >> 21) & 0b1111;

    return (op == 0) && (opcode == 2);
    }

void execute_sub_instruction(struct arm_state *as, unsigned int iw)
    {
    as->instr_comput++;

    unsigned int rd;
    unsigned int rn;
    unsigned int rm;
    unsigned int i;                   /* immediate */

    rd = (iw >> 12) & 0b1111;
    rn = (iw >> 16) & 0b1111;
    i = (iw >> 25) & 0b1;

    if(i == 0)
        {
        rm = iw & 0b1111;
        as->regs[rd] = as->regs[rn] - as->regs[rm];
        }
    else if(i == 1)
        {
        rm = iw & 0b11111111;
        as->regs[rd] = as->regs[rn] - rm;
        }

    as->regs[PC] = as->regs[PC] + 4;
    }

bool iw_is_cmp_instruction(unsigned int iw)
    {
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 26) & 0b11;
    opcode = (iw >> 21) & 0b1111;

    return (op == 0) && (opcode == 10);
    }

void execute_cmp_instruction(struct arm_state *as, unsigned int iw)
    {
    as->instr_comput++;

    unsigned int rd;
    unsigned int rn;
    unsigned int rm;
    unsigned int i;                   /* immediate */
    unsigned int comparison;          /* set conditions for eq and ne */
    unsigned int opCode1, opCode2;    /* set conditions for lt and gt */
    /* Reset conditions for next conditional instructions */
    as->condField[0] = 0;             /* eq */
    as->condField[1] = 0;             /* ne */
    as->condField[2] = 0;             /* lt */
    as->condField[3] = 0;             /* gt */

    rd = (iw >> 12) & 0b1111;
    rn = (iw >> 16) & 0b1111;
    i = (iw >> 25) & 0b1;

    if(i == 0)
        {
        rm = iw & 0b11111111;
        comparison = as->regs[rm] - as->regs[rn];
        opCode1 = as->regs[rn];
        opCode2 = as->regs[rm];
        }
    else if(i == 1)
        {
        rm = iw & 0b1111;
        comparison = as->regs[rn] - rm;
        opCode1 = as->regs[rn];
        opCode2 = rm;
        }

    if(comparison == 0)
        {
        as->condField[0] = 1;
        }
    else
        {
        as->condField[1] = 1;
        }

    if((signed int)opCode1 < (signed int)opCode2)
        {
        as->condField[2] = 1;
        }
    else
        {
        as->condField[2] = 0;
        }

    if((signed int)opCode1 > (signed int)opCode2)
        {
        as->condField[3] = 1;
        }
    else
        {
        as->condField[3] = 0;
        }

    as->regs[PC] = as->regs[PC] + 4;
    }

bool iw_is_ldr_instruction(unsigned int iw)
    {
    unsigned int op;
    unsigned int lBit;
    unsigned int bBit;

    op = (iw >> 26) & 0b11;
    lBit = (iw >> 20) & 0b1;
    bBit = (iw >> 22) & 0b1;

    return (op == 1) && (lBit == 1) && (bBit == 0);
    }

void execute_ldr_instruction(struct arm_state *as, unsigned int iw)
    {
    as->instr_memory++;

    unsigned int rd;
    unsigned int rn;
    unsigned int i;                   /* immediate */
    unsigned int *target;

    rd = (iw >> 12) & 0b1111;
    rn = (iw >> 16) & 0b1111;
    i = (iw >> 25) & 0b1;

    if(i == 0)
        {
        target = (unsigned int *) as->regs[rn];
        as->regs[rd] = (unsigned int) *target;
        }

    as->regs[PC] = as->regs[PC] + 4;
    }

bool iw_is_str_instruction(unsigned int iw)
    {
    unsigned int op;
    unsigned int lBit;
    unsigned int bBit;

    op = (iw >> 26) & 0b11;
    lBit = (iw >> 20) & 0b1;
    bBit = (iw >> 22) & 0b1;

    return (op == 1) && (lBit == 0) && (bBit == 0);
    }

void execute_str_instruction(struct arm_state *as, unsigned int iw)
    {
    as->instr_memory++;

    unsigned int rd;
    unsigned int rn;
    unsigned int rm;
    unsigned int i;
    unsigned int *target;

    rd = (iw >> 12) & 0b1111;
    rn = (iw >> 16) & 0b1111;
    i = (iw >> 25) & 0b1;

    if(i == 0)
        {
        unsigned int *target = (unsigned int *)as->regs[rn];
        *target = as->regs[rd];
        }

    as->regs[PC] = as->regs[PC] + 4;
    }

bool iw_is_ldrb_instruction(unsigned int iw)
    {
    unsigned int op;
    unsigned int lBit;
    unsigned int bBit;

    op = (iw >> 26) & 0b11;
    lBit = (iw >> 20) & 0b1;
    bBit = (iw >> 22) & 0b1;

    return (op == 1) && (lBit == 1) && (bBit == 1);
    }

void execute_ldrb_instruction(struct arm_state *as, unsigned int iw)
    {
    as->instr_memory++;

    unsigned int rd;
    unsigned int rn;
    unsigned int rm;
    unsigned int i;
    unsigned char *target;

    rd = (iw >> 12) & 0b1111;
    rn = (iw >> 16) & 0b1111;
    i = (iw >> 25) & 0b1;

    target = (unsigned char *) as->regs[rn];

    if(i == 0)
        {
        rm = iw & 0b11111111;
        as->regs[rd] = ((unsigned int) *target + rm);
        }
    else if(i == 1)
        {
        rm = iw & 0b1111;
        as->regs[rd] = (unsigned int)*(target + as->regs[rm]);
        }

    as->regs[PC] = as->regs[PC] + 4;
    }

bool iw_is_b_instruction(unsigned int iw)
    {
    unsigned int op;
    unsigned int linkBit;

    op = (iw >> 26) & 0b11;
    linkBit = (iw >> 24) & 0b1;

    return (op == 2) && (linkBit == 0);
    }

void execute_b_instruction(struct arm_state *as, unsigned int iw)
    {
    as->instr_branch++;

    unsigned int offset;
    unsigned int cond;                /* condition field */
    unsigned int instr;
    bool execute = false;

    offset = (iw & 0xFFFFFF);
    cond = (iw >> 28) & 0b1111;

    if(cond == 0b1110)
        {
        execute = true;
        }
    else if((cond == 0b0000) && (as->condField[0] == 1))
        {
        execute = true;
        }
    else if((cond == 0b0001) && (as->condField[1] == 1))
        {
        execute = true;
        }
    else if((cond == 0b1011) && (as->condField[2] == 1))
        {
        execute = true;
        }
    else if((cond == 0b1100) && (as->condField[3] == 1))
        {
        execute = true;
        }

    if(execute)
        {
        if(offset & 0x800000)
            {
            instr = 0xFF000000 + offset;
            }
        else
            {
            instr = offset;
            }
        /* Branch instructions contain a signed 2’s complement 24 bit offset */
        /* This is shifted left two bits, sign extended to 32 bits, and added to the PC */
        instr  = instr <<  2;
        as->regs[PC] = as->regs[PC] + 8 + instr;
        }
    else
        {
        as->regs[PC] = as->regs[PC] + 4;
        }

    }

bool iw_is_bl_instruction(unsigned int iw)
    {
    unsigned int op;
    unsigned int linkBit;

    op = (iw >> 26) & 0b11;
    linkBit = (iw >> 24) & 0b1;

    return (op == 0b10) && (linkBit == 0b1);
    }

void execute_bl_instruction(struct arm_state *as, unsigned int iw)
    {
    as->instr_branch++;

    unsigned int offset;
    unsigned int instr;

    offset = (iw & 0xFFFFFF);
    instr = 0xFF000000 | offset;
    /* Branch instructions contain a signed 2’s complement 24 bit offset */
    /* This is shifted left two bits, sign extended to 32 bits, and added to the PC */
    instr  = instr <<  2;
    /* Branch with Link (BL) writes the old PC into the link register (R14) of the current bank */
    as->regs[LR] = as->regs[PC] + 4;
    as->regs[PC] = as->regs[PC] + 8 + instr;
    }

bool iw_is_bx_instruction(unsigned int iw)
    {
    return ((iw >> 4) & 0xFFFFFF) == 0b000100101111111111110001;
    }

void execute_bx_instruction(struct arm_state *as, unsigned int iw)
    {
    as->instr_branch++;

    unsigned int rn;
    rn = iw & 0b1111;
    as->regs[PC] = as->regs[rn];
    }

void arm_state_execute_one(struct arm_state *as)
    {
    unsigned int iw;
    unsigned int *pc;

    pc = (unsigned int *) as->regs[PC];
    iw = *pc;
        
    if(iw_is_mov_instruction(iw))
        {
        execute_mov_instruction(as, iw);
        }
    else if(iw_is_add_instruction(iw)) 
        {
        execute_add_instruction(as, iw);
        }
    else if(iw_is_sub_instruction(iw))
        {
        execute_sub_instruction(as, iw);
        }
    else if(iw_is_cmp_instruction(iw))
        {
        execute_cmp_instruction(as, iw);
        }
    else if(iw_is_ldr_instruction(iw))
        {
        execute_ldr_instruction(as, iw);
        }
    else if(iw_is_str_instruction(iw))
        {
        execute_str_instruction(as, iw);
        }
    else if(iw_is_ldrb_instruction(iw))
        {
        execute_ldrb_instruction(as, iw);
        }
    else if(iw_is_b_instruction(iw)) 
        {
        execute_b_instruction(as, iw);
        }
    else if(iw_is_bl_instruction(iw)) 
        {
        execute_bl_instruction(as, iw);
        }
    else if(iw_is_bx_instruction(iw)) 
        {
        execute_bx_instruction(as, iw);
        }
    }

unsigned int arm_state_execute(struct arm_state *as)
    {
    while(as->regs[PC] != 0) 
        {
        arm_state_execute_one(as);
        }

    return as->regs[0];
    }

void emulate_sum_array()
    {
    printf("\t\tEmulating sum_array\n");
    struct arm_state *as;
    unsigned int rv;

    int array[5] = {1, 2, 3, 4, -5};

    as = arm_state_new(1024, (unsigned int *) sum_array_a, (unsigned int)array, 5, 0, 0);
    rv = arm_state_execute(as);
    printf("Arm emulation of sum array: %d\n", rv);
    printf("Arm code of sum array: %d\n", sum_array_a(array, 5));

    printf("\n\t\tData Analysis\n");
    unsigned int total_instr = as->instr_comput + as->instr_memory + as->instr_branch;
    printf("Total number of instructions executed: %d\n", total_instr);
    printf("\t-Total number of computation instructions: %d\n", as->instr_comput);
    printf("\t-Total number of memory instructions: %d\n", as->instr_memory);
    printf("\t-Total number of branch instructions: %d\n\n\n", as->instr_branch);

    arm_state_free(as);
    }

void emulate_find_max()
    {
    printf("\t\tEmulating find_max\n");
    struct arm_state *as;
    unsigned int rv;

    int array[6] = {1, 12, 3, 4, -5, 6};

    as = arm_state_new(1024, (unsigned int *) find_max_a, (unsigned int)array, 6, 0, 0);
    rv = arm_state_execute(as);
    printf("Arm emulation of find max in array: %d\n", rv);
    printf("Arm code of find max in array: %d\n", find_max_a(array, 5));

    printf("\n\t\tData Analysis\n");
    unsigned int total_instr = as->instr_comput + as->instr_memory + as->instr_branch;
    printf("Total number of instructions executed: %d\n", total_instr);
    printf("\t-Total number of computation instructions: %d\n", as->instr_comput);
    printf("\t-Total number of memory instructions: %d\n", as->instr_memory);
    printf("\t-Total number of branch instructions: %d\n\n\n", as->instr_branch);

    arm_state_free(as);
    }

void emulate_fib_iter()
    {
    printf("\t\tEmulating fib_iter\n");
    struct arm_state *as;
    unsigned int rv;

    as = arm_state_new(1024, (unsigned int *) fib_iter_a, 20, 0, 0, 0);
    rv = arm_state_execute(as);
    printf("Arm emulation of fibonacci iteration: %d\n", rv);
    printf("Arm code of fibonacci iteration: %d\n", fib_iter_a(20));

    printf("\n\t\tData Analysis\n");
    unsigned int total_instr = as->instr_comput + as->instr_memory + as->instr_branch;
    printf("Total number of instructions executed: %d\n", total_instr);
    printf("\t-Total number of computation instructions: %d\n", as->instr_comput);
    printf("\t-Total number of memory instructions: %d\n", as->instr_memory);
    printf("\t-Total number of branch instructions: %d\n\n\n", as->instr_branch);

    arm_state_free(as);
    }

void emulate_fib_rec()
    {
    printf("\t\tEmulating fib_rec\n");
    struct arm_state *as;
    unsigned int rv;

    as = arm_state_new(1024, (unsigned int *) fib_rec_a, 7, 0, 0, 0);
    rv = arm_state_execute(as);
    printf("Arm emulation of fibonacci recursion: %d\n", rv);
    printf("Arm code of fibonacci recursion: %d\n", fib_rec_a(7));

    printf("\n\t\tData Analysis\n");
    unsigned int total_instr = as->instr_comput + as->instr_memory + as->instr_branch;
    printf("Total number of instructions executed: %d\n", total_instr);
    printf("\t-Total number of computation instructions: %d\n", as->instr_comput);
    printf("\t-Total number of memory instructions: %d\n", as->instr_memory);
    printf("\t-Total number of branch instructions: %d\n\n\n", as->instr_branch);

    arm_state_free(as);
    }

void emulate_find_str()
    {
    printf("\t\tEmulating find_str\n");
    struct arm_state *as;
    unsigned int rv;

    char* a = "This is my sample example";
    char* d = "This";

    as = arm_state_new(1024, (unsigned int *) find_str_a, (unsigned int)a, (unsigned int)d, 0, 0);
    rv = arm_state_execute(as);
    printf("Arm emulation of find string: %d\n", rv);
    printf("Arm code of find string: %d\n", find_str_a(a, d));

    printf("\n\t\tData Analysis\n");
    unsigned int total_instr = as->instr_comput + as->instr_memory + as->instr_branch;
    printf("Total number of instructions executed: %d\n", total_instr);
    printf("\t-Total number of computation instructions: %d\n", as->instr_comput);
    printf("\t-Total number of memory instructions: %d\n", as->instr_memory);
    printf("\t-Total number of branch instructions: %d\n\n\n", as->instr_branch);

    arm_state_free(as);
    }
     
int main(int argc, char **argv)
    {
    emulate_sum_array();
    emulate_find_max();
    emulate_fib_iter();
    emulate_fib_rec();
    emulate_find_str();
    return 0;
    }