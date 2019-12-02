#pragma once
#include <memory>

using namespace std;

namespace MidIR {

	class Block {
	public:
		string label;
		vector<MidInstr> instrs;

		Block(std::string label) : label(label) {
			instrs.clear();
		}

		void addInstr(MidInstr instr) {
			instrs.push_back(instr);
		}
		
	};
	
}

using Blocks = std::vector<MidIR::Block>;
using BlocksPtr = std::shared_ptr<Blocks>;