#pragma once

using namespace std;

namespace MidIR {
	class copyHelper {
		map<string, string> dup2src;
	public:
		void instrCopyPrapa(MidInstr& instr);

		bool isDup(string name);
	};

	inline void copyHelper::instrCopyPrapa(MidInstr& instr) {
		const auto& loads = instr.getLoads();
		for (int i = 0; i < loads.size(); i++) {
			string load = loads[i];
			if (isDup(load)) {
				instr.copyReplace(load, dup2src[load]);
			}
		}

		instr.setDup();
		
		if (instr.has_dup) {
			if(instr.midOp == MidInstr::LOAD_GLOBAL || instr.midOp == MidInstr::LOAD_STACK) {
				dup2src[instr.var_name] = instr.dup;
				dup2src[instr.target] = instr.dup;
			} else if(instr.midOp == MidInstr::SAVE_GLOBAL || instr.midOp == MidInstr::SAVE_STACK) {
				dup2src[instr.var_name] = instr.dup;
			}
			
		}
		
	}

	inline bool copyHelper::isDup(string name) {
		if (Found(dup2src,name)) {
			return true;
		}
		return false;
	}

}
