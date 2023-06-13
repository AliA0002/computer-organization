#define _CRT_SECURE_NO_WARNINGS
/*
 * EECS 370, University of Michigan
 * Project 3: LC-2K Pipeline Simulator
 * Instructions are found in the project spec.
 * Make sure NOT to modify printState or any of the associated functions
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Machine Definitions
#define NUMMEMORY 65536 // maximum number of data words in memory
#define NUMREGS 8 // number of machine registers

#define ADD 0
#define NOR 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 // will not implemented for Project 3
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION (NOOP << 22)

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
    int eq;
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
	int cycles; // number of cycles run so far
} stateType;

static inline int opcode(int instruction) {
    return instruction>>22;
}

static inline int field0(int instruction) {
    return (instruction>>19) & 0x7;
}

static inline int field1(int instruction) {
    return (instruction>>16) & 0x7;
}

static inline int field2(int instruction) {
    return instruction & 0xFFFF;
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

// convert a 16-bit number into a 32-bit Linux integer
static inline int convertNum(int num) {
    return num - ( (num & (1<<15)) ? 1<<16 : 0 );
}

void printState(stateType*);
void printInstruction(int);
void readMachineCode(stateType*, char*); 

void initilizeRegisters(stateType* state)
{
    for (int i = 0; i < NUMREGS; ++i)
    {
        state->reg[i] = 0;
    }
    for (int i = 0; i < state->numMemory; ++i)
    {
        state->dataMem[i] = state->instrMem[i];
    }
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

void setEXStage(stateType* state, stateType* newState)
{
    newState->EXMEM.instr = state->IDEX.instr;
    newState->EXMEM.branchTarget = state->IDEX.pcPlus1 + state->IDEX.offset;
}

stateType state, newState;
int main(int argc, char* argv[])
{
    int current;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    readMachineCode(&state, argv[1]);
    initilizeRegisters(&state);
    setValues(&state);

    int regA_num, regB_num, EXEM_dest, MEMWB_dest, WBEND_dest;
    regA_num = regB_num = EXEM_dest = MEMWB_dest = WBEND_dest = 0;

    state.IFID.instr = state.IDEX.instr = state.EXMEM.instr
        = state.MEMWB.instr = state.WBEND.instr = NOOPINSTRUCTION;
    state.cycles = state.pc = 0;

    while (opcode(state.MEMWB.instr) != HALT) {
        printState(&state);

        newState = state;
        newState.cycles++;
        
        /* ---------------------- IF stage --------------------- */
        newState.IFID.instr = state.instrMem[state.pc];
        current = newState.IFID.instr;
        newState.IFID.pcPlus1 = state.pc + 1;
        newState.pc++;

        /* ---------------------- ID stage --------------------- */
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
        /* ---------------------- EX stage --------------------- */

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
            if (regA_num == regB_num) newState.EXMEM.eq = 1;
            else newState.EXMEM.eq = 0;
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
        /* ---------------------- WB stage --------------------- */
        newState.WBEND.instr = state.MEMWB.instr;
        current = newState.WBEND.instr;
        newState.WBEND.writeData = state.MEMWB.writeData;

        if (isLW(state.MEMWB.instr))
            newState.reg[field1(current)] = state.MEMWB.writeData;
        else if (isADD(state.MEMWB.instr) || isNOR(state.MEMWB.instr))
            newState.reg[field2(current)] = state.MEMWB.writeData;
        /* ------------------------ END ------------------------ */
        state = newState; /* this is the last statement before end of the loop. It marks the end 
        of the cycle and updates the current state with the values calculated in this cycle */
    }
    printf("machine halted\n");
    printf("total of %d cycles executed\n", state.cycles);
    printf("final state of machine:\n");
    printState(&state);
    exit(0);
}

void printInstruction(int instr) {
    switch (opcode(instr)) {
        case ADD:
            printf("add");
            break;
        case NOR:
            printf("nor");
            break;
        case LW:
            printf("lw");
            break;
        case SW:
            printf("sw");
            break;
        case BEQ:
            printf("beq");
            break;
        case JALR:
            printf("jalr");
            break;
        case HALT:
            printf("halt");
            break;
        case NOOP:
            printf("noop");
            break;
        default:
            printf(".fill %d", instr);
            return;
    }
    printf(" %d %d %d", field0(instr), field1(instr), field2(instr));
}

