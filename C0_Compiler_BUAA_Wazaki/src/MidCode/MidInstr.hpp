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
			LA,
			
			// 比较
			BEQ,	//==
			BNE,	//!=
			BGT,		// >
			BGE,		// >=
			BLT,	//<
			BLE,	//<=
			BGTZ,	//> 0
			BLTZ,
			BGEZ,
			BLEZ,	//<=0

			// Memory
			LOAD_GLOBAL,
			SAVE_GLOBAL,
			LOAD_STACK,
			SAVE_STACK,
			LOAD_GLOBAL_ARR,
			SAVE_GLOBAL_ARR,
			LOAD_STACK_ARR,
			SAVE_STACK_ARR,
			LOAD_POOL,
			SAVE_POOL,

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
			DEF_VAR,
			DEF_PARA,
			NOP,
			MID_SHOW,
		} midOp;

		MidInstr(MidOp op) : midOp(op) { init(); }
		MidInstr(MidOp op, string target) : midOp(op), target(target) {
			init();
		}
		MidInstr(MidOp op, string target, string source_a) : midOp(op), target(target), source_a(source_a) {
			init();
		}
		MidInstr(MidOp op, string target, string source_a, std::string source_b)
			: midOp(op), target(target), source_a(source_a), source_b(source_b) {
			init();
		}

		MidInstr(MidOp op, int target) : midOp(op), target(to_string(target)) {
			init();
		}
		MidInstr(MidOp op, string target, int source_a)
			: midOp(op), target(target), source_a(to_string(source_a)) {
			init();
		}
		MidInstr(MidOp op, string target, string source_a, int source_b)
			: midOp(op), target(target), source_a(source_a), source_b(to_string(source_b)) {
			init();
		}
		MidInstr(MidOp op, string target, int source_a, string source_b)
			: midOp(op), target(target), source_a(to_string(source_a)), source_b(source_b) {
			init();
		}

		
		string target{}, source_a{}, source_b{};
		string var_name{};
		bool show = false;
		int target_val, a_val, b_val, ans;
		bool target_has_val = false, a_has_val = false, b_has_val = false, has_ans = false;
		int line_num_in_block = -1;
		
		void init();
		void initNumber();
		void initGlobal();

		vector<string> getLoads();
		vector<string> getSaves();
		string getJumpTarget();
		bool changeJumpTarget(string label);

		void constReplace(string reg,int value);
		bool ansComputable();
		void compute();
		void optimize();
		
		
		bool isLoad_target();
		bool isLoad_a();
		bool isLoad_b();
		bool isSave_target();
		bool isSave_a();
		bool isSave_b();
		bool isGlobal();
		bool isArrMemory();
		bool isInput();
		bool isJump();
		bool isAlu();
		bool isMemorySave();
		bool doNotPraga();
		

	};

	inline void MidInstr::init() {
		initNumber();
		initGlobal();
	}

	inline void MidInstr::initNumber() {
		if(isNumber(target)) {
			target_has_val = true;
			target_val = stoi(target);
		}
		if(isNumber(source_a)) {
			a_has_val = true;
			a_val = stoi(source_a);
		}
		if (isNumber(source_b)) {
			b_has_val = true;
			b_val = stoi(source_b);
		}
	}

	inline void MidInstr::initGlobal() {
		if(isGlobal()) {
			if(!var_name.empty()) {
				var_name = FORMAT("_G{}", var_name);
			}
		}
	}

	inline vector<string> MidInstr::getLoads() {
		vector<std::string> loads;
		bool load_var_name = false;
		
		switch (midOp) {
		case LOAD_STACK:
		case LOAD_GLOBAL:
		case LOAD_GLOBAL_ARR:
		case LOAD_STACK_ARR:
			if (!var_name.empty()) {
				loads.push_back(var_name);
				load_var_name = true;
			}
		default:
			break;
		}

		if (isLoad_target() && !target_has_val && !load_var_name) {
			loads.push_back(target);
		}
		if (isLoad_a() && !a_has_val) {
			loads.push_back(source_a);
		}
		if (isLoad_b() && !b_has_val) {
			loads.push_back(source_b);
		}
		
		return loads;
	}

	inline vector<string> MidInstr::getSaves() {
		vector<std::string> saves;
		bool save_var_name = false;
		
		switch (midOp) {
		case SCAN_CHAR:
		case SCAN_GLOBAL_CHAR:
		case SCAN_GLOBAL_INT:
		case SCAN_INT:

		case SAVE_STACK:
		case SAVE_GLOBAL:
		case SAVE_GLOBAL_ARR:
		case SAVE_STACK_ARR:
			if (!var_name.empty()) {
				saves.push_back(var_name);
				save_var_name = true;
			}
		default:
			break;
		}
		
		if(isSave_target() && !save_var_name) {
			saves.push_back(target);
		}
		if(isSave_a()) {
			saves.push_back(source_a);
		}
		if (isSave_b()) {
			saves.push_back(source_b);
		}

		return saves;
	}

	inline string MidInstr::getJumpTarget() {
		if(isJump()) {
			switch (midOp) {
			case BLT:
			case BGT:
			case BLE:
			case BGE:
			case BEQ:
			case BNE:
				return source_b;
			case BGTZ:
			case BGEZ:
			case BLTZ:
			case BLEZ:
				return source_a;
			case CALL:
			case RETURN:
			case JUMP:
				return target;
			default:
				panic("not jump instr");
			}
		}
	}

	inline bool MidInstr::changeJumpTarget(string label) {
		switch (midOp) {
		case BLT:
		case BGT:
		case BLE:
		case BGE:
		case BEQ:
		case BNE:
			source_b = label;
			break;
		case BGTZ:
		case BGEZ:
		case BLTZ:
		case BLEZ:
			source_a = label;
			break;
		case CALL:
		case RETURN:
		case JUMP:
			target = label;
			break;
		default:
			panic("not jump instr");
		}
		return true;
	}

	inline void MidInstr::constReplace(string reg, int value) {
		if(reg == target&&isLoad_target()) {
			target_val = value;
			target = to_string(value);
			target_has_val = true;
		}
		if(reg == source_a&&isLoad_a()) {
			a_val = value;
			source_a = to_string(value);
			a_has_val = true;
		}
		if(reg == source_b&&isLoad_b()) {
			b_val = value;
			source_b = to_string(value);
			b_has_val = true;
		}
		//TODO ?
		if (midOp == LOAD_GLOBAL || midOp == LOAD_STACK) {
			midOp = LI;
			a_has_val = true;
			a_val = value;
			source_a = to_string(value);
		}
		if (midOp == SAVE_GLOBAL || midOp == SAVE_STACK) {
			has_ans = true;
			ans = value;
		}
	}

	inline bool MidInstr::ansComputable() {
		if(doNotPraga()) {
			return false;
		}
		if (isLoad_target() && !target_has_val)
			return false;
		if (isLoad_a() && !a_has_val)
			return false;
		if (isLoad_b() && !b_has_val)
			return false;
		return true;
	}

	inline void MidInstr::compute() {
		switch (midOp) {
		case ADD:
			ans = a_val + b_val;
			has_ans = true;
			break;
		case SUB:
			ans = a_val - b_val;
			has_ans = true;
			break;
		case MUL:
			ans = a_val * b_val;
			has_ans = true;
			break;
		case DIV:
			ans = a_val / b_val;
			has_ans = true;
			break;
		case MOVE:
			ans = a_val;
			has_ans = true;
			break;
		case LI:
			ans = a_val;
			has_ans = true;
			break;
		case BGTZ:
			ans = target_val > 0 ? 1 : 0;
			break;
		case BGEZ:
			ans = target_val >= 0 ? 1 : 0;
			break;
		case BLTZ:
			ans = target_val < 0 ? 1 : 0;
			break;
		case BLEZ:
			ans = target_val <= 0 ? 1 : 0;
			break;
		case BLT:
			ans = target_val < a_val ? 1 : 0;
			break;
		case BGT:
			ans = target_val > a_val ? 1 : 0;
			break;
		case BLE:
			ans = target_val <= a_val ? 1 : 0;
			break;
		case BGE:
			ans = target_val >= a_val ? 1 : 0;
			break;
		case BEQ:
			ans = target_val == a_val ? 1 : 0;
			break;
		case BNE:
			ans = target_val != a_val ? 1 : 0;
			break;
		case PRINT_INT:
		case PRINT_CHAR:
			ans = target_val;
			break;
		default:
			break;
		}
		optimize();
	}

	inline void MidInstr::optimize() {
		switch (midOp) {
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case MOVE:
		case LI:
			midOp = LI;
			source_a = to_string(ans);
			break;
		case BGTZ:
		case BGEZ:
		case BLTZ:
		case BLEZ:
			if (ans == 1) {
				midOp = JUMP;
				target = source_a;
			}
			else {
				midOp = NOP;
			}
			break;
		case BLT:
		case BGT:
		case BLE:
		case BGE:
		case BEQ:
		case BNE:
			if (ans == 1) {
				midOp = JUMP;
				target = source_b;
			}
			else {
				midOp = NOP;
			}
			break;
		case PRINT_INT:
		case PRINT_CHAR:
			target = to_string(ans);
			break;

		default:
			break;
		}
	}

	inline bool MidInstr::isGlobal() {
		switch (midOp) {
		case PRINT_GLOBAL_STR:
		case SCAN_GLOBAL_INT:
		case SCAN_GLOBAL_CHAR:
		case LOAD_GLOBAL:
		case SAVE_GLOBAL:
		case LOAD_GLOBAL_ARR:
		case SAVE_GLOBAL_ARR:
			return true;
		default:
			return false;
		}
	}

	inline bool MidInstr::isLoad_target() {
		switch (midOp) {
		case PRINT_CHAR:
		case PRINT_INT:
		case BLT:
		case BGT:
		case BLE:
		case BGE:
		case BEQ:
		case BNE:
		case BGTZ:
		case BGEZ:
		case BLTZ:
		case BLEZ:
		case SAVE_GLOBAL:
		case SAVE_STACK:
		case SAVE_GLOBAL_ARR:
		case SAVE_STACK_ARR:
		case PUSH_REG:
			return true;
		default:
			return false;
		}
	}

	inline bool MidInstr::isLoad_a() {
		switch (midOp) {
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case MOVE:
		case BLT:
		case BGT:
		case BLE:
		case BGE:
		case BEQ:
		case BNE:
			return true;
		default:
			return false;
		}
	}

	inline bool MidInstr::isLoad_b() {
		switch (midOp) {
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case LOAD_GLOBAL_ARR:
		case LOAD_STACK_ARR:
		case SAVE_GLOBAL_ARR:
		case SAVE_STACK_ARR:
			return true;
		default:
			return false;
		}
	}

	inline bool MidInstr::isSave_target() {
		switch (midOp) {
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case MOVE:
		case LI:
		case LA:
		case LOAD_GLOBAL:
		case LOAD_STACK:
		case LOAD_GLOBAL_ARR:
		case LOAD_STACK_ARR:
		case SCAN_INT:
		case SCAN_CHAR:
			return true;
		default:
			return false;
		}
	}

	inline bool MidInstr::isSave_a() {
		switch (midOp) {
		default:
			return false;
		}
	}

	inline bool MidInstr::isSave_b() {
		switch (midOp) {
		default:
			return false;
		}
	}

	bool  MidInstr::isArrMemory() {
		switch (midOp) {
		case SAVE_GLOBAL_ARR:
		case LOAD_GLOBAL_ARR:
		case SAVE_STACK_ARR:
		case LOAD_STACK_ARR:
			return true;
		default:
			return false;
		}
	}

	inline bool MidInstr::isInput() {
		switch (midOp) {
		case SCAN_INT:
		case SCAN_CHAR:
		case SCAN_GLOBAL_INT:
		case SCAN_GLOBAL_CHAR:
			return true;
		default:
			return false;
		}
	}

	inline bool MidInstr::isJump() {
		switch (midOp) {
		case BLT:
		case BGT:
		case BLE:
		case BGE:
		case BEQ:
		case BNE:
		case BGTZ:
		case BGEZ:
		case BLTZ:
		case BLEZ:
		case JUMP:
		case CALL:
		case RETURN:
			return true;
		default:
			return false;
		}
	}

	inline bool MidInstr::isAlu() {
		switch (midOp) {
		case ADD:
		case SUB:
		case MUL:
		case DIV:
			return true;
		default:
			return false;
		}
	}

	inline bool MidInstr::doNotPraga() {
		return isGlobal() || isArrMemory() || isInput();
	}

}
