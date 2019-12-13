#pragma once
#include "MidInstr.hpp"
#include "Block.hpp"
#include "Global.hpp"
#include "Func.hpp"
#include "../Optimizer/ActiveRange.hpp"

using namespace std;

namespace MidIR {

	namespace ExprOp {
		enum OP {
			PARE_L = 0,
			PARE_R,
			PLUS,
			MINUS,
			MULT,
			DIV,
			NEG,
		};
		const int op_priority[10] = { 0, 0, 1, 1, 2, 2, 3 };
	}

	class MidCode {
	public:
		MidCode() {
			cur_blocks = nullptr;
		}

		vector<GlobalDefine> global_defines;
		vector<Func> funcs;

		map<string, map<string, ActiveRange>> func_to_identRange;
		
		//temp
		string func_name;
		BlocksPtr cur_blocks;

		void addInstr(MidInstr instr) const;
		void addInstrs(vector<MidInstr> instrs);
		void saveVarName(string var_name);

		// global define
		int str_cnt = 0;
		int temp_cnt = 0;
		int after_ret_cnt = 0;
		int after_call_cnt = 0;
		string defineConstStr(string str);

		// new label
		void newBlock(string label);
		string getGlobalVarLabel(string ident);
		string getGlobalArrLabel(string ident);
		string getStrLabel(string str);
		string newTemp();

		//expr
		vector<ExprOp::OP> op_stack;
		vector<std::string> obj_stack;
		vector<vector<ExprOp::OP>> op_stack_stack{};
		vector< std::vector<std::string> > obj_stack_stack{};
		void pushExprOp(ExprOp::OP op);
		void exprPushObj_GlobalVar(string ident, int offset);
		void exprPushObj_StackVar(string ident, int offset);
		void exprPushObj_ImmInt(int value);
		void exprPushObj_Stack_Arr(int ident_offset,string expr_reg);
		void exprPushObj_GLOBAL_Arr(int ident_offset,string expr_reg);
		
		void exprPushReg(string reg);
		bool checkOp(ExprOp::OP);
		void popOp();
		void genExprOpInstr(MidInstr::MidOp);
		string genExprVal();
		void exprStart();
		void exprEnd();

		// IO
		void printStrInstr(string lable);
		void printReg(Symbol::SymbolType type,string expr_reg);
		void printLineInstr();
		
		//function
		void defineFunc(string funcName);
		void openStackSpace(int bytes);
		void popStack(int size);
		void addReturnInstr();
		void callInstr(string funcName);
		void pushRegInstr(string regName);	// no use
		void popRegInstr(string regName);	// no use
		string getReturnLabel() const;
		string getAfterReturnBlockName(string ret_name);
		string getAfterCallBlockName(string call_func_name);
		void moveReg(string target, string source);

		//Ìø×ªÓï¾ä if for
		struct BranchDefine {
			enum {
				IF_DEFINE,
				FOR_DEFINE,
				WHILE_DEFINE,
				DOWHILE_DEFINE
			}type_define;
			int name_id;
		};
		vector<BranchDefine> branch_stack;
		int if_cnt = 0;
		int for_cnt = 0;
		int while_cnt = 0;
		int do_cnt = 0;
		void defineIf();
		string getIfName();
		string getIfThenName();
		string getIfElseName();
		string getIfEndName();
		void defineFor();
		string getForStartName();
		string getForBodyName();
		string getForEndName();
		string getStartLoc();
		string getBodyLoc();
		void defineWhile();
		string getWhileName();
		string getWhileBodyName();
		string getWhileEndName();
		void defineDoWhile();
		string getDoWhileName();
		string getDoWhileEndName();
		vector<MidInstr> getCondInstrs(string block_name);
		void jumpInstr(string label);
		void genBranch(TokenType::tokenType cmpType, string lexprReg, string rexprReg);
		void genBranchRet(TokenType::tokenType cmpType, string lexprReg, string rexprReg);
		void endDefine();
	};

	inline void MidCode::addInstr(MidInstr instr) const {
		cur_blocks->back().addInstr(instr);
	}

	inline void MidCode::addInstrs(vector<MidInstr> instrs) {
		for(auto instr : instrs) {
			addInstr(instr);
		}
	}

	inline void MidCode::saveVarName(string var_name) {
		cur_blocks->back().instrs.back().var_name = var_name;
	}

#pragma region globaldefine
	inline string MidCode::defineConstStr(string str) {
		string str_label = getStrLabel(str);
		GlobalDefine item{ ConstStr{ str_label, str } };
		global_defines.push_back(item);
		return str_label;
	}

#pragma endregion 

