#pragma once
#include <iostream>
#include "Lexer.hpp"
#include "deque"
#include <map>
#include "Error.hpp"

namespace Parser2 {

	map<string, bool> funcRetMap;
	Symbol::SymbolTable symbolTable;
	int curLine, tokLine;

	class Parser2 {
	public:
		Parser2(Lexer::Lexer& lexer, ofstream& fout, Error::Error& err) : lexer(lexer), fout(fout), err(err) {
		}

		void program(); //程序

	private:
		token tok;
		tokenType tokType;
		deque<token> tokQueue;
		Lexer::Lexer& lexer;
		Error::Error& err;
		ofstream& fout;

		tokenType lookToken(int preN = 0);
		token eatToken(tokenType);

		void plusOp(); //＜加法运算符＞
		void mulOp(); //＜乘法运算符＞ 
		bool isMulOp(tokenType token); //增加函数，判断token是否是乘法运算符
		void relationalOp(); //＜关系运算符＞
		void strConst(); //字符串常量

		void constDec(); //常量说明
		void constDef(); //常量定义
		tuple<bool, token> integer(); //整数
		token unsignedInt(); //无符号整数
		void varDec(); //变量说明
		void varDef(); //变量定义
		void varDefName(); //增加函数，处理变量名
		void funcDef(); //函数定义
		void retfuncDef(); //有返回值函数定义
		void nonRetfuncDef(); //无返回值函数定义
		token headDec(); //声明头部
		vector<Symbol::SymbolItem> paraList(); //参数表
		void identType(); //类型标识符
		void compStatement(); //复合语句
		void statementBlock(); //语句块
		void statement(); //语句
		void funcCallStatement(); //函数调用语句
		vector<Symbol::SymbolType>  funcCallInputParaList(vector<Symbol::SymbolType>); //值参数表
		void assignStat(); //赋值语句
		Symbol::SymbolType expr(); //表达式
		void term(); //项
		void factor(); //因子
		void condition(); //条件
		void condStatement(); //条件语句
		void elseCondStat(); //条件语句else部分
		void cycleStatement(); //循环语句
		void step(); //步长
		void readStatement(); //读语句
		void writeStatement(); //写语句
		void writeStatContent(); //增加函数，处理写语言括号内的内容
		void retStatement(); //返回语句
		void mainFunc(); //主函数
		void addFuncName(bool ret); //功能：添加函数头与有无返回值的对应

		void error(Error::errorType);
		Error::errorStruct tokenToError(Error::errorType) const;
		void skip(Error::errorType);

		Symbol::SymbolType tokenType2SymbolType(tokenType);
		vector<Symbol::SymbolType> SymbolItemList2SymbolTypeList(vector<Symbol::SymbolItem>);
		void checkPush(bool);
		void checkFind(bool);
	};

	inline void Parser2::error(Error::errorType eType) {
		err.errorHandle(tokenToError(eType));
		skip(eType);
	}

	inline Error::errorStruct Parser2::tokenToError(Error::errorType eType) const {
		Error::errorStruct e;
		e.tok = tok;
		//e.line = tok.line;
		e.line = curLine;
		e.col = tok.col;
		e.type = eType;;
		return e;
	}

	inline void Parser2::skip(Error::errorType eType) {
		if (eType == Error::ERROR_TYPE_TO_CONST) {
			eatToken(lookToken());
		}
		//TODO
		// vector<tokenType> skipList = Error::ErrorSkipSet[eType];
		// tokenType temp = lookToken();
		// while (checkFind(skipList.begin(), skipList.end(), temp) == skipList.end()) {
		// 	eatToken(temp);
		// 	temp = lookToken();
		// }
	}

	inline Symbol::SymbolType Parser2::tokenType2SymbolType(tokenType tType) {
		if (tType == INTTK) {
			return Symbol::INT;
		} else if (tType == CHARTK) {
			return Symbol::CHAR;
		} else {
			//TODO error
		}
	}

	inline vector<Symbol::SymbolType> Parser2::SymbolItemList2SymbolTypeList(vector<Symbol::SymbolItem> paraList) {
		std::vector<Symbol::SymbolType> symbolTypeList;
		for (int i = 0; i < paraList.size(); i++) {
			symbolTypeList.push_back(paraList[i].getType());
		}
		return symbolTypeList;
	}

	inline void Parser2::checkPush(bool push) {
		if (!push) {
			error(Error::DUPDEFINE_IDENFR);
		}
	}

	inline void Parser2::checkFind(bool find) {
		if (!find) {
			error(Error::UNDEFINE_IDENFR);
		}
	}

