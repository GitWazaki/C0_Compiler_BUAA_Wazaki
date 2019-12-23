#pragma once
#include <ostream>

using namespace std;

namespace MidIR {

	class DefUseNode {


	public:

		int block_num,  line_num;
		string block_name;

		DefUseNode() {};
		DefUseNode(int block_num, string block,int num) : block_num(block_num), block_name(block), line_num(num) {}
		
		string toString() {
			return FORMAT("<{}:({},{})>", block_name, block_num, line_num);
		}

		friend std::ostream& operator<<(std::ostream& os, DefUseNode& obj) {
			return os << obj.toString();
		}

		friend bool operator<(const DefUseNode& lhs, const DefUseNode& rhs) {
			if (lhs.block_num < rhs.block_num)
				return true;
			if (rhs.block_num < lhs.block_num)
				return false;
			if (lhs.line_num < rhs.line_num)
				return true;
			if (rhs.line_num < lhs.line_num)
				return false;
			return lhs.block_name < rhs.block_name;
		}

		friend bool operator==(const DefUseNode& lhs, const DefUseNode& rhs) {
			return lhs.block_name == rhs.block_name
				&& lhs.block_num == rhs.block_num
				&& lhs.line_num == rhs.line_num;
		}

		friend bool operator!=(const DefUseNode& lhs, const DefUseNode& rhs) {
			return !(lhs == rhs);
		}
		
	};

	class DefUseChain {

	public:

		using Node = DefUseNode;
		using Chain = DefUseChain;
	
		Node def;
		vector<Node> uses;
		DefUseChain(){}
		DefUseChain(Node def) : def(def) {}

		set<Node> getDefUsesSet() {
			set<Node> ans;
			ans.insert(ans.begin(), def);
			for (Node use : uses) {
				ans.insert(use);
			}
			return ans;
		}

		bool overlap(Chain chain) {
			set<Node> def_uses_set_1 = getDefUsesSet();
			set<Node> def_uses_set_2 = chain.getDefUsesSet();
			for (Node node : def_uses_set_2) {
				if (Found(def_uses_set_1, node)) {
					return true;
				}
			}
			return false;
		}

		void chainMerge(Chain chain) {
			uses.push_back(chain.def);
			for (Node node : chain.uses) {
				uses.push_back(node);
			}
			sort();
		}

		void sort() {
			std::sort(uses.begin(), uses.end());
		}

		std::string toString() {
			std::ostringstream buf;
			buf << "{" << def;
			for (int i = 0; i < uses.size(); i++) {
				buf << ", " << uses[i];
			}
			buf << "}";
			return buf.str();
		}

		friend std::ostream& operator<<(std::ostream& os, DefUseChain& obj) {
			return os << obj.toString();
		}

		friend bool operator<(const DefUseChain& lhs, const DefUseChain& rhs) {
			if (lhs.def < rhs.def)
				return true;
			if (rhs.def < lhs.def)
				return false;
			return lhs.uses < rhs.uses;
		}
		friend bool operator==(const DefUseChain& lhs, const DefUseChain& rhs) {
			return lhs.def == rhs.def
				&& lhs.uses == rhs.uses;
		}

		friend bool operator!=(const DefUseChain& lhs, const DefUseChain& rhs) {
			return !(lhs == rhs);
		}
		
	};

	class DefUseWeb {

		using Node = DefUseNode;
		using Chain = DefUseChain;
		using Web = DefUseWeb;
	public:
		set<Chain> chains;

		bool canInsertChain(Chain chain) {
			for (Chain _chain : chains) {
				if (_chain.overlap(chain)) {
					return true;
				}
			}
			return false;
		}

		bool canMerge(Web web) {
			for(Chain chain : web.chains) {
				if(canInsertChain(chain)) {
					return true;
				}
			}
			return false;
		}

		void merge(Web web) {
			for (Chain chain : web.chains) {
				chains.insert(chain);
			}
		}

		void sort() {
			for (auto chain : chains) {
				chain.sort();
			}
		}

		friend std::ostream& operator<<(std::ostream& os, DefUseWeb& obj) {
			return os << obj.toString();
		}

		friend bool operator<(const DefUseWeb& lhs, const DefUseWeb& rhs) {
			return lhs.chains < rhs.chains;
		}

		friend bool operator==(const DefUseWeb& lhs, const DefUseWeb& rhs) {
			return lhs.chains == rhs.chains;
		}

		friend bool operator!=(const DefUseWeb& lhs, const DefUseWeb& rhs) {
			return !(lhs == rhs);
		}

		string toString() {
			std::ostringstream buf;
			buf << "{";
			for (auto i = chains.begin(); i != chains.end(); i++) {
				Chain chain = *i;
				buf << chain.toString() << ", ";
			}
			buf << "}";
			return buf.str();
		}
		
	};

	class InstrIterInFunc {
	public:
		Func& func;
		int block_num;
		int line_num;

		InstrIterInFunc(Func& func, int block_num, int line_num) : func(func), block_num(block_num), line_num(line_num){};

		Block& getBlock() {
			return func.blocks->at(block_num);
		}
		
		MidInstr& getInstr() {
			return func.blocks->at(block_num).instrs[line_num];
		}

		void setBlock(Block block) {
			func.blocks->at(block_num) = block;
		}
		
		void setInstr(MidInstr instr) {
			func.blocks->at(block_num).instrs[line_num] = instr;
		}

		bool next() {
			line_num++;
			while (line_num >= getBlock().instrs.size()) {
				block_num++;
				line_num = 0;
				if (block_num >= func.blocks->size()) {
					return false;
				}
			}
			return true;
		}

		bool prev() {
			line_num--;
			while (line_num < 0) {
				block_num--;
				if (block_num < 0) {
					return false;
				}
				line_num = getBlock().instrs.size() - 1;
				
			}
			return true;
		}
		
		
	};
	
}
