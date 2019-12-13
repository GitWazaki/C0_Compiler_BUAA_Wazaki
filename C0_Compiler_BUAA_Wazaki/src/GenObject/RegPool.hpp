#pragma once

using namespace std;

namespace GenObject {

	class RegPool {
	public:
		vector<string> globalRegs = {
			"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
			"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
			"$v1",
			"$k1",
		};

		int used_mem_size = 0;
		map<string, int> memPool;		//mem to loc 
		map<string, string> regPool;	//_T{} to reg
		map<string, bool> regAvailStateMap;		// reg to bool

		void resetPool();
		//reg
		string applyRegInPool(string id);
		bool checkAvailReg();
		string getAvailReg();
		
		//mem
		int getMemInPool(string id);
		
		//alloc
		void refrashPool(map<string, MidIR::ActiveRange> ident_to_range, int block_num, int line_num);
		
	};

	inline void RegPool::resetPool() {	// ��ͬfunctionˢ�¼Ĵ������ڴ��
		used_mem_size = 0;
		memPool.clear();
		regPool.clear();
		for (int i = 0; i < globalRegs.size(); i++) {
			regAvailStateMap[globalRegs[i]] = true;
		}
		// regs_stack.clear();
	}

	inline string RegPool::applyRegInPool(string id) {
		if (regPool.find(id) == regPool.end()) {	//���δ����
			if (checkAvailReg()) {	// �пյļĴ���
				string reg = getAvailReg(); //����Ĵ���
				regPool[id] = reg;
			}
			else {
				return id;	//�޿ռĴ���������ԭid: _T{}
			}
		}
		return regPool[id];	//����ѷ���
	}

	inline bool RegPool::checkAvailReg() {
		for (int i = 0; i < globalRegs.size(); i++) {
			if (regAvailStateMap[globalRegs[i]]) {
				return true;
			}
		}
		return false;
	}

	inline string RegPool::getAvailReg() {	// ��ȡ��һ���еļĴ���
		for (int i = 0; i < globalRegs.size(); i++) {
			if (regAvailStateMap[globalRegs[i]]) {
				regAvailStateMap[globalRegs[i]] = false;
				return globalRegs[i];
			}
		}
	}

	// memPool
	inline int RegPool::getMemInPool(string id) {
		if (memPool.find(id) == memPool.end()) {// ��δ�����ڴ�ռ�����з���
			memPool[id] = used_mem_size;
			return used_mem_size++;
		}
		return memPool[id];	//�ѷ����ڴ�ռ�
	}

	// ˢ�¼Ĵ����أ��ͷ�ʧЧ������ռ�õļĴ���
	inline void RegPool::refrashPool(map<string, MidIR::ActiveRange> ident_to_range, int block_num,
		int line_num) {
		vector<string> erase_list;
		for (auto id : regPool) {
			if (ident_to_range[id.first].checkOut(block_num, line_num)) {
				//�ҵ�����def-uses����Χ�ı���ռ�õļĴ���
				regAvailStateMap[id.second] = true;
				erase_list.push_back(id.first);
			}
		}

		for (auto id : erase_list) {
			regPool.erase(id);	//�ͷ�����
		}
	}

}