void printState(stateType *statePtr) {
    printf("\n@@@\n");
    printf("state before cycle %d starts:\n", statePtr->cycles);
    printf("\tpc = %d\n", statePtr->pc);

    printf("\tdata memory:\n");
    for (int i=0; i<statePtr->numMemory; ++i) {
        printf("\t\tdataMem[ %d ] = %d\n", i, statePtr->dataMem[i]);
    }
    printf("\tregisters:\n");
    for (int i=0; i<NUMREGS; ++i) {
        printf("\t\treg[ %d ] = %d\n", i, statePtr->reg[i]);
    }

    // IF/ID
    printf("\tIF/ID pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->IFID.instr);
    printInstruction(statePtr->IFID.instr);
    printf(" )\n");
    printf("\t\tpcPlus1 = %d", statePtr->IFID.pcPlus1);
    if(opcode(statePtr->IFID.instr) == NOOP){
        printf(" (Don't Care)");
    }
    printf("\n");
    
    // ID/EX
    int idexOp = opcode(statePtr->IDEX.instr);
    printf("\tID/EX pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->IDEX.instr);
    printInstruction(statePtr->IDEX.instr);
    printf(" )\n");
    printf("\t\tpcPlus1 = %d", statePtr->IDEX.pcPlus1);
    if(idexOp == NOOP){
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\treadRegA = %d", statePtr->IDEX.readRegA);
    if (idexOp >= HALT || idexOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\treadRegB = %d", statePtr->IDEX.readRegB);
    if(idexOp == LW || idexOp > BEQ || idexOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\toffset = %d", statePtr->IDEX.offset);
    if (idexOp != LW && idexOp != SW && idexOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");

    // EX/MEM
    int exmemOp = opcode(statePtr->EXMEM.instr);
    printf("\tEX/MEM pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->EXMEM.instr);
    printInstruction(statePtr->EXMEM.instr);
    printf(" )\n");
    printf("\t\tbranchTarget %d", statePtr->EXMEM.branchTarget);
    if (exmemOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\teq ? %s", (statePtr->EXMEM.eq ? "True" : "False"));
    if (exmemOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\taluResult = %d", statePtr->EXMEM.aluResult);
    if (exmemOp > SW || exmemOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\treadRegB = %d", statePtr->EXMEM.readRegB);
    if (exmemOp != SW) {
        printf(" (Don't Care)");
    }
    printf("\n");

    // MEM/WB
	int memwbOp = opcode(statePtr->MEMWB.instr);
    printf("\tMEM/WB pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->MEMWB.instr);
    printInstruction(statePtr->MEMWB.instr);
    printf(" )\n");
    printf("\t\twriteData = %d", statePtr->MEMWB.writeData);
    if (memwbOp >= SW || memwbOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");     

    // WB/END
	int wbendOp = opcode(statePtr->WBEND.instr);
    printf("\tWB/END pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->WBEND.instr);
    printInstruction(statePtr->WBEND.instr);
    printf(" )\n");
    printf("\t\twriteData = %d", statePtr->WBEND.writeData);
    if (wbendOp >= SW || wbendOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");

    printf("end state\n");
}

// File
#define MAXLINELENGTH 1000 // MAXLINELENGTH is the max number of characters we read

void readMachineCode(stateType *state, char* filename) {
    char line[MAXLINELENGTH];
    FILE *filePtr = fopen(filename, "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", filename);
        exit(1);
    }

    printf("instruction memory:\n");
    for (state->numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; ++state->numMemory) {
        if (sscanf(line, "%d", state->instrMem+state->numMemory) != 1) {
            printf("error in reading address %d\n", state->numMemory);
            exit(1);
        }
        printf("\tinstrMem[ %d ] = ", state->numMemory);
        printInstruction(state->dataMem[state->numMemory] = state->instrMem[state->numMemory]);
        printf("\n");
    }
}