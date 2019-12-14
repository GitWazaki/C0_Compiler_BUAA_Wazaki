#pragma once

using namespace std;

namespace MidIR {
	class copyHelper {
		MidCode midCodes;
		
	public:
		copyHelper(MidCode midCodes) :midCodes(midCodes) {};
		MidCode CopyPrapa();
		void copyPrapaDownToTop(Func& func);
		void copyPrapaUntilDef(Func& func);
		bool noFollowDef(string save_name, Block& block, int line_number);
		int findNextDefLoc(Block& block, int start, string save_name);
		void prapaRange(Block& block, int start, int end, string src, string copy);

		MidCode aluPrapa();
		void prapaInstrRange(Block& block, int start, int end, string alu_ans_reg, MidInstr alu_instr);
		
		
	};

	inline MidCode copyHelper::CopyPrapa() {
		ForFuncs(i, midCodes.funcs, func)
			copyPrapaDownToTop(func);
			copyPrapaUntilDef(func);
		EndFor
		return midCodes;
	}

	inline void copyHelper::copyPrapaDownToTop(Func& func) {
		ForBlocks(j, func.blocks, block)
			map<string, string> copy;

			ForInstrs(k, block.instrs, instr)

				if (instr.isLoad_target() && Found(copy, instr.target)) {
					instr.target = copy[instr.target];
				}
				if (instr.isLoad_a() && Found(copy, instr.source_a)) {
					instr.source_a = copy[instr.source_a];
				}
				if (instr.isLoad_b() && Found(copy, instr.source_b)) {
					instr.source_b = copy[instr.source_b];
				}
		
				if (instr.midOp == MidInstr::MOVE && startWith(instr.source_a, string("_T")) 
					&& noFollowDef(instr.source_a, block, k)){
					copy[instr.target] = instr.source_a;
				}
			EndFor
		EndFor
	}

	inline void copyHelper::copyPrapaUntilDef(Func& func) {
		ForBlocks(j, func.blocks, block)
			map<string, string> copy;
				ForInstrs(k, block.instrs, instr)
					if (instr.midOp == MidInstr::MOVE && startWith(instr.source_a, string("_T"))){
						int next_def = findNextDefLoc(block, k, instr.source_a);
						int end = min(next_def,int(block.instrs.size()));
						prapaRange(block, k + 1, end, instr.target, instr.source_a); 
					}
				EndFor
			EndFor
	}

	inline bool copyHelper::noFollowDef(string save_name, Block& block, int line_number) {
		for (int i = line_number; i < block.instrs.size(); i++) {
			auto& instr = block.instrs.at(i);
			if (getSubIndex(instr.getSaves(), save_name) != -1) {
				return false;
			}
		}
		return true;
	}

	inline int copyHelper::findNextDefLoc(Block& block, int start, string save_name) {
		if (!save_name.empty() && startWith(save_name, "_T")) {
			for (int i = start; i < block.instrs.size(); i++) {
				auto& instr = block.instrs.at(i);
				if (getSubIndex(instr.getSaves(), save_name) != -1) {
					return i;
				}
			}
		}
		return INT32_MAX - 1;
	}

	inline void copyHelper::prapaRange(Block& block, int start, int end, string src, string copy) {
		for (int i = start; i < end; i++) {
			auto& instr = block.instrs.at(i);
			if (instr.isLoad_target() && instr.target == src) {
				instr.target = copy;
			}
			if (instr.isLoad_a() && instr.source_a == src) {
				instr.source_a = copy;
			}
			if (instr.isLoad_b() && instr.source_b == src) {
				instr.source_b = copy;
			}
		}
	}

	inline MidCode copyHelper::aluPrapa() {
		ForFuncs(i, midCodes.funcs, func)
		
			ForBlocks(j, func.blocks, block)
				ForInstrs(k, block.instrs, instr)
					if(instr.isAlu()) {
						if(instr.midOp == MidInstr::DIV) {
							continue;
						}
						int next_a = findNextDefLoc(block, k, instr.source_a) + 1;
						int next_b = findNextDefLoc(block, k, instr.source_b) + 1;
						int end = min(next_a, next_b);
						end = min(end, int (block.instrs.size()));
						prapaInstrRange(block, k + 1, end, instr.target, instr);
					}
					
				EndFor
			EndFor
		EndFor
		return midCodes;
	}

	inline void copyHelper::prapaInstrRange(Block& block, int start, int end, string alu_ans_reg, MidInstr alu_instr) {
		for (int i = start; i < end; i++) {
			auto& instr = block.instrs.at(i);
			if (instr.midOp == MidInstr::MOVE && instr.source_a == alu_ans_reg) {
				alu_instr.target = instr.target;
				instr = alu_instr;
			}
		}
	}

}
