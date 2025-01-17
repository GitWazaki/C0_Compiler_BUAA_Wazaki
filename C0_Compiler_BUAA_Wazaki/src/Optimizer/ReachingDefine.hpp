#pragma once

namespace MidIR {

	class ReachingDefine {

		map<string, set<string>> gen, kill, in, out;
		set<string> block_names;

	public:
		void addGens(string block_name, vector<string> gens);
		void addKills(string block_name, vector<string> kills);
		void addBlockName(string block_name);

		set<string>& getInByBlock(string block);
		set<string>& getOutByBlock(string block);

		void build(FlowGraph& flowGraph);

		void printInOut();

	};

	inline void ReachingDefine::addGens(string block_name, vector<string> gens) {
		block_names.insert(block_name);
		for (string obj : gens) {
			gen[block_name].insert(obj);
		}
	}

	inline void ReachingDefine::addKills(string block_name, vector<string> kills) {
		block_names.insert(block_name);
		for (string obj : kills) {
			kill[block_name].insert(obj);
		}
	}

	inline void ReachingDefine::addBlockName(string block_name) {
		block_names.insert(block_name);
	}

	inline set<string>& ReachingDefine::getInByBlock(string block) {
		return in[block];
	}

	inline set<string>& ReachingDefine::getOutByBlock(string block) {
		return out[block];
	}

	inline void ReachingDefine::build(FlowGraph& flowGraph) {
		map<string, set<string>> last_in, last_out, last_gen, last_kill;
		while (true) {
			last_gen = gen;
			last_kill = kill;
			last_in = in;
			last_out = out;
			for (string block_name : block_names) {
				// 计算in集合 in[B] = U(out[preBlocks])
				vector<string> pre_blocks = flowGraph.getPreBlocks(block_name);
				set<string> preOuts{};
				for (int i = 0; i < pre_blocks.size(); i++) {
					block_names.insert(pre_blocks[i]);
					preOuts = setOr(preOuts, out[pre_blocks[i]]);
				}
				in[block_name] = preOuts;

				//计算out集合 out[B] = gen[B] ∪ (in[B] - kill[B])
				out[block_name] = setOr(gen[block_name], setSub(in[block_name], kill[block_name]));

			}
			if (last_gen == gen && last_kill == kill && last_in == in && last_out == out) {
				break;
			}
		}
	}

	inline void ReachingDefine::printInOut() {
		for (auto i = block_names.begin(); i != block_names.end(); i++) {
			const string& block_name = *i;

			print("Gen[{}]: ", block_name);
			for (auto j = gen[block_name].begin(); j != gen[block_name].end(); j++) {
				print("{}, ", *j);
			}
			println("");

			print("Kill[{}]: ", block_name);
			for (auto j = kill[block_name].begin(); j != kill[block_name].end(); j++) {
				print("{}, ", *j);
			}
			println("");

			print("In[{}]: ", block_name);
			for (auto j = in[block_name].begin(); j != in[block_name].end(); j++) {
				print("{}, ", *j);
			}
			println("");

			print("Out[{}]: ", block_name);
			for (auto j = out[block_name].begin(); j != out[block_name].end(); j++) {
				print("{}, ", *j);
			}
			println("");
			println("");
		}

	}

}
