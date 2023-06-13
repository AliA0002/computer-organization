/**
 * Project 1 
 * Assembler code fragment for LC-2K 
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000
#define NUMMEMORY 1000//65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */

#define ADD 0
#define NOR 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 /* JALR will not implemented for Project 3 */
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

typedef struct IFIDStruct {
    int instr;
    int pcPlus1;
} IFIDType;

typedef struct IDEXStruct {
    int instr;
    int pcPlus1;
    int readRegA;
    int readRegB;
    int offset;
} IDEXType;

typedef struct EXMEMStruct {
    int instr;
    int branchTarget;
    int aluResult;
    int readRegB;
} EXMEMType;

typedef struct MEMWBStruct {
    int instr;
    int writeData;
} MEMWBType;

typedef struct WBENDStruct {
    int instr;
    int writeData;
} WBENDType;

typedef struct stateStruct {
    int pc;
    int instrMem[NUMMEMORY];
    int dataMem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
    IFIDType IFID;
    IDEXType IDEX;
    EXMEMType EXMEM;
    MEMWBType MEMWB;
    WBENDType WBEND;
    int cycles; /* number of cycles run so far */
} stateType;

int convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit integer */
    if (num & (1 << 15)) {
        num -= (1 << 16);
    }
    return num;
}

int field0(int instruction)
{
    return((instruction >> 19) & 0x7);
}

int field1(int instruction)
{
    return((instruction >> 16) & 0x7);
}

int field2(int instruction)
{
    return(instruction & 0xFFFF);
}

int opcode(int instruction)
{
    return(instruction >> 22);
}

int isLW(int instr)
{
    if (opcode(instr) == LW) return 1;
    else return 0;
}

int isADD(int instr)
{
    if (opcode(instr) == ADD) return 1;
    else return 0;
}

int isNOR(int instr)
{
    if (opcode(instr) == NOR) return 1;
    else return 0;
}

void printInstruction(int instr)
{

    char opcodeString[10];

    if (opcode(instr) == ADD) {
        strcpy(opcodeString, "add");
    }
    else if (opcode(instr) == NOR) {
        strcpy(opcodeString, "nor");
    }
    else if (opcode(instr) == LW) {
        strcpy(opcodeString, "lw");
    }
    else if (opcode(instr) == SW) {
        strcpy(opcodeString, "sw");
    }
    else if (opcode(instr) == BEQ) {
        strcpy(opcodeString, "beq");
    }
    else if (opcode(instr) == JALR) {
        strcpy(opcodeString, "jalr");
    }
    else if (opcode(instr) == HALT) {
        strcpy(opcodeString, "halt");
    }
    else if (opcode(instr) == NOOP) {
        strcpy(opcodeString, "noop");
    }
    else {
        strcpy(opcodeString, "data");
    }
    printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
        field2(instr));
}

