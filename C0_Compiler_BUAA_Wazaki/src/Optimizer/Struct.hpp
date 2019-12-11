#pragma once
#include <ostream>

using namespace std;

namespace MidIR {

	class DefUseNode {

	public:

		string block_name;
		int block_num,  line_num;

		DefUseNode() {};
		DefUseNode(int block_num, string block,int num) : block_num(block_num), block_name(block), line_num(num) {}
		
		string toString() {
			return FORMAT("<{}-{}, {}>", block_num, block_name, line_num);
		}

		friend std::ostream& operator<<(std::ostream& os, DefUseNode& obj) {
			return os << obj.toString();
		}

		friend bool operator<(const DefUseNode& lhs, const DefUseNode& rhs) {
			if (lhs.block_name < rhs.block_name)
				return true;
			if (rhs.block_name < lhs.block_name)
				return false;
			if (lhs.block_num < rhs.block_num)
				return true;
			if (rhs.block_num < lhs.block_num)
				return false;
			return lhs.line_num < rhs.line_num;
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
	
}
