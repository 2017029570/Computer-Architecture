#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINELENGTH 1000

int readAndPars(FILE*, char*, char*, char*, char*, char*);
int isNumber(char*);

int main(int argc, char *argv[]) {
		char *inFileString, *outFileString;
		FILE *inFilePtr, *outFilePtr;
		char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH], arg1[MAXLINELENGTH], arg2[MAXLINELENGTH], labelcheck[MAXLINELENGTH][7];

		if(argc !=3) {
				printf("error: usage: %s <assembly-code-file> <machine-code-file>\n", argv[0]);
				exit(1);
		}

		inFileString = argv[1];
		outFileString = argv[2];

		inFilePtr = fopen(inFileString, "r");
		if(inFilePtr == NULL) {
				printf("error in opening %s\n", inFileString);
				exit(1);
		}
		
		outFilePtr = fopen(outFileString, "w");
		if(outFilePtr == NULL) {
				printf("error in opening %s\n", outFileString);
				exit(1);
		}
		
		int k=0;
		while(readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
				strcpy(labelcheck[k++], label);
		}
		
		rewind(inFilePtr);
		int machine;
		int pc = 0;
		while(readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
				if(!strcmp(opcode, "\0")) continue;
				else if(!strcmp(opcode, "add") || !strcmp(opcode, "nor")) {
					machine = RType(opcode, arg0, arg1, arg2);
				}

				else if(!strcmp(opcode, "lw") || !strcmp(opcode, "sw") || !strcmp(opcode, "beq")) {
						machine = IType(opcode, arg0, arg1, arg2, labelcheck, pc);
				}

				else if(!strcmp(opcode, "jalr")) {
						machine = JType(opcode, arg0, arg1);
				}

				else if(!strcmp(opcode, "halt") || !strcmp(opcode, "noop")) {
						machine = OType(opcode);
				}

				else if(!strcmp(opcode, ".fill")) {
						if(isNumber(arg0)) {
								sscanf(arg0, "%d", &machine);
						}
						else {
								for(int i=0;i<k;i++) {
										if(!strcmp(labelcheck[i],arg0)) {
												machine = i;
												break;
										}
										else if(i == MAXLINELENGTH-1) {
												printf("error: invalid label\n");
												exit(1);
										}
								}
						}
				}

				else {
						printf("error: unrecognized opcode\n");
						printf("%s\n",opcode);
						exit(1);
				}
				fprintf(outFilePtr,"%d\n",machine);
				pc++;
		}
		return(0);
}

int RType(char* opcode, char* arg0, char* arg1, char* arg2) {
		int machine=0;

		int regA, regB, destReg;

		sscanf(arg0, "%d", &regA);
		sscanf(arg1, "%d", &regB);
		sscanf(arg2, "%d", &destReg);

		if(!strcmp(opcode, "add")) {
			machine = 0<<22 | regA<<19 | regB<<16 | destReg;
		}

		else {
				machine = 1<<22 | regA<<19 | regB<<16 | destReg;
		}

		return machine;
}

int IType(char* opcode, char* arg0, char* arg1, char* arg2, char (*labelcheck)[7], int pc) {
		int machine = 0;

		int regA, regB, offset;

		sscanf(arg0, "%d", &regA);
		sscanf(arg1, "%d", &regB);

		if(isNumber(arg2)) {
				sscanf(arg2, "%d", &offset);
		}

		else {
				for(int i=0;i<MAXLINELENGTH;i++) {
						if(!strcmp(labelcheck[i], arg2)) {
								offset = i - pc - 1;
								break;
						}
						else if(i == MAXLINELENGTH-1) {
								printf("error: invalid label\n");
								exit(1);
						}
				}
		}

		if(!strcmp(opcode, "lw")) machine = 2<<22 | regA<<19 | regB<<16 | offset;
		else if(!strcmp(opcode, "sw")) machine = 3<<22 | regA<<19 | regB<<16 | offset;
		else 
			machine = 4<<22 | regA<<19 | regB<<16 | (offset&(65535));
				

		return machine;
}

int JType(char* opcode, char* arg0, char* arg1) {
		int machine; 
		 
		int regA, regB;

		sscanf(arg0, "%d",&regA);
		sscanf(arg1, "%d", &regB);

		machine = 5<<22 | regA<<19 | regB<<16 | 0;

		return machine;
}

int OType(char* opcode) {
		int machine;

		if(!strcmp(opcode, "halt"))
				machine = 6<<22 | 0;

		else machine = 7<<22 | 0;

		return machine;
}

int readAndParse(FILE* inFilePtr, char *label, char *opcode, char *arg0, char *arg1, char *arg2) {
		char line[MAXLINELENGTH];
		char *ptr = line;

		label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

		if(fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
				return(0);
		}

		if(strchr(line,'\n')==NULL) {
				printf("error: line too long\n");
				exit(1);
		}

		ptr = line;
		if(sscanf(ptr,"%[^\t\n\r]", label)) {
				ptr += strlen(label);
		}

		sscanf(ptr, "%*[\t\n\r]%[^\t\n\r]%*[\t\n\r]%[^\t\n\r]%*[\t\n\r]%[^\t\n\r]%*[\t\n\r]%[^\t\n\r]", opcode, arg0, arg1, arg2);
		return(1);
}

int isNumber(char *string) {
		int i;
		return( (sscanf(string, "%d", &i)) == 1);
}
