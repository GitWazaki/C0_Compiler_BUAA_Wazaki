#pragma once

using namespace std;

namespace MidIR {

	class FlowGraph {

		set<string> blockNames;
		
		map<string, vector<string>> preBlocks;
		map<string, vector<string>> followBlocks;
		map<string, int> name_to_num;
		
	public:
		void connectBlocks(string pre_block, string next_block);
		vector<string> getPreBlocks(string block_name);
		vector<string> getFollowBlocks(string block_name);
		map<string, vector<string>> getPreBlocksMap();
		map<string, vector<string>> getFollowBlocksMap();
		void addBlockNum(string block_name, int block_num);
		int getBlockNum(string block_name);
	};

	inline void FlowGraph::connectBlocks(string pre_block, string next_block) {
		preBlocks[next_block].push_back(pre_block);
		followBlocks[pre_block].push_back(next_block);
	}

	inline vector<string> FlowGraph::getPreBlocks(string block_name) {
		return preBlocks[block_name];
	}

	inline vector<string> FlowGraph::getFollowBlocks(string block_name) {
		return followBlocks[block_name];
	}

	inline map<string, vector<string>> FlowGraph::getPreBlocksMap() {
		return preBlocks;
	}

	inline map<string, vector<string>> FlowGraph::getFollowBlocksMap() {
		return followBlocks;
	}

	inline void FlowGraph::addBlockNum(string block_name, int block_num) {
		name_to_num[block_name] = block_num;
	}

	inline int FlowGraph::getBlockNum(string block_name) {
		return name_to_num[block_name];
	}

	class FlowHelper {
	public:
		FlowHelper() {};
		FlowGraph buildFlowGraphInFunc(Func& func);
		MidInstr getJumpInstr(Block& block);
	};

	inline FlowGraph FlowHelper::buildFlowGraphInFunc(Func& func) {
		FlowGraph flowGraph;
			ForBlocks(j, func.blocks, block)
				flowGraph.addBlockNum(block.label, j);
				if(j + 1 < blocks->size()) { // 最后一个block为总的return label
					MidInstr jumpInstr = getJumpInstr(block);
					if (jumpInstr.midOp != MidInstr::NOP && jumpInstr.midOp != MidInstr::CALL) {// 非函数调用的跳转
						flowGraph.connectBlocks(block.label, jumpInstr.getJumpTarget());
					}
					if (jumpInstr.midOp != MidInstr::JUMP) { // 如果不是无条件跳转，将前后block连接
						flowGraph.connectBlocks(blocks->at(j).label, blocks->at(j + 1).label);
					}
				}
			EndFor
		return flowGraph;
	}

	inline MidInstr FlowHelper::getJumpInstr(Block& block) {
		MidInstr nop = MidInstr(MidInstr::NOP);
		if (!block.instrs.empty()) {
			MidInstr last = block.instrs.back();
			if (last.isJump()) {
				return last;
			}
		}
		return nop;
	}

}
