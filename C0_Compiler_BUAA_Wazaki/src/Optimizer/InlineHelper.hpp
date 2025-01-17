#pragma once
#include "../tools/meow.hpp"

using namespace std;

namespace MidIR {

	class inlineHelper {
	public:
		MidCode midCodes;
		inlineHelper(MidCode midcodes) : midCodes(midcodes) {}
		
		MidCode makeFuncsInline();
		void replaceFuncsVars();
		void inlineNoLoopFuncs();
		
		void makeFuncInline(Func& inline_func);

		vector<string> getFuncVars(Func& func);
		vector<string> getFuncParas(const Func& func);
		void replaceFuncPara(Func& func, vector<string> paras);
		tuple<bool,string, int, int> findFuncCall(string func_name);
		Func& getFuncByName(string func_name);
		MidCode replaceeAllPara();

		void deleteInstrsBeforeCall(string inline_func_name, Func& src_func, int block_num, int line_num, vector<string> paras, int cnt);
		void deleteInstrsAfterCall(string inline_func_name, Func& src_func, int block_num, int line_num);
		void deleteCall(string inline_func_name, Func& src_func, int block_num, int line_num);
		void replaceFuncBody(Func inline_func, Func& src_func, int block_num, int cnt);
		
		bool hasCallInFunc(const Func& func);
		bool hasVarInFunc(const Func& func);
		bool hasLocalArr(const Func& func);
		bool isMainFunc(const Func& func);
		bool hasLoopInFunc(const Func& func);

		string getVarTempName(string ident);
		string getInlineTempName(string func_name, string ident, int index);
		string getInlineTempNameWithCnt(string func_name, string ident, int index, int cnt);
		
	};
	
#pragma endregion

	inline MidCode inlineHelper::makeFuncsInline() {
		replaceFuncsVars();
		inlineNoLoopFuncs();
		return midCodes;
	}

	inline void inlineHelper::replaceFuncsVars() {
		
		ForFuncs(i, midCodes.funcs, func)
			vector<string> idents = getFuncVars(func);
			ForBlocks(j, func.blocks, block)
				ForInstrs(k, block.instrs, instr)
					if (instr.midOp == MidInstr::DEF_VAR && getSubIndex(idents, instr.source_a) != -1) {
						instr = MidInstr(MidInstr::NOP);
					}
					if (instr.midOp == MidInstr::LOAD_STACK) {
						int index = getSubIndex(idents, instr.var_name);
						if (index == -1)
							continue;
						instr = MidInstr(MidInstr::MOVE, instr.target, getVarTempName(idents[index]));
					}
					if (instr.midOp == MidInstr::SAVE_STACK) {
							int index = getSubIndex(idents, instr.var_name);
							if (index == -1)
								continue;
							instr = MidInstr(MidInstr::MOVE, getVarTempName(idents[index]), instr.target);
					}
					if (instr.midOp == MidInstr::SCAN_INT || instr.midOp == MidInstr::SCAN_CHAR) {
						int index = getSubIndex(idents, instr.var_name);
						if (index == -1)
							continue;
						instr.target = getVarTempName(idents[index]);
						instr.var_name = getVarTempName(idents[index]);
					}
				EndFor
			EndFor
		EndFor
	}

	inline void inlineHelper::inlineNoLoopFuncs() {
		bool doInline;
		do {
			doInline = false;
			ForFuncs(i,midCodes.funcs,func)
				if (!hasCallInFunc(func) && !hasLocalArr(func) && !isMainFunc(func)) {
					if (!hasLoopInFunc(func)) {
						makeFuncInline(func);
						midCodes.funcs.erase(midCodes.funcs.begin() + i);
						i--;
						doInline = true;
					}
				}
			EndFor
		} while (doInline);
	}

	inline void inlineHelper::makeFuncInline(Func& inline_func) {
		vector<string> paras = getFuncParas(inline_func);
		replaceFuncPara(inline_func, paras);

		int find;
		string src_func_name;
		int block_num, line_num;
		int cnt = 0;
		while (true) {
			tie(find, src_func_name, block_num, line_num) = findFuncCall(inline_func.func_name);
			if (!find) return;
			Func& src_func = getFuncByName(src_func_name);
			cnt++;

			deleteInstrsBeforeCall(inline_func.func_name, src_func, block_num, line_num, paras, cnt);
			deleteInstrsAfterCall(inline_func.func_name, src_func, block_num, line_num);
			deleteCall(inline_func.func_name, src_func, block_num, line_num);
			replaceFuncBody(inline_func, src_func, block_num, cnt);
			
		}
		
	}

