#pragma once

using namespace std;

namespace MidIR {

	class DeadHelper {
		MidCode midCodes;
	public:
		DeadHelper(MidCode midCodes):midCodes(midCodes){}
		
		MidCode removeDeadCode();

		void removeDeadDownToTop();
		void removeDeadFuncGlobal();
		bool blockEndWithJump(Block& block);
	};

	inline MidCode DeadHelper::removeDeadCode() {
		removeDeadDownToTop();
		removeDeadFuncGlobal();
		return midCodes;
	}

	inline void DeadHelper::removeDeadDownToTop() {
		map<string, int> load_cnt;
		ForFuncs(i, midCodes.funcs, func)

			for (int j = func.blocks->size() - 1; j >= 0; j--) {
				auto& blocks = func.blocks;
				auto& block = func.blocks->at(j);

				for (int k = block.instrs.size() - 1; k >= 0; k--) {
					auto& instr = block.instrs.at(k);
					auto saves = instr.getSaves();
					if (!blockEndWithJump(block)) {
						for (auto save : saves) {
							if (!save.empty()
								&& !startWith(save, string("$"))
								&& !startWith(save, string("_G"))
								&& !instr.doNotPraga()
								&& load_cnt.find(save) == load_cnt.end()) {
								instr.midOp = MidInstr::NOP;
							}
						}
					}
					auto loads = instr.getLoads();
					// if (instr.isMemorySave()) {
					// 	loads.push_back(instr.target);
					// }
					for (auto& name : loads) {
						if (load_cnt.find(name) == load_cnt.end()) {
							load_cnt[name] = 1;
						}
					}
				}

			}

		EndFor
	}

	inline void DeadHelper::removeDeadFuncGlobal() {
		ForFuncs(i, midCodes.funcs, func)
			map<string, int> load_cnt;
		
			ForBlocks(j, func.blocks, block)
				ForInstrs(k, block.instrs, instr)
					auto loads = instr.getLoads();
					for (auto load : loads) {
						load_cnt[load]++;
					}
				EndFor
			EndFor

			ForBlocks(j, func.blocks, block)
				ForInstrs(k, block.instrs, instr)
					auto saves = instr.getSaves();
					for(auto save : saves) {
						if (!save.empty()
							&& !startWith(save, string("$"))
							&& !startWith(save, string("_G"))
							&& !instr.doNotPraga()
							&& load_cnt.find(save) == load_cnt.end()) {
							instr.midOp = MidInstr::NOP;
						}
					}
				EndFor
			EndFor
		
		EndFor
	}

	inline bool DeadHelper::blockEndWithJump(Block& block) {
		if (!block.instrs.empty()) {
			return block.instrs.at(block.instrs.size() - 1).isJump();
		}
		return false;
	}

}
