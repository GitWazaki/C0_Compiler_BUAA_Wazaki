#pragma once

using namespace std;

namespace MidIR {

	class FlowGraph {

		set<string> blockNames;
		
		map<string, vector<string>> preBlocks;
		map<string, vector<string>> followBlocks;
		map<string, int> name_to_num;
		
	public:
		void connectBlocks(string pre_block, string next_block);
		vector<string> getPreBlocks(string block_name);
		vector<string> getFollowBlocks(string block_name);
		map<string, vector<string>> getPreBlocksMap();
		map<string, vector<string>> getFollowBlocksMap();
		void addBlockNum(string block_name, int block_num);
		int getBlockNum(string block_name);
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

	inline map<string, vector<string>> FlowGraph::getPreBlocksMap() {
		return preBlocks;
	}

	inline map<string, vector<string>> FlowGraph::getFollowBlocksMap() {
		return followBlocks;
	}

	inline void FlowGraph::addBlockNum(string block_name, int block_num) {
		name_to_num[block_name] = block_num;
	}

	inline int FlowGraph::getBlockNum(string block_name) {
		return name_to_num[block_name];
	}

}
