#pragma once
#include "../MidCode/MidInstr.hpp"
#include "RegPool.hpp"

using namespace std;

namespace GenObject {
	
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
		void genInstrs(MidIR::Block& block);

		void insertAfter(vector<MidIR::MidInstr>& instrs, int& i, MidIR::MidInstr instr);
		void insertBefore(vector<MidIR::MidInstr>& instrs, int& i, MidIR::MidInstr instr);
		void assignRegs(MidIR::Block& block);
		void genInstr(MidIR::MidInstr instr);
		void pushRegPool();
		void popRegPool();
		
		// void getRegisters(std::vector<Reg> regs);
		// void releaseRegisters(std::vector<Reg> regs);

		//Reg and mem  assign

		std::vector<std::string> globalRegs = {
			"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
			"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
			"$v1"
		};

		int mempool_size = 0;
		map<string, int> memPool;		//mem to loc
		vector<int> pushPoolSize;		// 
		map<string, string> regPool;	//_T{} to reg
		map<string, bool> availReg;		// reg to bool
		vector<std::string> used_regs;	//func used regs
		vector<vector<string>> regs_stack;	//func used regs' stack
		
		void resetPool();
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
			resetPool();
			genFunc(funcs[i]);
		}
	}
	
	inline void GenMips::genFunc(MidIR::Func func) {
		write("");
		string func_name = func.func_name;
		auto& blocks = *(func.blocks);
		for (int i = 0; i < blocks.size(); i++) {
			genBlock(blocks[i]);
		}
	}

	inline void GenMips::genBlock(MidIR::Block& block) {
		assignRegs(block);
		genInstrs(block);
	}

	inline void GenMips::genInstrs(MidIR::Block& block) {
		write(FORMAT("{}:", block.label));
		vector<MidIR::MidInstr> instrs = block.instrs;
		for (int i = 0; i < instrs.size(); i++) {
			genInstr(instrs[i]);
		}
	}

	inline void GenMips::insertAfter(vector<MidIR::MidInstr>& instrs, int& i, MidIR::MidInstr instr) {
		instrs.insert(instrs.begin() + i + 1, instr);
	}

	void GenMips::insertBefore(vector<MidIR::MidInstr>& instrs, int& i, MidIR::MidInstr instr) {
		instrs.insert(instrs.begin() + i, instr);
		i++;
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
	
	inline void GenMips::assignRegs(MidIR::Block& block) {
		
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
			case MidIR::MidInstr::LOAD_GLOBAL_ARR:
				if (NOT_ALLAC(target)) {
					REGPOOL_SAVE(target, $t0);
				}
				if (NOT_ALLAC(source_a)) {
					REGPOOL_LOAD(source_a, $t1);
				}
				if (NOT_ALLAC(source_b)) {
					REGPOOL_LOAD(source_b, $t1);
				}
				break;
			case MidIR::MidInstr::SAVE_STA_ARR:
			case MidIR::MidInstr::SAVE_GLOBAL_ARR:
				if (NOT_ALLAC(target)) {
					REGPOOL_LOAD(target, $t0);
				}
				if (NOT_ALLAC(source_a)) {
					REGPOOL_LOAD(source_a, $t1);
				}
				if (NOT_ALLAC(source_b)) {
					REGPOOL_LOAD(source_b, $t1);
				}
				break;
			
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
			
			case MidIR::MidInstr::PUSH_REGPOOL:		// 保存当前使用过的寄存器
				used_regs.clear();
				for (int i = 0; i < globalRegs.size(); i++) {
					if (availReg[globalRegs[i]] == false) {
						used_regs.push_back(globalRegs[i]);
					}
				}
				regs_stack.push_back(used_regs);
				for (int j = 0; j < used_regs.size(); j++) {
					insertAfter(instrs, i, {MidIR::MidInstr::PUSH_REG, used_regs[j] });
				}
				break;
			case MidIR::MidInstr::POP_REGPOOL:		// 恢复函数调用前使用过的寄存器
				used_regs = regs_stack.back();
				regs_stack.pop_back();
				for (int j = 0; j < used_regs.size(); j++) {
					insertBefore(instrs, i, {MidIR::MidInstr::POP_REG, used_regs[j] });
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
		case MidIR::MidInstr::LOAD_GLOBAL_ARR:
			// lw temp arr_offset($gp+sub_reg>>4)
			write(FORMAT("sll $k0, {}, 2", instr.source_b));
			write(FORMAT("add $k0, $k0, $gp"));
			write(FORMAT("lw {}, {}($k0)",instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::SAVE_GLOBAL_ARR:
			// sw expr_ans arr_offset($gp+sub_reg>>4)
			write(FORMAT("sll $k0, {}, 2", instr.source_b));
			write("add $k0, $k0, $gp");
			write(FORMAT("sw {}, {}($k0)", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::LOAD_STA_ARR:
			// write(FORMAT("lw {}, ({})", instr.target, instr.source_a));
			write(FORMAT("sll $k0, {}, 2", instr.source_b));
			write("sub $k0, $fp, $k0");
			write(FORMAT("lw {}, {}($k0)", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::SAVE_STA_ARR:
			// write(FORMAT("sw {}, ({})", instr.target, instr.source_a));
			write(FORMAT("sll $k0, {}, 2", instr.source_b));
			write("sub $k0, $fp, $k0");
			write(FORMAT("sw {}, {}($k0)", instr.target, instr.source_a));
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

	inline void GenMips::pushRegPool() {	// 保存所有使用过的mem 到栈中
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

	inline void GenMips::popRegPool() {		//恢复mempool 从栈中
		int pop_mempool_size= pushPoolSize.back();
		for (int i = pop_mempool_size - 1; i >= 0; i--) {
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

#pragma region RegAssgin
	inline void GenMips::resetPool() {	// 不同function刷新寄存器与内存池
		mempool_size = 0;
		memPool.clear();
		regPool.clear();
		for (int i = 0; i < globalRegs.size(); i++) {
			availReg[globalRegs[i]] = true;
		}
		regs_stack.clear();
	}

	inline int GenMips::allocMemPool(string id) {
		memPool[id] = mempool_size;
		return mempool_size++;
	}

	inline int GenMips::getMemPool(string id) {
		if (memPool.find(id) == memPool.end()) { 
			return allocMemPool(id);	// 还未分配内存空间则进行分配
		}
		return memPool[id];	//已分配内存空间
	}

	inline string GenMips::getReg() {	// 获取第一空闲的寄存器
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
		if (regPool.find(id) == regPool.end()) {	//如果未分配
			if (haveAvailReg()) {	// 有空的寄存器
				allocRegPool(id);	//分配寄存器
			} else {
				return id;	//无空寄存器，返回原id: _T{}
			}
		}
		return regPool[id];	//如果已分配
	}

#pragma endregion
	
}
