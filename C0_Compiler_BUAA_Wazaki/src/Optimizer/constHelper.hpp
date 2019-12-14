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
			string load = loads[i];
			if(hasValue(load)) {
				instr.constReplace(load,name2constVal[load]);
			}
		}
		if(instr.doNotPraga()) {
			return;
		}
		if (instr.ansComputable()) {
			instr.compute();
		}

		const auto& saves = instr.getSaves();	//vector<string>
		for (int i = 0; i < saves.size(); i++) {
			string save = saves[i];
			if (instr.has_ans) {
				name2constVal[save] = instr.ans;
			} else {
				name2constVal.erase(save);
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
