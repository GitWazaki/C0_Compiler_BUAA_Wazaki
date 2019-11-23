#pragma once
#include "../MidCode/MidInstr.hpp"
using namespace std;

namespace GenObject {

	enum Reg {
		at,
		v0, v1,
		a0, a1, a2, a3,
		t0, t1, t2, t3, t4, t5, t6, t7, t8, t9,
		s0, s1, s2, s3, s4, s5, s6, s7,
		k0, k1,
		gp, sp, fp,
		ra
	};

	class GenMips {

		MidCode::MidCode& midCodes;
		ofstream& fout;
		ofstream& mout;
		bool output_mips = false;
		bool output_mid = false;

		void output(string);
		
	public:
		GenMips(MidCode::MidCode& midCodes, ofstream& mout, ofstream& fout, string output_setting) : midCodes(midCodes), mout(mout), fout(fout) {
			if (output_setting.find('o') != std::string::npos) {
				output_mips = true;
			}
			if (output_setting.find('m') != std::string::npos) {
				output_mid = true;
			}
		}

		void outputMidCode(MidCode::Block& block);
		
		void gen();
		void genRegPool();
		void genGlobals();
		void genGlobal(MidCode::GlobalDefine);
		void genFuncs();
		void genFunc(MidCode::Func);
		void genBlock(MidCode::Block& block);
		// void genText();
		void allocRegs(MidCode::Block& block);
		void genInstrs(MidCode::Block& block);
		void genInstr(MidCode::MidInstr instr);
		void insertAfter(vector<MidCode::MidInstr>& instrs, int& i, MidCode::MidInstr instr);
		void insertBefore(vector<MidCode::MidInstr>& instrs, int& i, MidCode::MidInstr instr);
		// void getRegisters(std::vector<Reg> regs);
		// void releaseRegisters(std::vector<Reg> regs);

	};

	inline void GenMips::output(string str) {
		if (output_mips) {
			fout << str << endl;
		}
	}

	inline void GenMips::outputMidCode(MidCode::Block& block) {
		auto& instrs = block.instrs;
		for(int i = 0; i < instrs.size(); i++) {
			mout << instrs[i].toString() << endl;
		}
	}

	inline void GenMips::gen() {
		fout << "# C0_Compiler_BUAA_Wazaki" << endl;
		fout << ".data" << endl;
		genGlobals();
		genRegPool();
		fout << ".text" << endl;
		fout << "j main" << endl;
		genFuncs();
	}

#define REGPOOL "REGPOOL"
	
	inline void GenMips::genRegPool() {
		output(FORMAT("{}: \n .align 2 \n .space {}", REGPOOL,POOLSIZE*4));
	}

	inline void GenMips::genGlobals() {
		vector<MidCode::GlobalDefine>& globals = midCodes.global_defines;
		for (int i = 0; i < globals.size(); i++) {
			genGlobal(globals[i]);
		}
	}

	inline void GenMips::genGlobal(MidCode::GlobalDefine define) {
		switch (define.type) {
		case MidCode::GlobalDefine::CONST_STR:
			output(FORMAT("{}:.asciiz \"{}\" ",define.constStr.lable,define.constStr.value));
			break;
		case MidCode::GlobalDefine::VAR_INT:
			output(FORMAT("{}: .space 4", define.varInt.label));
			break;
		case MidCode::GlobalDefine::VAR_INT_ARRAY:
			output(FORMAT("{}: .space {}", define.varIntArray.label, 4 * define.varIntArray.len));
			break;
		default:
			break;
		}
	}

	inline void GenMips::genFuncs() {
		auto& funcs = midCodes.funcs;
		for (int i = 0; i < funcs.size(); i++) {
			genFunc(funcs[i]);
		}
	}
	
	inline void GenMips::genFunc(MidCode::Func func) {
		string func_name = func.func_name;
		//output(FORMAT("{}:", func_name));
		auto& blocks = *(func.blocks);
		for (int i = 0; i < blocks.size(); i++) {
			genBlock(blocks[i]);
		}
	}