	inline vector<string> inlineHelper::getFuncVars(Func& func) {
		vector<string> vars;
		ForBlocks(j, func.blocks, block)
			ForInstrs(k, block.instrs, instr)
			if (instr.midOp == MidInstr::DEF_VAR && instr.source_b == "0") {
				vars.push_back(instr.source_a);
			}
			EndFor
		EndFor
		return vars;
	}

	inline vector<string> inlineHelper::getFuncParas(const Func& func) {
		vector<string> idents;
		ForBlocks(j, func.blocks, block)
			ForInstrs(k, block.instrs, instr)
			if (instr.midOp == MidInstr::DEF_PARA) {
				idents.push_back(instr.source_a);
			}
			EndFor
		EndFor
		return idents;
	}

	inline void inlineHelper::replaceFuncPara(Func& func, vector<string> paras) {
		ForBlocks(i, func.blocks, block)
			if (block.label == FORMAT("_RETURN_{}", func.func_name)) {
				block.instrs.clear();
				continue;
			}
			ForInstrs(j, block.instrs, instr)
				if (instr.midOp == MidInstr::DEF_PARA)
					instr = MidInstr{ MidInstr::NOP };
				if (instr.midOp == MidInstr::PUSH)
					instr = MidInstr(MidInstr::NOP);
				if (instr.midOp == MidInstr::LOAD_STACK) {
					int index = getSubIndex(paras, instr.var_name);
					if (index == -1)
						continue;
					instr = MidInstr(MidInstr::MOVE, instr.target, getInlineTempName(func.func_name, paras[index], index ));
				}
				if (instr.midOp == MidInstr::SAVE_STACK) {
					int index = getSubIndex(paras, instr.var_name);
					if (index == -1)
						continue;
					instr = MidInstr(MidInstr::MOVE, getInlineTempName(func.func_name, paras[index],  index), instr.target);
				}
			EndFor
		EndFor
	}

	inline tuple<bool,string, int, int> inlineHelper::findFuncCall(string func_name) {
		ForFuncs(i,midCodes.funcs, func)
			ForBlocks(j, func.blocks, block)
				ForInstrs(k, block.instrs, instr)
					if (instr.midOp == MidInstr::CALL && instr.target == func_name) {
						return make_tuple(true, func.func_name, j, k);
					}
				EndFor
			EndFor
		EndFor
		return  make_tuple(false,"", -1, -1);
	}

	inline Func& inlineHelper::getFuncByName(string func_name) {
		for (int i = 0; i < midCodes.funcs.size(); i++) {
			if (midCodes.funcs[i].func_name == func_name) {
				return midCodes.funcs[i];
			}
		}
	}

	inline MidCode inlineHelper::replaceeAllPara() {
		ForFuncs(i, midCodes.funcs, func)
			vector<string> paras = getFuncParas(func);
			ForBlocks(j, func.blocks, block)
				ForInstrs(k, block.instrs, instr)
					if (instr.midOp == MidInstr::DEF_PARA) {
						string type = instr.target;
						string para_name = instr.source_a;
						int index = getSubIndex(paras, instr.source_a);
						if (index != -1) {
							instr = MidInstr(MidInstr::LOAD_STACK, getVarTempName(instr.source_a), -index * 4);
							instr.var_name = FORMAT("para_{}_{}", type, para_name);
						}
					} else if (instr.midOp == MidInstr::LOAD_STACK) {
						if (getSubIndex(paras, instr.var_name) != -1) {
							instr = MidInstr(MidInstr::MOVE, instr.target, getVarTempName(instr.var_name));
						}
					} else if (instr.midOp == MidInstr::SAVE_STACK) {
						if (getSubIndex(paras, instr.var_name) != -1) {
							instr = MidInstr(MidInstr::MOVE, getVarTempName(instr.var_name), instr.target);
						}
					}
				EndFor
			EndFor
		EndFor
		return  midCodes;
	}

	inline void inlineHelper::deleteInstrsBeforeCall(string inline_func_name, Func& src_func, int block_num, int line_num, vector<string> paras, int cnt) {
		InstrIterInFunc iter = InstrIterInFunc(src_func, block_num, line_num);

		int index = paras.size() - 1;
		while (index >= 0) {
			iter.prev();
			if (iter.getInstr().midOp == MidInstr::PUSH_REG && iter.getInstr().source_a == inline_func_name && iter.getInstr().source_b == to_string(index)) {
				iter.getInstr() = MidInstr(MidInstr::MOVE, getInlineTempNameWithCnt(inline_func_name, paras[index], index,  cnt), iter.getInstr().target);
				index--;
			}
		}
		
		while (!(iter.getInstr().midOp == MidInstr::PUSH_REG && iter.getInstr().target == "$fp" && iter.getInstr().source_a == inline_func_name)) {
			iter.prev();
		}

		iter.setInstr({ MidInstr::NOP });		//remove push $fp
		iter.next();
		iter.setInstr({ MidInstr::NOP });	// remove push regpool
		
	}

