#pragma once

namespace MidIR {
	
	class Func {
	public:
		string func_name;
		BlocksPtr blocks;
		
		Func(string func_name):func_name(func_name) {
			blocks = std::make_shared<Blocks>();
			blocks->clear();
			blocks->push_back(Block(func_name));
		}
		
	};
	
}