#define NUMMEMORY	65536
#define NUMREGS	8
#define ADD	0
#define NOR	1
#define LW	2
#define SW	3
#define BEQ	4
#define JALR	5
#define HALT	6
#define NOOP	7

#define NOOPINSTRUCTION	0x1c00000

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
		int cycles;
} stateType;

int field0(int instruction) {
		return ( (instruction>>19) & 0x7);
}

int field1(int instruction) {
		return ( (instruction>>16) & 0x7);
}

int field2(int instruction) {
		return  (instruction & 0xFFFF);
}

int opcode(int instruction) {
		return(instruction>>22);
}

void printInstruction(int instr) {
		char opcodeString[10];

		if(opcode(instr)==ADD) {
				strcpy(opcodeString, "add");
		}
		
		else if(opcode(instr) == NOR) {
				strcpy(opcodeString, "nor");
		}

		else if(opcode(instr) == LW) {
				strcpy(opcodeString, "lw");
		}

		else if(opcode(instr) == SW) {
				strcpy(opcodeString, "sw");
		}

		else if(opcode(instr) == BEQ) {
				strcpy(opcodeString, "beq");
		}
	
		else if(opcode(instr) == JALR) {
				strcpy(opcodeString, "jalr");
		}
	
		else if(opcode(instr) == HALT) {
				strcpy(opcodeString, "halt");
		}
	
		else if(opcode(instr) == NOOP) {
				strcpy(opcodeString, "noop");
		}

		else {
				strcpy(opcodeString, "data");
		}
		
		printf("%s %d %d %d\n",opcodeString, field0(instr), field1(instr), field2(instr));
}

int convertNum(int num) {
		if(num & (1<<15)) {
				num -= (1<<16);
		}
		return num;
}

int hazard(stateType state) {
		int instr = opcode(state.IFID.instr);
		if(opcode(state.IDEX.instr) == LW) {
				int destReg = field1(state.IDEX.instr);
				int regA = field0(state.IFID.instr);
				int regB = field1(state.IFID.instr);
				if(instr == ADD || instr == NOR || instr == BEQ) {
						if(destReg == regA || destReg == regB) 
								return 1;
						else return 0;
				}
				else if(instr == LW || instr == SW) {
						if(destReg == regA)
								return 1;
						else return 0;
				}
		}

		return 0;
}

void forward(stateType state, int *regA, int *regB) {
		int instr = opcode(state.IDEX.instr);
		int reg0 = field0(state.IDEX.instr);
		int reg1 = field1(state.IDEX.instr);

		if(instr == JALR || instr == HALT || instr == NOOP) 
				return;

		int pinstr = opcode(state.WBEND.instr);
		int destReg;
		if(pinstr == LW) {
				destReg = field1(state.WBEND.instr);
				if(reg0 == destReg)
						*regA = state.WBEND.writeData;
				if(reg1 == destReg)
						*regB = state.WBEND.writeData;
		}
		else if(pinstr == ADD || pinstr == NOR) {
				destReg = field2(state.WBEND.instr);
				if(reg0 == destReg)
						*regA = state.WBEND.writeData;
				if(reg1 == destReg)
						*regB = state.WBEND.writeData;

		}

		pinstr = opcode(state.MEMWB.instr);
		if(pinstr == LW) {
				destReg = field1(state.MEMWB.instr);
				if(reg0 == destReg)
						*regA = state.MEMWB.writeData;
				if(reg1 == destReg)
						*regB = state.MEMWB.writeData;
		}
		else if(pinstr == ADD || pinstr == NOR) {
				destReg = field2(state.MEMWB.instr);
				if(reg0 == destReg)
						*regA = state.MEMWB.writeData;
				if(reg1 == destReg)
						*regB = state.MEMWB.writeData;
		}
		
		pinstr = opcode(state.EXMEM.instr);
		if(pinstr == ADD || pinstr == NOR) {
				destReg = field2(state.EXMEM.instr);
				if(reg0 == destReg)
						*regA = state.EXMEM.aluResult;
				if(reg1 == destReg)
						*regB = state.EXMEM.aluResult;
		}
}

