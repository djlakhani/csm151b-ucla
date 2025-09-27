#include "CPU.h"

instruction::instruction(bitset<32> fetch)
{
	//cout << fetch << endl;
	instr = fetch;
	//cout << instr << endl;
}

// Declaring static variables
const bitset<7> CPU::BLT_CODE("1100011");
const bitset<7> CPU::JALR_CODE("1100111");
const bitset<7> CPU::I_CODE("0010011");
const bitset<7> CPU::LOAD_CODE("0000011");
const bitset<7> CPU::STR_CODE("0100011");
const bitset<7> CPU::R_CODE("0110011");

CPU::CPU()
{
	
	PC = 0; 
	RS1 = 0;
	RS2 = 0;
	RD_idx = 0;
	immediate = 0;
	ALUOper = 0;

	// initialize control signals
	Branch = 0;
	MemRead = 0;
	ValtoReg = 0; // equivalent to MemtoReg, 0, 1, 2
	ALUOp = 0;
	MemWrite = 0;
	ALUSrc = 0;
	RegWrite = 0;
	ALUResult = 0;



	for (int i = 0; i < 4096; i++) {
		dmemory[i] = (0);
	}

	for (int i = 0; i < 32; i++) {
		regMemory[i] = (0);
	}
}

bitset<32> CPU::Fetch(bitset<8> *instmem) {
	bitset<32> instr = ((((instmem[PC + 3].to_ulong()) << 24)) + ((instmem[PC + 2].to_ulong()) << 16) + ((instmem[PC + 1].to_ulong()) << 8) + (instmem[PC + 0].to_ulong()));  //get 32 bit instruction
	PC += 4; //increment PC
	return instr;
}


bool CPU::Decode(instruction* curr) {
	bitset<32> opCodeMask(63);
	bitset<32> instr = curr->instr;

	if ((instr.to_ulong() && opCodeMask.to_ulong()) == 0) {
		return false;
	}

	ControlSignals(curr);

	Registers(curr);

	ImmediateGen(curr);

	ALUControl(curr);

	ALU(curr);

	MemoryWriteback(curr);

	ResetControlSignals();

	return true;
}


void CPU::Registers(instruction* curr) {
	bitset<32> instr = curr->instr;
	unsigned long regMask = 31;
	unsigned long RS1_idx = (instr.to_ulong() >> 15) & regMask;
	unsigned long RS2_idx = (instr.to_ulong() >> 20) & regMask;
	RD_idx = (instr.to_ulong() >> 7) & regMask;

	if ((0 <= RS1_idx) && (RS1_idx < 32)) {
		RS1 = regMemory[RS1_idx];
	}
	if ((0 <= RS2_idx) && (RS2_idx < 32)) {
		RS2 = regMemory[RS2_idx];
	}

	// cout << "Entering Register..." << endl;
	// cout << RS1_idx << " " << RS2_idx << " " << RD_idx << endl;
	// cout << "Exiting Registers..." << endl;
	return;
}


void CPU::ControlSignals(instruction* curr) {
	bitset<32> instr = curr->instr;
	unsigned long opCode = (instr.to_ulong() & 127);

	if ((opCode == BLT_CODE.to_ulong()) || (opCode == JALR_CODE.to_ulong()))
		Branch = 1;
	
	if (opCode == LOAD_CODE.to_ulong())
		MemRead = 1;

	if (opCode == LOAD_CODE.to_ulong())
		ValtoReg = 1;

	if (opCode == JALR_CODE.to_ulong())
		ValtoReg = 2;

	if (opCode == R_CODE.to_ulong())
		ALUOp = 0;

	if ((opCode == LOAD_CODE.to_ulong()) || (opCode == STR_CODE.to_ulong()) || (opCode == JALR_CODE.to_ulong()))
		// Both involve RS1 + immediate, forward ADD
		ALUOp = 1;

	if (opCode == I_CODE.to_ulong())
		ALUOp = 2;

	if (opCode == BLT_CODE.to_ulong())
		// forward SUB
		ALUOp = 3;


	if (opCode == STR_CODE.to_ulong())
		MemWrite = 1;

	if ((opCode == STR_CODE.to_ulong()) || (opCode == I_CODE.to_ulong()) || 
	(opCode == LOAD_CODE.to_ulong()) || (opCode == JALR_CODE.to_ulong()))
		ALUSrc = 1;

	if ((opCode == R_CODE.to_ulong()) || (opCode == JALR_CODE.to_ulong()) || 
	(opCode == I_CODE.to_ulong()) || (opCode == LOAD_CODE.to_ulong()))
		RegWrite = 1;

	// cout << "Entering Control Signals..." << endl;
	// cout << Branch << endl;
	// cout << MemRead << endl;
	// cout << ValtoReg << endl;
	// cout << ALUOp << endl;
	// cout << MemWrite << endl;
	// cout << ALUSrc << endl;
	// cout << RegWrite << endl;
	// cout << "Exiting Control Signals..." << endl;

	return;
}


