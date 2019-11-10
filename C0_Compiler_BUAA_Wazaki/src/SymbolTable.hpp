#pragma once
#include<string>
#include<vector>
#include <tuple>
using namespace  std;

namespace Symbol {
	class SymbolTable {
	public:
		//TODO
		bool push(SymbolItem symbol);	//bool
		tuple<bool, SymbolItem> findSymbol(string ident);
		tuple<bool, SymbolItem> findSymbolInScope(string ident);
		bool symbolExist(string ident);
		bool symbolExistInScope(string ident);
		void pushScope();
		void popScope();
		
	private:
		vector<SymbolItem> symbolStack;
		vector<SymbolItem> outdatedSymbol;
		vector<int> scopeIndexStack = {-1};
		
	};

	inline bool SymbolTable::push(SymbolItem symbol) {
		if(symbolExistInScope(symbol.getName())) {
			return false;
		} else {
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
		for (int i = symbolStack.size() - 1; i >= 0 && i >= scopeIndexStack.back(); i--) {
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

	inline void SymbolTable::pushScope() {
		scopeIndexStack.push_back(symbolStack.size());
	}

	inline void SymbolTable::popScope() {
		while (symbolStack.size() > scopeIndexStack.back()) {
			outdatedSymbol.push_back(symbolStack.back());
			symbolStack.pop_back();
		}
		scopeIndexStack.pop_back();
	}
}
