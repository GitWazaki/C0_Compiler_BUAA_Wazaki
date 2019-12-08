#pragma once

using namespace std;

namespace MidIR {

	class FlowGraph {
		map<string, vector<string>> preBlocks;
		map<string, vector<string>> followBlocks;

		set<string> blockNames;

	public:
		void connectBlocks(string pre_block, string next_block);
		vector<string> getPreBlocks(string block_name);
		vector<string> getFollowBlocks(string block_name);
		
	};

	inline void FlowGraph::connectBlocks(string pre_block, string next_block) {
		preBlocks[next_block].push_back(pre_block);
		followBlocks[pre_block].push_back(next_block);
	}

	inline vector<string> FlowGraph::getPreBlocks(string block_name) {
		return preBlocks[block_name];
	}

	inline vector<string> FlowGraph::getFollowBlocks(string block_name) {
		return followBlocks[block_name];
	}

}
