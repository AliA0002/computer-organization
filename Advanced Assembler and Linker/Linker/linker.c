/**
 * Project 2
 * LC-2K Linker
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXSIZE 300
#define MAXLINELENGTH 1000
#define MAXFILES 6

typedef struct FileData FileData;
typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct RelocationTableEntry RelocationTableEntry;
typedef struct CombinedFiles CombinedFiles;

struct SymbolTableEntry {
	char label[7];
	char location;
	int offset;
};

struct RelocationTableEntry {
	int offset;
	char inst[7];
	char label[7];
	int file;
};

struct FileData {
	int textSize;
	int dataSize;
	int symbolTableSize;
	int relocationTableSize;
	int textStartingLine; // in final executable
	int dataStartingLine; // in final executable
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry symbolTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
};

struct CombinedFiles {
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry     symTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
	int textSize;
	int dataSize;
	int symTableSize;
	int relocTableSize;
};

void Fix(FileData* file, int, int);
void setDataStart(FileData* file, int);
int isGlobal(char*);
void setDataStart(FileData* file, int);
void checkDuplicateLabels(FileData* file, int);
int getGlobal(FileData* file, int, char*);
int getOffset(int);
int convertNum(int);
int inText(char*);
int isStack(char*);
int totalSize(FileData* file, int, int);
int locationSymbol(char);
int updateDecimal(int, int);
int setBinary(int, int*);
int isFill(char*);
void printLinkedFile(FILE*, FileData*, int);

int main(int argc, char *argv[])
{
	char *inFileString, *outFileString;
	FILE *inFilePtr, *outFilePtr; 
	int i, j;
	int count_text = 0;
	int numDecimals = 0;

	if (argc <= 2) {
		printf("error: usage: %s <obj file> ... <output-exe-file>\n",
				argv[0]);
		exit(1);
	}

	outFileString = argv[argc - 1];

	outFilePtr = fopen(outFileString, "w");
	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileString);
		exit(1);
	}

	FileData file[MAXFILES];

  // read in all files and combine into a "master" file
	for (i = 0; i < argc - 2; i++) {
		inFileString = argv[i+1];

		inFilePtr = fopen(inFileString, "r");
		printf("opening %s\n", inFileString);

		if (inFilePtr == NULL) {
			printf("error in opening %s\n", inFileString);
			exit(1);
		}

		char line[MAXLINELENGTH];
		int sizeText, sizeData, sizeSymbol, sizeReloc;

		// parse first line of file
		fgets(line, MAXSIZE, inFilePtr);
		sscanf(line, "%d %d %d %d",
				&sizeText, &sizeData, &sizeSymbol, &sizeReloc);

		numDecimals += sizeText + sizeData;

		file[i].textSize = sizeText;
		file[i].dataSize = sizeData;
		file[i].symbolTableSize = sizeSymbol;
		file[i].relocationTableSize = sizeReloc;
		file[i].textStartingLine = count_text;


		// read in text section
		int instr;
		for (j = 0; j < sizeText; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			instr = atoi(line);
			file[i].text[j] = instr;
			count_text++;
		}

		// read in data section
		int data;
		for (j = 0; j < sizeData; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			data = atoi(line);
			file[i].data[j] = data;
		}

		// read in the symbol table
		char label[7];
		char type;
		int addr;
		for (j = 0; j < sizeSymbol; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%s %c %d",
					label, &type, &addr);
			file[i].symbolTable[j].offset = addr;
			strcpy(file[i].symbolTable[j].label, label);
			file[i].symbolTable[j].location = type;
			if (!strcmp(label, "Stack"))		//check if Stack is defined (results in error)
			{
				if(type != 'U') exit(1);
			}
		}

		// read in relocation table
		char opcode[7];
		for (j = 0; j < sizeReloc; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%d %s %s",
					&addr, opcode, label);
			file[i].relocTable[j].offset = addr;
			strcpy(file[i].relocTable[j].inst, opcode);
			strcpy(file[i].relocTable[j].label, label);
			file[i].relocTable[j].file	= i;
			
		}
		fclose(inFilePtr);
	} // end reading file
	
	// *** INSERT YOUR CODE BELOW ***
	//    Begin the linking process
	//    Happy coding!!!
	checkDuplicateLabels(file, argc - 2);

	setDataStart(file, argc);

	Fix(file, argc, numDecimals);

	printLinkedFile(outFilePtr, file, argc);

	exit(0);
}

void Fix(FileData* file, int argc, int numDecimals)
{
	int t, s, decimal;
	t = s = decimal = 0;
	for (int i = 0; i < argc - 2; i++)
	{
		for (int j = 0; j < file[i].relocationTableSize; j++)
		{
			if (inText(file[i].relocTable[j].inst))
			{
				if (isGlobal(file[i].relocTable[j].label))
				{
					decimal = file[i].text[file[i].relocTable[j].offset];
					if (isStack(file[i].relocTable[j].label))
					{
						file[i].text[file[i].relocTable[j].offset] = decimal + numDecimals;
					}
					else {
						t = getGlobal(file, argc, file[i].relocTable[j].label);
						file[i].text[file[i].relocTable[j].offset] = t + decimal;
					}
				}
				else
				{
					decimal = file[i].text[file[i].relocTable[j].offset];
					t = getOffset(decimal);
					s = totalSize(file, argc, i);
					file[i].text[file[i].relocTable[j].offset] = updateDecimal(t + s, decimal);
				}
			}
			if (isFill(file[i].relocTable[j].inst))
			{
				if (isGlobal(file[i].relocTable[j].label))
				{
					t = getGlobal(file, argc, file[i].relocTable[j].label);
					if (isStack(file[i].relocTable[j].label))
					{
						file[i].data[file[i].relocTable[j].offset] = numDecimals;
					}
					else
					{
						file[i].data[file[i].relocTable[j].offset] = t;
					}
				}
				else
				{
					t = file[i].data[file[i].relocTable[j].offset];
					s = 0;
					for (int n = i; n >= 0; --n)
					{
						if (n != i)
						{
							if (t >= file[i].textSize) s = s + file[n].dataSize + file[n].textSize;
							else s = s + file[n].textSize;
						}
					}
					s = s + t;
					file[i].data[file[i].relocTable[j].offset] = s;
				}
			}
		}
	}
	return;
}

void setDataStart(FileData* file, int argc)
{
	int temp = argc - 2;
	int index = 0;
	for (int i = 0; i < temp; i++)
	{
		index = index + file[i].textSize;
	}
	file[0].dataStartingLine = index;
	for (int i = 1; i < temp; i++)
	{
		index = index + file[i - 1].dataSize;
		file[i].dataStartingLine = index;
	}
}

int isGlobal(char* string)
{
	if (string[0] >= 'A' && string[0] <= 'Z') return 1;
	return 0;
}

void checkDuplicateLabels(FileData* file, int size)
{
	int index = 0;
	int i = 0;
	while (i < size)
	{
		for (int n = i + 1; n < size; n++)
		{
			for (int j = 0; j < file[n].symbolTableSize; j++)
			{
				if ((!strcmp(file[i].symbolTable[index].label, file[n].symbolTable[j].label))
					&& file[i].symbolTable[index].location != 'U' && file[n].symbolTable[j].location != 'U')
				{
					printf("duplicate label found\n");
					exit(1);
				}
			}
		}
		++index;
		if (index >= file[i].symbolTableSize)
		{
			index = 0;
			i++;
		}
	}
}

int getGlobal(FileData* file, int argc, char* string)
{
	int x = argc - 2;
	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < file[i].symbolTableSize; j++)
		{
			if (!strcmp(file[i].symbolTable[j].label, string))
			{
				if (locationSymbol(file[i].symbolTable[j].location) == 1)
				{
					if (i != 0) {
						return file[i].symbolTable[j].offset + file[i].textStartingLine;
					}
					else return file[i].symbolTable[j].offset + file[i].textStartingLine;
				}
				else if (locationSymbol(file[i].symbolTable[j].location) == 2)
					return file[i].symbolTable[j].offset + file[i].dataStartingLine;
				else if (locationSymbol(file[i].symbolTable[j].location) == 3 && isStack(string))
					return 0;
			}
		}
	}
	exit(1);

	return 0;
}

int getOffset(int x)
{
	int temp = 0b1111111111111111;
	int num = x & temp;
	return convertNum(num);
}

int convertNum(int num)
{
	if (num & (1 << 15)) {
		num -= (1 << 16);
	}
	return(num);
}

int inText(char *string)
{
	if ((!strcmp(string, "lw")) ||
		(!strcmp(string, "sw"))) return 1;
	return 0;
}

int isStack(char* string)
{
	if (!strcmp(string, "Stack")) return 1;
	return 0;
}

int locationSymbol(char symbol)
{
	if (symbol == 'T') return 1;
	else if (symbol == 'D') return 2;
	else if (symbol == 'U') return 3;
	return 0;
}

int updateDecimal(int decimal, int og)
{
	int Num[90] = { 0 };
	int original[90] = { 0 };
	int newDecimal = 0;
	int i = setBinary(decimal, Num);
	int j = setBinary(og, original);
	for (int k = 0; k < i; k++)
	{
		original[k] = Num[k];
	}
	for (int k = j - 1; k >= 0; --k)
	{
		newDecimal = newDecimal << 1 | original[k];
	}
	return newDecimal;
}

int setBinary(int value,int* Num)
{
	int index = 0;
	while (value > 0)
	{
		Num[index] = value % 2;
		value = value / 2;
		index++;
	}
	return index;
}

int totalSize(FileData* file, int argc, int x)
{
	int size = argc - 2;
	int temp = 0;
	for (int i = 0; i < size; i++)
	{
		if (i != x)
		{
			if (x == 0)
			{
				temp = temp + file[i].textSize;
			}
			else
			{
				if (i > x) temp = temp + file[i].textSize;
				else temp = temp + file[i].textSize + file[i].dataSize;
			}
		}
	}
	return temp;
}

int isFill(char* string)
{
	if (!strcmp(string, ".fill")) return 1;
	return 0;
}

void printLinkedFile(FILE* outFilePtr,FileData* file, int argc)
{
	int size = argc - 2;
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < file[i].textSize; j++)
		{
			fprintf(outFilePtr, "%d\n", file[i].text[j]);
		}
	}
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < file[i].dataSize; j++)
		{
			fprintf(outFilePtr, "%d\n", file[i].data[j]);
		}
	}
}