	inline void GenMips::genBlock(MidCode::Block& block) {
		outputMidCode(block);
		allocRegs(block);
		genInstrs(block);
	}

#define NOT_ALLAC(ir_reg) startWith(instrs[i].ir_reg, std::string("_T"))

#define REGPOOL_LOAD(ir_reg, assign_reg)	\
		do {	\
			auto& instr = instrs[i];	\
			int loc = 4 * stoi(instr.ir_reg.substr(2));	\
			instr.ir_reg = #assign_reg;	\
			insertBefore(instrs, i, MidCode::MidInstr(MidCode::MidInstr::LOAD_LAB_IMM, #assign_reg, REGPOOL, loc));	\
		} while(0)

#define REGPOOL_SAVE(ir_reg, assign_reg)	\
		do {	\
			auto& instr = instrs[i];	\
			int loc = 4 * stoi(instr.ir_reg.substr(2));	\
			instr.ir_reg = #assign_reg;	\
			insertAfter(instrs, i,MidCode::MidInstr(MidCode::MidInstr::SAVE_LAB_IMM, #assign_reg, REGPOOL, loc));	\
		} while(0)
	// TODO
	inline void GenMips::allocRegs(MidCode::Block& block) {
		vector<MidCode::MidInstr>& instrs = block.instrs;
		for (int i = 0; i < instrs.size(); i++) {
			switch (instrs[i].midOp) {
			case MidCode::MidInstr::PRINT_INT:
			case MidCode::MidInstr::PRINT_CHAR:
				if (NOT_ALLAC(target))
					REGPOOL_LOAD(target, $t0);
				break;
			case MidCode::MidInstr::LOAD_LABEL:
			case MidCode::MidInstr::LOAD_STACK:
				if (NOT_ALLAC(target))
					REGPOOL_SAVE(target, $t0);
				break;
			case MidCode::MidInstr::SAVE_LABEL:
			case MidCode::MidInstr::SAVE_STACK:
				if (NOT_ALLAC(target))
					REGPOOL_LOAD(target, $t0);
				break;
			case MidCode::MidInstr::ADD:
			case MidCode::MidInstr::SUB:
			case MidCode::MidInstr::MUL:
			case MidCode::MidInstr::DIV:
				if (NOT_ALLAC(target))
					REGPOOL_SAVE(target, $t0);
				if (NOT_ALLAC(source_a))
					REGPOOL_LOAD(source_a, $t1);
				if (NOT_ALLAC(source_b))
					REGPOOL_LOAD(source_b, $t2);
				break;
				
			case MidCode::MidInstr::BLT:
			case MidCode::MidInstr::BGT:
			case MidCode::MidInstr::BLE:
			case MidCode::MidInstr::BGE:
			case MidCode::MidInstr::BEQ:
			case MidCode::MidInstr::BNE:
				if (NOT_ALLAC(target))
					REGPOOL_LOAD(target, $t0);
				if (NOT_ALLAC(source_a))
					REGPOOL_LOAD(source_a, $t1);
			case MidCode::MidInstr::BGEZ:
				if (NOT_ALLAC(target))
					REGPOOL_LOAD(target, $t0);
				break;
				
			case MidCode::MidInstr::PUSH_REG:
				if (NOT_ALLAC(target))
					REGPOOL_LOAD(target, $t0);
				break;
			case MidCode::MidInstr::LOAD_STA_ARR:
				if(NOT_ALLAC(target)) {
					REGPOOL_SAVE(target, $t0);
				}
				if(NOT_ALLAC(source_a)) {
					REGPOOL_LOAD(source_a, $t1);
				}
				break;
			case MidCode::MidInstr::SAVE_STA_ARR:
				if (NOT_ALLAC(target)) {
					REGPOOL_LOAD(target, $t0);
				}
				if (NOT_ALLAC(source_a)) {
					REGPOOL_LOAD(source_a, $t1);
				}
				break;
			case MidCode::MidInstr::LOAD_ADDR:
				if(NOT_ALLAC(target)) {
					REGPOOL_LOAD(target, $t0);
				}
				break;
			default:
				break;
			}
		}
	}
	
	inline void GenMips::genInstrs(MidCode::Block& block) {
		output(FORMAT("{}:",block.label));
		vector<MidCode::MidInstr> instrs = block.instrs;
		for (int i = 0; i < instrs.size(); i++) {
			genInstr(instrs[i]);
		}
	}
	
	inline void GenMips::genInstr(MidCode::MidInstr instr) {
		switch (instr.midOp) {
		case MidCode::MidInstr::PRINT_GLOBAL_STR:
			output(FORMAT("la $a0, {}", instr.target));
			output("li $v0, 4");
			output("syscall");
			break;
		case MidCode::MidInstr::PRINT_LINE:
			output(FORMAT("li $a0, '\\n'"));
			output("li $v0, 11");
			output("syscall");
			break;
		case MidCode::MidInstr::PRINT_INT:
			output(FORMAT("move $a0, {}", instr.target));
			output("li $v0, 1");
			output("syscall");
			break;
		case MidCode::MidInstr::PRINT_CHAR:
			output(FORMAT("move $a0, {}", instr.target));
			output("li $v0, 11");
			output("syscall");
			break;
		case MidCode::MidInstr::SCAN_INT:
			output("li $v0, 5");
			output("syscall");
			output(FORMAT("sw $v0, {}($sp)", instr.target));
			break;
		case MidCode::MidInstr::SCAN_CHAR:
			output("li $v0, 12");
			output("syscall");
			output(FORMAT("sw $v0, {}($sp)", instr.target));
			break;
		case MidCode::MidInstr::SCAN_GLOBAL_INT:
			output("li $v0, 5");
			output("syscall");
			output(FORMAT("sw $v0, {}", instr.target));
			break;
		case MidCode::MidInstr::SCAN_GLOBAL_CHAR:
			output("li $v0, 12");
			output("syscall");
			output(FORMAT("sw $v0, {}", instr.target));
			break;
			
		case MidCode::MidInstr::ADD:
			output(FORMAT("addu {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidCode::MidInstr::SUB:
			output(FORMAT("subu {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidCode::MidInstr::MUL:
			output(FORMAT("mul {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidCode::MidInstr::DIV:
			output(FORMAT("div {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
			
		case MidCode::MidInstr::BEQ:
			output(FORMAT("beq {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidCode::MidInstr::BNE:
			output(FORMAT("bne {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidCode::MidInstr::BLE:
			output(FORMAT("ble {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidCode::MidInstr::BGE:
			output(FORMAT("bge {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidCode::MidInstr::BLT:
			output(FORMAT("blt {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidCode::MidInstr::BGT:
			output(FORMAT("bgt {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidCode::MidInstr::BGEZ:
			output(FORMAT("bgez {}, {}", instr.target, instr.source_a));
			break;
		case MidCode::MidInstr::BLEZ:
			output(FORMAT("blez {}, {}", instr.target, instr.source_a));
			break;
			
		case MidCode::MidInstr::LOAD_LABEL:
			output(FORMAT("lw {}, {}", instr.target, instr.source_a));
			break;
		case MidCode::MidInstr::LOAD_STACK:
			output(FORMAT("lw {}, {}($sp)", instr.target, instr.source_a));
			break;
		case MidCode::MidInstr::SAVE_LABEL:
			output(FORMAT("sw {}, {}", instr.target, instr.source_a));
			break;
		case MidCode::MidInstr::SAVE_STACK:
			output(FORMAT("sw {}, {}($sp)", instr.target, instr.source_a));
			break;
		case MidCode::MidInstr::LOAD_LAB_IMM:
			output(FORMAT("lw {}, {}+{}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidCode::MidInstr::SAVE_LAB_IMM:
			output(FORMAT("sw {}, {}+{}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidCode::MidInstr::LOAD_STA_ARR:
			output(FORMAT("lw {}, ({})", instr.target, instr.source_a));
			break;
		case MidCode::MidInstr::SAVE_STA_ARR:
			output(FORMAT("sw {}, ({})", instr.target, instr.source_a));
			break;
		case MidCode::MidInstr::LOAD_ADDR:
			output(FORMAT("la {}, {}", instr.target, instr.source_a));
			break;
			
		case MidCode::MidInstr::PUSH:
			output(FORMAT("addi $sp, $sp, -{}", instr.target));
			break;
		case MidCode::MidInstr::POP:
			output(FORMAT("addi $sp, $sp, {}", instr.target));
			break;
		case MidCode::MidInstr::CALL:
			output(FORMAT("jal {}", instr.target));
			break;
		case MidCode::MidInstr::RETURN:
			output(FORMAT("jr $ra"));
			break;
		case MidCode::MidInstr::JUMP:
			output(FORMAT("j {}", instr.target));
			break;
		case MidCode::MidInstr::PUSH_REG:
			output("addi $sp, $sp, -4");
			output(FORMAT("sw {}, ($sp)", instr.target));
			break;
		case MidCode::MidInstr::POP_REG:
			output(FORMAT("lw {}, ($sp)", instr.target));
			output("addi $sp, $sp, 4");
			break;
		default:break;
		}
	}

	inline void GenMips::insertAfter(vector<MidCode::MidInstr>& instrs, int& i, MidCode::MidInstr instr) {
		instrs.insert(instrs.begin() + i + 1, instr);
	}

	void GenMips::insertBefore(vector<MidCode::MidInstr>& instrs, int& i, MidCode::MidInstr instr) {
		instrs.insert(instrs.begin() + i, instr);
		i++;
	}
	
}