void printState(stateType* statePtr) {
		int i;
		printf("\n@@@\nstate before cycle %d starts\n", statePtr->cycles);
		printf("\tpc %d\n", statePtr->pc);

		printf("\tdata memory:\n");
		for(i = 0;i<statePtr->numMemory;i++) {
				printf("\t\tdataMem[ %d ] %d\n", i, statePtr->dataMem[i]);
		}

		printf("\tregisters:\n");
		for(i=0;i<NUMREGS;i++) {
				printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
		}
		printf("\tIFID:\n");
		printf("\t\tinstruction ");
		printInstruction(statePtr->IFID.instr);
		printf("\t\tpcPlus1 %d\n", statePtr->IFID.pcPlus1);
		printf("\tIDEX:\n");
		printf("\t\tinstruction ");
		printInstruction(statePtr->IDEX.instr);
		printf("\t\tpcPlus1 %d\n",statePtr->IDEX.pcPlus1);
		printf("\t\treadRegA %d\n", statePtr->IDEX.readRegA);
		printf("\t\treadRegB %d\n", statePtr->IDEX.readRegB);
		printf("\t\toffset %d\n", statePtr->IDEX.offset);
		printf("\tEXMEM:\n");
		printf("\t\tinstruction ");
		printInstruction(statePtr->EXMEM.instr);
		printf("\t\tbranchTarget %d\n",statePtr->EXMEM.branchTarget);
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


void run(stateType state, stateType newState) {

		while(1) {
				printState(&state);

				if(opcode(state.MEMWB.instr) == HALT) {
						printf("machine halted\n");
						printf("total of %d cycles executed\n",state.cycles);
						exit(0);
				}

				newState = state;
				newState.cycles++;

				/*-------------IF--------------*/
				newState.IFID.instr = state.instrMem[state.pc];
				newState.IFID.pcPlus1 = state.pc + 1;
				newState.pc = state.pc + 1;
				/*-------------ID--------------*/
				if(hazard(state)) {
						//if hazard occurs, 1 stall( noop )
						newState.IFID.instr = state.IFID.instr;
						newState.IFID.pcPlus1 = state.pc;
						newState.pc = state.pc;
						newState.IDEX.instr = NOOPINSTRUCTION;
						newState.IDEX.pcPlus1 = 0;
						newState.IDEX.readRegA = 0;
						newState.IDEX.readRegB = 0;
						newState.IDEX.offset = 0;
				}

				else {
						newState.IDEX.instr = state.IFID.instr;
						newState.IDEX.pcPlus1 = state.IFID.pcPlus1;
						newState.IDEX.readRegA = state.reg[field0(state.IFID.instr)];
						newState.IDEX.readRegB = state.reg[field1(state.IFID.instr)];
						newState.IDEX.offset = convertNum(field2(state.IFID.instr));
				}
				/*-------------EX--------------*/
				newState.EXMEM.instr = state.IDEX.instr;
				newState.EXMEM.branchTarget = state.IDEX.pcPlus1 + state.IDEX.offset;
				int regA = state.IDEX.readRegA;
				int regB = state.IDEX.readRegB;
				forward(state, &regA, &regB);
				int instr = opcode(state.IDEX.instr);
				if(instr == ADD) {
						newState.EXMEM.aluResult = regA + regB;
				}
				else if(instr == NOR) {
						newState.EXMEM.aluResult = ~(regA | regB);
				}
				else if(instr == LW || instr == SW) {
						newState.EXMEM.aluResult = regA + state.IDEX.offset;
				}
				else if(instr == BEQ) {
						newState.EXMEM.aluResult = regA - regB;
				}
				else if(instr == JALR) {
						printf("Cannot run.\n");
						exit(1);
				}
				else if(instr == HALT || instr == NOOP) {
						//newState.EXMEM.aluResult = 0;
				}

				else {
						printf("Cannot run.\n");
						exit(1);
				}
				newState.EXMEM.readRegB = regB;

				/*-------------MEM-------------*/
				newState.MEMWB.instr = state.EXMEM.instr;
				instr = opcode(state.EXMEM.instr);

				if(instr == ADD || instr == NOR) {
						newState.MEMWB.writeData = state.EXMEM.aluResult;
				}
				else if(instr == LW) {
						newState.MEMWB.writeData = state.dataMem[state.EXMEM.aluResult];
				}
				else if(instr == SW) {
						newState.dataMem[state.EXMEM.aluResult] = state.EXMEM.readRegB;
						newState.MEMWB.writeData = 0;
				}
				else if(instr == BEQ) {
						if(state.EXMEM.aluResult == 0) {
								newState.pc = state.EXMEM.branchTarget;
								newState.IFID.instr = NOOPINSTRUCTION;
								newState.IDEX.instr = NOOPINSTRUCTION;
								newState.EXMEM.instr = NOOPINSTRUCTION;
						}
						newState.MEMWB.writeData = 0;
				}
				else if(instr == JALR) {
						printf("Cannot run.\n");
						exit(1);
				}
				else if(instr == HALT || instr == NOOP) {
				//		newState.MEMWB.writeData = 0;
				}
				else {
						printf("Cannot run.\n");
						exit(1);
				}

				/*-------------WB--------------*/
				newState.WBEND.instr = state.MEMWB.instr;
				newState.WBEND.writeData = state.MEMWB.writeData;

				instr = opcode(state.MEMWB.instr);

				if(instr == ADD || instr == NOR) {
						newState.reg[field2(state.MEMWB.instr)] = state.MEMWB.writeData;
				}
				else if(instr == LW) {
						newState.reg[field1(state.MEMWB.instr)] = state.MEMWB.writeData;
				}
				else if(instr == SW || instr == BEQ || instr == HALT || instr == NOOP) {}
				else {
						printf("Cannot run.\n");
						exit(1);
				}
				newState.WBEND.writeData = state.MEMWB.writeData;
				state = newState;
		}
}

int main(int argc, char* argv[]) {
		stateType *state, *newState;
		char line[1000];
		FILE *file;
		state = malloc(sizeof(stateType));
		newState = malloc(sizeof(stateType));

		if(argc != 2) {
				printf("error: usage: %s <machine-code file>\n", argv[0]);
				exit(1);
		}

		file = fopen(argv[1], "r");
		if(file == NULL) {
				printf("error: can't open file %s", argv[1]);
				perror("fopen");
				exit(1);
		}

		memset(state, 0, sizeof(stateType));
		state->IFID.instr = NOOPINSTRUCTION;
		state->IDEX.instr = NOOPINSTRUCTION;
		state->EXMEM.instr = NOOPINSTRUCTION;
		state->MEMWB.instr = NOOPINSTRUCTION;
		state->WBEND.instr = NOOPINSTRUCTION;
		for(state->numMemory = 0; fgets(line, 1000, file) != NULL; state->numMemory++) {
				if(sscanf(line, "%d", state->instrMem + state->numMemory) != 1) {
						printf("error in reading address %d\n", state->numMemory);
						exit(1);
				}
				state->dataMem[state->numMemory] = state->instrMem[state->numMemory];
				printf("memory[%d]=%d\n", state->numMemory, state->instrMem[state->numMemory]);
		}

		printf("%d memory words\n", state->numMemory);
		printf("\tinstruction memory:\n");
		for(int i=0;i<state->numMemory;i++) {
				printf("\t\tinstrMem[ %d ] ", i);
				printInstruction(state->instrMem[i]);
		}
		
		run(*state, *newState);
}
