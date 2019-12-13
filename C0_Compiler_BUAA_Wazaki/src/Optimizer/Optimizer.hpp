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
		void copyPropagation();
		
		void removeDeadCode();

		void mergeBlocks();
		
		void buildIdentRange();
		bool checkDef(Block block, int line, string def);	// no use
		
		
		void funcsInline();

		//test
		void unitTestDefineUseChain();
		
	};

	inline MidCode Optimizer::optimize() {
		removeUselessRa();
		
		funcsInline();

		mergeBlocks();

		constReplace();
		copyPropagation();

		removeDeadCode();
		
		
		buildIdentRange();
		
		// unitTestDefineUseChain();
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

	inline void Optimizer::copyPropagation() {
		copyHelper copyHelper(midCodes);
		midCodes = copyHelper.CopyPrapa();
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
	}

#pragma endregion 


#pragma region test
	void Optimizer::unitTestDefineUseChain() {
		FlowGraph flow_graph;
		flow_graph.connectBlocks("B1", "B2");
		flow_graph.connectBlocks("B2", "B3");
		flow_graph.connectBlocks("B2", "B4");
		flow_graph.connectBlocks("B3", "B2");
		flow_graph.connectBlocks("B4", "B5");
		flow_graph.connectBlocks("B5", "B6");
		flow_graph.connectBlocks("B5", "exit");
		flow_graph.connectBlocks("B6", "B5");

		ConflictGraph define_use_chain;
		define_use_chain.addDef(1, "B1", 1, "a");
		define_use_chain.addDef(1, "B1", 2, "i");
		// define_use_chain.addDef(1, "B1", 3, "i");

		define_use_chain.addUse(2, "B2", 1, "i");

		define_use_chain.addDef(3, "B3", 1, "a");
		define_use_chain.addUse(3, "B3", 1, "a");
		define_use_chain.addUse(3, "B3", 1, "i");

		define_use_chain.addDef(3, "B3", 2, "i");
		define_use_chain.addUse(3, "B3", 2, "i");

		define_use_chain.addDef(4, "B4", 1, "b");
		define_use_chain.addUse(4, "B4", 1, "a");
		define_use_chain.addDef(4, "B4", 2, "i");

		define_use_chain.addUse(5, "B5", 1, "i");

		define_use_chain.addDef(6, "B6", 1, "b");
		define_use_chain.addUse(6, "B6", 1, "b");
		define_use_chain.addUse(6, "B6", 1, "i");

		define_use_chain.addDef(6, "B6", 2, "i");
		define_use_chain.addUse(6, "B6", 2, "i");


		define_use_chain.genConfict(flow_graph);
		define_use_chain.printConfictGraph();
	}
#pragma endregion 
	
}
