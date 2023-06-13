/**
 * Project 1 
 * Assembler code fragment for LC-2K 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

struct first_pass_struct
{
    char label[7], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
        arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    int address;
};
typedef struct first_pass_struct Pass1;

struct second_pass_struct
{
    char label[7], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
        arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    int address;
    int opcode_num;
    int rega, regb, offset;
};
typedef struct second_pass_struct Pass2;


int firstPass(FILE* inFilePtr, int i, Pass1 p1[])
{

    char line[MAXLINELENGTH];

    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        return(0);
    }

    /* check for line too long (by looking for a \n) */
    /*if (strchr(line, '\n') == NULL) {
         line too long
        printf("error: line too long\n");
        exit(1);
    }*/

    /* is there a label? */
    char* ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", p1[i].label)) {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(p1[i].label);
    }
    else p1[i].label[0] = '\0';


    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        p1[i].opcode, p1[i].arg0, p1[i].arg1, p1[i].arg2);
    p1[i].address = i;
    return(1);
}

int isNumber(char* string)
{
    /* return 1 if string is a number */
    int i;
    return((sscanf(string, "%d", &i)) == 1);
}

int returnLabelAddress(char* string, Pass1 p1[], int temp)
{
    for (int i = 0; i < temp; i++)
    {
        if (!strcmp(string, p1[i].label)) return i;
    }
    exit(1);
}

int duplicateLabel(Pass1 p1[], int i)
{
    for (int j = 0; j < i; j++)
    {
        if (p1[j].label[0] == '\0') continue;
        for (int k = j + 1; k < i; k++)
            if (!strcmp(p1[j].label, p1[k].label))
            {
                return 0;
            }
    }
    return(1);
}

int secondPass(FILE* inFilePtr, Pass2 p2[], int i)
{
    char line[MAXLINELENGTH];
    fgets(line, MAXLINELENGTH, inFilePtr);
    char* ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", p2[i].label)) {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(p2[i].label);
    }
    else p2[i].label[0] = '\0';

    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        p2[i].opcode, p2[i].arg0, p2[i].arg1, p2[i].arg2);
    return(1);
}

