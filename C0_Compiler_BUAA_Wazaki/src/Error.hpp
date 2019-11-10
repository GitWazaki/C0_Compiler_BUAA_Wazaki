#pragma once
#include<string>
#include<vector>
#include <algorithm>

using namespace std;

namespace Error {
	enum errorType {
		LEX_ERROR = 0,	//非法符号或不符合词法
		DUPDEFINE_IDENFR,	//名字重定义
		UNDEFINE_IDENFR,	//未定义的名字
		MISMATCHING_OF_PARANUM,	//函数参数格式不匹配
		MISMATCHING_OF_PARATYPE,	//函数参数类型不匹配
		ILLEGAL_TYPE_IN_CONDITION,	//条件判断中出现不合法的类型
		ERROR_RETURN_IN_NONRETFUNC,	//无返回值的函数存在不匹配的return语句
		ERROR_RETURN_IN_RETFUNC,	//有返回值的函数缺少return语句或存在不匹配的return语句
		ERROR_TYPE_OF_ARRAY_INDEX,	//数组元素的下标只能是整型表达式
		CHANGE_VAL_OF_CONST,		//不能改变常量的值
		MISS_SEMICN,	//应为分号
		MISS_RPARENT,	//应为右小括号')'
		MISS_RBRACK,	//应为右中括号']'
		MISS_WHILE_IN_DO_WHILE_STAT,	//do-while语句中缺少while
		ERROR_TYPE_TO_CONST,				//常量定义中=后面只能是整型或字符型常量
		OTHERS
	};

	static vector<tokenType> ErrorSkipSet[TOKEN_NUM]{
		//TODO
		{SEMICN},
	};

	struct errorStruct {
		token tok;
		errorType type;
		int line;
		int col;
		string toString() {
			return to_string(line) + " " + (char('a' + type));
		}
	};

	bool cmp(errorStruct e1, errorStruct e2) {
		int e1Line = e1.line;
		int e2Line = e2.line;
		int e1Col = e1.col;
		int e2Col = e2.col;
		if (e1Line == e2Line) {
			if (e1Col == e2Col) {
				return e1.type < e2.type;
			}
			else {
				return e1Col < e2Col;
			}
		}
		else {
			return e1Line < e2Line;
		}
	}

	class Error {
	public:
		Error(ofstream& fout): fout(fout) {
		}
		
		void errorHandle(errorStruct e);
		void errorPrint();

	private:
		ofstream& fout;
		// errorStruct err;
		// errorType errType;
		// vector<tokenType> skipList;
		vector<errorStruct> errorList;

	};

	inline void Error::errorHandle(errorStruct e) {
		errorList.push_back(e);
		
	}

	inline void Error::errorPrint() {
		sort(errorList.begin(), errorList.end(), cmp);
		int count = errorList.size();
		for(int i = 0;i<count;i++) {
			if(errorList[i].type!=OTHERS) {
				fout << errorList[i].toString() << endl;
			}
		}
	}

}