	inline void inlineHelper::deleteInstrsAfterCall(string inline_func_name, Func& src_func, int block_num, int line_num) {
		InstrIterInFunc iter = InstrIterInFunc(src_func, block_num, line_num);
		iter.next();
		iter.setInstr({ MidInstr::NOP });
		iter.next();
		iter.setInstr({ MidInstr::NOP });
		iter.next();
		iter.setInstr({ MidInstr::NOP });
	}

	inline void inlineHelper::deleteCall(string inline_func_name, Func& src_func, int block_num, int line_num) {
		InstrIterInFunc iter = InstrIterInFunc(src_func, block_num, line_num);
		iter.setInstr({ MidInstr::NOP }); // add fp para*4
		iter.prev();
		iter.setInstr({ MidInstr::NOP }); // call
	}

	inline void inlineHelper::replaceFuncBody(Func inline_func, Func& src_func, int block_num, int cnt) {
		vector<string> old_labels;
		for (int i = 0; i < inline_func.blocks->size(); i++) {
			old_labels.push_back(inline_func.blocks->at(i).label);
		}

		for (int i = inline_func.blocks->size() - 1; i >= 0; i--) {
			Block new_block = inline_func.blocks->at(i);
			new_block.label = FORMAT("inline_{}_{}", cnt, new_block.label);
			for (int j = 0; j < new_block.instrs.size(); j++) {
				MidInstr& instr = new_block.instrs[j];
				if (instr.isJump() && getSubIndex(old_labels, instr.getJumpTarget()) != -1) {
					instr.changeJumpTarget(FORMAT("inline_{}_{}", cnt, instr.getJumpTarget()));
				}
				if (startWith(instr.target, string("_T"))) {
					instr.target = FORMAT("{}_{}", instr.target, cnt);
				}
				if (startWith(instr.source_a, string("_T"))) {
					instr.source_a = FORMAT("{}_{}", instr.source_a, cnt);
				}
				if (startWith(instr.source_b, string("_T"))) {
					instr.source_b = FORMAT("{}_{}", instr.source_b, cnt);
				}
			}
			src_func.blocks->insert(src_func.blocks->begin() + block_num + 1, new_block);
		}
	}

	inline bool inlineHelper::hasCallInFunc(const Func& func) {
		ForBlocks(i, func.blocks, block)
			ForInstrs(j, block.instrs, instr)
			if (instr.midOp == MidInstr::CALL)
				return true;
			EndFor
		EndFor
		return false;
	}

	inline bool inlineHelper::hasVarInFunc(const Func& func) {
		ForBlocks(i, func.blocks, block)
			ForInstrs(j, block.instrs, instr)
			if (instr.midOp == MidInstr::DEF_VAR)
				return true;
			EndFor
		EndFor
		return false;
	}

	inline bool inlineHelper::hasLocalArr(const Func& func) {
		ForBlocks(i, func.blocks, block)
			ForInstrs(j, block.instrs, instr)
			if (instr.midOp == MidInstr::DEF_VAR && instr.source_b != "0")
				return true;
			EndFor
		EndFor
		return false;
	}

	inline bool inlineHelper::isMainFunc(const Func& func) {
		return func.func_name == "main";
	}

	inline bool inlineHelper::hasLoopInFunc(const Func& func) {
		ForBlocks(i, func.blocks, block)
			if (startWith(block.label, string("for"))
				|| startWith(block.label, string("while"))
				|| startWith(block.label, string("do"))
				)
				return true;
		EndFor
		return false;
	}

	inline string inlineHelper::getVarTempName(string ident) {
		return FORMAT("_T{}", ident);
	}

	inline string inlineHelper::getInlineTempName(string func_name, string ident, int index) {
		return FORMAT("_T0_F{}_P{}_{}", func_name, ident, index);
	}

	inline string inlineHelper::getInlineTempNameWithCnt(string func_name, string ident, int index, int cnt) {
		return FORMAT("{}_{}", getInlineTempName(func_name, ident, index), cnt);
	}

}
