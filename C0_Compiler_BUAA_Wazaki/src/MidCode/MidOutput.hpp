#pragma once
using namespace std;

namespace MidIR {
	
	class MidOutput {
		
	public:
		MidOutput(MidCode midCodes, string path, string output_setting): midCodes(midCodes) {
			if (output_setting.find('m') != std::string::npos) {
				output_mid = true;
			}
			fout.open(path);
		}

		void output();
		
	private:
		MidCode midCodes;
		ofstream fout;
		bool output_mid = false;
		string output_offset = "";
		
		void write(string str);
		void outputGloablDefine(GlobalDefine& global_define);
		void outputFunc(Func& func);
		void outputBlock(Block& block);
		void outputInstr(MidInstr instr);
		string removePrefix(string name);
		void pushIndent();
		void popIndent();
	};

	inline void MidOutput::output() {
		write("// C0_Compiler_BUAA_Wazaki gen Mid Code");

		for (int i = 0; i < midCodes.global_defines.size(); i++) {
			outputGloablDefine(midCodes.global_defines[i]);
		}
		write("");	//next line
		for (int i = 0; i < midCodes.funcs.size(); i++) {
			outputFunc(midCodes.funcs[i]);
			write("");
		}
	}

	inline void MidOutput::write(string str) {
		if (output_mid) {
			fout << output_offset << str << endl;
		}
	}

	inline void MidOutput::outputGloablDefine(GlobalDefine& global_define) {
		switch (global_define.type) {
		case GlobalDefine::CONST_STR:
			write(FORMAT("string_{} = \"{}\";", removePrefix(global_define.const_str.label), global_define.const_str.value));
			break;
		case GlobalDefine::VAR_INT:
			write(FORMAT("int {};", removePrefix(global_define.var_int.label)));
			break;
		case GlobalDefine::VAR_INT_ARRAY:
			write(FORMAT("int {}[{}];", removePrefix(global_define.var_int_array.label), global_define.var_int_array.len));
			break;
		default:;
		}
	}

	inline void MidOutput::outputFunc(Func& func) {
		write("");
		write(FORMAT("function {} start", func.func_name));
		pushIndent();
		for(int i = 0; i < func.blocks->size(); i++) {
			outputBlock(func.blocks->at(i));
		}
		popIndent();
		write("end funciton");
	}

	inline void MidOutput::outputBlock(Block& block) {
		write(FORMAT("Block {}",block.label));
		for (int i = 0; i < block.instrs.size(); i++) {
			outputInstr(block.instrs[i]);
		}
	}

