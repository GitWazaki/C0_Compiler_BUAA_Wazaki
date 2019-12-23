#pragma once

using namespace std;

namespace MidIR {

	class BlockHelper {
		MidCode midCodes;
	public:
		BlockHelper(MidCode midCodes) :midCodes(midCodes) {};
		
		MidCode mergeBlock();
		MidCode getMidCodes();
		
		void mergeEmptyBlocks();
		void mergeJumpNextBlocks();
		void mergeNoEntryBlocks();
		void mergeSimpleAdjBlocks();
		void removeUselessJump();
		
		void changeJumpTargetInFunc(Func& func, string empty_block_label, string next_block_label);
		void mergeNextBlock(BlocksPtr& blocks, int loc);
		bool hasMultiEntry(FlowGraph& flowGraph, string label);
		bool hasNoEntry(FlowGraph& flow_graph, string block_name);
		bool hasOnlySelfLoop(FlowGraph& flow_graph, string block_name);
		
	};

	inline MidCode BlockHelper::mergeBlock() {
		mergeEmptyBlocks();
		mergeJumpNextBlocks();
		mergeSimpleAdjBlocks();
		mergeNoEntryBlocks();
		removeUselessJump();
		mergeSimpleAdjBlocks();
		return midCodes;
	}

	inline MidCode BlockHelper::getMidCodes() {
		return midCodes;
	}

	inline void BlockHelper::mergeEmptyBlocks() {
		ForFuncs(i, midCodes.funcs, func)
		
			ForBlocks(j, func.blocks, block)
				if (block.instrs.size() == 0 && j < blocks->size()-1) {
					changeJumpTargetInFunc(func, blocks->at(j).label, blocks->at(j + 1).label);
					blocks->erase(blocks->begin() + j);
					j--;
				}
			EndFor
		
		EndFor
	}

	inline void BlockHelper::mergeJumpNextBlocks() {
		ForFuncs(i, midCodes.funcs, func)
			FlowHelper flowHelper;
			FlowGraph flow_graph = flowHelper.buildFlowGraphInFunc(func);
			bool diff;
			do {
				diff = false;
				ForBlocks(j, func.blocks, block)
				
					if (block.instrs.size() == 0 || j >= blocks->size() - 2)	//处理最后两个block
						break;
					MidInstr& last_instr = block.instrs.back();
					if (last_instr.midOp != MidInstr::JUMP) {
						continue;
					}
				
					string next_label = blocks->at(j + 1).label;
					if (last_instr.getJumpTarget() == next_label && !hasMultiEntry(flow_graph, next_label)) {
						last_instr = MidInstr(MidInstr::NOP);
						mergeNextBlock(blocks, j);
						diff = true;
					}
				
				EndFor
			} while (diff);
		
		EndFor
	}

	inline void BlockHelper::mergeNoEntryBlocks() {
		ForFuncs(i, midCodes.funcs, func)
			bool diff;
			do {
				diff = false;
				FlowGraph flow_graph = FlowHelper().buildFlowGraphInFunc(func);
				ForBlocks(j, func.blocks, block)
					if (j == 0)
						continue;
					if (hasNoEntry(flow_graph, block.label) || hasOnlySelfLoop(flow_graph, block.label)) {
						blocks->erase(blocks->begin() + j);
						j--;
						diff = true;
					}
				EndFor
			} while (diff);
		EndFor
	}

	inline void BlockHelper::mergeSimpleAdjBlocks() {
		ForFuncs(i, midCodes.funcs, func)
			FlowHelper flowHelper;
			FlowGraph flow_graph = flowHelper.buildFlowGraphInFunc(func);
			bool diff;
			do {
				diff = false;
				ForBlocks(j, func.blocks, block)
				
					if (block.instrs.size() == 0 || j >= blocks->size() - 2)
						break;
					MidInstr& last_instr = block.instrs.back();
					if (last_instr.isJump()) {
						continue;
					}
				
					string next_label = blocks->at(j + 1).label;
					if (!hasMultiEntry(flow_graph, next_label)) {
						mergeNextBlock(blocks, j);
						diff = true;
					}
				
				EndFor
			} while (diff);
		
		EndFor
	}

	inline void BlockHelper::removeUselessJump() {
		ForFuncs(i, midCodes.funcs, func)
		
			ForBlocks(j, func.blocks, block)
				
				if (block.instrs.size() == 0 || j >= blocks->size() - 2)
					break;
				MidInstr& last_instr = block.instrs.back();
				if (last_instr.midOp != MidInstr::JUMP) {
					continue;
				}

				string next_label = blocks->at(j + 1).label;
				if (last_instr.getJumpTarget() == next_label) {
					last_instr = MidInstr(MidInstr::NOP);
				}

			EndFor
		
		EndFor
	}

	inline void BlockHelper::changeJumpTargetInFunc(Func& func, string empty_block_label, string next_block_label) {
		ForBlocks(i, func.blocks, block)
			ForInstrs(j, block.instrs, instr)
				if (instr.isJump() && instr.getJumpTarget() == empty_block_label) {
					instr.changeJumpTarget(next_block_label);
				}
			EndFor
		EndFor
	}

	inline void BlockHelper::mergeNextBlock(BlocksPtr& blocks, int loc) {
		ForInstrs(i, blocks->at(loc + 1).instrs, instr)
			blocks->at(loc).instrs.push_back(instr);
		EndFor
		blocks->erase(blocks->begin() + loc + 1);
	}

	inline bool BlockHelper::hasMultiEntry(FlowGraph& flowGraph, string label) {
		return flowGraph.getPreBlocks(label).size() > 1;
	}

	inline bool BlockHelper::hasNoEntry(FlowGraph& flow_graph, string block_name) {
		return flow_graph.getPreBlocks(block_name).size() == 0;
	}

	inline bool BlockHelper::hasOnlySelfLoop(FlowGraph& flow_graph, string block_name) {
		auto preds = flow_graph.getPreBlocks(block_name);
		if (preds.size() == 1 && preds[0] == block_name) {
			return true;
		}
		return false;
	}

}
