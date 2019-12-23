#pragma once

using namespace std;

namespace MidIR {

	class LoopHelper {
		MidCode midCodes;
		int cnt = 0;
	public:
		LoopHelper(MidCode midCodes) :midCodes(midCodes) {};
		
		MidCode getMidCodes();
		void unfoldLoop();
		
		tuple<int, string> findLoopBodyName(Func& func, int block_num);
		int findLoopEndInstrLoc(Func& func, string loop_name);
		int getLoopTimes(Func& func, int for_block_index);
		int getLoopEndVal(Func& func, InstrIterInFunc i, string source_a);
		string getUnfoldName(string origin_name, int time);
		MidInstr getJumpInstr(Block& block);
		void changeJumpTargetInBlocks(vector<Block>& blocks, string src, string new_label);
	};


#pragma region loop

	inline MidCode LoopHelper::getMidCodes() {
		return midCodes;
	}

	inline void LoopHelper::unfoldLoop() {
		ForFuncs(i, midCodes.funcs, func)
			int factor = 10;
			ForBlocks(k, func.blocks, block)
				string loop_name;
				int loop_block_index;
				tie(loop_block_index, loop_name) = findLoopBodyName(func, k);
				if (loop_block_index == -4)
					continue;;

				int loop_endblock_index = findLoopEndInstrLoc(func, loop_name);
				if (loop_endblock_index == -421)
					continue;

				int for_times = getLoopTimes(func, loop_endblock_index);
				if (for_times == -421)
					continue;

				MidInstr& jump_instr = blocks->at(loop_endblock_index).instrs.back();
				if (for_times <= factor || !isNumber(jump_instr.source_a)) {
				// Unroll
					vector<Block> loop_blocks;
					for (int i = loop_block_index; i <= loop_endblock_index; i++) {
						loop_blocks.push_back(blocks->at(loop_block_index));
						blocks->erase(blocks->begin() + loop_block_index);
					}
					loop_blocks.back().instrs.pop_back();
					vector<string> block_names;
					for (int i = 0; i < loop_blocks.size(); i++) {
						block_names.push_back(loop_blocks[i].label);
					}

					for (int i = 0; i < for_times; i++) {
						// change name
						vector<Block> new_blocks = loop_blocks;
						for (int j = 0; j < loop_blocks.size(); j++) {
							cnt++;
							changeJumpTargetInBlocks(new_blocks, new_blocks[j].label, getUnfoldName(block_names[j], i));
							new_blocks[j].label = getUnfoldName(block_names[j], i);
						}
						// insert
						for (int j = new_blocks.size() - 1; j >= 0; j--) {
							blocks->insert(blocks->begin() + loop_block_index, new_blocks[j]);
						}
					}
				}
				else {
					// Unroll
					vector<Block> loop_blocks;
					for (int i = loop_block_index; i <= loop_endblock_index; i++) {
						loop_blocks.push_back(blocks->at(i));
						// blocks->erase(blocks->begin() + loop_block_index);
					}
					loop_blocks.back().instrs.pop_back();

					auto& jump_instr = blocks->at(loop_endblock_index).instrs.back();
					if (isNumber(jump_instr.source_a)) {
						jump_instr.source_a = to_string(stoi(jump_instr.source_a) - factor);
					}
					else {
						string source_a = jump_instr.source_a;
						MidInstr sub_instr = MidInstr(MidInstr::SUB, FORMAT("{}_unfold", source_a), source_a, factor);
						auto& end_instrs = blocks->at(loop_endblock_index).instrs;
						end_instrs.insert(end_instrs.end() - 1, sub_instr);
						auto& jump_instr = blocks->at(loop_endblock_index).instrs.back();
						jump_instr.source_a = FORMAT("{}_unfold", source_a);
					}
					vector<string> block_names;
					for (int i = 0; i < loop_blocks.size(); i++) {
						block_names.push_back(loop_blocks[i].label);
					}

					for (int i = 0; i < factor; i++) {
						// change name
						vector<Block> new_blocks = loop_blocks;
						for (int j = 0; j < loop_blocks.size(); j++) {
							cnt++;
							changeJumpTargetInBlocks(new_blocks, new_blocks[j].label, getUnfoldName(block_names[j], i));
							new_blocks[j].label = getUnfoldName(block_names[j], i);
						}
						// insert
						for (int j = new_blocks.size() - 1; j >= 0; j--) {
							blocks->insert(blocks->begin() + loop_block_index, new_blocks[j]);
						}
					}
				}
			return;

		EndFor
	EndFor
		
	}

