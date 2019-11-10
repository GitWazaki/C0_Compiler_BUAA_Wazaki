#pragma once
#include <string>
#include <vector>
using namespace std;
namespace Symbol {
	enum SymbolType {
		INT,
		CHAR,
		VOID,
	};

	enum SymbolKind {
		CONST,
		VAR,
		ARRAY,
		FUNC,
		PARA,
	};

	class SymbolItem {
	public:
		SymbolItem() {}
		
		SymbolItem(string name, SymbolKind kind, SymbolType type, string constVal = "") {
			// const,var,para
			this->name = name;
			this->kind = kind;
			this->type = type;
			if (kind == CONST) {
				is_const = true;
				this->constVal = constVal;
			}

		}

		SymbolItem(string name, SymbolKind kind, SymbolType type, int len) {
			// array
			this->name = name;
			this->kind = kind;
			this->type = type;
			this->len = len;
		}

		SymbolItem(string name, SymbolKind kind, SymbolType type, vector<SymbolType> paraList) {
			// Func
			this->name = name;
			this->kind = kind;
			this->type = type;
			this->paraTypeList = paraList;
			if (type == INT || type == CHAR) {
				retFunc = true;
			}
		}

		string getName() {
			return this->name;
		}

		SymbolType getType() {
			return this->type;
		}

		vector<SymbolType> getParaTypeList() {
			return paraTypeList;
		}

		bool isConst() {
			return this->is_const;
		}

		bool isChar() {
			return type == CHAR;
		}
		
	private:
		string name;
		SymbolKind kind;
		SymbolType type;
		string constVal{};
		int len = -1;
		bool is_const = false;
		bool retFunc = false;
		vector<SymbolType> paraTypeList;
	};
}