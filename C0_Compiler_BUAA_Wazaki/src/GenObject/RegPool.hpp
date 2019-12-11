#pragma once

using namespace std;

namespace GenObject {

	class RegPool {
	public:
		vector<string> globalRegs = {
			"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
			"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
			"$v1"
		};

		int mempool_size = 0;
		map<string, int> memPool;		//mem to loc 
		map<string, string> regPool;	//_T{} to reg
		map<string, bool> availReg;		// reg to bool

		void resetPool();
		int allocMemPool(string id);
		int getMemPool(string id);
		string getReg();
		bool haveAvailReg();
		void allocRegPool(string id);
		string getRegPool(string id);

		void checkPool(map<string, MidIR::ActiveRange> ident_to_range, int block_num, int line_num);
		
	};

	inline void RegPool::resetPool() {	// 不同function刷新寄存器与内存池
		mempool_size = 0;
		memPool.clear();
		regPool.clear();
		for (int i = 0; i < globalRegs.size(); i++) {
			availReg[globalRegs[i]] = true;
		}
		// regs_stack.clear();
	}

	inline int RegPool::allocMemPool(string id) {
		memPool[id] = mempool_size;
		return mempool_size++;
	}

	inline int RegPool::getMemPool(string id) {
		if (memPool.find(id) == memPool.end()) {
			return allocMemPool(id);	// 还未分配内存空间则进行分配
		}
		return memPool[id];	//已分配内存空间
	}

	inline string RegPool::getReg() {	// 获取第一空闲的寄存器
		for (int i = 0; i < globalRegs.size(); i++) {
			if (availReg[globalRegs[i]]) {
				availReg[globalRegs[i]] = false;
				return globalRegs[i];
			}
		}
	}

	inline bool RegPool::haveAvailReg() {
		for (int i = 0; i < globalRegs.size(); i++) {
			if (availReg[globalRegs[i]]) {
				return true;
			}
		}
		return false;
	}

	inline void RegPool::allocRegPool(string id) {
		string reg = getReg();
		regPool[id] = reg;
	}

	inline string RegPool::getRegPool(string id) {
		if (regPool.find(id) == regPool.end()) {	//如果未分配
			if (haveAvailReg()) {	// 有空的寄存器
				allocRegPool(id);	//分配寄存器
			}
			else {
				return id;	//无空寄存器，返回原id: _T{}
			}
		}
		return regPool[id];	//如果已分配
	}

	inline void RegPool::checkPool(map<string, MidIR::ActiveRange> ident_to_range, int block_num,
		int line_num) {
		vector<string> erase_list;
		for (auto id : regPool) {
			if (ident_to_range[id.first].checkOut(block_num, line_num)) {
				availReg[id.second] = true;
				erase_list.push_back(id.first);
			}
		}

		for (auto id : erase_list) {
			regPool.erase(id);
		}
	}

}
