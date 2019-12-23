#pragma once
#include "Struct.hpp"

using namespace std;

namespace MidIR {
	class NodeRange {
	public:
		DefUseNode first, last;
		NodeRange() {};
		NodeRange(DefUseNode node) : first(node), last(node) {}
		NodeRange(DefUseNode first, DefUseNode last) : first(min(first, last)), last(max(first, last)) {}

		bool checkIn(int block_num, int line_num);
		bool checkOut(int block_num, int line_num);
		
	};

	inline bool NodeRange::checkIn(int block_num, int line_num) {
		// return (first.block_num <= block_num || (first.block_num == block_num && first.line_num <= line_num))
		// 	&& (block_num <= last.block_num || (block_num == last.block_num && line_num <= last.line_num));
		return tie(first.block_num, first.line_num) <= tie(block_num, line_num)
			&& tie(block_num, line_num) <= tie(last.block_num, last.line_num);
	}

	inline bool NodeRange::checkOut(int block_num, int line_num) {
		return !checkIn(block_num, line_num);
	}

	
	class VarRange {
	public:
		vector<NodeRange> ranges;
		VarRange() {}
		VarRange(NodeRange range) {
			ranges.push_back(range);
		}

		void addRange(NodeRange range);
		bool checkIn(int block_num, int line_num);
		bool checkOut(int block_num, int line_num);
		
	};

	inline void VarRange::addRange(NodeRange range) {
		ranges.push_back(range);
	}

	inline bool VarRange::checkIn(int block_num, int line_num) {
		for (auto& range : ranges) {
			if (range.checkIn(block_num, line_num))
				return true;
		}
		return false;
		
	}

	inline bool VarRange::checkOut(int block_num, int line_num) {
		return !checkIn(block_num, line_num);
	}
	
}
