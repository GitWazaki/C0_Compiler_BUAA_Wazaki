#pragma once
#include "../MidCode/MidInstr.hpp"
#include "RegPool.hpp"

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

		MidIR::MidCode midCodes;
		ofstream& fout;
		bool output_mips = false;

		void write(string);
		
	public:
		GenMips(MidIR::MidCode midCodes, ofstream& fout, string output_setting) : midCodes(midCodes), fout(fout) {
			if (output_setting.find('o') != std::string::npos) {
				output_mips = true;
			}
		}
		
		void gen();
		void genRegPool();
		void genGlobals();
		void genGlobal(MidIR::GlobalDefine);
		void genFuncs();
		void genFunc(MidIR::Func);
		void genBlock(MidIR::Block& block);
		// void genText();
		void allocRegs(MidIR::Block& block);
		void genInstrs(MidIR::Block& block);
		void genInstr(MidIR::MidInstr instr);
		void pushRegPool();
		void popRegPool();
		void insertAfter(vector<MidIR::MidInstr>& instrs, int& i, MidIR::MidInstr instr);
		void insertBefore(vector<MidIR::MidInstr>& instrs, int& i, MidIR::MidInstr instr);
		// void getRegisters(std::vector<Reg> regs);
		// void releaseRegisters(std::vector<Reg> regs);

		//Reg assign
		int mempool_size = 0;
		map<string, int> memPool;

		std::vector<std::string> globalRegs = {
			"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
			"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
			"$v1"
		};
		map<string, string> regPool;
		map<string, bool> availReg;
		vector<vector<string>> t_stack;

		vector<std::string> ts;
		std::vector<int> pushPoolSize;
		
		void resetRegPool();
		int allocMemPool(string id);
		int getMemPool(string id);
		string getReg();
		bool haveAvailReg();
		void allocRegPool(string id);
		string getRegPool(string id);
		
	};

	inline void GenMips::write(string str) {
		if (output_mips) {
			fout << str << endl;
		}
	}

	inline void GenMips::gen() {
		fout << "# C0_Compiler_BUAA_Wazaki gen MIPS" << endl;
		fout << ".data" << endl;
		genGlobals();
		// genRegPool();
		fout << ".text" << endl;
		fout << "j main" << endl;
		genFuncs();
	}

#define REGPOOL "REGPOOL"
	
	inline void GenMips::genRegPool() {
		write(FORMAT("{}: \n .align 2 \n .space {}", REGPOOL,POOLSIZE*4));
	}

	inline void GenMips::genGlobals() {
		vector<MidIR::GlobalDefine>& globals = midCodes.global_defines;
		for (int i = 0; i < globals.size(); i++) {
			genGlobal(globals[i]);
		}
	}

	inline void GenMips::genGlobal(MidIR::GlobalDefine define) {
		switch (define.type) {
		case MidIR::GlobalDefine::CONST_STR:
			write(FORMAT("{}:.asciiz \"{}\" ",define.const_str.label,define.const_str.value));
			break;
		// case MidIR::GlobalDefine::VAR_INT:
		// 	write(FORMAT("{}: .space 4", define.var_int.label));
		// 	break;
		// case MidIR::GlobalDefine::VAR_INT_ARRAY:
		// 	write(FORMAT("{}: .space {}", define.var_int_array.label, 4 * define.var_int_array.len));
		// 	break;
		default:
			break;
		}
	}

	inline void GenMips::genFuncs() {
		auto& funcs = midCodes.funcs;
		for (int i = 0; i < funcs.size(); i++) {
			resetRegPool();
			genFunc(funcs[i]);
		}
	}
	
	inline void GenMips::genFunc(MidIR::Func func) {
		string func_name = func.func_name;
		//write(FORMAT("{}:", func_name));
		auto& blocks = *(func.blocks);
		for (int i = 0; i < blocks.size(); i++) {
			genBlock(blocks[i]);
		}
	}

	inline void GenMips::genBlock(MidIR::Block& block) {
		allocRegs(block);
		genInstrs(block);
	}

#define REGPOOL_START 10000 * 4

#define NOT_ALLAC(ir_reg) startWith(instrs[i].ir_reg, std::string("_T"))