void printState(stateType* statePtr)
{
    int i;
    printf("\n@@@\nstate before cycle %d starts\n", statePtr->cycles);
    printf("\tpc %d\n", statePtr->pc);

    printf("\tdata memory:\n");
    for (i = 0; i < statePtr->numMemory; i++) {
        printf("\t\tdataMem[ %d ] %d\n", i, statePtr->dataMem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("\tIFID:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IFID.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IFID.pcPlus1);
    printf("\tIDEX:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IDEX.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IDEX.pcPlus1);
    printf("\t\treadRegA %d\n", statePtr->IDEX.readRegA);
    printf("\t\treadRegB %d\n", statePtr->IDEX.readRegB);
    printf("\t\toffset %d\n", statePtr->IDEX.offset);
    printf("\tEXMEM:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->EXMEM.instr);
    printf("\t\tbranchTarget %d\n", statePtr->EXMEM.branchTarget);
    printf("\t\taluResult %d\n", statePtr->EXMEM.aluResult);
    printf("\t\treadRegB %d\n", statePtr->EXMEM.readRegB);
    printf("\tMEMWB:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->MEMWB.instr);
    printf("\t\twriteData %d\n", statePtr->MEMWB.writeData);
    printf("\tWBEND:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->WBEND.instr);
    printf("\t\twriteData %d\n", statePtr->WBEND.writeData);
}

void setValues(stateType* state)
{
    state->IFID.pcPlus1 = -12973480;
    state->IDEX.readRegA = 6;
    state->IDEX.readRegB = 1;
    state->IDEX.offset = 0;
    state->EXMEM.branchTarget = -12974332;
    state->EXMEM.aluResult = -14024712;
    state->EXMEM.readRegB = 12;
    state->MEMWB.writeData = -14040720;
    state->WBEND.writeData = -4262240;
}

int isNumber(char* string)
{
    /* return 1 if string is a number */
    int i;
    return((sscanf(string, "%d", &i)) == 1);
}

void readFileIntoMemory(FILE* fileptr, char* line, stateType* state)
{
    for (state->numMemory = 0; fgets(line, MAXLINELENGTH, fileptr) != NULL; state->numMemory++)
    {
        if (sscanf(line, "%d", state->instrMem + state->numMemory) != 1)
        {
            printf("error in reading address %d\n", state->numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state->numMemory, state->instrMem[state->numMemory]);
    }

    printf("%d memory words\n", state->numMemory);
    printf("\tinstruction memory:\n");
    for (int i = 0; i < state->numMemory; i++)
    {
        printf("\t\tinstrMem[ %d ] ", i);
        printInstruction(state->instrMem[i]);
    }
}

void setEXStage(stateType* state, stateType* newState)
{
    newState->EXMEM.instr = state->IDEX.instr;
    newState->EXMEM.branchTarget = state->IDEX.pcPlus1 + state->IDEX.offset;
}

void initializeRegisters(stateType* state)
{
    for (int i = 0; i < NUMREGS; i++)
    {
        state->reg[i] = 0;
    }
    for (int i = 0; i < state->numMemory; i++)
    {
        state->dataMem[i] = state->instrMem[i];
    }
}

int main(int argc, char* argv[])
{
    stateType state, newState;
    FILE* fileptr;
    char line[MAXLINELENGTH];
    int current;

    if (argc != 2)
    {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    fileptr = fopen(argv[1], "r");

    if (fileptr == NULL)
    {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    readFileIntoMemory(fileptr, line, &state);
    initializeRegisters(&state);
    setValues(&state);

    int regA_num, regB_num, EXEM_dest, MEMWB_dest, WBEND_dest;
    regA_num = regB_num = EXEM_dest = MEMWB_dest = WBEND_dest = 0;


    state.IFID.instr = state.IDEX.instr = state.EXMEM.instr
        = state.MEMWB.instr = state.WBEND.instr = NOOPINSTRUCTION;
    state.cycles = state.pc = 0;

    while (1) {

        printState(&state);

        /* check for halt */
        if (opcode(state.MEMWB.instr) == HALT) {
            printf("machine halted\n");
            printf("total of %d cycles executed\n", state.cycles);
            exit(0);
        }

        newState = state;
        newState.cycles++;

        /* --------------------- IF stage --------------------- */
        newState.IFID.instr = state.instrMem[state.pc];
        current = newState.IFID.instr;
        newState.IFID.pcPlus1 = state.pc + 1;
        newState.pc++;

        //---------------------- ID stage --------------------- 
        newState.IDEX.instr = state.IFID.instr;
        current = newState.IDEX.instr;
        newState.IDEX.pcPlus1 = state.IFID.pcPlus1;

       // ***********************CHECK FOR LW DATA HAZARD*******************
        if (isLW(state.IDEX.instr) && (field0(newState.IDEX.instr) == field1(state.IDEX.instr)
            || field1(newState.IDEX.instr) == field1(state.IDEX.instr)))

        {
            newState.IDEX.instr = NOOPINSTRUCTION;
            newState.pc = state.pc;
            newState.IFID = state.IFID;
        }
        else
        {
            newState.IDEX.offset = convertNum(field2(state.IFID.instr));
            newState.IDEX.readRegA = state.reg[field0(state.IFID.instr)];
            newState.IDEX.readRegB = state.reg[field1(state.IFID.instr)];
        }
        // -------------------- - EX stage---------------------
        int regA_current, regB_current;
        setEXStage(&state, &newState);
        regA_num = state.IDEX.readRegA;
        regB_num = state.IDEX.readRegB;
        current = newState.EXMEM.instr;
        regA_current = field0(newState.EXMEM.instr);
        regB_current = field1(newState.EXMEM.instr);
        //***********************CHECKING FOR HAZARDS**********************

        if (isLW(state.WBEND.instr)) WBEND_dest = field1(state.WBEND.instr);
        else WBEND_dest = field2(state.WBEND.instr);

        if (isLW(state.WBEND.instr) || isADD(state.WBEND.instr)
            || isNOR(state.WBEND.instr))
        {
            if (regA_current == WBEND_dest) regA_num = state.WBEND.writeData;
            if (regB_current == WBEND_dest) regB_num = state.WBEND.writeData;
        }

        if (isLW(state.MEMWB.instr)) MEMWB_dest = field1(state.MEMWB.instr);
        else MEMWB_dest = field2(state.MEMWB.instr);

        if (isLW(state.MEMWB.instr) || isADD(state.MEMWB.instr)
            || isNOR(state.MEMWB.instr))
        {
            if (regA_current == MEMWB_dest) regA_num = state.MEMWB.writeData;
            if (regB_current == MEMWB_dest) regB_num = state.MEMWB.writeData;
        }

        if (isLW(state.EXMEM.instr)) EXEM_dest = field1(state.EXMEM.instr);
        else EXEM_dest = field2(state.EXMEM.instr);

        if (isLW(state.EXMEM.instr) || isADD(state.EXMEM.instr)
            || isNOR(state.EXMEM.instr))
        {
            if (regA_current == EXEM_dest) regA_num = state.EXMEM.aluResult;
            else if (regB_current == EXEM_dest) regB_num = state.EXMEM.aluResult;
        }

        if (opcode(current) == LW)
        {
            newState.EXMEM.aluResult = regA_num + state.IDEX.offset;
        }
        else if (opcode(current) == NOR)
        {
            newState.EXMEM.aluResult = ~(regA_num | regB_num);
        }
        else if (opcode(current) == ADD)
        {
            newState.EXMEM.aluResult = regA_num + regB_num;
        }
        else if (opcode(current) == SW)
        {
            newState.EXMEM.aluResult = regA_num + state.IDEX.offset;
        }
        else if (opcode(current) == BEQ)
        {
            newState.EXMEM.aluResult = regA_num - regB_num;
        }
        if (opcode(current) != NOOP) {
            newState.EXMEM.readRegB = regB_num;
        }
        
        /* --------------------- MEM stage --------------------- */
        newState.MEMWB.instr = state.EXMEM.instr;
        current = newState.MEMWB.instr;

        if (isLW(current)) newState.MEMWB.writeData = state.dataMem[state.EXMEM.aluResult];
        else if (opcode(current) == BEQ)
        {
            if (state.EXMEM.aluResult == 0)
            {
                newState.pc = state.EXMEM.branchTarget;
                newState.IFID.instr = NOOPINSTRUCTION;
                newState.IDEX.instr = NOOPINSTRUCTION;
                newState.EXMEM.instr = NOOPINSTRUCTION;
            }
        }
        else if (opcode(current) == SW) newState.dataMem[state.EXMEM.aluResult] = state.EXMEM.readRegB;
        //write back if opcode isnt noop or halt
        else if (opcode(current) != NOOP && opcode(current) != HALT)
            newState.MEMWB.writeData = state.EXMEM.aluResult;

        /* --------------------- WB stage --------------------- */
        newState.WBEND.instr = state.MEMWB.instr;
        current = newState.WBEND.instr;
        newState.WBEND.writeData = state.MEMWB.writeData;

        if (isLW(state.MEMWB.instr))
            newState.reg[field1(current)] = state.MEMWB.writeData;
        else if (isADD(state.MEMWB.instr) || isNOR(state.MEMWB.instr))
            newState.reg[field2(current)] = state.MEMWB.writeData;

        state = newState; /* this is the last statement before end of the loop.
                It marks the end of the cycle and updates the
                current state with the values calculated in this
                cycle */
    }
    exit(0);
}