	inline void MidCode::newBlock(string label) {
		cur_blocks->push_back(Block(label));
	}

	inline string MidCode::getGlobalVarLabel(string identName) {
		return FORMAT("_GLOBAL_VAR_{}", identName);
	}

	inline string MidCode::getGlobalArrLabel(string identName) {
		return FORMAT("_GLOBAL_ARR_{}", identName);
	}

	inline string MidCode::getStrLabel(string str) {
		return FORMAT("_STRING_{}", str_cnt++);
	}

	inline string MidCode::newTemp() {
		return FORMAT("_T{}", temp_cnt++);
	}

	inline string MidCode::getReturnLabel() const {
		return FORMAT("_RETURN_{}", func_name);
	}

	inline string MidCode::getAfterReturnBlockName(string ret_name) {
		return FORMAT("_AFTER_RETURN_{}_{}",ret_name, after_ret_cnt++);
	}

	inline string MidCode::getAfterCallBlockName(string call_func_name) {
		return FORMAT("_AFTER_CALL_{}_{}", call_func_name, after_call_cnt++);
	}

	inline void MidCode::moveReg(string target, string source) {
		addInstr(MidInstr(MidInstr::MOVE, target, source));
	}

#pragma region expr

	// expr
	inline void MidCode::pushExprOp(ExprOp::OP op) {
		switch (op) {
		case ExprOp::PARE_L:
			op_stack.push_back(op);
			break;
		case ExprOp::PARE_R:
			while (op_stack.back() != ExprOp::PARE_L) {
				popOp();
			}
			op_stack.pop_back();
			break;
		case ExprOp::PLUS:
		case ExprOp::MINUS:
		case ExprOp::MULT:
		case ExprOp::DIV:
		case ExprOp::NEG:
			while (checkOp(op))
				popOp();
			op_stack.push_back(op);
			break;
		default:;
		}
	}

	inline void MidCode::exprPushObj_GlobalVar(string ident, int offset) {
		string t = newTemp();
		addInstr(MidInstr{ MidInstr::LOAD_GLOBAL, t, offset });
		obj_stack.push_back(t);
	}

	inline void MidCode::exprPushObj_StackVar(string ident, int offset) {
		string t = newTemp();
		addInstr(MidInstr(MidInstr::LOAD_STACK, t, to_string(offset)));
		obj_stack.push_back(t);
	}

	inline void MidCode::exprPushObj_ImmInt(int value) {
		string t = newTemp();
		addInstr(MidInstr(MidInstr::LI, t, value));
		obj_stack.push_back(t);
	}

	inline void MidCode::exprPushObj_Stack_Arr(int ident_offset, string sub_reg) {
		auto t = newTemp();
		addInstr(MidInstr(MidInstr::LOAD_STACK_ARR, t, ident_offset, sub_reg));
		obj_stack.push_back(t);
	}

	inline void MidCode::exprPushObj_GLOBAL_Arr(int ident_offset, string sub_reg) {
		auto t = newTemp();
		addInstr(MidInstr(MidInstr::LOAD_GLOBAL_ARR, t, ident_offset, sub_reg));
		obj_stack.push_back(t);
	}


	inline void MidCode::exprPushReg(string reg) {
		auto t = newTemp();
		addInstr(MidInstr(MidInstr::MOVE, t, reg));
		obj_stack.push_back(t);
	}

	inline bool MidCode::checkOp(ExprOp::OP op) {
		if (!op_stack.empty() && ExprOp::op_priority[op_stack.back()] >= ExprOp::op_priority[op]) {
			return true;
		}
		return false;
	}

	inline void MidCode::popOp() {

		switch (op_stack.back()) {
		case ExprOp::PARE_L:
			break;
		case ExprOp::PARE_R:
			break;
		case ExprOp::PLUS:
			genExprOpInstr(MidInstr::ADD);
			break;
		case ExprOp::MINUS:
			genExprOpInstr(MidInstr::SUB);
			break;
		case ExprOp::MULT:
			genExprOpInstr(MidInstr::MUL);
			break;
		case ExprOp::DIV:
			genExprOpInstr(MidInstr::DIV);
			break;
		case ExprOp::NEG:
			genExprOpInstr(MidInstr::NEG);
			break;
		default:
			break;
		}
		op_stack.pop_back();
	}

