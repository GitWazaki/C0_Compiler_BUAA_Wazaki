#pragma once
#include<string>
#include<vector>
#include <algorithm>

using namespace std;

namespace Error {
	enum errorType {
		LEX_ERROR = 0,	//�Ƿ����Ż򲻷��ϴʷ�
		DUPDEFINE_IDENFR,	//�����ض���
		UNDEFINE_IDENFR,	//δ���������
		MISMATCHING_OF_PARANUM,	//����������ʽ��ƥ��
		MISMATCHING_OF_PARATYPE,	//�����������Ͳ�ƥ��
		ILLEGAL_TYPE_IN_CONDITION,	//�����ж��г��ֲ��Ϸ�������
		ERROR_RETURN_IN_NONRETFUNC,	//�޷���ֵ�ĺ������ڲ�ƥ���return���
		ERROR_RETURN_IN_RETFUNC,	//�з���ֵ�ĺ���ȱ��return������ڲ�ƥ���return���
		ERROR_TYPE_OF_ARRAY_INDEX,	//����Ԫ�ص��±�ֻ�������ͱ��ʽ
		CHANGE_VAL_OF_CONST,		//���ܸı䳣����ֵ
		MISS_SEMICN,	//ӦΪ�ֺ�
		MISS_RPARENT,	//ӦΪ��С����')'
		MISS_RBRACK,	//ӦΪ��������']'
		MISS_WHILE_IN_DO_WHILE_STAT,	//do-while�����ȱ��while
		ERROR_TYPE_TO_CONST,				//����������=����ֻ�������ͻ��ַ��ͳ���
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
