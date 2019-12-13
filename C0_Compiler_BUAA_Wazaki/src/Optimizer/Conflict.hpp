#pragma once

using namespace std;

namespace MidIR {

	class ConflictGraph {
		using Node = DefUseNode;
		using Chain = DefUseChain;
		using Web = DefUseWeb;

		ReachingDefine reaching_define;
		
		map<string, set<Node>> ident_defs_map, ident_uses_map;
		set<string> idents;
		
		map<string, set<Web>> ident_webs_map;
		map<string, ActiveRange> ident_range_map;

	public:
		void addDef(int block_num, string block_name, int number, string ident);
		void addUse(int block_num, string block_name, int number, string ident);
		void reAddDef();
		
		void genConfict(FlowGraph& flow_graph);
		void genWeb(string ident, FlowGraph& flow_graph);
		ActiveRange genRange(string ident);
		ActiveRange rangeMerge(ActiveRange r1, ActiveRange r2);

		vector<ActiveRange> getLoops(FlowGraph& flow_graph);
		void fixChainLoop(Chain& chain, vector<ActiveRange>& loops);

		map<string, ActiveRange> getRangeMap();

		void printConfictGraph();
		
	};

	inline void ConflictGraph::addDef(int block_num, string block_name, int number, string ident) {
		idents.insert(ident);
		Node new_node{ block_num, block_name, number };

		set<Node> def_nodes = ident_defs_map[ident];
		for (Node def_node : def_nodes) {
			if (def_node != new_node) {
				reaching_define.addKills(block_name, { def_node.toString() });
			}
		}
		ident_defs_map[ident].insert(new_node);
		reaching_define.addGens(block_name, { new_node.toString() });
	}

	inline void ConflictGraph::addUse(int block_num, string block_name, int number, string ident) {
		idents.insert(ident);
		Node new_node{ block_num, block_name, number };
		ident_uses_map[ident].insert(new_node);
		reaching_define.addBlockName(block_name);
	}

	inline void ConflictGraph::reAddDef() {
		for (string ident : idents) {
			set<Node> def_nodes = ident_defs_map[ident];
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
			genWeb(ident,flow_graph);
		}
		for (auto ident : idents) {
			ident_range_map[ident] = genRange(ident);
		}
	}

	inline void ConflictGraph::genWeb(string ident, FlowGraph& flow_graph) {
		
		set<Node> defs = ident_defs_map[ident];
		set<Node> uses = ident_uses_map[ident];
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

		if (!(startWith(ident, string("_T")) && isdigit(ident[2]))) {
			auto loops = getLoops(flow_graph);
			for (auto& chain : chains) {
				fixChainLoop(chain, loops);
			}
		}

		vector<Chain> old_chains;
		do {
			old_chains = chains;
			for (int i = 0; i < chains.size(); i++) {
				for (int j = 0; j < chains.size(); j++) {
					if (i != j && chains[i].overlap(chains[j])) {
						if (chains[i].def < chains[j].def) {
							chains[i].chainMerge(chains[j]);
							chains.erase(chains.begin() + j);
						}
						else {
							chains[j].chainMerge(chains[i]);
							chains.erase(chains.begin() + i);
						}
						goto outside_merge_chains;
					}
				}
			}
		outside_merge_chains:;
		} while (old_chains != chains);
		
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
		
		vector<Web> old_web;
		do {	//ѭ���ϲ�webs�е�ÿһ��web��ֱ�����ܺϲ�
			old_web = webs;
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
		} while(old_web != webs);
		
		//Ϊÿһ������ident��������webs
		for (Web& web : webs) {
			web.sort();
			ident_webs_map[ident].insert(web);
		}
		
	}

	inline ActiveRange ConflictGraph::genRange(string ident) {
		set<Web> webs = ident_webs_map[ident];

		if (webs.empty()) {	//ȫ�ֱ�������block��û��def������websΪ��
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

	inline vector<ActiveRange> ConflictGraph::getLoops(FlowGraph& flow_graph) {
		vector<ActiveRange> loops;
		for (auto edge : flow_graph.getFollowBlocksMap()) {	//edge block_name to vector<follow block name>
			string from_block_name = edge.first;
			for (string to_block_name : edge.second) {
				int from_block_num = flow_graph.getBlockNum(from_block_name);
				int to_block_num = flow_graph.getBlockNum(to_block_name);
				if (to_block_num <= from_block_num) {
					loops.push_back(ActiveRange(Node(to_block_num, to_block_name, 0), Node(from_block_num, from_block_name, INT32_MAX)));
				}
			}
		}
		return loops;
	}

	inline void ConflictGraph::fixChainLoop(Chain& chain, vector<ActiveRange>& loops) {
		bool change;
		do {
			change = false;
			for (ActiveRange loop : loops) {
				for (Node use : chain.uses) {
					if (loop.checkIn(use.block_num, use.line_num) && getSubIndex(chain.uses, loop.last) == -1) {
						chain.uses.push_back(loop.last);
						change = true;
						goto fixChainLoop_outside;
					}
				}
			}
		fixChainLoop_outside:;
		} while (change);

		chain.sort();
	}

	inline map<string, ActiveRange> ConflictGraph::getRangeMap() {
		return ident_range_map;
	}

	inline void ConflictGraph::printConfictGraph() {
		reAddDef();
		reaching_define.printInOut();
		for (auto i = idents.begin(); i != idents.end(); i++) {
			print("{}: ", *i);
			auto webs = ident_webs_map[*i];
			for (auto j = webs.begin(); j != webs.end(); j++) {
				println("W{}", *j);
			}
		}
	}

}