	inline void MidCode::genExprOpInstr(MidInstr::MidOp op) {
		string temp;
		string source_a, source_b;
		if (op == MidInstr::NEG) {
			temp = newTemp();
			source_b = obj_stack.back();
			addInstr(MidInstr(MidInstr::SUB, temp, "$0", source_b));	//temp = $0 - source_b
			obj_stack.push_back(temp);
		}
		else {
			temp = newTemp();
			source_b = obj_stack.back();
			obj_stack.pop_back();
			source_a = obj_stack.back();
			obj_stack.pop_back();
			addInstr(MidInstr(op, temp, source_a, source_b));
			obj_stack.push_back(temp);
		}
	}

	inline string MidCode::genExprVal() {
		while (!op_stack.empty()) {
			popOp();
		}
		string expr_val = obj_stack.back();
		obj_stack.pop_back();
		exprEnd();
		return expr_val;
	}

	inline void MidCode::exprStart() {
		op_stack_stack.push_back(op_stack);
		obj_stack_stack.push_back(obj_stack);
		op_stack.clear();
		obj_stack.clear();
	}

	inline void MidCode::exprEnd() {
		op_stack = op_stack_stack.back();
		op_stack_stack.pop_back();
		obj_stack = obj_stack_stack.back();
		obj_stack_stack.pop_back();
	}

#pragma endregion 

#pragma region io
	// IO
	inline void MidCode::printStrInstr(string lable) {
		addInstr(MidInstr{ MidInstr::PRINT_GLOBAL_STR, lable });
	}

	inline void MidCode::printReg(Symbol::SymbolType type, string expr_reg) {
		if (type == Symbol::INT) {
			addInstr({ MidIR::MidInstr::PRINT_INT, expr_reg });
		} else {
			addInstr({ MidIR::MidInstr::PRINT_CHAR, expr_reg });
		}
	}

	inline void MidCode::printLineInstr() {
		addInstr(MidInstr{ MidInstr::PRINT_LINE });
	}
#pragma endregion 

#pragma region function
	//function
	inline void MidCode::defineFunc(string funcName) {
		func_name = funcName;
		funcs.push_back(Func(funcName));
		cur_blocks = funcs.back().blocks;
	}

	inline void MidCode::openStackSpace(int bytes) {
		if (bytes != 0) {
			addInstr(MidInstr{ MidInstr::PUSH, to_string(bytes) });
		}
	}

	inline void MidCode::popStack(int size) {
		// if (size != 0) {
			addInstr(MidInstr(MidInstr::POP, to_string(size)));
		// }
	}

	inline void MidCode::addReturnInstr() {
		addInstr(MidInstr(MidInstr::RETURN));
	}

	inline void MidCode::callInstr(string funcName) {
		addInstr(MidInstr(MidInstr::CALL, funcName));
	}

	inline void MidCode::pushRegInstr(string regName) {
		addInstr(MidInstr(MidInstr::PUSH_REG, regName));
	}

	inline void MidCode::popRegInstr(string regName) {
		addInstr(MidInstr(MidInstr::POP_REG, regName));
	}
#pragma endregion

#pragma region branch

	inline void MidCode::defineIf() {
		if_cnt++;
		BranchDefine if_define{ BranchDefine::IF_DEFINE,if_cnt };
		branch_stack.push_back(if_define);
	}

	inline string MidCode::getIfName() {
		return FORMAT("if_{}", branch_stack.back().name_id);
	}

	inline string MidCode::getIfThenName() {
		return FORMAT("if_{}_than", branch_stack.back().name_id);
	}

	inline string MidCode::getIfElseName() {
		return FORMAT("if_{}_else", branch_stack.back().name_id);
	}

	inline string MidCode::getIfEndName() {
		return FORMAT("if_{}_end", branch_stack.back().name_id);
	}

	inline void MidCode::defineFor() {
		for_cnt++;
		BranchDefine for_define{ BranchDefine::FOR_DEFINE,for_cnt };
		branch_stack.push_back(for_define);
	}

	inline string MidCode::getForStartName() {
		return FORMAT("for_{}_start", branch_stack.back().name_id);
	}

	inline string MidCode::getForBodyName() {
		return FORMAT("for_{}_body", branch_stack.back().name_id);
	}

	inline string MidCode::getForEndName() {
		return FORMAT("for_{}_end", branch_stack.back().name_id);
	}

	inline string MidCode::getStartLoc() {
		switch (branch_stack.back().type_define) {
		case BranchDefine::IF_DEFINE:
			return getIfElseName();
		case BranchDefine::FOR_DEFINE:
			return getForEndName();
		case BranchDefine::WHILE_DEFINE:
			return getWhileEndName();
		case BranchDefine::DOWHILE_DEFINE:
			return getDoWhileEndName();
		default:break;
		}
	}

