#pragma once
#include "../tools/meow.hpp"
#include "../MidCode/MidInstr.hpp"
#include "RegPool.hpp"

using namespace std;

namespace GenObject {
	
	class GenMips {

		MidIR::MidCode midCodes;
		ofstream& fout;
		bool output_mips = false;

		RegPool reg_pool;

		string cur_func_name{};
		
	public:
		GenMips(MidIR::MidCode midCodes, ofstream& fout, string output_setting) : midCodes(midCodes), fout(fout) {
			if (output_setting.find('o') != std::string::npos) {
				output_mips = true;
			}
		}
		
		void write(string);
		
		void gen();
		void genRegPool();
		void genGlobals();
		void genGlobal(MidIR::GlobalDefine);
		void genFuncs();
		void genFunc(MidIR::Func);
		void genBlock(MidIR::Block& block, int block_num);
		void genInstrs(MidIR::Block& block);

		void insertAfter(vector<MidIR::MidInstr>& instrs, int& i, MidIR::MidInstr instr);
		void insertBefore(vector<MidIR::MidInstr>& instrs, int& i, MidIR::MidInstr instr);
		void assignRegs(MidIR::Block& block, int block_num);
		void genInstr(MidIR::MidInstr instr);
		void pushRegPool(int size);
		void popRegPool(MidIR::MidInstr instr);
		
		vector<int> pushPoolSize;		// 
		vector<std::string> used_regs;	//func used regs
		vector<vector<string>> regs_stack;	//func used regs' stack
		vector<string> need_push_stack;

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
		fout << ".text" << endl;
		write(FORMAT("addi $k1, $gp, {}", REGPOOL_LOC));
		fout << "j main" << endl;
		genFuncs();
	}

	inline void GenMips::genGlobals() {
		vector<MidIR::GlobalDefine>& globals = midCodes.global_defines;
		for (int i = 0; i < globals.size(); i++) {
			if (globals[i].type == MidIR::GlobalDefine::CONST_STR) {
				write(FORMAT("{}:.asciiz \"{}\" ", globals[i].const_str.label, globals[i].const_str.value));
			}
		}
	}

	inline void GenMips::genFuncs() {
		auto& funcs = midCodes.funcs;
		for (int i = 0; i < funcs.size(); i++) {
			reg_pool.resetPool();
			regs_stack.clear();
			genFunc(funcs[i]);
		}
	}
	
	inline void GenMips::genFunc(MidIR::Func func) {
		write("");
		cur_func_name = func.func_name;
		auto& blocks = *(func.blocks);
		for (int i = 0; i < blocks.size(); i++) {
			genBlock(blocks[i],i);
		}
	}

	inline void GenMips::genBlock(MidIR::Block& block, int block_num) {
		write(FORMAT("{}:", block.label));
		assignRegs(block,block_num);
		genInstrs(block);
	}

