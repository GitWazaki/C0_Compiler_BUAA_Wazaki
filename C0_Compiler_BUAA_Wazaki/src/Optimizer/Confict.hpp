#pragma once

using namespace std;

namespace MidIR {

	class ConfictGraph {
		using Node = useNode;
		using Chain = useChain;
		using Web = useWeb;

		ReachingDefine reaching_define;
		
		map<string, set<Node>> ident_to_defs, ident_to_uses;
		set<string> idents;
		
		map<string, set<Web>> ident_to_webs;

	public:
		void addDef(string ident, string block_name, int number);
		void addUse(string ident, string block_name, int number);
		void reAddDef();
		
		void genConfict(FlowGraph& flow_graph);
		void genWeb(string ident);

		void printConfictGraph();
		
	};

	inline void ConfictGraph::addDef(string ident, string block_name, int number) {
		idents.insert(ident);
		Node new_node{ block_name, number };

		set<Node> def_nodes = ident_to_defs[ident];
		for (Node def_node : def_nodes) {
			if (def_node != new_node) {
				reaching_define.addKills(block_name, { def_node.toString() });
			}
		}
		ident_to_defs[ident].insert(new_node);
		reaching_define.addGens(block_name, { new_node.toString() });
	}

	inline void ConfictGraph::addUse(string ident, string block_name, int number) {
		idents.insert(ident);
		Node new_node{ block_name, number };

		ident_to_uses[ident].insert(new_node);
	}

	inline void ConfictGraph::reAddDef() {
		for (string ident : idents) {
			set<Node> def_nodes = ident_to_defs[ident];
			for (Node node_i : def_nodes) {
				for(Node node_j : def_nodes) {
					if (node_i != node_j) {
						reaching_define.addKills(node_i.block, { node_j.toString() });
					}
				}
			}
		}
	}

	inline void ConfictGraph::genConfict(FlowGraph& flow_graph) {
		reaching_define.build(flow_graph);
		for (string ident : idents) {
			genWeb(ident);
		}
	}

	inline void ConfictGraph::genWeb(string ident) {
		
		set<Node> defs = ident_to_defs[ident];
		set<Node> uses = ident_to_uses[ident];
		vector<Chain> chains;
		vector<Web> webs;

		// 为每个def初始化链
		for (Node def : defs) {
			chains.push_back(Chain(def));
		}
		//把遍历uses，把use加入每一个使用了它的def链中
		for (Node use : uses) {
			for (Chain& chain : chains) {
				if (Found(reaching_define.getInByBlock(use.block), chain.def.toString())) {
					chain.uses.push_back(use);
				}
			}
		}
		// 对于chains中每一条建好的def-uses链，判断能否加入到webs网络的每一条web中去
		for (int i = 0; i < chains.size(); i++) {
			bool new_one = true;
			Chain& chain = chains[i];
			for (int j = 0; j < webs.size(); j++) {
				Web& web = webs[j];
				if (web.canInsertChain(chain)) {	// 该链chain可以加入到网络web中
					web.chains.insert(chains[i]);
					new_one = false;
					break;
				}
			}
			if (new_one) {	// chain没有可以加入的web则新建一个web加入webs中
				Web web;
				web.chains.insert(chains[i]);
				webs.push_back(web);
			}
		}
		int old_size;
		do {	//循环合并webs中的每一个web，直至不能合并
			old_size = webs.size();
			for(int i = 0; i < webs.size() - 1; i++) {
				Web& web1 = webs[i];
				Web& web2 = webs[i + 1];
				if(web1.canMerge(web2)) {
					web1.merge(web2);
					webs.erase(webs.begin()+i+1);
					i--;
				}
			}	
		} while(old_size != webs.size());
		//为每一个变量ident设置它的webs
		for (Web web : webs) {
			ident_to_webs[ident].insert(web);
		}
		
	}

	inline void ConfictGraph::printConfictGraph() {
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
