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
		
		void output(string str);
		void gloablDefineOutput(GlobalDefine& global_define);
		void midFuncOutput(Func& func);
		void midBlockOutput(Block& block);
		void midInstrOutput(MidInstr instr);
		string removePrefix(string name);
	};

	inline void MidOutput::output() {
		output("// C0_Compiler_BUAA_Wazaki gen Mid Code");

		for (int i = 0; i < midCodes.global_defines.size(); i++) {
			gloablDefineOutput(midCodes.global_defines[i]);
		}
		output("");	//next line
		for (int i = 0; i < midCodes.funcs.size(); i++) {
			midFuncOutput(midCodes.funcs[i]);
			output("");
		}
	}

	inline void MidOutput::output(string str) {
		if (output_mid) {
			fout  << str << endl;
		}
	}

	inline void MidOutput::gloablDefineOutput(GlobalDefine& global_define) {
		switch (global_define.type) {
		case GlobalDefine::CONST_STR:
			output(FORMAT("string_{} = \"{}\";", removePrefix(global_define.const_str.label), global_define.const_str.value));
			break;
		case GlobalDefine::VAR_INT:
			output(FORMAT("int {};", removePrefix(global_define.var_int.label)));
			break;
		case GlobalDefine::VAR_INT_ARRAY:
			output(FORMAT("int {}[{}];", removePrefix(global_define.var_int_array.label), global_define.var_int_array.len));
			break;
		default:;
		}
	}

	inline void MidOutput::midFuncOutput(Func& func) {
		output("");
		output(FORMAT("__function {} start__", func.func_name));
		for(int i = 0; i < func.blocks->size(); i++) {
			midBlockOutput(func.blocks->at(i));
		}
		output("__end funciton__");
	}

	inline void MidOutput::midBlockOutput(Block& block) {
		output(FORMAT("Block {}",block.label));
		for (int i = 0; i < block.instrs.size(); i++) {
			midInstrOutput(block.instrs[i]);
		}
	}

	inline void MidOutput::midInstrOutput(MidInstr instr) {

		instr.target = removePrefix(instr.target);
		instr.source_a = removePrefix(instr.source_a);
		instr.source_b = removePrefix(instr.source_b);

		bool showVarName = instr.var_name.size() > 0;

		switch (instr.midOp) {
		case MidInstr::PRINT_LINE:
			output("print('\\n')");
			break;
		case MidInstr::PRINT_GLOBAL_STR:
		case MidInstr::PRINT_INT:
		case MidInstr::PRINT_CHAR:
			if (showVarName) {
				output(FORMAT("scanf({});", instr.var_name));
				break;
			}
			output(FORMAT("print({});", instr.target));
			break;
		case MidInstr::SCAN_GLOBAL_INT:
		case MidInstr::SCAN_GLOBAL_CHAR:
		case MidInstr::SCAN_INT:
		case MidInstr::SCAN_CHAR:
			if (showVarName) {
				output(FORMAT("scanf({});", instr.var_name));
				break;
			}
			output(FORMAT("scanf({});", instr.target));
			break;
		case MidInstr::ADD:
			output(FORMAT("{} = {} + {};", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::SUB:
			output(FORMAT("{} = {} - {};", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::MUL:
			output(FORMAT("{} = {} * {};", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::DIV:
			output(FORMAT("{} = {} / {};", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::LI:
			output(FORMAT("{} = {};", instr.target, instr.source_a));
			break;
		case MidInstr::LA:
			output(FORMAT("{} = {};", instr.target, instr.source_a));
			break;
		case MidInstr::MOVE:
			output(FORMAT("{} = {};", instr.target, instr.source_a));
			break;
		case MidInstr::BEQ:
			output(FORMAT("if ({} == {}) jump {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::BNE:
			output(FORMAT("if ({} != {}) jump {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::BGT:
			output(FORMAT("if ({} > {}) jump {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::BGE:
			output(FORMAT("if ({} >= {}) jump {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::BLT:
			output(FORMAT("if ({} < {}) jump {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::BLE:
			output(FORMAT("if ({} <= {}) jump {}", instr.target, instr.source_a, instr.source_b));
			break;
		case MidInstr::BGTZ:
			output(FORMAT("if ({} > 0) jump {}", instr.target, instr.source_a));
			break;
		case MidInstr::BGEZ:
			output(FORMAT("if ({} >= 0) jump {}", instr.target, instr.source_a));
			break;
		case MidInstr::BLTZ:
			output(FORMAT("if ({} < 0) jump {}", instr.target, instr.source_a));
			break;
		case MidInstr::BLEZ:
			output(FORMAT("if ({} <= 0) jump {}", instr.target, instr.source_a));
			break;
		case MidInstr::LOAD_GLOBAL:
			if(showVarName) {
				output(FORMAT("{} = {};", instr.target, instr.var_name));
			} else {
				output(FORMAT("{} = $gp[{}];", instr.target, instr.source_a));
			}
			break;
		case MidInstr::SAVE_GLOBAL:
			if (showVarName) {
				output(FORMAT("{} = {};", instr.var_name, instr.target));
			} else {
				output(FORMAT("$gp[{}] = {};", instr.source_a, instr.target));
			}
			break;
		case MidInstr::LOAD_STACK:
			if (showVarName) {
				output(FORMAT("{} = {};", instr.target, instr.var_name));
			} else {
				output(FORMAT("{} = $sp[{}];", instr.target, instr.source_a));
			}
			break;
		case MidInstr::SAVE_STACK:
			if (showVarName) {
				output(FORMAT("{} = {};", instr.var_name, instr.target));
			} else {
				output(FORMAT("$sp[{}] = {};", instr.source_a, instr.target));
			}
			break;
		case MidInstr::LOAD_GLOBAL_ARR:
		case MidInstr::LOAD_STACK_ARR:
			if (showVarName) {
				output(FORMAT("{} = {}[{}] ;", instr.target, instr.var_name, instr.source_b));
				break;
			}
			output(FORMAT("{} = [{}];", instr.target, instr.source_a));
			break;
		case MidInstr::SAVE_GLOBAL_ARR:
		case MidInstr::SAVE_STACK_ARR:
			if (showVarName) {
				output(FORMAT("{}[{}] = {};", instr.var_name, instr.source_b, instr.target));
				break;
			}
			output(FORMAT("[{}] = {};", instr.source_a, instr.target));	//TODO ??
			break;
		case MidInstr::PUSH:
			output(FORMAT("push {}Bit;", instr.target));
			break;
		case MidInstr::POP:
			output(FORMAT("pop {}Bit;", instr.target));
			break;
		case MidInstr::PUSH_REG:
			output(FORMAT("push {};", instr.target));
			break;
		case MidInstr::POP_REG:
			output(FORMAT("pop {}Bit;", instr.target));
			break;
		case MidInstr::PUSH_REGPOOL:
			output("push pool;");
			break;
		case MidInstr::POP_REGPOOL:
			output("pop pool;");
			break;
			
		case MidInstr::CALL:
			output(FORMAT("call {};", instr.target));
			break;
		case MidInstr::RETURN:
			output(FORMAT("return {};", instr.target));
			break;
		case MidInstr::JUMP:
			output(FORMAT("jump {};", instr.target));
			break;
		case MidInstr::DEF_VAR:
			if(instr.source_b != "0") {
				output(FORMAT("ARR {} {}[{}];", instr.target, instr.source_a, instr.source_b));
			} else {
				output(FORMAT("VAR {} {};", instr.target, instr.source_a));
			}
			break;
		case MidInstr::DEF_PARA:
			output(FORMAT("PARA {} {};", instr.target, instr.source_a));
			break;
		case MidInstr::MID_SHOW:
			output(instr.target);
			
		default:break;
		}
	}

	inline string MidOutput::removePrefix(string name) {
		if(startWith(name, "_STRING_")) {
			name = name.substr(8);
		} else if(startWith(name, "_G")) {
			name = name.substr(1);
		} else if (startWith(name, "_T")){
			name = name.substr(1);
		}
		return name;
	}
}
