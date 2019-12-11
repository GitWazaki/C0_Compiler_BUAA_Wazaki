#pragma once

using namespace std;

namespace MidIR {

	class ConflictGraph {
		using Node = DefUseNode;
		using Chain = DefUseChain;
		using Web = DefUseWeb;

		ReachingDefine reaching_define;
		
		map<string, set<Node>> ident_to_defs, ident_to_uses;
		set<string> idents;
		
		map<string, set<Web>> ident_to_webs;
		map<string, ActiveRange> ident_to_range;

	public:
		void addDef(int block_num, string block_name, int number, string ident);
		void addUse(int block_num, string block_name, int number, string ident);
		void reAddDef();
		
		void genConfict(FlowGraph& flow_graph);
		void genWeb(string ident);
		ActiveRange genRange(string ident);
		ActiveRange rangeMerge(ActiveRange r1, ActiveRange r2);

		map<string, ActiveRange> getRanges();

		void printConfictGraph();
		
	};

	inline void ConflictGraph::addDef(int block_num, string block_name, int number, string ident) {
		idents.insert(ident);
		Node new_node{ block_num, block_name, number };

		set<Node> def_nodes = ident_to_defs[ident];
		for (Node def_node : def_nodes) {
			if (def_node != new_node) {
				reaching_define.addKills(block_name, { def_node.toString() });
			}
		}
		ident_to_defs[ident].insert(new_node);
		reaching_define.addGens(block_name, { new_node.toString() });
	}

	inline void ConflictGraph::addUse(int block_num, string block_name, int number, string ident) {
		idents.insert(ident);
		Node new_node{ block_num, block_name, number };

		ident_to_uses[ident].insert(new_node);
	}

	inline void ConflictGraph::reAddDef() {
		for (string ident : idents) {
			set<Node> def_nodes = ident_to_defs[ident];
			for (Node node_i : def_nodes) {
				for(Node node_j : def_nodes) {
					if (node_i != node_j) {
						reaching_define.addKills(node_i.block_name, { node_j.toString() });
					}
				}
			}
		}
	}

	inline void ConflictGraph::genConfict(FlowGraph& flow_graph) {
		reAddDef();
		reaching_define.build(flow_graph);
		for (auto ident : idents) {
			genWeb(ident);
		}
		for (auto ident : idents) {
			cout << ident;
			ident_to_range[ident] = genRange(ident);
		}
	}

	inline void ConflictGraph::genWeb(string ident) {
		
		set<Node> defs = ident_to_defs[ident];
		set<Node> uses = ident_to_uses[ident];
		vector<Chain> chains;
		vector<Web> webs;

		// Ϊÿ��def��ʼ����
		for (Node def : defs) {
			chains.push_back(Chain(def));
		}
		//�ѱ���uses����use����ÿһ��ʹ��������def����
		for (Node use : uses) {
			for (Chain& chain : chains) {
				if (Found(reaching_define.getInByBlock(use.block_name), chain.def.toString())
					|| Found(reaching_define.getOutByBlock(use.block_name),chain.def.toString())
					) {
					chain.uses.push_back(use);
				}
			}
		}
		// ����chains��ÿһ�����õ�def-uses�����ж��ܷ���뵽webs�����ÿһ��web��ȥ
		for (int i = 0; i < chains.size(); i++) {
			bool new_one = true;
			Chain& chain = chains[i];
			for (int j = 0; j < webs.size(); j++) {
				Web& web = webs[j];
				if (web.canInsertChain(chain)) {	// ����chain���Լ��뵽����web��
					web.chains.insert(chains[i]);
					new_one = false;
					break;
				}
			}
			if (new_one) {	// chainû�п��Լ����web���½�һ��web����webs��
				Web web;
				web.chains.insert(chains[i]);
				webs.push_back(web);
			}
		}
		int old_size;
		do {	//ѭ���ϲ�webs�е�ÿһ��web��ֱ�����ܺϲ�
			old_size = webs.size();
			for(int i = 0; i < webs.size(); i++) {
				for (int j = 0; j < webs.size(); j++) {
					Web& web1 = webs[i];
					Web& web2 = webs[j];
					if(i == j) {
						continue;
					}
					if (web1.canMerge(web2)) {
						web1.merge(web2);
						webs.erase(webs.begin() + j);
						goto restart;
					}
				}
			}
		restart:;
		} while(old_size != webs.size());
		
		//Ϊÿһ������ident��������webs
		for (Web& web : webs) {
			web.sort();
			ident_to_webs[ident].insert(web);
		}
		
	}

	inline ActiveRange ConflictGraph::genRange(string ident) {
		set<Web> webs = ident_to_webs[ident];

		if (webs.empty()) {
			return ActiveRange(Node(0, "Global", 0), Node(INT32_MAX, "Global", INT32_MAX));
		}
		
		ActiveRange range(webs.begin()->chains.begin()->def);

		for(Web web : webs) {
			for(Chain chain : web.chains) {
				range = rangeMerge(range, { chain.def });
				for(Node use : chain.uses) {
					range = rangeMerge(range, { use });
				}
			}
		}

		return range;
	}

	inline ActiveRange ConflictGraph::rangeMerge(ActiveRange r1, ActiveRange r2) {
		return ActiveRange(min(r1.first, r2.first), max(r1.last, r2.last));
	}

	inline map<string, ActiveRange> ConflictGraph::getRanges() {
		return ident_to_range;
	}

	inline void ConflictGraph::printConfictGraph() {
		reAddDef();
		reaching_define.printInOut();
		for (auto i = idents.begin(); i != idents.end(); i++) {
			print("{}: ", *i);
			auto webs = ident_to_webs[*i];
			for (auto j = webs.begin(); j != webs.end(); j++) {
				println("W{}", *j);
			}
		}
	}

}
