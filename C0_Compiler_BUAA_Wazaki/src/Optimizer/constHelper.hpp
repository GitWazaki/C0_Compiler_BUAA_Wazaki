#pragma once

using namespace std;

namespace MidIR {
	class constHelper {

		map<string, int> name2constVal;

	public:
		void instrConstReplace(MidInstr& instr);
		
		bool hasValue(string name);

	};

	void constHelper::instrConstReplace(MidInstr& instr) {
		const auto& loads = instr.getLoads();	//vector<string>
		for(int i = 0; i < loads.size(); i++) {
			auto load = loads[i];
			if(hasValue(load)) {
				instr.constReplace(load,name2constVal[load]);
			}
		}
		// if(instr.isArrMemory() ) {
		// 	return;
		// }
		if(instr.isGlobal()) {
			return;
		}
		if (instr.ansComputable()) {
			instr.compute();
			instr.optimize();
		}

		const auto& saves = instr.getSaves();	//vector<string>
		for (int i = 0; i < saves.size(); i++) {
			auto save = saves[i];
			if (instr.has_ans) {
				name2constVal[save] = instr.ans;
			}
		}
		
	}

	inline bool constHelper::hasValue(string name) {
		if(name2constVal.find(name) != name2constVal.end()) {
			return true;
		}
		return false;
	}
}
