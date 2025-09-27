#include <iostream>
#include <bitset>
#include <stdio.h>
#include<stdlib.h>
#include <string>
using namespace std;


class instruction {
public:
	bitset<32> instr;//instruction
	instruction(bitset<32> fetch); // constructor

};

class CPU {
private:
	int dmemory[4096]; //data memory byte addressable in little endian fashion;
	int PC; //pc 
	int RS1;
	int RS2;
	int RD_idx;
	int immediate;
	int regMemory[32];
	int ALUOper; // 0 - ADD, 1 - SUB, 2 - XOR, 3 - SRA, 5 - AND
	int ALUResult;

	// control signals
	int Branch;
	int MemRead;
	int ValtoReg; // equivalent to MemtoReg, 0, 1, 2
	int ALUOp;
	int MemWrite;
	int ALUSrc;
	int RegWrite;


	// OP Code
	static const bitset<7> BLT_CODE;
	static const bitset<7> JALR_CODE;
	static const bitset<7> I_CODE;
	static const bitset<7> LOAD_CODE;
	static const bitset<7> STR_CODE;
	static const bitset<7> R_CODE;


public:
	CPU();
	int readPC();
	bitset<32> Fetch(bitset<8> *instmem);
	bool Decode(instruction* instr);
	void Registers(instruction* instr);
	void ControlSignals(instruction* instr);
	void ImmediateGen(instruction* instr);
	void ALUControl(instruction* instr);
	void ALU(instruction* instr);
	void MemoryWriteback(instruction* instr);
	void ResetControlSignals();
	int RegAccess(int regNum);
	
};