int main(int argc, char* argv[])
{
    Pass1 p1[100];
    Pass2 p2[100];
    char* inFileString, * outFileString;
    FILE* inFilePtr, * outFilePtr;
    int i = 0;
    int machine_code, regA, regB, regD, offsetfield, opcode_code;

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    while (i >= 0)
    {
        if (firstPass(inFilePtr, i, p1) == 0) break;
        i++;
    }

    if (!duplicateLabel(p1, i)) exit(1); //checks for duplicate labels

    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);


    int temp = i;
    i = 0;
    while (i < temp)
    {
        if (secondPass(inFilePtr, p2, i) == 0) break;

        machine_code = 0;
        regA = regB = regD = offsetfield = opcode_code = 0;

        if (!strcmp(p2[i].opcode, "add"))
        {
            opcode_code = 0b000 << 22;
            regA = atoi(p2[i].arg0);
            regB = atoi(p2[i].arg1);
            if (!isNumber(p2[i].arg2)) exit(1);
            regD = atoi(p2[i].arg2);
            regA = regA << 19;
            regB = regB << 16;
            machine_code = opcode_code | regA;
            machine_code = machine_code | regB;
            machine_code = machine_code | regD;
            fprintf(outFilePtr, "%d\n", machine_code);
        }
        else if (!strcmp(p2[i].opcode, "nor"))
        {
            opcode_code = 0b001 << 22;
            regA = atoi(p2[i].arg0);
            regB = atoi(p2[i].arg1);
            if (!isNumber(p2[i].arg2)) exit(1);
            regD = atoi(p2[i].arg2);
            regA = regA << 19;
            regB = regB << 16;
            machine_code = opcode_code | regA;
            machine_code = machine_code | regB;
            machine_code = machine_code | regD;
            fprintf(outFilePtr, "%d\n", machine_code);
        }
        else if (!strcmp(p2[i].opcode, "lw"))
        {
            opcode_code = 0b010 << 22;;
            regA = atoi(p2[i].arg0);
            regB = atoi(p2[i].arg1);
            if (isNumber(p2[i].arg2))
            {
                offsetfield = atoi(p2[i].arg2);
                if (offsetfield > 32767 || offsetfield < -32767) exit(1);
                if (offsetfield < 0)
                {
                    int temp_num = 0b1111111111111111;
                    temp_num = offsetfield & temp_num;
                    offsetfield = temp_num;
                }
            }
            else
            {
                offsetfield = returnLabelAddress(p2[i].arg2, p1, temp);
            }
            regA = regA << 19;
            regB = regB << 16;
            machine_code = opcode_code | regA;
            machine_code = machine_code | regB;
            machine_code = machine_code | offsetfield;
            fprintf(outFilePtr, "%d\n", machine_code);
        }
        else if (!strcmp(p2[i].opcode, "sw"))
        {
            opcode_code = 0b011 << 22;;
            regA = atoi(p2[i].arg0);
            regB = atoi(p2[i].arg1);
            if (isNumber(p2[i].arg2))
            {
                offsetfield = atoi(p2[i].arg2);
                if (offsetfield > 32767 || offsetfield < -32767) exit(1);
                if (offsetfield < 0)
                {
                    int temp_num = 0b1111111111111111;
                    temp_num = offsetfield & temp_num;
                    offsetfield = temp_num;
                }
            }
            else
            {
                offsetfield = returnLabelAddress(p2[i].arg2, p1, temp);
            }
            regA = regA << 19;
            regB = regB << 16;
            machine_code = opcode_code | regA;
            machine_code = machine_code | regB;
            machine_code = machine_code | offsetfield;
            fprintf(outFilePtr, "%d\n", machine_code);
        }
        else if (!strcmp(p2[i].opcode, "beq"))
        {
            opcode_code = 0b100 << 22;
            regA = atoi(p2[i].arg0);
            regB = atoi(p2[i].arg1);
            regA = regA << 19;
            regB = regB << 16;
            if (isNumber(p2[i].arg2))
            {
                offsetfield = atoi(p1[i].arg2);
                if (offsetfield > 32767 || offsetfield < -32767) exit(1);
                if (offsetfield < 0)
                {
                    int temp_num = 0b1111111111111111;
                    temp_num = offsetfield & temp_num;
                    offsetfield = temp_num;
                }
            }
            else
            {
                offsetfield = returnLabelAddress(p2[i].arg2, p1, temp) - (1 + i);
                if (offsetfield > 32767 || offsetfield < -32767) exit(1);
                if (offsetfield < 0)
                {
                    int temp_num = 0b1111111111111111;
                    temp_num = offsetfield & temp_num;
                    offsetfield = temp_num;
                }
            }
            machine_code = opcode_code | regA;
            machine_code = machine_code | regB;
            machine_code = machine_code | offsetfield;
            fprintf(outFilePtr, "%d\n", machine_code);
        }
        else if (!strcmp(p2[i].opcode, "jalr"))
        {
            opcode_code = 0b101 << 22;
            regA = atoi(p2[i].arg0) << 19;
            regB = atoi(p2[i].arg1) << 16;
            machine_code = opcode_code | regA;
            machine_code = machine_code | regB;
            fprintf(outFilePtr, "%d\n", machine_code);
        }
        else if (!strcmp(p2[i].opcode, "halt"))
        {
            opcode_code = 0b110 << 22;
            fprintf(outFilePtr, "%d\n", opcode_code);
        }
        else if (!strcmp(p2[i].opcode, "noop"))
        {
            opcode_code = 0b111 << 22;
            fprintf(outFilePtr, "%d\n", opcode_code);
        }
        else if (!strcmp(p2[i].opcode, ".fill"))
        {
            int num;
            if (isNumber(p2[i].arg0))
            {
                num = atoi(p2[i].arg0);
                fprintf(outFilePtr, "%d\n", num);
            }
            else
            {
                fprintf(outFilePtr, "%d\n", returnLabelAddress(p2[i].arg0, p1, temp));
            }
        }
        else //error
        {
            exit(1);
        }
        i++;
    }
    exit(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if successfully read
 *
 * exit(1) if line is too long.
 */




