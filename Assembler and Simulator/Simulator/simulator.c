/**
 * Project 1
 * EECS 370 LC-2K Instruction-level simulator
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

const int mask = 0b111;

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);
int convertNum(int);

void initialize(stateType *state)
{
    state->pc = 0;
    for (int i = 0; i < NUMREGS; i++)
    {
        state->reg[i] = 0;
    }
}

int get_Opcode(stateType* state, int pc)
{
    return ((mask << 22 & state->mem[pc]) >> 22);
}

int get_regA(stateType* state, int pc)
{
    return ((mask << 19 & state->mem[pc]) >> 19);
}

int get_regB(stateType* state, int pc)
{
    return ((mask << 16 & state->mem[pc]) >> 16);
}

int get_destReg(stateType* state, int pc)
{
    return mask & state->mem[pc];
}

int get_offsetField(stateType* state, int pc)
{
    //use convertnum
    int temp = 0b1111111111111111;
    int num = state->mem[pc] & temp;
    return convertNum(num);
}

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
    int num_instructions = 0;
    int regA, regB, regD;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    initialize(&state);

    /* read the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
            state.numMemory++) {

        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    while (state.pc < state.numMemory)
    {
        int num = 0;
        regA = regB = regD = 0;
        int opcode = get_Opcode(&state, state.pc);
        num_instructions++;
        printState(&state);
        if (opcode == 0)
        {
            //state.reg[get_destReg(&state, state.pc)] = state.reg[get_regA(&state, state.pc)] + state.reg[get_regB(&state, state.pc)];
            regD = get_destReg(&state, state.pc);
            regA = get_regA(&state, state.pc);
            regB = get_regB(&state, state.pc);
            num = state.reg[regA] + state.reg[regB];
            state.reg[regD] = num;
            state.pc++;
        }
        else if (opcode == 1)
        {
            num = state.reg[get_regA(&state, state.pc)] | state.reg[get_regB(&state, state.pc)];
            num = ~num;
            state.reg[get_destReg(&state, state.pc)] = num;
            state.pc++;
            
        }
        else if (opcode == 2)
        {
            num = state.reg[get_regA(&state, state.pc)] + get_offsetField(&state, state.pc);
            state.reg[get_regB(&state, state.pc)] = state.mem[num];
            state.pc++;
        }
        else if (opcode == 3)
        {
            num = state.reg[get_regA(&state, state.pc)] + get_offsetField(&state, state.pc);
            state.mem[num] = state.reg[get_regB(&state, state.pc)];
            state.pc++;
        }
        else if (opcode == 4)
        {
            if (state.reg[get_regA(&state, state.pc)] == state.reg[get_regB(&state, state.pc)])
            {
                state.pc = state.pc + 1 + get_offsetField(&state, state.pc);
            }
            else
            {
                state.pc++;
            }
        }
        else if (opcode == 5)
        {
            state.reg[get_regB(&state, state.pc)] = state.pc + 1;
            state.pc = state.reg[get_regA(&state, state.pc)];
        }
        else if (opcode == 6)
        {
            state.pc++;
            printf("machine halted\n");
            printf("total of %d instructions executed\n",num_instructions);
            printf("final state of machine:\n");
            printState(&state);
            exit(0);
        }
        else if (opcode == 7)
        {
            state.pc++;
        }
        else exit(1);
    }
    return(0);
}

void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
              printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
              printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

int convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit Linux integer */
    if (num & (1<<15) ) {
        num -= (1<<16);
    }
    return(num);
}