	inline void MidOutput::outputInstr(MidInstr instr) {

		instr.target = removePrefix(instr.target);
		instr.source_a = removePrefix(instr.source_a);
		instr.source_b = removePrefix(instr.source_b);
		instr.dup = removePrefix(instr.dup);

		bool showVarName = instr.var_name.size() > 0;

		// write(FORMAT("{} {}, {}, {}",instr.midOp,instr.target,instr.source_a,instr.source_b));
		
		switch (instr.midOp) {
		case MidInstr::PRINT_LINE:
			write("print('\\n')");
			break;
		case MidInstr::PRINT_GLOBAL_STR:
		case MidInstr::PRINT_INT:
		case MidInstr::PRINT_CHAR:
			if (showVarName) {
				// if(instr.has_dup) {
				// 	write(FORMAT("scanf({});", instr.dup));
				// } else {
					write(FORMAT("scanf({});", instr.var_name));
				// }
				break;
			}
			write(FORMAT("print({});", instr.target));
			break;
		case MidInstr::SCAN_GLOBAL_INT:
		case MidInstr::SCAN_GLOBAL_CHAR:
		case MidInstr::SCAN_INT:
		case MidInstr::SCAN_CHAR:
			if (showVarName) {
				// if(instr.has_dup) {
				// 	write(FORMAT("scanf({});", instr.dup));
				// } else {
					write(FORMAT("scanf({});", instr.var_name));
				// }
				break;
			}
			write(FORMAT("scanf({});", instr.target));
			break;
		case MidInstr::ADD:
			write(FORMAT("{} = {} + {};", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::SUB:
			write(FORMAT("{} = {} - {};", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::MUL:
			write(FORMAT("{} = {} * {};", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::DIV:
			write(FORMAT("{} = {} / {};", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::LI:
			write(FORMAT("{} = {};", instr.target, instr.source_a));
			break;
		case MidInstr::LA:
			write(FORMAT("{} = {};", instr.target, instr.source_a));
			break;
		case MidInstr::MOVE:
			write(FORMAT("{} = {};", instr.target, instr.source_a));
			break;
		case MidInstr::BEQ:
			write(FORMAT("if ({} == {}) jump {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::BNE:
			write(FORMAT("if ({} != {}) jump {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::BGT:
			write(FORMAT("if ({} > {}) jump {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::BGE:
			write(FORMAT("if ({} >= {}) jump {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::BLT:
			write(FORMAT("if ({} < {}) jump {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::BLE:
			write(FORMAT("if ({} <= {}) jump {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::BGEZ:
			write(FORMAT("if ({} >= 0) jump {}", instr.target, instr.source_a));
			break;
		case MidInstr::BLEZ:
			write(FORMAT("if ({} <= 0) jump {}", instr.target, instr.source_a));
			break;
			
		// case MidInstr::LOAD_LABEL:
		// case MidInstr::SAVE_LABEL:
		// case MidInstr::LOAD_LAB_IMM:
		// case MidInstr::SAVE_LAB_IMM:
		case MidInstr::LOAD_GLOBAL:
			if(showVarName) {
				// if(instr.has_dup) {
				// 	write(FORMAT("{} = {};", instr.target, instr.dup));
				// } else {
					write(FORMAT("{} = {};", instr.target, instr.var_name));
				// }
			} else {
				write(FORMAT("{} = $gp[{}];", instr.target, instr.source_a));
			}
			break;
		case MidInstr::SAVE_GLOBAL:
			if (showVarName) {
				// if (instr.has_dup) {
				// 	write(FORMAT("{} = {};", instr.dup, instr.target));
				// }
				// else {
					write(FORMAT("{} = {};", instr.var_name, instr.target));
				// }
			} else {
				write(FORMAT("$gp[{}] = {};", instr.source_a, instr.target));
			}
			break;
		case MidInstr::LOAD_STACK:
			if (showVarName) {
				// if (instr.has_dup) {
				// 	write(FORMAT("{} = {};", instr.target, instr.dup));
				// }
				// else {
					write(FORMAT("{} = {};", instr.target, instr.var_name));
				// }
			} else {
				write(FORMAT("{} = $sp[{}];", instr.target, instr.source_a));
			}
			break;
		case MidInstr::SAVE_STACK:
			if (showVarName) {
				// if (instr.has_dup) {
				// 	write(FORMAT("{} = {};", instr.dup, instr.target));
				// }
				// else {
					write(FORMAT("{} = {};", instr.var_name, instr.target));
				// }
			} else {
				write(FORMAT("$sp[{}] = {};", instr.source_a, instr.target));
			}
			break;
		case MidInstr::LOAD_GLOBAL_ARR:
		case MidInstr::LOAD_STACK_ARR:
			if (showVarName) {
				write(FORMAT("{} = {}[{}] ;", instr.target, instr.var_name, instr.source_b));
				break;
			}
			write(FORMAT("{} = [{}];", instr.target, instr.source_a));
			break;
		case MidInstr::SAVE_GLOBAL_ARR:
		case MidInstr::SAVE_STACK_ARR:
			if (showVarName) {
				write(FORMAT("{}[{}] = {};", instr.var_name, instr.source_b, instr.target));
				break;
			}
			write(FORMAT("[{}] = {};", instr.source_a, instr.target));	//TODO ??
			break;
		case MidInstr::PUSH:
			write(FORMAT("push {}B;", instr.target));
			break;
		case MidInstr::POP:
			write(FORMAT("pop {}B;", instr.target));
			break;
		case MidInstr::PUSH_REG:
			write(FORMAT("push {};", instr.target));
			break;
		case MidInstr::POP_REG:
			write(FORMAT("pop {}B;", instr.target));
			break;
		case MidInstr::PUSH_REGPOOL:
			write("push pool;");
			break;
		case MidInstr::POP_REGPOOL:
			write("pop pool;");
			break;
			
		case MidInstr::CALL:
			write(FORMAT("call {};", instr.target));
			break;
		case MidInstr::RETURN:
			write(FORMAT("return {};", instr.target));
			break;
		case MidInstr::JUMP:
			write(FORMAT("jump {};", instr.target));
			break;
		case MidInstr::MID_SHOW:
			write(instr.target);
			
		default:break;
		}
	}

	inline string MidOutput::removePrefix(string name) {
		if(startWith(name, "_STRING_")) {
			name = name.substr(8);
		} else if(startWith(name, "_GLOBAL_")) {
			name = name.substr(12);
		} else if (startWith(name, "_T")){
			name = name.substr(1);
		}
		// else if (name == "$v0") {
		// 	
		// }
		return name;
	}

	inline void MidOutput::pushIndent() {
		output_offset += "  ";
	}

	inline void MidOutput::popIndent() {
		output_offset = output_offset.substr(2);
	}

}
