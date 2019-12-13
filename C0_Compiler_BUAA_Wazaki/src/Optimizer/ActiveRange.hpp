#pragma once
#include "Struct.hpp"

using namespace std;

namespace MidIR {
	class ActiveRange {
	public:
		DefUseNode first, last;
		ActiveRange() {};
		ActiveRange(DefUseNode node) : first(node), last(node) {}
		ActiveRange(DefUseNode first, DefUseNode last) : first(min(first, last)), last(max(first, last)) {}


		bool checkOut(int block_num, int line_num);

		bool checkIn(int block_num, int line_num);
	};

	inline bool ActiveRange::checkOut(int block_num, int line_num) {
		return !checkIn(block_num, line_num);
	}

	inline bool ActiveRange::checkIn(int block_num, int line_num) {
		// return (first.block_num <= block_num || (first.block_num == block_num && first.line_num <= line_num))
		// 	&& (block_num <= last.block_num || (block_num == last.block_num && line_num <= last.line_num));
			return tie(first.block_num, first.line_num) <= tie(block_num, line_num)
		&& tie(block_num, line_num) <= tie(last.block_num, last.line_num);
	}

}