void CPU::ImmediateGen(instruction* curr) {
	bitset<32> instr = curr->instr;
	int immMask = 4095;
	int imm1 = (instr.to_ulong() >> 20) & immMask;

	// sign extension
	if (imm1 & 0x800) {
    	imm1 |= 0xFFFFF000;
	}

	int immMaskMS = 127;
	unsigned long part1 = ((instr.to_ulong() >> 25) & immMaskMS) << 5;
	int immMaskLS = 31;
	unsigned long part2 = ((instr.to_ulong() >> 7) & immMaskLS);
	int imm2 = part1 + part2;

	// sign extension
	if (imm2 & 0x800) {
    	imm2 |= 0xFFFFF000;
	}
	
	// cout << "Entering Immediate..." << endl;
	// cout << imm1 << " " << imm2 << endl;
	// cout << "Exiting Immediate..." << endl;


	unsigned long opCode = (instr.to_ulong() & 127);
	if ((opCode == BLT_CODE.to_ulong()) || (opCode == STR_CODE.to_ulong()))
		immediate = imm2;
	if ((opCode == LOAD_CODE.to_ulong()) || (opCode == I_CODE.to_ulong()) || (opCode == JALR_CODE.to_ulong()))
		immediate = imm1;

	
	return;
}

void CPU::ALUControl(instruction* curr) {
	bitset<32> instr = curr->instr;
	unsigned long func3 = (instr.to_ulong() >> 12) & 0x07;
	unsigned long func7 = (instr.to_ulong() >> 25) & 0x07F;
	if (ALUOp == 0) {			// R-type Instruction
		if (func3 == 0) {
			if (func7 == 0) {
				ALUOper = 0;	// ADD
				return;
			}
			else {
				ALUOper = 1;	// SUB
				return;
			}
		}
		else {
			if (func7 == 0) {
				ALUOper = 2;   // XOR
				return;
			}
			else {
				ALUOper = 3;	// SRA
				return;
			}
		}
	}
	if (ALUOp == 1) {
		ALUOper = 0;	// RS1 + Immediate for LOAD, STORE, JALR
		return;
	}
	if (ALUOp == 2) {	// I-type Instruction
		if (func3 == 0) {
			ALUOper = 0;   // ADDI
			return;
		}
		else {
			ALUOper = 5;  // ANDI
			return;
		}
	}
	if (ALUOp == 3) {
		ALUOper = 1;	// SUB for BLT
		return;
	}

	cout << "Entering ALUControl..." << endl;
	cout << ALUOper << endl;
	cout << "Exiting ALUControl..." << endl; 
}

void CPU::ALU(instruction* curr) {
	int num1 = RS1;
	int num2 = RS2;
	if (ALUSrc == 1) {
		num2 = immediate;
	}

	if (ALUOper == 0) {		// ADD
		ALUResult = num1 + num2;
		cout << "Add in ALU" << endl;
		return;
	}

	if (ALUOper == 1) {		// SUB
		ALUResult = num1 - num2;
		cout << "Sub in ALU" << endl;
		return;
	}

	if (ALUOper == 2) {		// XOR
		ALUResult = num1 ^ num2;
		cout << "Xor in ALU" << endl;
		return;
	}

	if (ALUOper == 3) {		// SRA
		ALUResult = num1 >> num2;
		cout << "Sra in ALU" << endl;
		return;
	}

	if (ALUOper == 5) {		// AND
		ALUResult = num1 & num2;
		cout << "And in ALU" << endl;
		return;
	}

	cout << "Entering ALU..." << endl;
	cout << ALUResult << endl;
	cout << "Exiting ALU..." << endl; 

}

void CPU::MemoryWriteback(instruction* curr) {
	if (MemWrite == 1) {
		dmemory[ALUResult] = RS2;
	}
	if (MemRead == 1 && ValtoReg == 1) {
		if ((0 < RD_idx) && (RD_idx < 32)) {
			regMemory[RD_idx] = dmemory[ALUResult];
		}
	}
	if (MemWrite == 0 && MemRead == 0 && ValtoReg == 0) {
		if ((0 < RD_idx) && (RD_idx < 32)) {
			regMemory[RD_idx] = ALUResult;
		}
	}

	// JALR
	if (Branch == 1) {
		if (ValtoReg == 2) {
			if ((0 < RD_idx) && (RD_idx < 32)) {
				regMemory[RD_idx] = PC;
				PC = ALUResult;
			}
		}
		else {
			if (ALUResult < 0) {
				PC = PC - 4 + immediate;
			}
		}
	}
}

void CPU::ResetControlSignals() {
	RS1 = 0;
	RS2 = 0;
	RD_idx = 0;
	immediate = 0;
	ALUOper = 0;

	Branch = 0;
	MemRead = 0;
	ValtoReg = 0; // equivalent to MemtoReg, 0, 1, 2
	ALUOp = 0;
	MemWrite = 0;
	ALUSrc = 0;
	RegWrite = 0;
	ALUResult = 0;
}

int CPU::RegAccess(int regNum) {
	return regMemory[regNum];
}

int CPU::readPC()
{
	return PC;
}