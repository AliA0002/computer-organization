/**
 * Project 1 
 * Assembler code fragment for LC-2K 
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

int symbol_table_index = 0;
int fill_index = 0;
int relocation_index = 0;

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

struct Header
{
    int text, data, symbol_table, relocation_table;
}; typedef struct Header Header;

struct Symbol_Table
{
    char label[7];
    char symbol;
    int offset;
}; typedef struct Symbol_Table Symbol_Table;

struct Relocation_Table
{
    int offset;
    char opcode[7];
    char label[7];
}; typedef struct Relocation_Table Relocation_Table;

int checkCapital(char* string)
{
    if (string[0] >= 'A' && string[0] <= 'Z') return 1;
    else return 0;
}

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

int checkIfExists(Symbol_Table *symbol, char* string, int total)
{
    for (int k = 0; k < total; k++)
    {
        if (!strcmp(string, symbol[k].label)) return 1;
    }
    return 0;
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
    if (checkCapital(string)) return 0;
    else exit(1);
}

int labelCount(Pass1 p1[], char* string, int total)
{
    int count = 0;
    for (int k = 0; k < total; k++)
    {
        if (!strcmp(string, p1[k].label)) count++;
    }

    return count;
}

int duplicateLabel(Pass1* p1, int i)
{
    for (int k = 0; k < i; k++)
    {
        if (p1[k].label[0] == '\0') continue;
        for (int j = k + 1; j < i; j++)
        {
            if (!strcmp(p1[k].label, p1[j].label)) exit(1);
        }
    }
    return 1;
}

int secondPass(FILE* inFilePtr, Pass2 p2[], int i, Pass1* p1, int total, Header* header, Symbol_Table* symbol_table, Relocation_Table* relocation_table)
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


    /**************************************SYMBOL TABLE********************************************/
    if (checkCapital(p2[i].label) && labelCount(p1, p2[i].label, total) == 1) //if global and no duplicates
    {
        header->symbol_table++;
        if (strcmp(p2[i].opcode, ".fill"))  //Text Symbol
        {
            strcpy(symbol_table[symbol_table_index].label, p2[i].label);
            symbol_table[symbol_table_index].symbol = 'T';
            symbol_table[symbol_table_index].offset = i;
            symbol_table_index++;
        }
        else if (!strcmp(p2[i].opcode, ".fill"))    //Data Symbol
        {
            strcpy(symbol_table[symbol_table_index].label, p2[i].label);
            symbol_table[symbol_table_index].symbol = 'D';
            symbol_table[symbol_table_index].offset = fill_index;
            symbol_table_index++;
        }
    }
    /********************************************************************/
    //check for duplicates
    /***CHECK FOR UNDEFINED SYMBOLS***/
    if ((!strcmp(p2[i].opcode, "sw") || !strcmp(p2[i].opcode, "lw")) && checkCapital(p2[i].arg2))
    {
        if (!checkIfExists(symbol_table, p2[i].arg2, symbol_table_index))
        {
            if (labelCount(p1, p2[i].arg2, total) == 0)
            {
                header->symbol_table++;
                strcpy(symbol_table[symbol_table_index].label, p2[i].arg2);
                symbol_table[symbol_table_index].symbol = 'U';
                symbol_table[symbol_table_index].offset = 0;
                symbol_table_index++;
            }
        }
    }
    else if (!strcmp(p2[i].opcode, "beq") && checkCapital(p2[i].arg2) && labelCount(p1, p2[i].arg2, total) == 0) exit(1);
    else if (!strcmp(p2[i].opcode, ".fill") && checkCapital(p2[i].arg0) && labelCount(p1, p2[i].arg0, total) == 0)
    {
        if (!checkIfExists(symbol_table, p2[i].arg0, symbol_table_index))
        {
            header->symbol_table++;
            strcpy(symbol_table[symbol_table_index].label, p2[i].arg0);
            symbol_table[symbol_table_index].symbol = 'U';
            symbol_table[symbol_table_index].offset = 0;
            symbol_table_index++;
        }
    }


    /***RELOCATION SYMBOl***/
    if ((!strcmp(p2[i].opcode, "sw") || !strcmp(p2[i].opcode, "lw")) && !isNumber(p2[i].arg2))
    {
        header->relocation_table++;
        strcpy(relocation_table[relocation_index].label, p2[i].arg2);
        strcpy(relocation_table[relocation_index].opcode, p2[i].opcode);
        relocation_table[relocation_index].offset = i;
        relocation_index++;
    }
    else if (!strcmp(p2[i].opcode, ".fill") && !isNumber(p2[i].arg0))
    {
        header->relocation_table++;
        strcpy(relocation_table[relocation_index].label, p2[i].arg0);
        strcpy(relocation_table[relocation_index].opcode, p2[i].opcode);
        relocation_table[relocation_index].offset = fill_index;
        relocation_index++;
    }

    if (!strcmp(p2[i].opcode, ".fill"))
    {
        fill_index++;
    }

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
    Header header = { 0,0,0,0 };
    Symbol_Table symbol[100];
    Relocation_Table relocation[100];
    int assembly_code_text[100];
    int assembly_code_data[100];
    int data_index = 0;


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

    duplicateLabel(p1, i);

    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);

    fill_index = 0;
    int temp = i;
    i = 0;
    while (i < temp)
    {
        if (secondPass(inFilePtr, p2, i, p1, temp, &header, symbol, relocation) == 0) break;

        machine_code = 0;
        regA = regB = regD = offsetfield = opcode_code = 0;

        if (!strcmp(p2[i].opcode, "add"))
        {
            header.text++;
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
            assembly_code_text[i] = machine_code;
        }
        else if (!strcmp(p2[i].opcode, "nor"))
        {
            header.text++;
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
            assembly_code_text[i] = machine_code;
        }
        else if (!strcmp(p2[i].opcode, "lw"))
        {
            header.text++;
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
            assembly_code_text[i] = machine_code;
        }
        else if (!strcmp(p2[i].opcode, "sw"))
        {
            header.text++;
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
            assembly_code_text[i] = machine_code;
        }
        else if (!strcmp(p2[i].opcode, "beq"))
        {
            header.text++;
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
            assembly_code_text[i] = machine_code;
        }
        else if (!strcmp(p2[i].opcode, "jalr"))
        {
            header.text++;
            opcode_code = 0b101 << 22;
            regA = atoi(p2[i].arg0) << 19;
            regB = atoi(p2[i].arg1) << 16;
            machine_code = opcode_code | regA;
            machine_code = machine_code | regB;
            assembly_code_text[i] = machine_code;
        }
        else if (!strcmp(p2[i].opcode, "halt"))
        {
            header.text++;
            opcode_code = 0b110 << 22;
            assembly_code_text[i] = opcode_code;
        }
        else if (!strcmp(p2[i].opcode, "noop"))
        {
            header.text++;
            opcode_code = 0b111 << 22;
            assembly_code_text[i] = opcode_code;
        }
        else if (!strcmp(p2[i].opcode, ".fill"))
        {
            header.data++;
            int num;
            if (isNumber(p2[i].arg0))
            {
                num = atoi(p2[i].arg0);
                assembly_code_data[data_index] = num;
            }
            else
            {
                assembly_code_data[data_index] = returnLabelAddress(p2[i].arg0, p1, temp);
            }
            data_index++;
        }
        else //error
        {
            exit(1);
        }
        i++;
    }

    //*******PRINTING HEADER******
    fprintf(outFilePtr, "%d %d %d %d\n", header.text, header.data, header.symbol_table, header.relocation_table);

    //*******PRINTING TEXT*******
    for (int t = 0; t < i-fill_index; t++)
    {
        fprintf(outFilePtr, "%d\n", assembly_code_text[t]);
    }

    //*******PRINTING DATA*******
    for (int t = 0; t < data_index; t++)
    {
        fprintf(outFilePtr, "%d\n", assembly_code_data[t]);
    }
        
    //*******PRINTING SYMBOL TABLE*******
    for (int t = 0; t != header.symbol_table; t++)
    {
        fprintf(outFilePtr, "%s %c %d\n", symbol[t].label, symbol[t].symbol, symbol[t].offset);
    }

    //*******PRINTING RELOCATION TABLE*******
    for (int t = 0; t != header.relocation_table; t++)
    {
        fprintf(outFilePtr, "%d %s %s\n", relocation[t].offset, relocation[t].opcode, relocation[t].label);
    }

    exit(0);
}