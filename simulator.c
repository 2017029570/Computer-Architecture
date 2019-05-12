#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536
#define NUMREGS 8
#define MAXLINELENGTH 1000

typedef struct stateStruct {
		int pc;
		int mem[NUMMEMORY];
		int reg[NUMREGS];
		int numMemory;
}stateType;

int convertNum(int num);

void decompose(int mc, int *opcode, int *reg1, int *reg2, int *offset);

void printState(stateType*);

int main(int argc, char *argv[]) {
		char line[MAXLINELENGTH];
		stateType state;
		FILE *filePtr;

		if(argc!=2) {
				printf("error: usage: %s<machine-code file>\n",argv[0]);
				exit(1);
		}

		filePtr = fopen(argv[1],"r");
		if(filePtr == NULL) {
				printf("error: can't open file %s",argv[1]);
				perror("fopen");
				exit(1);
		}
		
		state.pc = 0;
		for(int i=0;i<NUMREGS;i++) {
				state.reg[i] = 0;
		}
		for(state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++) {
				if(sscanf(line, "%d", state.mem+state.numMemory) != 1) {
						printf("error in reading address %d\n", state.numMemory);
					exit(1);
				}
				printf("memory[%d]=%d\n",state.numMemory, state.mem[state.numMemory]);
		}

		int opcode;
		int reg1;
		int reg2;
		int offset;
		int counter = 0;
		printf("\n");

		while(1) {
				printState(&state);
				decompose(state.mem[state.pc], &opcode, &reg1, &reg2, &offset);
				printf("%d\n",opcode);
				if(opcode == 0) {
						state.reg[offset] = state.reg[reg1] + state.reg[reg2];
				}
				else if(opcode == 1) {
						state.reg[offset] = ~(state.reg[reg1] | state.reg[reg2]);
				}
				else if(opcode == 2) {
						state.reg[reg2] = state.mem[state.reg[reg1] + offset];
				}
				else if(opcode == 3) {
						state.mem[state.reg[reg1] + offset] = state.reg[reg2];
				}
				else if(opcode == 4) {
						if(state.reg[reg1] == state.reg[reg2]) {
								state.pc += offset;
						}
				}
				else if(opcode == 5) {
						state.reg[reg2] = state.pc + 1;
						state.pc = state.reg[reg1] - 1;
				}
				else if(opcode == 6) {
						state.pc++;
						printState(&state);
						break;
				}
				else if(opcode == 7) {
				}
				state.pc++;
				counter++;
		}
		return(0);
}

void decompose(int mc, int *opcode, int *reg1, int *reg2, int *offset) {
		*opcode = (mc >> 22) & 7;
		*reg1 = (mc >> 19) & 7;
		*reg2 = (mc >> 16) & 7;
		*offset = convertNum(mc&65535);
}

void printState(stateType *statePtr) {
		int i;
		printf("\n@@@\nstate:\n");
		printf("\tpc %d\n",statePtr->pc);
		printf("\tmemory:\n");
		for(i=0;i<statePtr->numMemory;i++) {
				printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
		}
		printf("\tregisters:\n");
		for(i=0;i<NUMREGS;i++) {
				printf("\t\treg[ %d ] %d\n",i, statePtr->reg[i]);
		}
		printf("endstate\n");
}

int convertNum(int num) {
		if(num & (1<<15)) {
				num -= (1<<16);
		}
		return num;
}
