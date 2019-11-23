#pragma once

using namespace std;

namespace MidCode {

	struct ConstStr {
		string lable;
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

		ConstStr constStr;
		VarInt varInt;
		VarIntArr varIntArray;

		GlobalDefine(ConstStr constStr) : type(CONST_STR), constStr(constStr) {}

		GlobalDefine(VarIntArr var_int_array) : type(VAR_INT_ARRAY), varIntArray(var_int_array) {}

		GlobalDefine(VarInt varInt) : type(VAR_INT), varInt(varInt) {}
		
	};
	
}