#define REGPOOL_LOAD(ir_reg, assign_reg)	\
		do {	\
			auto& instr = instrs[i];	\
			int loc = 4 * getMemPool(instr.ir_reg.substr(2));	\
			instr.ir_reg = #assign_reg;	\
			insertBefore(instrs, i, MidIR::MidInstr(MidIR::MidInstr::LOAD_GLOBAL, #assign_reg, REGPOOL_START+loc));	\
		} while(0)

#define REGPOOL_SAVE(ir_reg, assign_reg)	\
		do {	\
			auto& instr = instrs[i];	\
			int loc = 4 * getMemPool(instr.ir_reg.substr(2));	\
			instr.ir_reg = #assign_reg;	\
			insertAfter(instrs, i,MidIR::MidInstr(MidIR::MidInstr::SAVE_GLOBAL, #assign_reg, REGPOOL_START+loc));	\
		} while(0)

#define assignReg(ir_reg)	\
		do {	\
			if (startWith(ir_reg, std::string("_T"))) {	\
				ir_reg = getRegPool(ir_reg);	\
			} \
		} while(0)
	
	// TODO
	inline void GenMips::allocRegs(MidIR::Block& block) {
		
		vector<MidIR::MidInstr>& instrs = block.instrs;
		
		for (int i = 0; i < instrs.size(); i++) {
			
			auto& instr = instrs[i];
			assignReg(instr.target);
			assignReg(instr.source_a);
			assignReg(instr.source_b);
			
			switch (instr.midOp) {
			case MidIR::MidInstr::PRINT_LINE: break;
			case MidIR::MidInstr::PRINT_GLOBAL_STR: break;
			case MidIR::MidInstr::PRINT_INT:
			case MidIR::MidInstr::PRINT_CHAR:
				if (NOT_ALLAC(target))
					REGPOOL_LOAD(target, $t0);
				break;
			case MidIR::MidInstr::SCAN_INT: break;
			case MidIR::MidInstr::SCAN_CHAR: break;
			case MidIR::MidInstr::SCAN_GLOBAL_INT: break;
			case MidIR::MidInstr::SCAN_GLOBAL_CHAR: break;
			case MidIR::MidInstr::LOAD_STA_ARR:
			case MidIR::MidInstr::SAVE_STA_ARR:
				if (NOT_ALLAC(target)) {
					REGPOOL_SAVE(target, $t0);
				}
				if (NOT_ALLAC(source_a)) {
					REGPOOL_LOAD(source_a, $t1);
				}
				break;
			
			// case MidIR::MidInstr::LOAD_LABEL:
			// case MidIR::MidInstr::SAVE_LABEL:
			case MidIR::MidInstr::LOAD_GLOBAL:
			case MidIR::MidInstr::LOAD_STACK:
				if (NOT_ALLAC(target))
					REGPOOL_SAVE(target, $t0);
				break;
			case MidIR::MidInstr::SAVE_GLOBAL:
			case MidIR::MidInstr::SAVE_STACK:
				if (NOT_ALLAC(target))
					REGPOOL_LOAD(target, $t0);
				break;
				
			case MidIR::MidInstr::ADD:
			case MidIR::MidInstr::SUB:
			case MidIR::MidInstr::MUL:
			case MidIR::MidInstr::DIV:
			case MidIR::MidInstr::MOVE:
			case MidIR::MidInstr::LI:
				if (NOT_ALLAC(target))
					REGPOOL_SAVE(target, $t0);
				if (NOT_ALLAC(source_a))
					REGPOOL_LOAD(source_a, $t1);
				if (NOT_ALLAC(source_b))
					REGPOOL_LOAD(source_b, $t2);
				break;
			case MidIR::MidInstr::LA:
				if (NOT_ALLAC(target)) {
					REGPOOL_LOAD(target, $t0);
				}
				break;
				
			case MidIR::MidInstr::BLT:
			case MidIR::MidInstr::BGT:
			case MidIR::MidInstr::BLE:
			case MidIR::MidInstr::BGE:
			case MidIR::MidInstr::BEQ:
			case MidIR::MidInstr::BNE:
				if (NOT_ALLAC(target))
					REGPOOL_LOAD(target, $t0);
				if (NOT_ALLAC(source_a))
					REGPOOL_LOAD(source_a, $t1);
			case MidIR::MidInstr::BGEZ:
			case MidIR::MidInstr::BLEZ:
				if (NOT_ALLAC(target))
					REGPOOL_LOAD(target, $t0);
				break;
				
			case MidIR::MidInstr::PUSH_REG:
				if (NOT_ALLAC(target))
					REGPOOL_LOAD(target, $t0);
				break;
			case MidIR::MidInstr::POP_REG: break;
			
			case MidIR::MidInstr::PUSH_REGPOOL:
				ts.clear();
				for (int i = 0; i < globalRegs.size(); i++) {
					if (availReg[globalRegs[i]] == false) {
						ts.push_back(globalRegs[i]);
					}
				}
				t_stack.push_back(ts);
				for (int j = 0; j < ts.size(); j++) {
					insertAfter(instrs, i, {MidIR::MidInstr::PUSH_REG, ts[j] });
				}
				break;
			case MidIR::MidInstr::POP_REGPOOL:
				ts = t_stack.back();
				t_stack.pop_back();
				for (int j = 0; j < ts.size(); j++) {
					insertBefore(instrs, i, {MidIR::MidInstr::POP_REG, ts[j] });
				}
				break;
				
			case MidIR::MidInstr::CALL: break;
			case MidIR::MidInstr::RETURN: break;
			case MidIR::MidInstr::JUMP: break;
				
			default:
				break;
			}
		}
	}
	
	inline void GenMips::genInstrs(MidIR::Block& block) {
		write(FORMAT("{}:",block.label));
		vector<MidIR::MidInstr> instrs = block.instrs;
		for (int i = 0; i < instrs.size(); i++) {
			genInstr(instrs[i]);
		}
	}
	
	inline void GenMips::genInstr(MidIR::MidInstr instr) {
		switch (instr.midOp) {
		case MidIR::MidInstr::PRINT_LINE:
			write(FORMAT("li $a0, '\\n'"));
			write("li $v0, 11");
			write("syscall");
			break;
		case MidIR::MidInstr::PRINT_GLOBAL_STR:
			write(FORMAT("la $a0, {}", instr.target));
			write("li $v0, 4");
			write("syscall");
			break;
		case MidIR::MidInstr::PRINT_INT:
			write(FORMAT("move $a0, {}", instr.target));
			write("li $v0, 1");
			write("syscall");
			break;
		case MidIR::MidInstr::PRINT_CHAR:
			write(FORMAT("move $a0, {}", instr.target));
			write("li $v0, 11");
			write("syscall");
			break;
		case MidIR::MidInstr::SCAN_GLOBAL_INT:
			write("li $v0, 5");
			write("syscall");
			write(FORMAT("sw $v0, {}($gp)", instr.target));
			break;
		case MidIR::MidInstr::SCAN_GLOBAL_CHAR:
			write("li $v0, 12");
			write("syscall");
			write(FORMAT("sw $v0, {}($gp)", instr.target));
			break;
		case MidIR::MidInstr::SCAN_INT:
			write("li $v0, 5");
			write("syscall");
			write(FORMAT("sw $v0, {}($fp)", instr.target));
			break;
		case MidIR::MidInstr::SCAN_CHAR:
			write("li $v0, 12");
			write("syscall");
			write(FORMAT("sw $v0, {}($fp)", instr.target));
			break;
			
		case MidIR::MidInstr::ADD:
			write(FORMAT("addu {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::SUB:
			write(FORMAT("subu {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::MUL:
			write(FORMAT("mul {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::DIV:
			write(FORMAT("div {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::LI:
			write(FORMAT("li {}, {}", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::MOVE:
			write(FORMAT("move {}, {}", instr.target, instr.source_a));
			break;
			
		case MidIR::MidInstr::BEQ:
			write(FORMAT("beq {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::BNE:
			write(FORMAT("bne {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::BGT:
			write(FORMAT("bgt {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::BGE:
			write(FORMAT("bge {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::BLT:
			write(FORMAT("blt {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::BLE:
			write(FORMAT("ble {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::BGEZ:
			write(FORMAT("bgez {}, {}", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::BLEZ:
			write(FORMAT("blez {}, {}", instr.target, instr.source_a));
			break;
			
		// case MidIR::MidInstr::LOAD_LABEL:
		// 	write(FORMAT("lw {}, {}", instr.target, instr.source_a));
		// 	break;
		// case MidIR::MidInstr::SAVE_LABEL:
		// 	write(FORMAT("sw {}, {}", instr.target, instr.source_a));
		// 	break;
		case MidIR::MidInstr::LOAD_GLOBAL:
			write(FORMAT("lw {}, {}($gp)", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::SAVE_GLOBAL:
			write(FORMAT("sw {}, {}($gp)", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::LOAD_STACK:
			write(FORMAT("lw {}, {}($fp)", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::SAVE_STACK:
			write(FORMAT("sw {}, {}($fp)", instr.target, instr.source_a));
			break;
		// case MidIR::MidInstr::LOAD_LAB_IMM:
		// 	write(FORMAT("lw {}, {}+{}", instr.target, instr.source_a, instr.source_b));
		// 	break;
		// case MidIR::MidInstr::SAVE_LAB_IMM:
		// 	write(FORMAT("sw {}, {}+{}", instr.target, instr.source_a, instr.source_b));
		// 	break;
		case MidIR::MidInstr::LOAD_STA_ARR:
			write(FORMAT("lw {}, ({})", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::SAVE_STA_ARR:
			write(FORMAT("sw {}, ({})", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::LA:
			write(FORMAT("la {}, {}", instr.target, instr.source_a));
			break;
			
		case MidIR::MidInstr::PUSH:
			write(FORMAT("addi $sp, $sp, -{}", instr.target));
			break;
		case MidIR::MidInstr::POP:
			write(FORMAT("addi $sp, $sp, {}", instr.target));
			break;
		case MidIR::MidInstr::PUSH_REG:
			write(FORMAT("sw {}, ($sp)", instr.target));
			write("addi $sp, $sp, -4");
			break;
		case MidIR::MidInstr::POP_REG:
			write("addi $sp, $sp, 4");
			write(FORMAT("lw {}, ($sp)", instr.target));
			break;
		case MidIR::MidInstr::PUSH_REGPOOL:
			pushRegPool();
			break;
		case MidIR::MidInstr::POP_REGPOOL:
			popRegPool();
			break;
			
		case MidIR::MidInstr::CALL:
			write(FORMAT("jal {}", instr.target));
			break;
		case MidIR::MidInstr::RETURN:
			write(FORMAT("jr $ra"));
			break;
		case MidIR::MidInstr::JUMP:
			write(FORMAT("j {}", instr.target));
			break;
		default:break;
		}
	}

	inline void GenMips::pushRegPool() {
		pushPoolSize.push_back(mempool_size);
		for (int i = 0; i < mempool_size; i++) {
			write(FORMAT("lw {}, {}($gp)", "$k0", REGPOOL_START + 4 * i));
			write(FORMAT("sw {}, ($sp)", "$k0"));
			write("addi $sp, $sp, -4");
		}
		// for (int i = 0; i < mempool_size; i++) {
		// 	write(FORMAT("lw {}, {}+{}", "$k0", "REGPOOL", 4 * i));
		// 	write(FORMAT("sw {}, ($sp)", "$k0"));
		// 	write("addi $sp, $sp, -4");
		// }
	}

	inline void GenMips::popRegPool() {
		for (int i = pushPoolSize.back() - 1; i >= 0; i--) {
			write("addi $sp, $sp, 4");
			write(FORMAT("lw {}, ($sp)", "$k0"));
			write(FORMAT("sw {}, {}($gp)", "$k0", REGPOOL_START + 4 * i));
		}
		// for (int i = pushPoolSize.back() - 1; i >= 0; i--) {
		// 	write("addi $sp, $sp, 4");
		// 	write(FORMAT("lw {}, ($sp)", "$k0"));
		// 	write(FORMAT("sw {}, {}+{}", "$k0", "REGPOOL", 4 * i));
		// }
		pushPoolSize.pop_back();
	}

	inline void GenMips::insertAfter(vector<MidIR::MidInstr>& instrs, int& i, MidIR::MidInstr instr) {
		instrs.insert(instrs.begin() + i + 1, instr);
	}

	void GenMips::insertBefore(vector<MidIR::MidInstr>& instrs, int& i, MidIR::MidInstr instr) {
		instrs.insert(instrs.begin() + i, instr);
		i++;
	}

#pragma region RegAssgin
	inline void GenMips::resetRegPool() {
		mempool_size = 0;
		memPool.clear();
		regPool.clear();
		for (int i = 0; i < globalRegs.size(); i++) {
			availReg[globalRegs[i]] = true;
		}
		t_stack.clear();
	}

	inline int GenMips::allocMemPool(string id) {
		memPool[id] = mempool_size;
		return mempool_size++;
	}

	inline int GenMips::getMemPool(string id) {
		if (memPool.find(id) == memPool.end()) {
			return allocMemPool(id);
		}
		return memPool[id];
	}

	inline string GenMips::getReg() {
		for (int i = 0; i < globalRegs.size(); i++) {
			if (availReg[globalRegs[i]]) {
				availReg[globalRegs[i]] = false;
				return globalRegs[i];
			}
		}
	}

	inline bool GenMips::haveAvailReg() {
		for (int i = 0; i < globalRegs.size(); i++) {
			if (availReg[globalRegs[i]]) {
				return true;
			}
		}
		return false;
	}

	inline void GenMips::allocRegPool(string id) {
		string reg = getReg();
		regPool[id] = reg;
	}

	inline string GenMips::getRegPool(string id) {
		if (regPool.find(id) == regPool.end()) {	//Î´·ÖÅä
			if (haveAvailReg()) {	// ÓÐ¿ÕµÄ¼Ä´æÆ÷
				allocRegPool(id);	//·ÖÅä¼Ä´æÆ÷
			} else {
				return id;	//ÎÞ¿Õ¼Ä´æÆ÷£¬·µ»ØÔ­id: _T{}
			}
		}
		return regPool[id];
	}

#pragma endregion
	
}