	inline tokenType Parser2::lookToken(int preN) {
		while (tokQueue.size() <= preN) {
			token t = lexer.getToken(preN);
			if(t.val!="") {
				tokQueue.push_back(t);
			} else {
				lexer.eatToken();
			}
		}
		tok = tokQueue[preN];
		tokType = tok.type;
		if(preN == 0) {
			tokLine = tok.line;
		}
		return tokType;
	}

	inline token Parser2::eatToken(tokenType type) {
		token temp;
		if (tokQueue.empty()) {
			tokQueue.push_back(lexer.getToken());
		}
		if ((temp = tokQueue.front()).type == type) {
			tokQueue.pop_front();
			lexer.eatToken();
			curLine = tokLine;
			//lookToken(); //可去除
		} else {
			switch (type) {
			case SEMICN:
				error(Error::MISS_SEMICN);
				break;
			case RPARENT:
				error(Error::MISS_RPARENT);
				break;
			case RBRACK:
				error(Error::MISS_RBRACK);
				break;
			default:
				error(Error::OTHERS);
				break;
			}
		}
		fout << temp.toString() << endl;
		return temp;
	}

	//＜程序＞::= ［＜常量说明＞］［＜变量说明＞］{<函数定义>}＜主函数＞
	inline void Parser2::program() {
		switch (lookToken()) {
		case CONSTTK:
		case INTTK:
		case CHARTK:
		case VOIDTK:
			constDec();
			varDec();
			funcDef();
			mainFunc();
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<程序>" << endl;
		err.errorPrint();
	}

	//＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}	
	inline void Parser2::constDec() {
		bool hasConstDec = false;
		while (lookToken() == CONSTTK) {
			eatToken(CONSTTK);
			constDef();
			eatToken(SEMICN);
			if (!hasConstDec) {
				hasConstDec = true;
			}
		}
		if (hasConstDec) {
			fout << "<常量说明>" << endl;
		}
	}

	// ＜常量定义＞ ::= int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞} | char＜标识符＞＝＜字符＞{ ,＜标识符＞＝＜字符＞ }
	inline void Parser2::constDef() {
		token constName;
		token constVal;
		bool check;
		switch (lookToken()) {
		case INTTK:
			eatToken(INTTK);
			constName = eatToken(IDENFR);
			eatToken(ASSIGN);
			tie(check,constVal) = integer();
			if(check==false) {
				error(Error::ERROR_TYPE_TO_CONST);
			}
			checkPush(symbolTable.push(Symbol::SymbolItem(
				constName.val, Symbol::CONST, Symbol::INT, constVal.val)));
			while (lookToken() == COMMA) {
				//{,＜标识符＞＝＜整数＞}
				eatToken(COMMA);
				constName = eatToken(IDENFR);
				eatToken(ASSIGN);
				tie(check, constVal) = integer();
				if (check == false) {
					error(Error::ERROR_TYPE_TO_CONST);
				}
				checkPush(symbolTable.push(Symbol::SymbolItem(
					constName.val, Symbol::CONST, Symbol::INT, constVal.val)));
			}
			break;
		case CHARTK:
			eatToken(CHARTK);
			constName = eatToken(IDENFR);
			eatToken(ASSIGN);
			if(lookToken()!=CHARCON) {
				error(Error::ERROR_TYPE_TO_CONST);
			}
			constVal = eatToken(CHARCON);
			checkPush(symbolTable.push(Symbol::SymbolItem(
				constName.val, Symbol::CONST, Symbol::CHAR, constVal.val)));
			while (lookToken() == COMMA) {
				//{ ,＜标识符＞＝＜字符＞ }
				eatToken(COMMA);
				constName = eatToken(IDENFR);
				eatToken(ASSIGN);
				constVal = eatToken(CHARCON);
				checkPush(symbolTable.push(Symbol::SymbolItem(
					constName.val, Symbol::CONST, Symbol::CHAR, constVal.val)));
			}
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<常量定义>" << endl;
	}

	//＜整数＞ ::= [＋|－]＜无符号整数＞
	inline tuple<bool, token> Parser2::integer() {
		token intTok;
		bool is_sucess = true;
		switch (lookToken()) {
		case INTCON:
			intTok = unsignedInt();
			break;
		case PLUS:
			eatToken(PLUS);
			intTok = unsignedInt();
			break;
		case MINU:
			eatToken(MINU);
			intTok = unsignedInt();
			intTok.val = "-" + intTok.val;
			break;
		default:
			is_sucess = false;
			error(Error::OTHERS);
			break;
		}
		fout << "<整数>" << endl;
		return make_tuple(is_sucess, intTok);
	}

	//＜无符号整数＞  ::= ＜非零数字＞｛＜数字＞｝| 0 := INTCON
	inline token Parser2::unsignedInt() {
		token intTok;
		switch (lookToken()) {
		case INTCON:
			if (tok.val[0] == '0' && tok.val != "0") {
				error(Error::LEX_ERROR);
			}
			intTok = eatToken(INTCON);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<无符号整数>" << endl;
		return intTok;
	}

	//＜变量说明＞  :: = ＜变量定义＞; {＜变量定义＞;}
	//一到多个{＜变量定义＞;}
	inline void Parser2::varDec() {
		bool hasVarDef = false;
		while (true) {
			switch (lookToken()) {
			case IDENFR:
				goto VARDEFEND;
			case INTTK:
			case CHARTK:
				switch (lookToken(2)) {
				case SEMICN:
				case COMMA:
				case LBRACK:
					if (!hasVarDef) {
						hasVarDef = true;
					}
					varDef();
					eatToken(SEMICN);
					break;
				case LPARENT:
					goto VARDEFEND;
				default:
					error(Error::OTHERS);
					break;
				}
				break;
			case VOIDTK:
			case IFTK:
			case DOTK:
			case WHILETK:
			case FORTK:
			case SCANFTK:
			case PRINTFTK:
			case RETURNTK:
			case SEMICN:
			case LBRACE:
			case RBRACE:
				goto VARDEFEND;;
			default:
				error(Error::OTHERS);
				break;
			}
		}
	VARDEFEND:
		if (hasVarDef) {
			fout << "<变量说明>" << endl;
		}

	}

	//＜变量定义＞  :: = ＜类型标识符＞ <变量名> { , <变量名> }
	inline void Parser2::varDef() {
		Symbol::SymbolType stype;
		token ident;
		token len;
		switch (lookToken()) {
		case INTTK:
		case CHARTK:
			stype = tokenType2SymbolType(tokType);
			eatToken(tokType); //eat ＜类型标识符＞
			// varDefName(); //<变量名>
			switch (lookToken()) {
			case IDENFR:
				ident = eatToken(IDENFR);
				switch (lookToken()) {
				case SEMICN:
				case COMMA:
					checkPush(symbolTable.push(Symbol::SymbolItem(ident.val, Symbol::ARRAY, stype)));
					break;
				case LBRACK:
					eatToken(LBRACK);
					len = unsignedInt();
					if (len.val == "0") {
						error(Error::LEX_ERROR);
					}
					eatToken(RBRACK);
					checkPush(symbolTable.push(Symbol::SymbolItem(
						ident.val, Symbol::ARRAY, stype, atoi(len.val.c_str()))));
					break;
				default:
					error(Error::OTHERS);
					break;
				}
				// if (lookToken() == LBRACK) {
				// 	//'['＜无符号整数＞']'
				// 	eatToken(LBRACK);
				// 	len = unsignedInt();
				// 	eatToken(RBRACK);
				// }
				break;
			default:
				error(Error::OTHERS);
				break;
			}
			while (lookToken() == COMMA) {
				//{ , <变量名> }
				eatToken(COMMA);
				// varDefName();
				switch (lookToken()) {
				case IDENFR:
					ident = eatToken(IDENFR);
					switch (lookToken()) {
					case SEMICN:
					case COMMA:
						checkPush(symbolTable.push(Symbol::SymbolItem(ident.val, Symbol::ARRAY, stype)));
						break;
					case LBRACK:
						eatToken(LBRACK);
						len = unsignedInt();
						error(Error::OTHERS);
						eatToken(RBRACK);
						checkPush(symbolTable.push(Symbol::SymbolItem(
							ident.val, Symbol::ARRAY, stype, atoi(len.val.c_str()))));
						break;
					default:
						error(Error::OTHERS);
						break;
					}
					// if (lookToken() == LBRACK) {
					// 	//'['＜无符号整数＞']'
					// 	eatToken(LBRACK);
					// 	len = unsignedInt();
					// 	eatToken(RBRACK);
					// }
					break;
				default:
					error(Error::OTHERS);
					break;
				}
			}
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<变量定义>" << endl;
	}

	//<变量名> ::= (＜标识符＞ | ＜标识符＞'['＜无符号整数＞']')
	//TODO 未用到
	inline void Parser2::varDefName() {
		switch (lookToken()) {
		case IDENFR:
			eatToken(IDENFR);
			if (lookToken() == LBRACK) {
				//'['＜无符号整数＞']'
				eatToken(LBRACK);
				unsignedInt();
				eatToken(RBRACK);
			}
			break;
		default:
			error(Error::OTHERS);
			break;
		}
	}

	Symbol::SymbolType funcDefType;
	int funcRetCnt;
	
	//<函数定义> ::= ＜有返回值函数定义＞|＜无返回值函数定义＞
	inline void Parser2::funcDef() {
		while (true) {
			funcRetCnt = 0;
			switch (lookToken()) {
			case INTTK:
			case CHARTK:
				funcDefType = tokenType2SymbolType(tokType);
				retfuncDef();
				if(funcRetCnt == 0) {
					error(Error::ERROR_RETURN_IN_RETFUNC);
				}
				break;
			case VOIDTK:
				switch (lookToken(1)) {
				case IDENFR:
					funcDefType = Symbol::VOID;
					nonRetfuncDef();
					break;
				case MAINTK:
					return; //跳出循环
				default:
					error(Error::OTHERS);
					break;
				}
				break;
			default:
				error(Error::OTHERS);
				break;
			}
		}
	}

	//＜有返回值函数定义＞  :: = ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'
	inline void Parser2::retfuncDef() {
		Symbol::SymbolType stype;
		token ident;
		vector<Symbol::SymbolItem> para_list;
		switch (lookToken()) {
		case INTTK:
		case CHARTK:
			stype = tokenType2SymbolType(tokType);
			// ＜声明头部＞
			ident = headDec();
			// '('＜参数表＞')'
			eatToken(LPARENT);
			para_list = paraList();
			eatToken(RPARENT);
			checkPush(symbolTable.push(Symbol::SymbolItem(ident.val, Symbol::FUNC, stype,
				SymbolItemList2SymbolTypeList(para_list))));
			symbolTable.pushScope();
			for (int i = 0; i < para_list.size(); i++) {
				checkPush(symbolTable.push(para_list[i]));
			}
			// '{'＜复合语句＞'}'
			eatToken(LBRACE);
			compStatement();
			eatToken(RBRACE);
			symbolTable.popScope();
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<有返回值函数定义>" << endl;
	}

	//＜无返回值函数定义＞  :: = void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}'
	inline void Parser2::nonRetfuncDef() {
		token ident;
		vector<Symbol::SymbolItem> para_list;
		switch (lookToken()) {
		case VOIDTK:
			eatToken(VOIDTK);
			// funcRetMap.insert(pair<string, bool>(tok.val, false));
			addFuncName(false);
			ident = eatToken(IDENFR);
			//'('＜参数表＞')'
			eatToken(LPARENT);
			para_list = paraList();
			eatToken(RPARENT);
			checkPush(symbolTable.push(Symbol::SymbolItem(ident.val, Symbol::FUNC, Symbol::VOID, SymbolItemList2SymbolTypeList(para_list))));
			symbolTable.pushScope();
			for (int i = 0; i < para_list.size(); i++) {
				checkPush(symbolTable.push(para_list[i]));
			}
			//'{'＜复合语句＞'}'
			eatToken(LBRACE);
			compStatement();
			eatToken(RBRACE);
			symbolTable.popScope();
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<无返回值函数定义>" << endl;
	}

	// ＜声明头部＞ ::=  int＜标识符＞ |char＜标识符＞
	// 返回<标识符>token,用于symbol ident
	inline token Parser2::headDec() {
		token t;
		switch (lookToken()) {
		case INTTK:
			eatToken(INTTK);
			// funcRetMap.insert(pair<string, bool>(tok.val, true));
			addFuncName(true);
			t = eatToken(IDENFR);
			break;
		case CHARTK:
			eatToken(CHARTK);
			// funcRetMap.insert(pair<string, bool>(tok.val, true));
			addFuncName(true);
			t = eatToken(IDENFR);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<声明头部>" << endl;
		return t;
	}

	//＜参数表＞ ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞
	inline vector<Symbol::SymbolItem> Parser2::paraList() {
		vector<Symbol::SymbolItem> para_list;
		Symbol::SymbolType stype;
		token ident;
		switch (lookToken()) {
		case INTTK:
		case CHARTK:
			stype = tokenType2SymbolType(tokType);
			identType();
			ident = eatToken(IDENFR);
			para_list.push_back(Symbol::SymbolItem(ident.val, Symbol::PARA, stype));
			break;
		case RPARENT:
			break; //为空由此跳出，去输出
		default:
			error(Error::OTHERS);
			break;
		}
		while (lookToken() == COMMA) {
			// {, ＜类型标识符＞＜标识符＞}
			eatToken(COMMA);
			switch (lookToken()) {
			case INTTK:
			case CHARTK:
				stype = tokenType2SymbolType(tokType);
				identType();
				ident = eatToken(IDENFR);
				break;
			case RPARENT:
				break;
			default:
				error(Error::OTHERS);
				break;
			}
			para_list.push_back(Symbol::SymbolItem(ident.val, Symbol::PARA, stype));
		}
		fout << "<参数表>" << endl;
		return para_list;
	}

	//＜类型标识符＞ ::=  int | char
	inline void Parser2::identType() {
		switch (lookToken()) {
		case INTTK:
			eatToken(INTTK);
			break;
		case CHARTK:
			eatToken(CHARTK);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
	}

	//＜复合语句＞ ::= ［＜常量说明＞］［＜变量说明＞］＜语句列＞
	inline void Parser2::compStatement() {
		constDec();
		varDec();
		statementBlock();
		fout << "<复合语句>" << endl;
	}

	//＜语句列＞ ::= ｛＜语句＞｝
	inline void Parser2::statementBlock() {
		while (true) {
			switch (lookToken()) {
			case IDENFR:
			case IFTK:
			case DOTK:
			case WHILETK:
			case FORTK:
			case SCANFTK:
			case PRINTFTK:
			case RETURNTK:
			case SEMICN:
			case LBRACE:
				statement();
				break;
			case RBRACE: // 从 '}' 跳出
				goto EndStatBlock;
			default:
				error(Error::OTHERS);
				break;
			}
		}
	EndStatBlock:
		fout << "<语句列>" << endl;
	}

	// ＜语句＞ :: = ＜条件语句＞｜＜循环语句＞ | '{'＜语句列＞'}' | ＜有返回值函数调用语句 ＞;
	// | ＜无返回值函数调用语句＞;｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜空＞; | ＜返回语句＞;
	inline void Parser2::statement() {
		switch (lookToken()) {
		case IDENFR:
			switch (lookToken(1)) {
			case LPARENT: //函数调用语句
				funcCallStatement();
				eatToken(SEMICN);
				break;
			case ASSIGN: //赋值语句
			case LBRACK: //数组a[i]的赋值语句
				assignStat();
				eatToken(SEMICN);
				break;
			default:
				error(Error::OTHERS);
				break;
			}
			break;
		case IFTK: //条件语句
			condStatement();
			break;
		case DOTK: //循环语句do...while
			cycleStatement();
			break;
		case WHILETK: //循环语句while
			cycleStatement();
			break;
		case FORTK: //循环语句for
			cycleStatement();
			break;
		case SCANFTK: //读语句
			readStatement();
			eatToken(SEMICN);
			break;
		case PRINTFTK: //写语句
			writeStatement();
			eatToken(SEMICN);
			break;
		case RETURNTK: //返回语句
			retStatement();
			eatToken(SEMICN);
			break;
		case SEMICN: //空语句
			eatToken(SEMICN);
			break;
		case LBRACE: //语句块
			eatToken(LBRACE);
			statementBlock();
			eatToken(RBRACE);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<语句>" << endl;
	}

	//＜有/无返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'
	inline void Parser2::funcCallStatement() {
		bool ret = true;
		token ident;
		Symbol::SymbolItem symbol;
		bool find;
		vector<Symbol::SymbolType> inputParaTypeList, exceptedParaTypeList;
		switch (lookToken()) {
		case IDENFR:
			ret = funcRetMap[tok.val];
			ident = eatToken(IDENFR);
			tie(find, symbol) = symbolTable.findSymbol(ident.val);
			checkFind(find);
			eatToken(LPARENT);
			exceptedParaTypeList = symbol.getParaTypeList();
			inputParaTypeList = funcCallInputParaList(exceptedParaTypeList);
			if(inputParaTypeList.size()!= exceptedParaTypeList.size()) {
				error(Error::MISMATCHING_OF_PARANUM);
			}
			eatToken(RPARENT);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		//区分有返回值函数调用语句和无返回值函数调用语句
		if (ret) {
			fout << "<有返回值函数调用语句>" << endl;
		} else {
			fout << "<无返回值函数调用语句>" << endl;
		}
	}

	// ＜值参数表＞ ::= ＜表达式＞{,＜表达式＞}｜＜空＞
	inline vector<Symbol::SymbolType> Parser2::funcCallInputParaList(vector<Symbol::SymbolType> expectedParaTypeList) {
		Symbol::SymbolType exprType;
		vector<Symbol::SymbolType> inputParaTypeList;
		int count = 0;
		switch (lookToken()) {
		case IDENFR:
		case INTCON:
		case CHARCON:
		case PLUS:
		case MINU:
		case LPARENT:
			exprType = expr();
			if(expectedParaTypeList.size() > count && expectedParaTypeList[count++] != exprType) {
				error(Error::MISMATCHING_OF_PARATYPE);
			}
			inputParaTypeList.push_back(exprType);
			while (lookToken() == COMMA) {
				//{,＜表达式＞}
				eatToken(COMMA);
				exprType = expr();
				if (expectedParaTypeList.size() > count && expectedParaTypeList[count++] != exprType) {
					error(Error::MISMATCHING_OF_PARATYPE);
				}
				inputParaTypeList.push_back(exprType);
			}
			break;
		case RPARENT:
			break; //()值参数表为空，跳出去输出
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<值参数表>" << endl;
		return inputParaTypeList;
	}

	//＜赋值语句＞ ::= ＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞
	inline void Parser2::assignStat() {
		token ident;
		Symbol::SymbolItem symbol;
		Symbol::SymbolType exprType;
		bool find;
		switch (lookToken()) {
		case IDENFR:
			ident = eatToken(IDENFR);
			tie(find, symbol) = symbolTable.findSymbol(ident.val);
			checkFind(find); //检查标识符是否定义
			if (symbol.isConst()) {
				//改变常量的值
				error(Error::CHANGE_VAL_OF_CONST);
			}
			if (lookToken() == LBRACK) {
				//'['＜表达式＞']'
				eatToken(LBRACK);
				exprType = expr();
				if (exprType != Symbol::INT) {
					//数组元素的下标只能是整型表达式
					error(Error::ERROR_TYPE_OF_ARRAY_INDEX);
				}
				eatToken(RBRACK);
			}
			eatToken(ASSIGN);
			expr();
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<赋值语句>" << endl;
	}

	//＜表达式＞ ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞} 
	inline Symbol::SymbolType Parser2::expr() {
		//TODO 改进
		Symbol::SymbolType exprType = Symbol::SymbolType::INT;
		token ident;
		Symbol::SymbolItem symbol;
		bool find;
		bool first = true;
		bool noProOp = true;
		while (true) {
			switch (lookToken()) {
			case PLUS:
			case MINU:
				if (first) {
					eatToken(tokType);
					noProOp = false;
				}
			case IDENFR:
			case INTCON:
			case CHARCON:
			case LPARENT:
				if (first && noProOp) {
					if (lookToken() == IDENFR) {
						tie(find, symbol) = symbolTable.findSymbol(tok.val);
						checkFind(find);
						if (symbol.isChar()) {
							exprType = Symbol::CHAR;
						}
					} else if (lookToken() == CHARCON) {
						exprType = Symbol::CHAR;
					}
				}
				if (!first) {
					exprType = Symbol::INT;
					plusOp();
				}
				term();
				if (first) {
					first = false;
				}
				break;
				//FOLLOW集合
			case LSS:
			case LEQ:
			case GEQ:
			case GRE:
			case EQL:
			case NEQ:
			case SEMICN:
			case COMMA:
			case RPARENT:
			case RBRACK:
				if (!first) {
					goto EndExpr;
				}
			default:
				error(Error::OTHERS);
				goto EndExpr;			//TODO try change
				break;
			}
		}
	EndExpr:
		fout << "<表达式>" << endl;
		return exprType;
	}

	//＜项＞ :: = ＜因子＞{ ＜乘法运算符＞＜因子＞ }
	inline void Parser2::term() {
		switch (lookToken()) {
		case IDENFR:
		case INTCON:
		case CHARCON:
		case PLUS:
		case MINU:
		case LPARENT:
			factor();
			while (isMulOp(lookToken())) {
				//{ ＜乘法运算符＞＜因子＞ }
				mulOp();
				factor();
			}
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<项>" << endl;
	}

	//＜因子＞ ::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|'('＜表达式＞')'
	//				｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞
	inline void Parser2::factor() {
		Symbol::SymbolType exprType;
		switch (lookToken()) {
		case IDENFR:
			switch (lookToken(1)) {
			case LPARENT: //＜有返回值函数调用语句＞
				funcCallStatement();
				break;
			case LBRACK:
			case LEQ:
			case NEQ:
			case RPARENT:
			case EQL:
			case LSS:
			case MINU:
			case GEQ:
			case COMMA:
			case GRE:
			case MULT:
			case DIV:
			case PLUS:
			case RBRACK:
			case SEMICN:
			doident:
				eatToken(IDENFR); //＜标识符＞｜＜标识符＞'['＜表达式＞']'
				if (lookToken() == LBRACK) {
					eatToken(LBRACK);
					exprType = expr();
					if(exprType!=Symbol::INT) {
						//数组元素的下标只能是整型表达式
						error(Error::ERROR_TYPE_OF_ARRAY_INDEX);
					}
					eatToken(RBRACK);
				}
				break;
			default:
				error(Error::OTHERS);
				goto doident;	//TODO try change
				break;
			}
			break;
		case CHARCON: //＜字符＞
			eatToken(CHARCON);
			break;
		case INTCON:
		case PLUS:
		case MINU: //<整数>
			integer();
			break;
		case LPARENT: //'('＜表达式＞')'
			eatToken(LPARENT);
			expr();
			eatToken(RPARENT);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<因子>" << endl;
	}

	//＜条件语句＞ ::= if '('＜条件＞')'＜语句＞［else＜语句＞］
	inline void Parser2::condStatement() {
		switch (lookToken()) {
		case IFTK:
			eatToken(IFTK);
			eatToken(LPARENT);
			condition(); //＜条件＞
			eatToken(RPARENT);
			statement(); //语句
			elseCondStat(); //else语句,一定进入else，但else可能是空，即不存在else
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<条件语句>" << endl;
	}

	//<else语句> ::= else＜语句＞
	inline void Parser2::elseCondStat() {
		switch (lookToken()) {
		case IDENFR:
		case IFTK:
			return;
		case ELSETK:
			eatToken(ELSETK);
			statement();
			break;
		case DOTK:
		case WHILETK:
		case FORTK:
		case SCANFTK:
		case PRINTFTK:
		case RETURNTK:
		case SEMICN:
		case LBRACE:
		case RBRACE:
			return;
		default:
			error(Error::OTHERS);
			break;
		}
	}

	//＜条件＞ ::=  ＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞
	inline void Parser2::condition() {
		Symbol::SymbolType lexprType;
		Symbol::SymbolType rexprType;
		switch (lookToken()) {
		case IDENFR:
		case INTCON:
		case CHARCON:
		case PLUS:
		case MINU:
		case LPARENT:
			lexprType = expr();
			if(lexprType != Symbol::INT) {
				error(Error::ILLEGAL_TYPE_IN_CONDITION);
			}
			// 判断有无 逻辑运算符 
			switch (lookToken()) {
				//＜关系运算符＞＜表达式＞
			case LSS:
			case LEQ:
			case GEQ:
			case GRE:
			case EQL:
			case NEQ:
				relationalOp();
				rexprType = expr();
				if (rexprType != Symbol::INT) {
					error(Error::ILLEGAL_TYPE_IN_CONDITION);
				}
				break;
			case SEMICN:
			case RPARENT:
				goto EndCondition;
			default:
				error(Error::OTHERS);
				break;
			}
			break;
		default:
			error(Error::OTHERS);
			break;
		}
	EndCondition:
		fout << "<条件>" << endl;
	}

	// ＜循环语句＞   :: = while '('＜条件＞')'＜语句＞ | do＜语句＞while '('＜条件＞')'
	// | for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+| -)＜步长＞')'＜语句＞
	inline void Parser2::cycleStatement() {
		switch (lookToken()) {
		case DOTK: //do＜语句＞while '('＜条件＞')'
			eatToken(DOTK);
			statement();
			if(lookToken()!=WHILETK) {
				error(Error::MISS_WHILE_IN_DO_WHILE_STAT);
			}
			eatToken(WHILETK);
			eatToken(LPARENT);
			condition();
			eatToken(RPARENT);
			break;
		case WHILETK: //while '('＜条件＞')'＜语句＞
			eatToken(WHILETK);
			eatToken(LPARENT);
			condition();
			eatToken(RPARENT);
			statement();
			break;
		case FORTK: //for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+| -)＜步长＞')'＜语句＞
			eatToken(FORTK);
			eatToken(LPARENT);
			eatToken(IDENFR);
			eatToken(ASSIGN);
			expr();
			eatToken(SEMICN);
			condition();
			eatToken(SEMICN);
			eatToken(IDENFR);
			eatToken(ASSIGN);
			eatToken(IDENFR);
			plusOp();
			step();
			eatToken(RPARENT);
			statement();
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<循环语句>" << endl;
	}

	//＜步长＞::= ＜无符号整数＞ 
	inline void Parser2::step() {
		switch (lookToken()) {
		case INTCON:
			unsignedInt();
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<步长>" << endl;
	}

	//＜读语句＞    :: = scanf '('＜标识符＞{ ,＜标识符＞ }')'
	inline void Parser2::readStatement() {
		switch (lookToken()) {
		case SCANFTK:
			eatToken(SCANFTK);
			eatToken(LPARENT);
			eatToken(IDENFR);
			while (lookToken() == COMMA) {
				//{ ,＜标识符＞ }
				eatToken(COMMA);
				eatToken(IDENFR);
			}
			eatToken(RPARENT);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<读语句>" << endl;
	}

	//＜写语句＞    ::= printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')'
	inline void Parser2::writeStatement() {
		switch (lookToken()) {
		case PRINTFTK:
			eatToken(PRINTFTK);
			eatToken(LPARENT);
			writeStatContent();
			eatToken(RPARENT);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<写语句>" << endl;
	}

	//<写语句内容> ::= ＜字符串＞,＜表达式＞  | ＜字符串＞ | ＜表达式＞
	inline void Parser2::writeStatContent() {
		switch (lookToken()) {
		case STRCON:
			strConst(); //＜字符串＞
			if (lookToken() == COMMA) {
				//,＜表达式＞
				eatToken(COMMA);
				expr();
			}
			break;
		case IDENFR:
		case INTCON:
		case CHARCON:
		case PLUS:
		case MINU:
		case LPARENT:
			expr(); //＜表达式＞
			break;
		default:
			error(Error::OTHERS);
			break;
		}
	}

	//＜返回语句＞ ::=  return['('＜表达式＞')']  
	inline void Parser2::retStatement() {
		Symbol::SymbolType exprType;
		switch (lookToken()) {
		case RETURNTK:
			eatToken(RETURNTK);
			switch (lookToken()) {
			case SEMICN:
				if (funcDefType != Symbol::VOID) {
					error(Error::ERROR_RETURN_IN_RETFUNC);
				}
				break;
			case LPARENT:
				//'('＜表达式＞')'] 
				eatToken(LPARENT);
				exprType = expr();
				if(funcDefType== Symbol::INT  && exprType!= Symbol::INT) {
					error(Error::ERROR_RETURN_IN_RETFUNC);
				}
				if (funcDefType == Symbol::CHAR && exprType != Symbol::CHAR) {
					error(Error::ERROR_RETURN_IN_RETFUNC);
				}
				if (funcDefType == Symbol::VOID) {
					error(Error::ERROR_RETURN_IN_NONRETFUNC);
				}
				eatToken(RPARENT);
				funcRetCnt++;
				break;
			default:
				error(Error::OTHERS);
				break;
			}
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<返回语句>" << endl;
	}

	//＜主函数＞ ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’
	inline void Parser2::mainFunc() {
		switch (lookToken()) {
		case VOIDTK:
			funcDefType = Symbol::VOID;
			eatToken(VOIDTK);
			eatToken(MAINTK);
			eatToken(LPARENT);
			eatToken(RPARENT);
			eatToken(LBRACE);
			symbolTable.pushScope();
			compStatement();
			eatToken(RBRACE);
			symbolTable.popScope();
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<主函数>" << endl;
	}

	inline void Parser2::addFuncName(bool ret) {
		lookToken();
		funcRetMap.insert(pair<string, bool>(tok.val, ret));
	}

	//＜加法运算符＞ ::= +｜-
	inline void Parser2::plusOp() {
		switch (lookToken()) {
		case PLUS:
			eatToken(PLUS);
			break;
		case MINU:
			eatToken(MINU);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
	}

	//＜乘法运算符＞  ::= *｜/
	inline void Parser2::mulOp() {
		switch (lookToken()) {
		case MULT:
			eatToken(MULT);
			break;
		case DIV:
			eatToken(DIV);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
	}

	inline bool Parser2::isMulOp(tokenType type) {
		return type == MULT || type == DIV;
	}

	//＜关系运算符＞  ::=  <｜<=｜>｜>=｜!=｜==
	inline void Parser2::relationalOp() {
		switch (lookToken()) {
		case LSS:
			eatToken(LSS);
			break;
		case LEQ:
			eatToken(LEQ);
			break;
		case GEQ:
			eatToken(GEQ);
			break;
		case GRE:
			eatToken(GRE);
			break;
		case EQL:
			eatToken(EQL);
			break;
		case NEQ:
			eatToken(NEQ);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
	}

	//字符串常量
	inline void Parser2::strConst() {
		switch (lookToken()) {
		case STRCON:
			eatToken(STRCON);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		fout << "<字符串>" << endl;
	}

}
