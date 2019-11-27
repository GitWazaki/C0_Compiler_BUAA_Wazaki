#pragma once
#define MCN 50

using namespace std;

namespace MidCode {
	class MidInstr {
	public:
		enum  MidOp {
			//输入输出
			PRINT_LINE,
			PRINT_GLOBAL_STR,
			PRINT_INT,
			PRINT_CHAR,
			SCAN_GLOBAL_INT,
			SCAN_GLOBAL_CHAR,
			SCAN_INT,
			SCAN_CHAR,
			// ALU
			ADD,
			SUB,
			MUL,
			DIV,
			NEG,
			// 比较
			BEQ,	//==
			BNE,	//!=
			BGT,		// >
			BGE,		// >=
			BLT,	//<
			BLE,	//<=
			BGEZ,	//>=0
			BLEZ,	//<=0
			// Memory
			LOAD_LABEL,
			SAVE_LABEL,
			LOAD_STACK,
			SAVE_STACK,
			LOAD_LAB_IMM,
			SAVE_LAB_IMM,
			LOAD_STA_ARR,
			SAVE_STA_ARR,
			LOAD_ADDR,
			// Stack
			PUSH,
			POP,
			PUSH_REG,
			POP_REG,
			CALL,
			RETURN,
			JUMP,
			
		} midOp;
		
		string target, source_a, source_b;
		
		MidInstr(MidOp op) : midOp(op) {}
		MidInstr(MidOp op, string target) : midOp(op), target(target) {}
		MidInstr(MidOp op, string target, string source_a) : midOp(op), target(target), source_a(source_a) {}
		MidInstr(MidOp op, string target, string source_a, std::string source_b)
			: midOp(op), target(target), source_a(source_a), source_b(source_b) {}

		MidInstr(MidOp op, int target) : midOp(op), target(to_string(target)) {}
		MidInstr(MidOp op, string target, int source_a)
		: midOp(op), target(target), source_a(to_string(source_a)){}
		MidInstr(MidOp op, string target, string source_a, int source_b)
		: midOp(op), target(target), source_a(source_a), source_b(to_string(source_b)) {}

		string toString() {
			string str = "";
			str += e2s[midOp];
			str += " ";
			if (!target.empty()) {
				str += target;
				str += " ";
			}
			if (!source_a.empty()) {
				str += source_a;
				str += " ";
			}
			if (!source_b.empty()) {
				str += source_b;
				str += " ";
			}
			return str;
		}

	private:
		string e2s[MCN] = {
			//输入输出
			"PRINT_LINE",
			"PRINT_GLOBAL_STR",
			"PRINT_INT",
			"PRINT_CHAR",
			"SCAN_GLOBAL_INT",
			"SCAN_GLOBAL_CHAR",
			"SCAN_INT",
			"SCAN_CHAR",
			// ALU
			"ADD",
			"SUB",
			"MUL",
			"DIV",
			"NEG",
			// 比较
			"BEQ",	//==
			"BNE",	//!=
			"BGT",		//>
			"BGE",		//>=
			"BLT",	//<
			"BLE",	//<=
			"BGEZ",
			"BLEZ",
			// Memory
			"LOAD_LABEL",
			"SAVE_LABEL",
			"LOAD_STACK",
			"SAVE_STACK",
			"LOAD_LAB_IMM",
			"SAVE_LAB_IMM",
			"LOAD_STA_ARR",
			"SAVE_STA_ARR",
			"LOAD_ADDR",
			// Stack
			"PUSH",
			"POP",
			// function
			"PUSH_REG",
			"POP_REG",
			"CALL",
			"RETURN",
			"JUMP",
		};
	};
}