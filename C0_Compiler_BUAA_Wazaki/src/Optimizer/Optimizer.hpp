#pragma once
#include "constHelper.hpp"
#include "Flow.hpp"
#include "ReachingDefine.hpp"
#include "Struct.hpp"
#include "Confict.hpp"

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
		MidInstr getJumpInstr(Block& block);
		
		//methods
		void removeUselessRa();
		void constReplace();
		FlowGraph buildFlowGraph();
		
		//checker
		bool noCallInFunc(Func func);
		
		//modify
		void removeRa(Func& func);

		//test
		void unitTestDefineUseChain();

	};

	inline MidCode Optimizer::optimize() {
		removeUselessRa();
		constReplace();
		FlowGraph flow_graph = buildFlowGraph();
		
		unitTestDefineUseChain();
		return midCodes;
	}

#pragma region tools

#define ForFuncs(i, _funcs, func) auto& funcs = _funcs; \
	for (int i = 0; i < funcs.size(); i++) {	\
		auto& func = funcs[i];

#define ForBlocks(i, _blocks, block) auto& blocks = _blocks;	\
	for (int i = 0; i < blocks->size(); i++) {		\
		auto& block = blocks->at(i);

#define ForInstrs(i, _instrs, instr) auto& instrs = _instrs;	\
	for (int j = 0; j < instrs.size(); j++) {	\
		auto& instr = instrs[j];

#define EndFor }
	
	
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

	inline MidInstr Optimizer::getJumpInstr(Block& block) {
		MidInstr nop = MidInstr(MidInstr::NOP);
		if (!block.instrs.empty()) {
			MidInstr last = block.instrs.back();
			if (last.isJump()) {
				return last;
			}
		}	
		return nop;
	}
#pragma endregion

#pragma region methods
	inline void Optimizer::removeUselessRa() {
		ForFuncs(i, midCodes.funcs, func)
			if(noCallInFunc(func)) {
				removeRa(func);
			}
		EndFor
		removeRa(funcs.back());
	}

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

	inline FlowGraph Optimizer::buildFlowGraph() {
		FlowGraph flowGraph;
		ForFuncs(i, midCodes.funcs, func)
			ForBlocks(j, func.blocks, block)
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
		EndFor
		return flowGraph;
	}

#pragma endregion 

#pragma region checker
	inline bool Optimizer::noCallInFunc(Func func) {
		ForBlocks(i,func.blocks, block)
			ForInstrs(j, block.instrs, instr)
				if (instr.midOp == MidInstr::CALL)
					return false;
			EndFor
		EndFor
		return true;
	}
#pragma endregion 

#pragma region modify
	inline void Optimizer::removeRa(Func& func) {
		ForBlocks(i,func.blocks,block)
			ForInstrs(j,block.instrs,instr)
				if (instr.midOp == MidInstr::POP_REG || instr.midOp == MidInstr::PUSH_REG) {
					if (instr.target == "$ra") {
						deleteInstr(instrs, j, instr);
					}
				}
			EndFor
		EndFor
	}
#pragma endregion

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

		ConfictGraph define_use_chain;
		define_use_chain.addDef("a","B1", 1);
		define_use_chain.addDef("i","B1", 2);
								   
		define_use_chain.addUse("i","B2", 1);
								   
		define_use_chain.addDef("a","B3", 1);
		define_use_chain.addUse("a","B3", 1);
		define_use_chain.addUse("i","B3", 1);
								   
		define_use_chain.addDef("i","B3", 2);
		define_use_chain.addUse("i","B3", 2);
								   
		define_use_chain.addDef("b","B4", 1);
		define_use_chain.addUse("a","B4", 1);
		define_use_chain.addDef("i","B4", 2);
								   
		define_use_chain.addUse("i","B5", 1);
								   
		define_use_chain.addDef("b","B6", 1);
		define_use_chain.addUse("b","B6", 1);
		define_use_chain.addUse("i","B6", 1);
								   
		define_use_chain.addDef("i","B6", 2);
		define_use_chain.addUse("i","B6", 2);


		define_use_chain.genConfict(flow_graph);
		define_use_chain.printConfictGraph();
	}
	
}
