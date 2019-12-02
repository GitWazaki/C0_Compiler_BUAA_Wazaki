#pragma once

using namespace std;

namespace MidIR {

	struct ConstStr {
		string label;
		string value;
	};

	// struct ConstInt {
	// 	string label;
	// 	string value;
	// };	

	struct VarInt {
		string label;
		//string value;
	};

	struct VarIntArr {
		string label;
		int len;
	};

	class GlobalDefine {

	public:
		enum {
			CONST_STR,
			// CONST_INT,
			// CONST_CHAR,
			VAR_INT,
			// VAR_CHAR,
			VAR_INT_ARRAY,
		} type;

		ConstStr const_str;
		VarInt var_int;
		VarIntArr var_int_array;

		GlobalDefine(ConstStr constStr) : type(CONST_STR), const_str(constStr) {}

		GlobalDefine(VarInt varInt) : type(VAR_INT), var_int(varInt) {}

		GlobalDefine(VarIntArr var_int_array) : type(VAR_INT_ARRAY), var_int_array(var_int_array) {}
		
	};
	
}