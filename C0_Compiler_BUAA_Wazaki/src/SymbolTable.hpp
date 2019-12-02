#pragma once
#include "Symbol.hpp"
#include<string>
#include<vector>
#include <tuple>
using namespace  std;

namespace Symbol {
	class SymbolTable {
	public:
		bool push(SymbolItem symbol);
		
		tuple<bool, SymbolItem> findSymbol(string ident);
		tuple<bool, SymbolItem> findSymbolInScope(string ident);
		bool symbolExist(string ident);
		bool symbolExistInScope(string ident);
		bool checkSymbolIsGlobal(string ident);
		bool checkSymbolIsConst(string ident);
		bool checkSymbolIsChar(string ident);
		string getConstSymbolValue(string ident);
		
		void pushScope();
		void popScope();
		int getScope();
		// void addScopeOffset(int bytes);
		// void subScopeOffset(int bytes);
		// void clearScopeOffset();
		int getStackScopeBytes();
		int getStackOffsetBytesByIdent(string ident);
		int getGlobalOffsetBytesByIdent(string ident);
		
		tuple<int, SymbolItem> findSymbolAndScope(string ident);

	private:
		vector<SymbolItem> symbolStack;
		vector<SymbolItem> outdatedSymbol;
		vector<int> scopeStack = {-1};
		
	};

	inline bool SymbolTable::push(SymbolItem symbol) {
		if(symbolExistInScope(symbol.getName())) {
			return false;
		} else {
			if (scopeStack.back() == -1) {
				symbol.isGlobal = true;
			} else {
				symbol.isGlobal = false;
			}
			symbolStack.push_back(symbol);
			return true;
		}
	}

	inline tuple<bool, SymbolItem> SymbolTable::findSymbol(string ident) {
		for (int i = symbolStack.size() - 1; i >= 0; i--) {
			if (symbolStack[i].getName() == ident) {
				return make_tuple(true, symbolStack[i]);
			}
		}
		return make_tuple(false, SymbolItem{});
	}

	inline tuple<bool, SymbolItem> SymbolTable::findSymbolInScope(string ident) {
		for (int i = symbolStack.size() - 1; i >= 0 && i >= scopeStack.back(); i--) {
			if (symbolStack[i].getName() == ident) {
				return make_tuple(true, symbolStack[i]);
			}
		}
		return make_tuple(false, SymbolItem{});
	}

	inline bool SymbolTable::symbolExistInScope(string ident) {
		bool exist;
		tie(exist,ignore) = findSymbolInScope(ident);
		return exist;
	}

	inline bool SymbolTable::checkSymbolIsGlobal(string ident) {
		SymbolItem symbol;
		tie(std::ignore, symbol) = findSymbol(ident);
		return symbol.isGlobal;
	}

	inline bool SymbolTable::checkSymbolIsConst(string ident) {
		SymbolItem symbol;
		tie(std::ignore, symbol) = findSymbol(ident);
		return symbol.isConst();
	}

	inline bool SymbolTable::checkSymbolIsChar(string ident) {
		SymbolItem symbol;
		tie(std::ignore, symbol) = findSymbol(ident);
		return symbol.getType() == CHAR;
	}

	inline string SymbolTable::getConstSymbolValue(string ident) {
		SymbolItem symbol;
		tie(std::ignore, symbol) = findSymbol(ident);
		return symbol.getConstVal();
	}

	inline void SymbolTable::pushScope() {
		scopeStack.push_back(symbolStack.size());
	}

	inline void SymbolTable::popScope() {
		while (symbolStack.size() > scopeStack.back()) {
			outdatedSymbol.push_back(symbolStack.back());
			symbolStack.pop_back();
		}
		scopeStack.pop_back();
	}

	inline int SymbolTable::getScope() {
		return scopeStack.back();
	}
#pragma region extra_scope_offset
	// int extra_scope_offset = 0;
	// inline void SymbolTable::addScopeOffset(int bytes) {
	// 	extra_scope_offset += bytes;
	// }
	// inline void SymbolTable::subScopeOffset(int bytes) {
	// 	extra_scope_offset -= bytes;
	// }
	//
	// inline void SymbolTable::clearScopeOffset() {
	// 	extra_scope_offset = 0;
	// }
#pragma endregion
	
	inline int SymbolTable::getStackScopeBytes() {
		//用于计算函数运行栈空间
		int bytes = 0;
		for (int i = scopeStack.back(); i < symbolStack.size(); i++) {
			bytes += symbolStack[i].getBytes();
		}
		return bytes;
	}

	inline int SymbolTable::getStackOffsetBytesByIdent(string ident) {
		//计算表中一元素，相对当前栈顶$sp的偏移量
		int bytes = 0;
		for (int i = scopeStack.back(); i < symbolStack.size(); i++) {
			if (symbolStack[i].getName() == ident)
				break;
			bytes += symbolStack[i].getBytes();
		}
		return -bytes;
	}

	inline int SymbolTable::getGlobalOffsetBytesByIdent(string ident) {
		int bytes = 0;
		for (int i = 0; i < scopeStack[1]; i++) {
			if (symbolStack[i].getName() == ident)
				break;
			bytes += symbolStack[i].getBytes();
		}
		return bytes;
	}

	inline tuple<int, SymbolItem> SymbolTable::findSymbolAndScope(string ident) {
		//TODO 在读语句处可替换为findSymbol
		for (int i = symbolStack.size() - 1; i >= 0; i--) {
			if (symbolStack[i].getName() == ident) {
				return make_tuple(i, symbolStack[i]);
			}
		}
		return make_tuple(-1, SymbolItem{});
	}
	
}
