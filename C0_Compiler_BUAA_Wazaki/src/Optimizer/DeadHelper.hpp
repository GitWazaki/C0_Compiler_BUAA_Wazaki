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
		set<int> getLoopblocksNum(FlowGraph &flowGraph);
	};

	inline MidCode DeadHelper::removeDeadCode() {
		removeDeadFuncGlobal();
		removeDeadDownToTop();
		//TODO
		return midCodes;
	}

	inline void DeadHelper::removeDeadDownToTop() {
		FlowHelper flowHelper;
		map<string, int> load_used_map;
		ForFuncs(i, midCodes.funcs, func)
			FlowGraph flowGraph = flowHelper.buildFlowGraphInFunc(func);
			set<int> loop_blocks_num = getLoopblocksNum(flowGraph);
		
;			for (int j = func.blocks->size() - 1; j >= 0; j--) {
				auto& block = func.blocks->at(j);
	
				for (int k = block.instrs.size() - 1; k >= 0; k--) {
					auto& instr = block.instrs.at(k);

					vector<string> loads = instr.getLoads();
					for (string& name : loads) {
						if (load_used_map.find(name) == load_used_map.end()) {
							load_used_map[name] = 1;
						}
					}
					
					if (notFound(loop_blocks_num,flowGraph.getBlockNum(block.label))) {
						vector<string> saves = instr.getSaves();
						for (string save : saves) {
							if (!save.empty() && !instr.doNotPraga() &&
								!startWith(save, string("$")) && 
								!startWith(save, string("_G")) && 
								load_used_map.find(save) == load_used_map.end()) 
							{
								instr.midOp = MidInstr::NOP;
							}
							load_used_map[save] = 0;
						}
					}
					
				}

			}

		EndFor
	}

	inline void DeadHelper::removeDeadFuncGlobal() {
		ForFuncs(i, midCodes.funcs, func)
			map<string, int> load_used_map;
		
			ForBlocks(j, func.blocks, block)
				ForInstrs(k, block.instrs, instr)
					auto loads = instr.getLoads();
					for (auto load : loads) {
						load_used_map[load]++;
					}
				EndFor
			EndFor

			ForBlocks(j, func.blocks, block)
				ForInstrs(k, block.instrs, instr)
					auto saves = instr.getSaves();
					for(auto save : saves) {
						if (!save.empty() && !instr.doNotPraga() && 
							!startWith(save, string("$")) && 
							!startWith(save, string("_G")) && 
							load_used_map.find(save) == load_used_map.end()) 
						{
							instr.midOp = MidInstr::NOP;
						}
					}
				EndFor
			EndFor
		
		EndFor
	}

	inline set<int> DeadHelper::getLoopblocksNum(FlowGraph& flowGraph) {
		set<int> loop_blocks_num;
		for (auto map : flowGraph.getFollowBlocksMap()) {
			string src_block = map.first;
			for (string next_block : map.second) {
				int src_block_num = flowGraph.getBlockNum(src_block);
				int next_block_id = flowGraph.getBlockNum(next_block);
				if (next_block_id <= src_block_num) {
					for (int i = next_block_id; i <= src_block_num; i++) {
						loop_blocks_num.insert(i);
					}
				}
			}
		}
		return loop_blocks_num;
	}


}