	inline void GenMips::genInstrs(MidIR::Block& block) {
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
	
	inline void GenMips::assignRegs(MidIR::Block& block, int block_num) {
		
		vector<MidIR::MidInstr>& instrs = block.instrs;
		map<string, MidIR::VarRange> ident_to_range = midCodes.func_to_identRange[cur_func_name];
		
		for (int i = 0; i < instrs.size(); i++) {
			if (!ident_to_range.empty()) {
				// 若不是gen MIPS 阶段生成的指令（无行号），则刷新RegPool
				if (instrs[i].line_num_in_block != -1) {
					reg_pool.refrashPool(ident_to_range, block_num, instrs[i].line_num_in_block);
				}

				//尝试为中间代码中的临时变量分配寄存器
				if (instrs[i].isSave_target()) {
					assignReg(instrs[i].target);
				}
				else {
					getReg(instrs[i].target);
				}
				if (instrs[i].isSave_a()) {
					assignReg(instrs[i].source_a);
				}
				else {
					getReg(instrs[i].source_a);
				}
				if (instrs[i].isSave_b()) {
					assignReg(instrs[i].source_b);
				}
				else {
					getReg(instrs[i].source_b);
				}

				//对还未分配寄存器的中间变量，使用内存
				if (HAVE_NOT_ALLAC(target)) {
					if (instrs[i].isLoad_target())
						REGPOOL_LOAD(target, $a1);
					if (instrs[i].isSave_target())
						REGPOOL_SAVE(target, $a1);
				}
				if (HAVE_NOT_ALLAC(source_a)) {
					if (instrs[i].isLoad_a())
						REGPOOL_LOAD(source_a, $a2);
					if (instrs[i].isSave_a())
						REGPOOL_SAVE(source_a, $a2);
				}
				if (HAVE_NOT_ALLAC(source_b)) {
					if (instrs[i].isLoad_b())
						REGPOOL_LOAD(source_b, $a3);
					if (instrs[i].isSave_b())
						REGPOOL_SAVE(source_b, $a3);
				}
			}

			//对函数调用前后的现场保存PUSH_REGPOOL与POP_REGPOOL特殊处理
			if (instrs[i].midOp == MidIR::MidInstr::PUSH_REGPOOL) {		// 保存当前使用过的寄存器
				// int push_num = stoi(instrs[i].target);
				
				used_regs.clear();
				for (int i = 0; i < globalRegs.size(); i++) {	//记录下检查所有寄存器的使用情况
					if (reg_pool.regAvailStateMap[globalRegs[i]] == false) {
						used_regs.push_back(globalRegs[i]);	// 记录下将使用过的寄存器
					}
				}
				regs_stack.push_back(used_regs);
				for (int j = 0; j < used_regs.size(); j++) {	//对所有使用过的寄存器压栈
					insertAfter(instrs, i, { MidIR::MidInstr::PUSH_REG, used_regs[j] });
					// push_num++;
				}
				// instrs[i].source_b = to_string(reg_pool.used_mem_size);	//no use
				need_push_stack.push_back(instrs[i].target);
				
			} else if (instrs[i].midOp == MidIR::MidInstr::POP_REGPOOL) {
				// 保存的所有使用过的寄存器出栈
				used_regs = regs_stack.back();
				for (int j = 0; j < used_regs.size(); j++) {
					insertBefore(instrs, i, { MidIR::MidInstr::POP_REG, used_regs[j] });
				}
				regs_stack.pop_back();
				
				instrs[i].source_a = to_string(-4 * pushPoolSize.back());
				pushPoolSize.pop_back();
				
				reg_pool.popMemPool();
			} else if (instrs[i].midOp == MidIR::MidInstr::CALL) {
				if (need_push_stack.back() == instrs[i].target) {
					pushPoolSize.push_back(reg_pool.used_mem_size);
					instrs[i].source_a = to_string(4 * pushPoolSize.back());
					reg_pool.pushMemPool();

					need_push_stack.pop_back();
				}
				else {
					instrs[i].source_a = "";
				}
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
			if (isNumber(instr.target)) {
				write(FORMAT("li $a0, {}", instr.target));
			} else {
				write(FORMAT("move $a0, {}", instr.target));
			}
			write("li $v0, 1");
			write("syscall");
			break;
		case MidIR::MidInstr::PRINT_CHAR:
			if (isNumber(instr.target)) {
				write(FORMAT("li $a0, {}", instr.target));
			} else {
				write(FORMAT("move $a0, {}", instr.target));
			}
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
			if(startWith(instr.target, string("$"))) {
				write(FORMAT("move {}, $v0", instr.target));
			} else {
				write(FORMAT("sw $v0, {}($fp)", instr.target));
			}
			break;
		case MidIR::MidInstr::SCAN_CHAR:
			write("li $v0, 12");
			write("syscall");
			if (startWith(instr.target, string("$"))) {
				write(FORMAT("move {}, $v0", instr.target));
			}
			else {
				write(FORMAT("sw $v0, {}($fp)", instr.target));
			}
			break;
			
		case MidIR::MidInstr::ADD:
			if (isNumber(instr.source_b)) {
				write(FORMAT("addi {}, {}, {}", instr.target, instr.source_a, instr.source_b));
				break;
			}
			if (isNumber(instr.source_a)) {
				write(FORMAT("addi {}, {}, {}", instr.target, instr.source_b, instr.source_a));
				break;
			}
			write(FORMAT("addu {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::SUB:
			if (isNumber(instr.source_b)) {
				write(FORMAT("subi {}, {}, {}", instr.target, instr.source_a, instr.source_b));
				break;
			}
			if (isNumber(instr.source_a)) {
				write(FORMAT("li $k0, {}", instr.source_a));
				instr.source_a = "$k0";
				write(FORMAT("sub {}, {}, {}", instr.target, instr.source_a, instr.source_b));
				break;
			}
			write(FORMAT("subu {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::MUL:
			if (isNumber(instr.source_b)) {
				// if(_Is_pow_2(stoi(instr.source_b))) {
				// 	write(FORMAT("sll {}, {}, {}", instr.target, instr.source_a, log2(stoi(instr.source_b))));
				// 	break;
				// }
				write(FORMAT("li $k0, {}", instr.source_b));
				instr.source_b = "$k0";
			}
			if (isNumber(instr.source_a)) {
				// if (_Is_pow_2(stoi(instr.source_a))) {
				// 	write(FORMAT("sll {}, {}, {}", instr.target, instr.source_b, log2(stoi(instr.source_a))));
				// 	break;
				// }
				write(FORMAT("li $k0, {}", instr.source_a));
				instr.source_a = "$k0";
			}
			write(FORMAT("mul {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::DIV:
			if (isNumber(instr.source_b)) {
				if (is_power_2(stoi(instr.source_b)) && stoi(instr.source_b) > 0) {
					write(FORMAT("srl {}, {}, {}", instr.target, instr.source_a, log2(stoi(instr.source_b))));
					break;
				}
				write(FORMAT("li $k0, {}", instr.source_b));
				instr.source_b = "$k0";
			}
			if (isNumber(instr.source_a)) {
				write(FORMAT("li $k0, {}", instr.source_a));
				instr.source_a = "$k0";
			}
			write(FORMAT("div {}, {}", instr.source_a, instr.source_b));
			write(FORMAT("mflo {}", instr.target));
			break;
		case MidIR::MidInstr::LI:
			write(FORMAT("li {}, {}", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::LA:
			write(FORMAT("la {}, {}", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::MOVE:
			write(FORMAT("move {}, {}", instr.target, instr.source_a));
			break;
			
		case MidIR::MidInstr::BEQ:
			if (isNumber(instr.target)) {
				write(FORMAT("beq {}, {}, {}", instr.source_a, instr.target, instr.source_b));
				break;
			}
			write(FORMAT("beq {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::BNE:
			if (isNumber(instr.target)) {
				write(FORMAT("bne {}, {}, {}", instr.source_a, instr.target, instr.source_b));
				break;
			}
			write(FORMAT("bne {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::BGT:
			if (isNumber(instr.target)) {
				write(FORMAT("blt {}, {}, {}", instr.source_a, instr.target, instr.source_b));
				break;
			}
			write(FORMAT("bgt {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::BGE:
			if (isNumber(instr.target)) {
				write(FORMAT("ble {}, {}, {}", instr.source_a, instr.target, instr.source_b));
				break;
			}
			write(FORMAT("bge {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::BLT:
			if (isNumber(instr.target)) {
				write(FORMAT("bgt {}, {}, {}", instr.source_a, instr.target, instr.source_b));
				break;
			}
			write(FORMAT("blt {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::BLE:
			if (isNumber(instr.target)) {
				write(FORMAT("bge {}, {}, {}", instr.source_a, instr.target, instr.source_b));
				break;
			}
			write(FORMAT("ble {}, {}, {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidIR::MidInstr::BGTZ:
			write(FORMAT("bgtz {}, {}", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::BGEZ:
			write(FORMAT("bgez {}, {}", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::BLTZ:
			write(FORMAT("bltz {}, {}", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::BLEZ:
			write(FORMAT("blez {}, {}", instr.target, instr.source_a));
			break;
			
		case MidIR::MidInstr::LOAD_GLOBAL:
			write(FORMAT("lw {}, {}($gp)", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::LOAD_STACK:
			write(FORMAT("lw {}, {}($fp)", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::SAVE_GLOBAL:
			if (isNumber(instr.target)) {
				write(FORMAT("li $k0, {}", instr.target));
				instr.target = "$k0";
			}
			write(FORMAT("sw {}, {}($gp)", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::SAVE_STACK:
			if (isNumber(instr.target)) {
				write(FORMAT("li $k0, {}", instr.target));
				instr.target = "$k0";
			}
			write(FORMAT("sw {}, {}($fp)", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::LOAD_GLOBAL_ARR:
			// lw temp arr_offset($gp+sub_reg>>4)
			if (isNumber(instr.source_b)) {
				write(FORMAT("addi $k0, $gp, {}", stoi(instr.source_b) * 4));
			} else {
				write(FORMAT("sll $k0, {}, 2", instr.source_b));
				write(FORMAT("add $k0, $k0, $gp"));
			}
			if (isNumber(instr.target)) {
				write(FORMAT("li $at, {}", instr.target));
				instr.target = "$at";
			}
			write(FORMAT("lw {}, {}($k0)",instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::SAVE_GLOBAL_ARR:
			// sw expr_ans arr_offset($gp+sub_reg>>4)
			if (isNumber(instr.source_b)) {
				write(FORMAT("addi $k0, $gp, {}", stoi(instr.source_b) * 4));
			} else {
				write(FORMAT("sll $k0, {}, 2", instr.source_b));
				write("add $k0, $k0, $gp");
			}
			if (isNumber(instr.target)) {
				write(FORMAT("li $at, {}", instr.target));
				instr.target = "$at";
			}
			write(FORMAT("sw {}, {}($k0)", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::LOAD_STACK_ARR:
			if (isNumber(instr.source_b)) {
				write(FORMAT("addi $k0, $fp, -{}", stoi(instr.source_b) * 4));
			} else {
				write(FORMAT("sll $k0, {}, 2", instr.source_b));
				write("sub $k0, $fp, $k0");
			}
			if (isNumber(instr.target)) {
				write(FORMAT("li $at, {}", instr.target));
				instr.target = "$at";
			}
			write(FORMAT("lw {}, {}($k0)", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::SAVE_STACK_ARR:
			if (isNumber(instr.source_b)) {
				write(FORMAT("addi $k0, $fp, -{}", stoi(instr.source_b) * 4));
			} else {
				write(FORMAT("sll $k0, {}, 2", instr.source_b));
				write("sub $k0, $fp, $k0");
			}
			if (isNumber(instr.target)) {
				write(FORMAT("li $at, {}", instr.target));
				instr.target = "$at";
			}
			write(FORMAT("sw {}, {}($k0)", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::LOAD_POOL:
			write(FORMAT("lw {}, {}($k1)", instr.target, instr.source_a));
			break;
		case MidIR::MidInstr::SAVE_POOL:
			write(FORMAT("sw {}, {}($k1)", instr.target, instr.source_a));
			break;;
			
		case MidIR::MidInstr::PUSH:
			write(FORMAT("addi $sp, $sp, -{}", instr.target));
			break;
		case MidIR::MidInstr::POP:
			write(FORMAT("addi $sp, $sp, {}", instr.target));
			break;
		case MidIR::MidInstr::PUSH_REG:
			if (isNumber(instr.target)) {
				write(FORMAT("li $k0, {}", instr.target));
				instr.target = "$k0";
			}
			write(FORMAT("sw {}, ($sp)", instr.target));
			write("addi $sp, $sp, -4");
			break;
		case MidIR::MidInstr::POP_REG:
			write("addi $sp, $sp, 4");
			write(FORMAT("lw {}, ($sp)", instr.target));
			break;
		case MidIR::MidInstr::PUSH_REGPOOL:
			// pushRegPool(stoi(instr.source_b));//no use
			break;
		case MidIR::MidInstr::POP_REGPOOL:
			popRegPool(instr);
			break;
		case MidIR::MidInstr::CALL:
			if (isNumber(instr.source_a)) {
				write(FORMAT("addi $k1, $k1, {}", instr.source_a));
			}
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

	inline void GenMips::pushRegPool(int size) {	// 保存所有使用过的mem到栈中
	}

	inline void GenMips::popRegPool(MidIR::MidInstr instr) {		//恢复mempool 从栈中
		write(FORMAT("addi $k1, $k1, {}", instr.source_a));
	}

}