	inline string MidCode::getBodyLoc() {
		switch (branch_stack.back().type_define) {
		case BranchDefine::FOR_DEFINE:
			return getForBodyName();
		case BranchDefine::WHILE_DEFINE:
			return getWhileBodyName();
		default:break;
		}
	}

	inline void MidCode::defineWhile() {
		while_cnt++;
		BranchDefine while_define{ BranchDefine::WHILE_DEFINE,while_cnt };
		branch_stack.push_back(while_define);
	}

	inline string MidCode::getWhileName() {
		return FORMAT("while_{}", branch_stack.back().name_id);
	}

	inline string MidCode::getWhileBodyName() {
		return FORMAT("while_{}_body", branch_stack.back().name_id);
	}

	inline string MidCode::getWhileEndName() {
		return FORMAT("while_{}_end", branch_stack.back().name_id);
	}

	inline void MidCode::defineDoWhile() {
		do_cnt++;
		BranchDefine do_define{ BranchDefine::DOWHILE_DEFINE,do_cnt };
		branch_stack.push_back(do_define);
	}

	inline string MidCode::getDoWhileName() {
		return FORMAT("dowhile_{}", branch_stack.back().name_id);
	}

	inline string MidCode::getDoWhileEndName() {
		return FORMAT("dowhile_{}_end", branch_stack.back().name_id);
	}

	inline vector<MidInstr> MidCode::getCondInstrs(string block_name) {
		vector<MidInstr> condInstrs;
		for(int i = 0; i < cur_blocks->size();i++) {
			auto& block = cur_blocks->at(i);
			if (block.label == block_name) {
				condInstrs = block.instrs;
			}
		}
		condInstrs.pop_back();	// pop old branch instr
		return condInstrs;
	}

	inline void MidCode::jumpInstr(string label) {
		addInstr(MidInstr(MidInstr::JUMP, label));
	}

	inline void MidCode::genBranch(TokenType::tokenType cmpType, string lexprReg, string rexprReg) {
		switch (cmpType) {
		case TokenType::EQL:
			addInstr(MidInstr(MidInstr::BNE, lexprReg, rexprReg, getStartLoc()));
			break;
		case TokenType::NEQ:
			addInstr(MidInstr(MidInstr::BEQ, lexprReg, rexprReg, getStartLoc()));
			break;
		case TokenType::GRE:
			addInstr(MidInstr(MidInstr::BLE, lexprReg, rexprReg, getStartLoc()));
			break;
		case TokenType::GEQ:
			addInstr(MidInstr(MidInstr::BLT, lexprReg, rexprReg, getStartLoc()));
			break;
		case TokenType::LSS:
			addInstr(MidInstr(MidInstr::BGE, lexprReg, rexprReg, getStartLoc()));
			break;
		case TokenType::LEQ:
			addInstr(MidInstr(MidInstr::BGT, lexprReg, rexprReg, getStartLoc()));
			break;
		default:
			break;
		}
	}

	inline void MidCode::genBranchRet(TokenType::tokenType cmpType, string lexprReg, string rexprReg) {
		switch (cmpType) {
		case TokenType::EQL:
			addInstr(MidInstr(MidInstr::BEQ, lexprReg, rexprReg, getBodyLoc()));
			break;
		case TokenType::NEQ:
			addInstr(MidInstr(MidInstr::BNE, lexprReg, rexprReg, getBodyLoc()));
			break;
		case TokenType::GRE:
			addInstr(MidInstr(MidInstr::BGT, lexprReg, rexprReg, getBodyLoc()));
			break;
		case TokenType::GEQ:
			addInstr(MidInstr(MidInstr::BGE, lexprReg, rexprReg, getBodyLoc()));
			break;
		case TokenType::LSS:
			addInstr(MidInstr(MidInstr::BLT, lexprReg, rexprReg, getBodyLoc()));
			break;
		case TokenType::LEQ:
			addInstr(MidInstr(MidInstr::BLE, lexprReg, rexprReg, getBodyLoc()));
			break;
		default:
			break;
		}
	}

	inline void MidCode::endDefine() {
		branch_stack.pop_back();
	}

#pragma endregion 
#define ForFuncs(i, _funcs, func)  \
	for (int i = 0; i < _funcs.size(); i++) {	\
		auto& funcs = _funcs;\
		auto& func = funcs[i];

#define ForBlocks(i, _blocks, block) \
	for (int i = 0; i < _blocks->size(); i++) {		\
		auto& blocks = _blocks;	\
		auto& block = blocks->at(i);

#define ForInstrs(i, _instrs, instr) \
	for (int i = 0; i < _instrs.size(); i++) {	\
		auto& instrs = _instrs;	\
		auto& instr = instrs[i];

#define EndFor }
}
