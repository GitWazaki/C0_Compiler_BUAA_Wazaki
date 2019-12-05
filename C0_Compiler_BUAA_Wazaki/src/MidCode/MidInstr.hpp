#pragma once
#define MCN 50

using namespace std;

namespace MidIR {
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
			LI,
			MOVE,

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
			// LOAD_LABEL,		//no use
			// SAVE_LABEL,		//no ues
			// LOAD_LAB_IMM,		//no use
			// SAVE_LAB_IMM,		//no use
			LOAD_GLOBAL,
			SAVE_GLOBAL,
			LOAD_STACK,
			SAVE_STACK,
			LOAD_GLOBAL_ARR,
			SAVE_GLOBAL_ARR,
			LOAD_STA_ARR,
			SAVE_STA_ARR,
			LA,

			// Stack
			PUSH,
			POP,
			PUSH_REG,
			POP_REG,
			PUSH_REGPOOL,
			POP_REGPOOL,

			CALL,
			RETURN,
			JUMP,

			MID_SHOW,
		} midOp;

		string target, source_a, source_b;
		string var_name;
		bool show;

		MidInstr(MidOp op) : midOp(op) {}
		MidInstr(MidOp op, string target) : midOp(op), target(target) {}
		MidInstr(MidOp op, string target, string source_a) : midOp(op), target(target), source_a(source_a) {}
		MidInstr(MidOp op, string target, string source_a, std::string source_b)
			: midOp(op), target(target), source_a(source_a), source_b(source_b) {}

		MidInstr(MidOp op, int target) : midOp(op), target(to_string(target)) {}
		MidInstr(MidOp op, string target, int source_a)
			: midOp(op), target(target), source_a(to_string(source_a)) {}
		MidInstr(MidOp op, string target, string source_a, int source_b)
			: midOp(op), target(target), source_a(source_a), source_b(to_string(source_b)) {}
		MidInstr(MidOp op, string target, int source_a, string source_b)
			: midOp(op), target(target), source_a(to_string(source_a)), source_b(source_b) {}

	};
}