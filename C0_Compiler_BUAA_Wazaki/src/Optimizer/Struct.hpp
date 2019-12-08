#pragma once
#include <ostream>

using namespace std;

namespace MidIR {

	class useNode {
	public:

		string block, number;
		
		useNode(string block, int number) : block(block), number(to_string(number)) {}
		
		string toString() {
			return FORMAT("<{}, {}>", block, number);
		}

		friend std::ostream& operator<<(std::ostream& os, useNode& obj) {
			return os << obj.toString();
		}

		friend bool operator<(const useNode& lhs, const useNode& rhs) {
			if (lhs.block < rhs.block)
				return true;
			if (rhs.block < lhs.block)
				return false;
			return lhs.number < rhs.number;
		}

		friend bool operator==(const useNode& lhs, const useNode& rhs) {
			return lhs.block == rhs.block
				&& lhs.number == rhs.number;
		}

		friend bool operator!=(const useNode& lhs, const useNode& rhs) {
			return !(lhs == rhs);
		}
		
	};

	class useChain {
	public:

		using Node = useNode;
		using Chain = useChain;
	
		Node def;
		vector<Node> uses;
		useChain(Node def) : def(def) {}

		set<Node> getDefAndUsesSet() {
			set<Node> ans;
			ans.insert(ans.begin(), def);
			for (Node use : uses) {
				ans.insert(use);
			}
			return ans;
		}

		bool overlap(Chain chain) {
			set<Node> def_uses_set_1 = getDefAndUsesSet();
			set<Node> def_uses_set_2 = chain.getDefAndUsesSet();
			for (Node node : def_uses_set_2) {
				if (Found(def_uses_set_1, node)) {
					return true;
				}
			}
			return false;
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

		friend std::ostream& operator<<(std::ostream& os, useChain& obj) {
			return os << obj.toString();
		}

		friend bool operator<(const useChain& lhs, const useChain& rhs) {
			if (lhs.def < rhs.def)
				return true;
			if (rhs.def < lhs.def)
				return false;
			return lhs.uses < rhs.uses;
		}
		
	};

	class useWeb {

		using Node = useNode;
		using Chain = useChain;
		using Web = useWeb;
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

		friend std::ostream& operator<<(std::ostream& os, useWeb& obj) {
			return os << obj.toString();
		}

		friend bool operator<(const useWeb& lhs, const useWeb& rhs) {
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
