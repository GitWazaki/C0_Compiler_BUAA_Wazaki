#pragma once
#include "Flow.hpp"
#include "constHelper.hpp"
#include "copyHelper.hpp"
#include "DeadHelper.hpp"
#include "BlockHelper.hpp"
#include "ReachingDefine.hpp"
#include "Struct.hpp"
#include "ActiveRange.hpp"
#include "Conflict.hpp"
#include "InlineHelper.hpp"
#include "LoopHelper.hpp"


using namespace std;

namespace MidIR {

	class Optimizer {

	public:
		Optimizer(MidCode midCodes) :midCodes(midCodes) {}

		MidCode optimize();

	private:
		MidCode midCodes;

		// tools
		void insertBefore(vector<MidInstr>& instrs, int& i, MidInstr instr);
		void insertAfter(vector<MidInstr>& instrs, int& i, MidInstr instr);
		void deleteInstr(vector<MidInstr>& instrs, int& i, MidInstr instr);
		
		//methods
		void removeUselessRa();
		bool noCallInFunc(Func func);
		void removeRa(Func& func);
		
		void constReplace();
		
		void copyPrapaAndRemove();
		void copyPropa();
		void aluMovePropa();
		void aluCopyPropa();
		
		void removeDeadCode();

		void mergeBlocks();
		
		void buildIdentRange();
		bool checkDef(Block block, int line, string def);	// no use
		
		void funcsInline();
		void unfoldLoop();
	};

	inline MidCode Optimizer::optimize() {
		removeUselessRa();
		
		funcsInline();
		
		mergeBlocks();
		
		constReplace();
		copyPrapaAndRemove();
		
		buildIdentRange();
		
		return midCodes;
	}

#pragma region tools

#define needAssign(reg) !reg.empty() && !startWith(reg, string("$"))
	
	
	inline void Optimizer::insertBefore(vector<MidInstr>& instrs, int& i, MidInstr instr) {
		instrs.insert(instrs.begin() + i, instr);
		i++;
	}

	inline void Optimizer::insertAfter(vector<MidInstr>& instrs, int& i, MidInstr instr) {
		instrs.insert(instrs.begin() + i + 1, instr);
	}

	inline void Optimizer::deleteInstr(vector<MidInstr>& instrs, int& i, MidInstr instr) {
		instrs.erase(instrs.begin() + i);
	}
	
#pragma endregion

#pragma region removeRa
	inline void Optimizer::removeUselessRa() {
		ForFuncs(i, midCodes.funcs, func)
			if(noCallInFunc(func)) {
				removeRa(func);
			}
		EndFor
		removeRa(midCodes.funcs.back());
	}

	inline bool Optimizer::noCallInFunc(Func func) {
		ForBlocks(i, func.blocks, block)
			ForInstrs(j, block.instrs, instr)
			if (instr.midOp == MidInstr::CALL)
				return false;
			EndFor
		EndFor
		return true;
	}

	inline void Optimizer::removeRa(Func& func) {
		ForBlocks(i, func.blocks, block)
			ForInstrs(j, block.instrs, instr)
			if (instr.midOp == MidInstr::POP_REG || instr.midOp == MidInstr::PUSH_REG) {
				if (instr.target == "$ra") {
					deleteInstr(instrs, j, instr);
				}
			}
		EndFor
			EndFor
	}
#pragma endregion 
	
#pragma region propagation	
	inline void Optimizer::constReplace() {
		ForFuncs(i, midCodes.funcs,func)
			ForBlocks(j,func.blocks, block)
				constHelper constHelper;
				ForInstrs(k,block.instrs,instr)
					constHelper.instrConstReplace(instr);
				EndFor
			EndFor
		EndFor
	}

	inline void Optimizer::copyPrapaAndRemove() {
		copyPropa();
		aluMovePropa();
		aluCopyPropa();
	}

	inline void Optimizer::copyPropa() {
		copyHelper copyHelper(midCodes);
		midCodes = copyHelper.CopyPrapa();
		removeDeadCode();
	}

	inline void Optimizer::aluMovePropa() {
		copyHelper copyHelper(midCodes);
		midCodes = copyHelper.aluPrapaWithMove();
		midCodes = copyHelper.CopyPrapa();
		removeDeadCode();
	}

	inline void Optimizer::aluCopyPropa() {
		copyHelper copyHelper(midCodes);
		midCodes = copyHelper.aluPrapaWithCopy();
		midCodes = copyHelper.CopyPrapa();
		removeDeadCode();
	}

#pragma endregion

#pragma region removeDeadCode
	
	inline void Optimizer::removeDeadCode() {
		DeadHelper deadHelper(midCodes);
		midCodes = deadHelper.removeDeadCode();
	}

#pragma endregion

#pragma region blockMerge

	inline void Optimizer::mergeBlocks() {
		BlockHelper blockHelper(midCodes);
		midCodes = blockHelper.mergeBlock();
	}

#pragma endregion 
	
	
#pragma region dataFlowAnylsis	

	inline void Optimizer::buildIdentRange() {
		ForFuncs(i, midCodes.funcs, func)
			FlowHelper flowHelper;
			FlowGraph flow_graph = flowHelper.buildFlowGraphInFunc(func);
			ConflictGraph conflictGraph;
			ForBlocks(block_num, func.blocks, block)
				ForInstrs(line_num, block.instrs, instr)
					instr.line_num_in_block = line_num;
					vector<string> loads = instr.getLoads();
					for(string load : loads) {
						if(needAssign(load)) {
							conflictGraph.addUse(block_num, block.label, line_num, load);
						}
					}
					vector<string> saves = instr.getSaves();
					for (string save : saves) {
						if (needAssign(save)) {
							conflictGraph.addDef(block_num, block.label, line_num, save);
						}
					}
				EndFor
			EndFor
			conflictGraph.genConfict(flow_graph);
			// conflictGraph.printConfictGraph();
			midCodes.func_to_identRange[func.func_name] = conflictGraph.getRangeMap();
		EndFor
	}

	inline bool Optimizer::checkDef(Block block, int line, string def) {
		for (int i = line + 1; i < block.instrs.size(); i++) {
			for(string load : block.instrs[i].getLoads()) {
				if(load == def) {
					return true;
				}
			}
			for(string save : block.instrs[i].getSaves()) {
				if (save == def) {
					return false;
				}
			}
			
		}
		return true;
	}

#pragma endregion


#pragma region  func_inline

	inline void Optimizer::funcsInline() {
		inlineHelper inlineHelper(midCodes);
		midCodes = inlineHelper.makeFuncsInline();
		midCodes = inlineHelper.replaceeAllPara();
	}

#pragma endregion
	
	inline void Optimizer::unfoldLoop() {
		LoopHelper loopHelper(midCodes);
		loopHelper.unfoldLoop();
		midCodes = loopHelper.getMidCodes();
	}
}