	inline tuple<int, string> LoopHelper::findLoopBodyName(Func& func, int block_num) {
		auto& block = func.blocks->at(block_num);
		if (startWith(block.label, string("for_body"))
			|| startWith(block.label, string("while_body"))
			)
		{
			string loop_name = block.label;
			return make_tuple(block_num, loop_name);
		}
		return make_tuple(-421, "None");
	}

	inline int LoopHelper::findLoopEndInstrLoc(Func& func, string loop_name) {
		ForBlocks(j, func.blocks, block)
			MidInstr jump_instr = getJumpInstr(block);
			if (jump_instr.isJump() && jump_instr.getJumpTarget() == loop_name)
				return j;
		EndFor
		return -421;
	}

	inline int LoopHelper::getLoopTimes(Func& func, int for_block_index) {
		InstrIterInFunc iter(func, for_block_index, func.blocks->at(for_block_index).instrs.size() - 1);

		int loop_times = -421;

		MidInstr loop_branch_instr = iter.getInstr();
		string loop_var = iter.getInstr().target;

		int loop_end;
		if (iter.getInstr().a_has_val) {
			loop_end = iter.getInstr().a_val;
		}
		else {
			loop_end = getLoopEndVal(func, iter, iter.getInstr().source_a);
		}
		
		string loop_start_var;
		while (iter.prev()) {
			if ((iter.getInstr().midOp == MidInstr::ADD && iter.getInstr().target == loop_var)) {
				if (iter.getInstr().b_val != 1)
					break;
				loop_start_var = iter.getInstr().source_a;
				break;
			}
		}

		int loop_start = -421;
		while (iter.prev()) {
			if (iter.getInstr().midOp == MidInstr::LI && iter.getInstr().target == loop_start_var) {
				loop_start = iter.getInstr().a_val;
				break;
			}
		}

		if (loop_start != -421 && loop_end != -421) {
			switch (loop_branch_instr.midOp) {
			case MidInstr::BLT:
				loop_times = loop_end - loop_start;
				break;
			case MidInstr::BLE:
				loop_times = loop_end - loop_start + 1;
			default:;
			}

		}

		return loop_times;
	}

	inline int LoopHelper::getLoopEndVal(Func& func, InstrIterInFunc i, string source_a) {
		int end = -421;
		while (i.prev()) {
			if (i.getInstr().midOp == MidInstr::LI && i.getInstr().target == source_a) {
				end = stoi(i.getInstr().source_a);
				break;
			}
			if (getSubIndex(i.getInstr().getSaves(), source_a) != -1)
				break;
		}
		return end;
	}

	inline string LoopHelper::getUnfoldName(string origin_name, int time) {
		return FORMAT("{}_unfold_{}", origin_name, cnt);
	}

	inline MidInstr LoopHelper::getJumpInstr(Block& block) {
		MidInstr nop = MidInstr(MidInstr::NOP);
		if (block.instrs.empty()) {
			return nop;
		}
		MidInstr last = block.instrs.back();
		if (last.isJump()) {
			return last;
		}
		else {
			return nop;
		}
	}

	inline void LoopHelper::changeJumpTargetInBlocks(vector<Block>& blocks, string src, string new_label) {
		for (auto& block : blocks) {
			ForInstrs(k, block.instrs, instr)
				if (instr.isJump() && instr.getJumpTarget() == src) {
					instr.changeJumpTarget(new_label);
				}
			EndFor
		}
	}

#pragma endregion 
	
}