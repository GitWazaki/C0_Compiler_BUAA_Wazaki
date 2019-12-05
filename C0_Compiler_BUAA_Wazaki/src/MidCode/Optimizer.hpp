#pragma once

using namespace std;

namespace MidIR {

	class Optimizer {

	public:
		Optimizer(MidCode midCodes) :midCodes(midCodes) {}

		MidCode optimize();

	private:
		MidCode midCodes;

		// tools
		void insertBefore(vector<MidInstr>& instrs, int& i, MidInstr instr);
		void insertAfter(vector<MidInstr>& instrs, int& i, MidInstr instr);
		void deleteInstr(vector<MidInstr>& instrs, int& i, MidInstr instr);

		//methods
		void removeUselessRa();

		
		//checker
		bool noCallInFunc(Func func);
		
		//modify
		void removeRa(Func& func);


	};

	inline MidCode Optimizer::optimize() {
		removeUselessRa();
		return midCodes;
	}

#pragma region tools

#define ForFuncs(i, func) auto& funcs = midCodes.funcs; \
	for (int i = 0; i < funcs.size(); i++) {	\
		auto& func = funcs[i];

#define ForBlocks(i, block) auto& blocks = func.blocks;	\
	for (int i = 0; i < blocks->size(); i++) {		\
		auto& block = blocks->at(i);

#define ForInstrs(i, instr) auto& instrs = block.instrs;	\
	for (int j = 0; j < instrs.size(); j++) {	\
		auto& instr = instrs[j];

#define EndFor }
	
	
	inline void Optimizer::insertBefore(vector<MidInstr>& instrs, int& i, MidInstr instr) {
		instrs.insert(instrs.begin() + i, instr);
		i++;
	}

	inline void Optimizer::insertAfter(vector<MidInstr>& instrs, int& i, MidInstr instr) {
		instrs.insert(instrs.begin() + i + 1, instr);
	}

	inline void Optimizer::deleteInstr(vector<MidInstr>& instrs, int& i, MidInstr instr) {
		instrs.erase(instrs.begin() + i);
	}
#pragma endregion

#pragma region methods
	inline void Optimizer::removeUselessRa() {
		ForFuncs(i,func)
			if(noCallInFunc(func)) {
				removeRa(func);
			}
		EndFor
		removeRa(funcs.back());
	}
	
#pragma endregion 

#pragma region checker
	inline bool Optimizer::noCallInFunc(Func func) {
		ForBlocks(i,block)
			ForInstrs(j,instr)
				if (instr.midOp == MidInstr::CALL)
					return false;
			EndFor
		EndFor
		return true;
	}
#pragma endregion 

#pragma region modify
	inline void Optimizer::removeRa(Func& func) {
		ForBlocks(i,block)
			ForInstrs(j,instr)
				if (instr.midOp == MidInstr::POP_REG || instr.midOp == MidInstr::PUSH_REG) {
					if (instr.target == "$ra") {
						deleteInstr(instrs, j, instr);
					}
				}
			EndFor
		EndFor
	}
#pragma endregion
	
}
