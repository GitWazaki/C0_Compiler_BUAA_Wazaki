#pragma once

using namespace std;

namespace GenObject {

	class RegPool {
	public:

		map<string, string> regPool;	//_T{} to reg
		map<string, bool> regAvailStateMap;		// reg to bool
		int used_mem_size = 0;
		map<string, int> memPool;		//mem to loc
		vector<map<string, int>> memPool_stack;
		vector<int> mem_size_stack;
		
		void resetPool();
		//reg
		string applyRegInPool(string id);
		bool checkAvailReg();
		string getAvailReg();
		string getRegInPool(string id);
		//mem
		int getMemInPool(string id);
		void pushMemPool();
		void popMemPool();
		//alloc
		void refrashPool(map<string, MidIR::VarRange> ident_to_range, int block_num, int line_num);
		
	};

	inline void RegPool::resetPool() {	// 不同function刷新寄存器与内存池
		used_mem_size = 0;
		memPool.clear();
		regPool.clear();
		for (int i = 0; i < globalRegs.size(); i++) {
			regAvailStateMap[globalRegs[i]] = true;
		}
		mem_size_stack.clear();
		memPool_stack.clear();
	}

	inline string RegPool::applyRegInPool(string id) {
		if (regPool.find(id) == regPool.end()) {	//如果未分配
			if (checkAvailReg()) {	// 有空的寄存器
				string reg = getAvailReg(); //分配寄存器
				regPool[id] = reg;
			}
			else {
				return id;	//无空寄存器，返回原id: _T{}
			}
		}
		return regPool[id];	//如果已分配
	}

	inline bool RegPool::checkAvailReg() {
		for (int i = 0; i < globalRegs.size(); i++) {
			if (regAvailStateMap[globalRegs[i]]) {
				return true;
			}
		}
		return false;
	}

	inline string RegPool::getAvailReg() {	// 获取第一空闲的寄存器
		for (int i = 0; i < globalRegs.size(); i++) {
			if (regAvailStateMap[globalRegs[i]]) {
				regAvailStateMap[globalRegs[i]] = false;
				return globalRegs[i];
			}
		}
	}

	inline string RegPool::getRegInPool(string id) {
		if (notFound(regPool, id)) {
			return id;
		}
		return regPool[id];
	}

	// memPool
	inline int RegPool::getMemInPool(string id) {
		if (memPool.find(id) == memPool.end()) {// 还未分配内存空间则进行分配
			memPool[id] = used_mem_size;
			return used_mem_size++;
		}
		return memPool[id];	//已分配内存空间
	}

	inline void RegPool::pushMemPool() {
		memPool_stack.push_back(memPool);
		mem_size_stack.push_back(used_mem_size);
		memPool.clear();
		used_mem_size = 0;
	}

	inline void RegPool::popMemPool() {
		memPool = memPool_stack.back();
		used_mem_size = mem_size_stack.back();
		memPool_stack.pop_back();
		mem_size_stack.pop_back();
	}

	// 刷新寄存器池，释放失效变量所占用的寄存器
	inline void RegPool::refrashPool(map<string, MidIR::VarRange> ident_to_range, int block_num,
		int line_num) {
		vector<string> erase_list;
		for (auto id : regPool) {
			if (ident_to_range[id.first].checkOut(block_num, line_num)) {
				//找到超出def-uses链范围的变量占用的寄存器
				regAvailStateMap[id.second] = true;
				erase_list.push_back(id.first);
			}
		}

		for (auto id : erase_list) {
			regPool.erase(id);	//释放它们
		}
	}

}
