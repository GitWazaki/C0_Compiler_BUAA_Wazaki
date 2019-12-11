#pragma once

#include "Lexer.hpp"
#include "Error.hpp"
#include "MidCode/MidCode.hpp"

namespace Parser2 {

	map<string, bool> funcRetMap;
	Symbol::SymbolTable symbolTable;
	int curLine, tokLine;
	int para_num;
	
	class Parser2 {
	public:
		Parser2(Lexer::Lexer& lexer, ofstream& fout, Error::Error& err, string output_setting) : lexer(lexer), fout(fout), err(err) {
			if (output_setting.find('s') != std::string::npos) {
				output_syntax = true;
			}
		}

		MidIR::MidCode parser();
		

	private:
		token tok;
		TokenType::tokenType tokType;
		deque<token> tokQueue;
		Lexer::Lexer& lexer;
		ofstream& fout;
		Error::Error& err;
		bool output_syntax = false;

		MidIR::MidCode midCodes;

		TokenType::tokenType lookToken(int preN = 0);
		token eatToken(TokenType::tokenType);
		void output(string);

		void program(); //程序
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
		tuple<Symbol::SymbolType,string> expr(); //表达式
		Symbol::SymbolType term(); //项
		Symbol::SymbolType factor(); //因子
		void condition(); //条件
		void condStatement(); //条件语句
		void elseCondStat(); //条件语句else部分
		void cycleStatement(); //循环语句
		token step(); //步长
		void readStatement(); //读语句
		void writeStatement(); //写语句
		void writeStatContent(); //增加函数，处理写语言括号内的内容
		void retStatement(); //返回语句
		void mainFunc(); //主函数
		
		void addFuncName(bool ret); //功能：添加函数头与有无返回值的对应
		token plusOp(); //＜加法运算符＞
		void mulOp(); //＜乘法运算符＞ 
		bool isMulOp(TokenType::tokenType token); //增加函数，判断token是否是乘法运算符
		token relationalOp(); //＜关系运算符＞
		string strConst(); //字符串常量

		//error
		void error(Error::errorType);
		Error::errorStruct tokenToError(Error::errorType) const;
		void skip(Error::errorType);
		void checkPush(bool);
		void checkFind(bool);
		
		//Symbol Table
		Symbol::SymbolType tokenType2SymbolType(TokenType::tokenType);	//int 或 char 转 Symbol type
		string symbolType2String(Symbol::SymbolType type);
		vector<Symbol::SymbolType> SymbolItemList2SymbolTypeList(vector<Symbol::SymbolItem>);	//用于函数Symbol中参数表paralist的存储

		//栈
		void pushRegToStack(string reg_name);
		void popRegFromStack(string reg_name);
		void assignToIdent(string ans_reg, string ident);
		void exprPushIdent(token token);
		void scanf2midInstr(Symbol::SymbolItem) ;
		// void pushRegPool();
		// void popRegPool();
		//数组
		string getArrAddr(string arr_name, string expr_reg);
		void pushArr(string arr_name, string sub_reg);
		void assignToArr(string arr_name,string sub_reg, string expr_ans);
		//show mid
		void midShow(string show);
		
	};

	inline MidIR::MidCode Parser2::parser() {
		program();
		err.errorPrint();
		return midCodes;
	}

	inline TokenType::tokenType Parser2::lookToken(int preN) {
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

	inline token Parser2::eatToken(TokenType::tokenType type) {
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
			case TokenType::SEMICN:
				error(Error::MISS_SEMICN);
				break;
			case TokenType::RPARENT:
				error(Error::MISS_RPARENT);
				break;
			case TokenType::RBRACK:
				error(Error::MISS_RBRACK);
				break;
			default:
				error(Error::OTHERS);
				break;
			}
		}
		output(temp.toString());
		return temp;
	}

	inline void Parser2::output(string s) {
		if (output_syntax) {
			fout << s << endl;
		}
	}

	//＜程序＞::= ［＜常量说明＞］［＜变量说明＞］{<函数定义>}＜主函数＞
	inline void Parser2::program() {
		switch (lookToken()) {
		case TokenType::CONSTTK:
		case TokenType::INTTK:
		case TokenType::CHARTK:
		case TokenType::VOIDTK:
			constDec();
			varDec();
			funcDef();
			mainFunc();
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<程序>");
	}

	//＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}	
	inline void Parser2::constDec() {
		bool hasConstDec = false;
		while (lookToken() == TokenType::CONSTTK) {
			eatToken(TokenType::CONSTTK);
			constDef();
			eatToken(TokenType::SEMICN);
			if (!hasConstDec) {
				hasConstDec = true;
			}
		}
		if (hasConstDec) {
			output("<常量说明>");
		}
	}

	// ＜常量定义＞ ::= int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞} | char＜标识符＞＝＜字符＞{ ,＜标识符＞＝＜字符＞ }
	inline void Parser2::constDef() {
		token constName;
		token constVal;
		bool check;
		switch (lookToken()) {
		case TokenType::INTTK:
			eatToken(TokenType::INTTK);
			constName = eatToken(TokenType::IDENFR);
			eatToken(TokenType::ASSIGN);
			tie(check,constVal) = integer();
			if(check==false) {
				error(Error::ERROR_TYPE_TO_CONST);
			}
			checkPush(symbolTable.push(Symbol::SymbolItem(
				constName.val, Symbol::CONST, Symbol::INT, constVal.val)));
			while (lookToken() == TokenType::COMMA) {
				//{,＜标识符＞＝＜整数＞}
				eatToken(TokenType::COMMA);
				constName = eatToken(TokenType::IDENFR);
				eatToken(TokenType::ASSIGN);
				tie(check, constVal) = integer();
				if (check == false) {
					error(Error::ERROR_TYPE_TO_CONST);
				}
				checkPush(symbolTable.push(Symbol::SymbolItem(
					constName.val, Symbol::CONST, Symbol::INT, constVal.val)));
			}
			break;
		case TokenType::CHARTK:
			eatToken(TokenType::CHARTK);
			constName = eatToken(TokenType::IDENFR);
			eatToken(TokenType::ASSIGN);
			if(lookToken()!= TokenType::CHARCON) {
				error(Error::ERROR_TYPE_TO_CONST);
			}
			constVal = eatToken(TokenType::CHARCON);
			checkPush(symbolTable.push(Symbol::SymbolItem(
				constName.val, Symbol::CONST, Symbol::CHAR, constVal.val)));
			while (lookToken() == TokenType::COMMA) {
				//{ ,＜标识符＞＝＜字符＞ }
				eatToken(TokenType::COMMA);
				constName = eatToken(TokenType::IDENFR);
				eatToken(TokenType::ASSIGN);
				constVal = eatToken(TokenType::CHARCON);
				checkPush(symbolTable.push(Symbol::SymbolItem(
					constName.val, Symbol::CONST, Symbol::CHAR, constVal.val)));
			}
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<常量定义>");
	}

	//＜整数＞ ::= [＋|－]＜无符号整数＞
	inline tuple<bool, token> Parser2::integer() {
		token intTok;
		bool is_sucess = true;
		switch (lookToken()) {
		case TokenType::INTCON:
			intTok = unsignedInt();
			break;
		case TokenType::PLUS:
			eatToken(TokenType::PLUS);
			intTok = unsignedInt();
			break;
		case TokenType::MINU:
			eatToken(TokenType::MINU);
			intTok = unsignedInt();
			intTok.val = "-" + intTok.val;
			break;
		default:
			is_sucess = false;
			error(Error::OTHERS);
			break;
		}
		output("<整数>");
		return make_tuple(is_sucess, intTok);
	}

	//＜无符号整数＞  ::= ＜非零数字＞｛＜数字＞｝| 0 := INTCON
	inline token Parser2::unsignedInt() {
		token intTok;
		switch (lookToken()) {
		case TokenType::INTCON:
			if (tok.val[0] == '0' && tok.val != "0") {
				error(Error::LEX_ERROR);
			}
			intTok = eatToken(TokenType::INTCON);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<无符号整数>");
		return intTok;
	}

	//＜变量说明＞  :: = ＜变量定义＞; {＜变量定义＞;}
	//一到多个{＜变量定义＞;}
	inline void Parser2::varDec() {
		bool hasVarDef = false;
		while (true) {
			switch (lookToken()) {
			case TokenType::IDENFR:
				goto VARDEFEND;
			case TokenType::INTTK:
			case TokenType::CHARTK:
				switch (lookToken(2)) {
				case TokenType::SEMICN:
				case TokenType::COMMA:
				case TokenType::LBRACK:
					if (!hasVarDef) {
						hasVarDef = true;
					}
					varDef();
					eatToken(TokenType::SEMICN);
					break;
				case TokenType::LPARENT:
					goto VARDEFEND;
				default:
					error(Error::OTHERS);
					break;
				}
				break;
			case TokenType::VOIDTK:
			case TokenType::IFTK:
			case TokenType::DOTK:
			case TokenType::WHILETK:
			case TokenType::FORTK:
			case TokenType::SCANFTK:
			case TokenType::PRINTFTK:
			case TokenType::RETURNTK:
			case TokenType::SEMICN:
			case TokenType::LBRACE:
			case TokenType::RBRACE:
				goto VARDEFEND;;
			default:
				error(Error::OTHERS);
				break;
			}
		}
	VARDEFEND:
		if (hasVarDef) {
			output("<变量说明>");
		}

	}

	//＜变量定义＞  :: = ＜类型标识符＞ <变量名> { , <变量名> }
	inline void Parser2::varDef() {
		Symbol::SymbolType stype;
		token ident;
		token lenToken;
		switch (lookToken()) {
		case TokenType::INTTK:
		case TokenType::CHARTK:
			stype = tokenType2SymbolType(tokType);
			eatToken(tokType); //eat ＜类型标识符＞
			// varDefName(); 原<变量名>函数展开了	
			switch (lookToken()) {
			case TokenType::IDENFR:
				ident = eatToken(TokenType::IDENFR);
				switch (lookToken()) {
				case TokenType::SEMICN:
				case TokenType::COMMA:
					checkPush(symbolTable.push(Symbol::SymbolItem(ident.val, Symbol::VAR, stype)));
					if (symbolTable.getScope() == -1) {
						midCodes.defineGlobalInt(ident.val);
					} else {
						midShow(FORMAT("{} {}", symbolType2String(stype), ident.val));
					}
					break;
				case TokenType::LBRACK:
					// 数组定义
					eatToken(TokenType::LBRACK);
					lenToken = unsignedInt();
					if (lenToken.val == "0") {
						error(Error::LEX_ERROR);
					}
					eatToken(TokenType::RBRACK);
					checkPush(symbolTable.push(Symbol::SymbolItem(
						ident.val, Symbol::ARRAY, stype, atoi(lenToken.val.c_str()))));
					if (symbolTable.getScope() == -1) { //全局数组定义
						midCodes.defineGlobalIntArray(ident.val, stoi(lenToken.val));
					} else {
						midShow(FORMAT("{} {}[{}]", symbolType2String(stype), ident.val, lenToken.val));
					}
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
			
			while (lookToken() == TokenType::COMMA) {
				//{ , <变量名> }
				eatToken(TokenType::COMMA);
				// varDefName();
				switch (lookToken()) {
				case TokenType::IDENFR:
					ident = eatToken(TokenType::IDENFR);
					switch (lookToken()) {
					case TokenType::SEMICN:
					case TokenType::COMMA:
						checkPush(symbolTable.push(Symbol::SymbolItem(ident.val, Symbol::VAR, stype)));
						if(symbolTable.getScope() == -1) {
							midCodes.defineGlobalInt(ident.val);
						} else {
							midShow(FORMAT("{} {}", symbolType2String(stype), ident.val));
						}
						break;
					case TokenType::LBRACK:
						eatToken(TokenType::LBRACK);
						lenToken = unsignedInt();
						error(Error::OTHERS);
						eatToken(TokenType::RBRACK);
						checkPush(symbolTable.push(Symbol::SymbolItem(
							ident.val, Symbol::ARRAY, stype, atoi(lenToken.val.c_str()))));
						if (symbolTable.getScope() == -1) { //ARRAY DEFINE
							midCodes.defineGlobalIntArray(ident.val, stoi(lenToken.val));
						} else {
							midShow(FORMAT("{} {}[{}]", symbolType2String(stype), ident.val, lenToken.val));
						}
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
			}
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		// if (symbolTable.getScope() > 0) {
		// 	midCodes.openStackVarsSpace(symbolTable.getStackScopeBytes() - 4*para_num);
		// }
		output("<变量定义>");
	}

	//<变量名> ::= (＜标识符＞ | ＜标识符＞'['＜无符号整数＞']')
	//TODO 未用到
	inline void Parser2::varDefName() {
		switch (lookToken()) {
		case TokenType::IDENFR:
			eatToken(TokenType::IDENFR);
			if (lookToken() == TokenType::LBRACK) {
				//'['＜无符号整数＞']'
				eatToken(TokenType::LBRACK);
				unsignedInt();
				eatToken(TokenType::RBRACK);
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
			case TokenType::INTTK:
			case TokenType::CHARTK:
				funcDefType = tokenType2SymbolType(tokType);
				retfuncDef();
				if(funcRetCnt == 0) {
					error(Error::ERROR_RETURN_IN_RETFUNC);
				}
				break;
			case TokenType::VOIDTK:
				switch (lookToken(1)) {
				case TokenType::IDENFR:
					funcDefType = Symbol::VOID;
					nonRetfuncDef();
					break;
				case TokenType::MAINTK:
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
			midCodes.addReturnInstr();
		}
	}

	//＜有返回值函数定义＞  :: = ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'
	inline void Parser2::retfuncDef() {
		Symbol::SymbolType stype;
		token ident;
		vector<Symbol::SymbolItem> para_list;
		switch (lookToken()) {
		case TokenType::INTTK:
		case TokenType::CHARTK:
			stype = tokenType2SymbolType(tokType);
			// ＜声明头部＞
			ident = headDec();
			midCodes.defineFunc(ident.val);
			// '('＜参数表＞')'
			eatToken(TokenType::LPARENT);
			para_list = paraList();
			para_num = para_list.size();
			eatToken(TokenType::RPARENT);
			checkPush(symbolTable.push(Symbol::SymbolItem(ident.val, Symbol::FUNC, stype,
				SymbolItemList2SymbolTypeList(para_list))));
			symbolTable.pushScope();
			for (int i = 0; i < para_list.size(); i++) {
				checkPush(symbolTable.push(para_list[i]));
			}
			// '{'＜复合语句＞'}'
			eatToken(TokenType::LBRACE);
			compStatement();
			eatToken(TokenType::RBRACE);
			
			midCodes.popStack(symbolTable.getStackScopeBytes());
			// midCodes.popStack(symbolTable.getStackScopeBytes() - para_num * 4);
			symbolTable.popScope(); 
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<有返回值函数定义>");
	}

	//＜无返回值函数定义＞  :: = void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}'
	inline void Parser2::nonRetfuncDef() {
		token ident;
		vector<Symbol::SymbolItem> para_list;
		switch (lookToken()) {
		case TokenType::VOIDTK:
			eatToken(TokenType::VOIDTK);
			// funcRetMap.insert(pair<string, bool>(tok.val, false));
			addFuncName(false);
			ident = eatToken(TokenType::IDENFR);
			midCodes.defineFunc(ident.val);
			//'('＜参数表＞')'
			eatToken(TokenType::LPARENT);
			para_list = paraList();
			para_num = para_list.size();
			eatToken(TokenType::RPARENT);
			checkPush(symbolTable.push(Symbol::SymbolItem(ident.val, Symbol::FUNC, Symbol::VOID, SymbolItemList2SymbolTypeList(para_list))));
			symbolTable.pushScope();
			for (int i = 0; i < para_list.size(); i++) {
				checkPush(symbolTable.push(para_list[i]));
			}
			//'{'＜复合语句＞'}'
			eatToken(TokenType::LBRACE);
			compStatement();
			eatToken(TokenType::RBRACE);
			
			midCodes.popStack(symbolTable.getStackScopeBytes());
			// midCodes.popStack(symbolTable.getStackScopeBytes() - para_num * 4);
			symbolTable.popScope();
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<无返回值函数定义>");
	}

	// ＜声明头部＞ ::=  int＜标识符＞ |char＜标识符＞
	// 返回<标识符>token,用于symbol ident
	inline token Parser2::headDec() {
		token t;
		switch (lookToken()) {
		case TokenType::INTTK:
			eatToken(TokenType::INTTK);
			// funcRetMap.insert(pair<string, bool>(tok.val, true));
			addFuncName(true);
			t = eatToken(TokenType::IDENFR);
			break;
		case TokenType::CHARTK:
			eatToken(TokenType::CHARTK);
			// funcRetMap.insert(pair<string, bool>(tok.val, true));
			addFuncName(true);
			t = eatToken(TokenType::IDENFR);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<声明头部>");
		return t;
	}

	//＜参数表＞ ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞
	inline vector<Symbol::SymbolItem> Parser2::paraList() {
		vector<Symbol::SymbolItem> para_list;
		Symbol::SymbolType stype;
		token ident;
		switch (lookToken()) {
		case TokenType::INTTK:
		case TokenType::CHARTK:
			stype = tokenType2SymbolType(tokType);
			identType();
			ident = eatToken(TokenType::IDENFR);
			midShow(FORMAT("para {} {}", symbolType2String(stype), ident.val));
			para_list.push_back(Symbol::SymbolItem(ident.val, Symbol::PARA, stype));
			break;
		case TokenType::RPARENT:
			break; //为空由此跳出，去输出
		default:
			error(Error::OTHERS);
			break;
		}
		while (lookToken() == TokenType::COMMA) {
			// {, ＜类型标识符＞＜标识符＞}
			eatToken(TokenType::COMMA);
			switch (lookToken()) {
			case TokenType::INTTK:
			case TokenType::CHARTK:
				stype = tokenType2SymbolType(tokType);
				identType();
				ident = eatToken(TokenType::IDENFR);
				midShow(FORMAT("para {} {}", symbolType2String(stype), ident.val));
				break;
			case TokenType::RPARENT:
				break;
			default:
				error(Error::OTHERS);
				break;
			}
			para_list.push_back(Symbol::SymbolItem(ident.val, Symbol::PARA, stype));
		}
		output("<参数表>");
		return para_list;
	}

	//＜类型标识符＞ ::=  int | char
	inline void Parser2::identType() {
		switch (lookToken()) {
		case TokenType::INTTK:
			eatToken(TokenType::INTTK);
			break;
		case TokenType::CHARTK:
			eatToken(TokenType::CHARTK);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
	}

	//＜复合语句＞ ::= ［＜常量说明＞］［＜变量说明＞］＜语句列＞
	inline void Parser2::compStatement() {
		// 定义局部常量与变量
		constDec();
		varDec();
		// if (symbolTable.getScope() == 0) {
		// 	//main 函数，且无全局定义
		// 	midCodes.openStackVarsSpace(symbolTable.getStackScopeBytes());
		// } else 
		// if (symbolTable.getScope() > 0) {
			midCodes.openStackSpace(symbolTable.getStackScopeBytes() - 4*para_num);
		// }
		pushRegToStack("$ra");	//压入返回地址
		statementBlock();
		midCodes.newBlock(midCodes.getReturnLabel());
		popRegFromStack("$ra");	// 恢复返回地址
		output("<复合语句>");
	}

	//＜语句列＞ ::= ｛＜语句＞｝
	inline void Parser2::statementBlock() {
		while (true) {
			switch (lookToken()) {
			case TokenType::IDENFR:
			case TokenType::IFTK:
			case TokenType::DOTK:
			case TokenType::WHILETK:
			case TokenType::FORTK:
			case TokenType::SCANFTK:
			case TokenType::PRINTFTK:
			case TokenType::RETURNTK:
			case TokenType::SEMICN:
			case TokenType::LBRACE:
				statement();
				break;
			case TokenType::RBRACE: // 从 '}' 跳出
				goto EndStatBlock;
			default:
				error(Error::OTHERS);
				break;
			}
		}
	EndStatBlock:
		output("<语句列>");
	}

	// ＜语句＞ :: = ＜条件语句＞｜＜循环语句＞ | '{'＜语句列＞'}' | ＜有返回值函数调用语句 ＞;
	// | ＜无返回值函数调用语句＞;｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜空＞; | ＜返回语句＞;
	inline void Parser2::statement() {
		switch (lookToken()) {
		case TokenType::IDENFR:
			switch (lookToken(1)) {
			case TokenType::LPARENT: //函数调用语句
				funcCallStatement();
				eatToken(TokenType::SEMICN);
				break;
			case TokenType::ASSIGN: //赋值语句
			case TokenType::LBRACK: //数组a[i]的赋值语句
				assignStat();
				eatToken(TokenType::SEMICN);
				break;
			default:
				error(Error::OTHERS);
				break;
			}
			break;
		case TokenType::IFTK: //条件语句
			condStatement();
			break;
		case TokenType::DOTK: //循环语句do...while
			cycleStatement();
			break;
		case TokenType::WHILETK: //循环语句while
			cycleStatement();
			break;
		case TokenType::FORTK: //循环语句for
			cycleStatement();
			break;
		case TokenType::SCANFTK: //读语句
			readStatement();
			eatToken(TokenType::SEMICN);
			break;
		case TokenType::PRINTFTK: //写语句
			writeStatement();
			eatToken(TokenType::SEMICN);
			break;
		case TokenType::RETURNTK: //返回语句
			retStatement();
			eatToken(TokenType::SEMICN);
			break;
		case TokenType::SEMICN: //空语句
			eatToken(TokenType::SEMICN);
			break;
		case TokenType::LBRACE: //语句块
			eatToken(TokenType::LBRACE);
			statementBlock();
			eatToken(TokenType::RBRACE);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<语句>");
	}

	//＜有/无返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'
	inline void Parser2::funcCallStatement() {
		bool ret = true;
		token ident;
		Symbol::SymbolItem symbol;
		bool find;
		vector<Symbol::SymbolType> inputParaTypeList, exceptedParaTypeList;

		pushRegToStack("$fp");
		midCodes.addInstr({ MidIR::MidInstr::PUSH_REGPOOL });
		
		switch (lookToken()) {
		case TokenType::IDENFR:
			ret = funcRetMap[tok.val];
			ident = eatToken(TokenType::IDENFR);
			tie(find, symbol) = symbolTable.findSymbol(ident.val);
			checkFind(find);
			eatToken(TokenType::LPARENT);
			exceptedParaTypeList = symbol.getParaTypeList();
			inputParaTypeList = funcCallInputParaList(exceptedParaTypeList);
			if(inputParaTypeList.size()!= exceptedParaTypeList.size()) {
				error(Error::MISMATCHING_OF_PARANUM);
			}
			eatToken(TokenType::RPARENT);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		
		// midCodes.addInstr({ MidIR::MidInstr::PUSH_REGPOOL ,int(inputParaTypeList.size())});
		midCodes.addInstr({ MidIR::MidInstr::ADD, "$fp", "$sp", int(inputParaTypeList.size() * 4) });
		
		midCodes.callInstr(ident.val);
		// popRegPool();
		midCodes.addInstr({ MidIR::MidInstr::POP_REGPOOL });

		// midCodes.popStack(int(inputParaTypeList.size() * 4));
		
		popRegFromStack("$fp");
		
		//区分有返回值函数调用语句和无返回值函数调用语句
		if (ret) {
			output("<有返回值函数调用语句>");
		} else {
			output("<无返回值函数调用语句>");
		}
	}

	// ＜值参数表＞ ::= ＜表达式＞{,＜表达式＞}｜＜空＞
	inline vector<Symbol::SymbolType> Parser2::funcCallInputParaList(vector<Symbol::SymbolType> expectedParaTypeList) {
		Symbol::SymbolType exprType;
		vector<Symbol::SymbolType> inputParaTypeList;
		int count = 0;
		string expr_ans_reg;
		switch (lookToken()) {
		case TokenType::IDENFR:
		case TokenType::INTCON:
		case TokenType::CHARCON:
		case TokenType::PLUS:
		case TokenType::MINU:
		case TokenType::LPARENT:
			midCodes.exprStart();
			tie(exprType, expr_ans_reg) = expr();
			pushRegToStack(expr_ans_reg);
			if(expectedParaTypeList.size() > count && expectedParaTypeList[count++] != exprType) {
				error(Error::MISMATCHING_OF_PARATYPE);
			}
			inputParaTypeList.push_back(exprType);
			while (lookToken() == TokenType::COMMA) {
				//{,＜表达式＞}
				eatToken(TokenType::COMMA);
				midCodes.exprStart();
				tie(exprType, expr_ans_reg) = expr();
				pushRegToStack(expr_ans_reg);
				if (expectedParaTypeList.size() > count && expectedParaTypeList[count++] != exprType) {
					error(Error::MISMATCHING_OF_PARATYPE);
				}
				inputParaTypeList.push_back(exprType);
			}
			break;
		case TokenType::RPARENT:
			break; //()值参数表为空，跳出去输出
		default:
			error(Error::OTHERS);
			break;
		}
		output("<值参数表>");
		return inputParaTypeList;
	}

	//＜赋值语句＞ ::= ＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞
	inline void Parser2::assignStat() {
		token ident,arr_token;
		Symbol::SymbolItem symbol;
		Symbol::SymbolType exprType;
		bool find;
		string expr_ans_reg,arr_sub_reg;
		bool isArray = false;
		string addr;
		
		switch (lookToken()) {
		case TokenType::IDENFR:
			ident = eatToken(TokenType::IDENFR);
			tie(find, symbol) = symbolTable.findSymbol(ident.val);
			checkFind(find); //检查标识符是否定义
			if (symbol.isConst()) {
				//改变常量的值
				error(Error::CHANGE_VAL_OF_CONST);
			}
			// assign to array
			if (lookToken() == TokenType::LBRACK) {
				//'['＜表达式＞']'
				eatToken(TokenType::LBRACK);
				midCodes.exprStart();
				tie(exprType, arr_sub_reg) = expr();
				if (exprType != Symbol::INT) {
					//数组元素的下标只能是整型表达式
					error(Error::ERROR_TYPE_OF_ARRAY_INDEX);
				}
				eatToken(TokenType::RBRACK);
				isArray = true;
			}
			eatToken(TokenType::ASSIGN);
			midCodes.exprStart();
			tie(std::ignore, expr_ans_reg) = expr();
			
			if(isArray) {
				// addr = getArrAddr(ident.val, arr_expr_reg);
				// midCodes.addInstr({ MidIR::MidInstr::SAVE_STACK_ARR, expr_ans_reg, addr });
				assignToArr(ident.val, arr_sub_reg, expr_ans_reg);
			} else {
				assignToIdent(expr_ans_reg, ident.val);
			}
			midCodes.showVarName(ident.val);
			
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<赋值语句>");
	}

	//＜表达式＞ ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞} 
	inline tuple<Symbol::SymbolType, string> Parser2::expr() {
		Symbol::SymbolType exprType,termType;
		token ident;
		Symbol::SymbolItem symbol;
		bool find;
		bool first = true;
		bool noProOp = true;
		while (true) {
			switch (lookToken()) {
			case TokenType::PLUS:
			case TokenType::MINU:
				if (first) {
					if(lookToken()== TokenType::MINU) {
						midCodes.pushExprOp(MidIR::ExprOp::NEG);
					}
					eatToken(tokType);
					noProOp = false;
				}
			case TokenType::IDENFR:
			case TokenType::INTCON:
			case TokenType::CHARCON:
			case TokenType::LPARENT:
				// if (first && noProOp) {
				// 	if (lookToken() == TokenType::IDENFR) {
				// 		tie(find, symbol) = symbolTable.findSymbol(tok.val);
				// 		checkFind(find);
				// 		if (symbol.isChar()) {
				// 			exprType = Symbol::CHAR;
				// 		}
				// 	} else if (lookToken() == TokenType::CHARCON) {
				// 		exprType = Symbol::CHAR;
				// 	}
				// }
				if (!first) {
					exprType = Symbol::INT;
					token opToken = plusOp();
					if(opToken.type == TokenType::PLUS) {
						midCodes.pushExprOp(MidIR::ExprOp::PLUS);
					} else if (opToken.type == TokenType::MINU) {
						midCodes.pushExprOp(MidIR::ExprOp::MINUS);
					}
				}
				termType = term();
				if (first) {
					if(noProOp) {
						exprType = termType;
					} else {
						exprType = Symbol::INT;
					}
					first = false;
				}
				break;
				//FOLLOW集合
			case TokenType::LSS:
			case TokenType::LEQ:
			case TokenType::GEQ:
			case TokenType::GRE:
			case TokenType::EQL:
			case TokenType::NEQ:
			case TokenType::SEMICN:
			case TokenType::COMMA:
			case TokenType::RPARENT:
			case TokenType::RBRACK:
				if (!first) {
					goto EndExpr;
				}
			default:
				error(Error::OTHERS);
				goto EndExpr;			//TODO handle error, try change
				break;
			}
		}
	EndExpr:
		output("<表达式>");
		return make_tuple(exprType, midCodes.genExprVal());
	}

	//＜项＞ :: = ＜因子＞{ ＜乘法运算符＞＜因子＞ }
	inline Symbol::SymbolType Parser2::term() {
		Symbol::SymbolType term_type;
		switch (lookToken()) {
		case TokenType::IDENFR:
		case TokenType::INTCON:
		case TokenType::CHARCON:
		case TokenType::PLUS:
		case TokenType::MINU:
		case TokenType::LPARENT:
			term_type = factor();
			while (isMulOp(lookToken())) {
				//{ ＜乘法运算符＞＜因子＞ }
				mulOp();
				factor();
				term_type = Symbol::INT;
			}
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<项>");
		return term_type;
	}

	//＜因子＞ ::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|'('＜表达式＞')'
	//				｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞
	inline Symbol::SymbolType Parser2::factor() {
		Symbol::SymbolType expr_type;
		Symbol::SymbolType factor_type = Symbol::INT;
		Symbol::SymbolItem symbol;
		string expr_reg;
		token ident, intToken, char_token;
		string addr;
		bool find;
		switch (lookToken()) {
		case TokenType::IDENFR:
			tie(find, symbol) = symbolTable.findSymbol(tok.val);
			checkFind(find);
			if (symbol.isChar()) {
				factor_type = Symbol::CHAR;
			}
			switch (lookToken(1)) {
			case TokenType::LPARENT: //＜有返回值函数调用语句＞
				funcCallStatement();
				midCodes.exprPushReg("$v0");
				break;
			case TokenType::LBRACK:
			case TokenType::LEQ:
			case TokenType::NEQ:
			case TokenType::RPARENT:
			case TokenType::EQL:
			case TokenType::LSS:
			case TokenType::MINU:
			case TokenType::GEQ:
			case TokenType::COMMA:
			case TokenType::GRE:
			case TokenType::MULT:
			case TokenType::DIV:
			case TokenType::PLUS:
			case TokenType::RBRACK:
			case TokenType::SEMICN:
			doident:
				ident = eatToken(TokenType::IDENFR); //＜标识符＞｜＜标识符＞'['＜表达式＞']'
				if (lookToken() == TokenType::LBRACK) {
					eatToken(TokenType::LBRACK);
					midCodes.exprStart();
					tie(expr_type, expr_reg) = expr();
					if(expr_type!=Symbol::INT) {
						//数组元素的下标只能是整型表达式
						error(Error::ERROR_TYPE_OF_ARRAY_INDEX);
					}
					eatToken(TokenType::RBRACK);
					
					// addr = getArrAddr(ident.val, expr_val);
					// midCodes.exprPushObj_Stack_Arr(addr);
					pushArr(ident.val, expr_reg);
					midCodes.showVarName(ident.val);
					
					goto facotr_end;
				}
				exprPushIdent(ident);	//非数组 执行push ident
				break;
			default:
				error(Error::OTHERS);
				goto doident;	// TODO handle error, try change
				break;
			}
			break;
		case TokenType::CHARCON: //＜字符＞
			char_token = eatToken(TokenType::CHARCON);
			midCodes.exprPushObj_ImmInt(int(char_token.val[0]));
			factor_type = Symbol::CHAR;
			break;
		case TokenType::INTCON:
		case TokenType::PLUS:
		case TokenType::MINU: //<整数>
			tie(std::ignore, intToken) = integer();
			midCodes.exprPushObj_ImmInt(stoi(intToken.val));
			break;
		case TokenType::LPARENT: //'('＜表达式＞')'
			eatToken(TokenType::LPARENT);
			midCodes.pushExprOp(MidIR::ExprOp::PARE_L);
			midCodes.exprStart();
			tie(expr_type, expr_reg) = expr();
			midCodes.exprPushReg(expr_reg);
			eatToken(TokenType::RPARENT);
			midCodes.pushExprOp(MidIR::ExprOp::PARE_R);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		facotr_end:
		output("<因子>");
		return factor_type;
	}

	//＜条件语句＞ ::= if '('＜条件＞')'＜语句＞［else＜语句＞］
	inline void Parser2::condStatement() {
		switch (lookToken()) {
		case TokenType::IFTK:
			midCodes.defineIf();
			eatToken(TokenType::IFTK);
			midCodes.newBlock(midCodes.getIfName());
			eatToken(TokenType::LPARENT);
			condition(); //＜条件＞
			eatToken(TokenType::RPARENT);
			midCodes.newBlock(midCodes.getIfThenName());
			statement(); //语句
			midCodes.jumpInstr(midCodes.getIfEndName());
			midCodes.newBlock(midCodes.getIfElseName());
			elseCondStat(); //else语句,一定进入else，但else可能是空，即不存在else
			midCodes.newBlock(midCodes.getIfEndName());
			midCodes.endDefine();
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<条件语句>");
	}

	//<else语句> ::= else＜语句＞
	inline void Parser2::elseCondStat() {
		switch (lookToken()) {
		case TokenType::IDENFR:
		case TokenType::IFTK:
			return;
		case TokenType::ELSETK:
			eatToken(TokenType::ELSETK);
			statement();
			break;
		case TokenType::DOTK:
		case TokenType::WHILETK:
		case TokenType::FORTK:
		case TokenType::SCANFTK:
		case TokenType::PRINTFTK:
		case TokenType::RETURNTK:
		case TokenType::SEMICN:
		case TokenType::LBRACE:
		case TokenType::RBRACE:
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
		string lexprVal, rexprVal;
		token cmpToken;
		
		switch (lookToken()) {
		case TokenType::IDENFR:
		case TokenType::INTCON:
		case TokenType::CHARCON:
		case TokenType::PLUS:
		case TokenType::MINU:
		case TokenType::LPARENT:
			midCodes.exprStart();
			tie(lexprType, lexprVal) = expr();
			if(lexprType != Symbol::INT) {
				error(Error::ILLEGAL_TYPE_IN_CONDITION);
			}
			// 判断有无 逻辑运算符 
			switch (lookToken()) {
				//＜关系运算符＞＜表达式＞
			case TokenType::LSS:
			case TokenType::LEQ:
			case TokenType::GEQ:
			case TokenType::GRE:
			case TokenType::EQL:
			case TokenType::NEQ:
				cmpToken = relationalOp();
				midCodes.exprStart();
				tie(rexprType, rexprVal) = expr();
				if (rexprType != Symbol::INT) {
					error(Error::ILLEGAL_TYPE_IN_CONDITION);
				}
				midCodes.genBranch(cmpToken.type,lexprVal,rexprVal);
				break;
			case TokenType::SEMICN:
			case TokenType::RPARENT:
				midCodes.addInstr({ MidIR::MidInstr::BLEZ,lexprVal,midCodes.getCondJmpLoc() });
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
		output("<条件>");
	}

	// ＜循环语句＞   :: = while '('＜条件＞')'＜语句＞ | do＜语句＞while '('＜条件＞')'
	// | for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+| -)＜步长＞')'＜语句＞
	inline void Parser2::cycleStatement() {
		token for_init_token;
		string for_init_val_reg;
		token step_ident_token;
		token step_op_token;
		token step_val_token;
		
		switch (lookToken()) {
		case TokenType::DOTK: //do＜语句＞while '('＜条件＞')'
			midCodes.defineDoWhile();
			midCodes.newBlock(midCodes.getDoWhileName());
			eatToken(TokenType::DOTK);
			statement();
			if(lookToken()!= TokenType::WHILETK) {
				error(Error::MISS_WHILE_IN_DO_WHILE_STAT);
			}
			eatToken(TokenType::WHILETK);
			eatToken(TokenType::LPARENT);
			condition();
			eatToken(TokenType::RPARENT);
			midCodes.jumpInstr(midCodes.getDoWhileName());
			midCodes.newBlock(midCodes.getDoWhileEndName());
			midCodes.endDefine();
			break;
		case TokenType::WHILETK: //while '('＜条件＞')'＜语句＞
			midCodes.defineWhile();
			midCodes.newBlock(midCodes.getWhileName());
			eatToken(TokenType::WHILETK);
			eatToken(TokenType::LPARENT);
			condition();
			eatToken(TokenType::RPARENT);
			statement();
			midCodes.jumpInstr(midCodes.getWhileName());
			midCodes.newBlock(midCodes.getWhileEndName());
			midCodes.endDefine();
			break;
		case TokenType::FORTK: //for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+| -)＜步长＞')'＜语句＞
			midCodes.defineFor();
			eatToken(TokenType::FORTK);
			eatToken(TokenType::LPARENT);
			// ＜标识符＞＝＜表达式＞
			for_init_token = eatToken(TokenType::IDENFR);
			eatToken(TokenType::ASSIGN);
			midCodes.exprStart();
			tie(std::ignore, for_init_val_reg) = expr();
			assignToIdent(for_init_val_reg,for_init_token.val);
			eatToken(TokenType::SEMICN);
			// ＜条件＞
			midCodes.newBlock(midCodes.getForStartName());
			condition();
			//＜标识符＞＝＜标识符＞(+| -)＜步长＞
			eatToken(TokenType::SEMICN);
			eatToken(TokenType::IDENFR);
			eatToken(TokenType::ASSIGN);
			step_ident_token = eatToken(TokenType::IDENFR);
			step_op_token = plusOp();
			step_val_token = step();
			eatToken(TokenType::RPARENT);
			
			midCodes.newBlock(midCodes.getForBodyName());
			statement();

			midCodes.exprStart();
			exprPushIdent(step_ident_token);
			if(step_op_token.type==TokenType::PLUS) {
				midCodes.pushExprOp(MidIR::ExprOp::PLUS);
			} else {
				midCodes.pushExprOp(MidIR::ExprOp::MINUS);
			}
			midCodes.exprPushObj_ImmInt(stoi(step_val_token.val));
			assignToIdent(midCodes.genExprVal(), step_ident_token.val);

			midCodes.jumpInstr(midCodes.getForStartName());
			midCodes.newBlock(midCodes.getForEndName());
			midCodes.endDefine();
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<循环语句>");
	}

	//＜步长＞::= ＜无符号整数＞ 
	inline token Parser2::step() {
		token step;
		switch (lookToken()) {
		case TokenType::INTCON:
			step = unsignedInt();
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<步长>");
		return step;
	}

	vector<token> scanfIdentList;
	
	//＜读语句＞    :: = scanf '('＜标识符＞{ ,＜标识符＞ }')'
	inline void Parser2::readStatement() {
		token ident;
		// int scope;	//no use
		Symbol::SymbolItem symbol;
		
		switch (lookToken()) {
		case TokenType::SCANFTK:
			eatToken(TokenType::SCANFTK);
			eatToken(TokenType::LPARENT);
			scanfIdentList.clear();
			ident = eatToken(TokenType::IDENFR);
			scanfIdentList.push_back(ident);
			while (lookToken() == TokenType::COMMA) {
				//{ ,＜标识符＞ }
				eatToken(TokenType::COMMA);
				ident = eatToken(TokenType::IDENFR);
				scanfIdentList.push_back(ident);
			}
			
			for (int i = 0; i < scanfIdentList.size(); i++) { 
				tie(std::ignore, symbol) = symbolTable.findSymbolAndScope(scanfIdentList[i].val);
				scanf2midInstr(symbol);
			}
			
			eatToken(TokenType::RPARENT);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<读语句>");
	}

	//＜写语句＞    ::= printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')'
	inline void Parser2::writeStatement() {
		switch (lookToken()) {
		case TokenType::PRINTFTK:
			eatToken(TokenType::PRINTFTK);
			eatToken(TokenType::LPARENT);
			writeStatContent();
			eatToken(TokenType::RPARENT);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		midCodes.printLineInstr();
		output("<写语句>");
	}

	//<写语句内容> ::= ＜字符串＞,＜表达式＞  | ＜字符串＞ | ＜表达式＞
	inline void Parser2::writeStatContent() {
		string str;
		string str_lable;
		Symbol::SymbolType expr_type;
		string expr_ans_reg;
		//TODO print expr
		switch (lookToken()) {
		case TokenType::STRCON:
			str = strConst(); //＜字符串＞
			str_lable = midCodes.defineConstStr(str);
			midCodes.printStrInstr(str_lable);
			if (lookToken() == TokenType::COMMA) {
				//,＜表达式＞
				eatToken(TokenType::COMMA);
				midCodes.exprStart();
				tie(expr_type, expr_ans_reg) = expr(); //＜表达式＞
				midCodes.printReg(expr_type, expr_ans_reg);
			}
			break;
		case TokenType::IDENFR:
		case TokenType::INTCON:
		case TokenType::CHARCON:
		case TokenType::PLUS:
		case TokenType::MINU:
		case TokenType::LPARENT:
			midCodes.exprStart();
			tie(expr_type, expr_ans_reg) = expr(); //＜表达式＞
			midCodes.printReg(expr_type, expr_ans_reg);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
	}

	//＜返回语句＞ ::=  return['('＜表达式＞')']  
	inline void Parser2::retStatement() {
		Symbol::SymbolType expr_type;
		string expr_val_reg;
		switch (lookToken()) {
		case TokenType::RETURNTK:
			eatToken(TokenType::RETURNTK);
			switch (lookToken()) {
			case TokenType::SEMICN:
				if (funcDefType != Symbol::VOID) {
					error(Error::ERROR_RETURN_IN_RETFUNC);
				}
				midCodes.jumpInstr(midCodes.getReturnLabel());
				midCodes.newBlock(midCodes.getAfterReturnBlockName(midCodes.getReturnLabel()));
				break;
			case TokenType::LPARENT:
				//'('＜表达式＞')']
				eatToken(TokenType::LPARENT);
				midCodes.exprStart();
				tie(expr_type, expr_val_reg) = expr();
				midCodes.moveReg("$v0", expr_val_reg);
				midCodes.jumpInstr(midCodes.getReturnLabel());
				midCodes.newBlock(midCodes.getAfterReturnBlockName(midCodes.getReturnLabel()));
				if(funcDefType== Symbol::INT  && expr_type != Symbol::INT) {
					error(Error::ERROR_RETURN_IN_RETFUNC);
				}
				if (funcDefType == Symbol::CHAR && expr_type != Symbol::CHAR) {
					error(Error::ERROR_RETURN_IN_RETFUNC);
				}
				if (funcDefType == Symbol::VOID) {
					error(Error::ERROR_RETURN_IN_NONRETFUNC);
				}
				eatToken(TokenType::RPARENT);
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
		output("<返回语句>");
	}

	//＜主函数＞ ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’
	inline void Parser2::mainFunc() {
		switch (lookToken()) {
		case TokenType::VOIDTK:
			funcDefType = Symbol::VOID;
			eatToken(TokenType::VOIDTK);
			eatToken(TokenType::MAINTK);
			eatToken(TokenType::LPARENT);
			eatToken(TokenType::RPARENT);
			eatToken(TokenType::LBRACE);
			midCodes.defineFunc("main");
			midCodes.addInstr({ MidIR::MidInstr::MOVE, "$fp", "$sp" });
			symbolTable.pushScope();
			para_num = 0;
			compStatement();
			eatToken(TokenType::RBRACE);
			midCodes.popStack(symbolTable.getStackScopeBytes());
			// midCodes.popStack(symbolTable.getStackScopeBytes() - para_num * 4);
			symbolTable.popScope();
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<主函数>");
	}

	inline void Parser2::addFuncName(bool ret) {
		lookToken();
		funcRetMap.insert(pair<string, bool>(tok.val, ret));
	}

	//＜加法运算符＞ ::= +｜-
	inline token Parser2::plusOp() {
		token t;
		switch (lookToken()) {
		case TokenType::PLUS:
			t = eatToken(TokenType::PLUS);
			break;
		case TokenType::MINU:
			t = eatToken(TokenType::MINU);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		return t;
	}

	//＜乘法运算符＞  ::= *｜/
	inline void Parser2::mulOp() {
		switch (lookToken()) {
		case TokenType::MULT:
			eatToken(TokenType::MULT);
			midCodes.pushExprOp(MidIR::ExprOp::MULT);
			break;
		case TokenType::DIV:
			eatToken(TokenType::DIV);
			midCodes.pushExprOp(MidIR::ExprOp::DIV);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
	}

	inline bool Parser2::isMulOp(TokenType::tokenType type) {
		return type == TokenType::MULT || type == TokenType::DIV;
	}

	//＜关系运算符＞  ::=  <｜<=｜>｜>=｜!=｜==
	inline token Parser2::relationalOp() {
		token cmpToken;
		switch (lookToken()) {
		case TokenType::LSS:
			cmpToken = eatToken(TokenType::LSS);
			break;
		case TokenType::LEQ:
			cmpToken = eatToken(TokenType::LEQ);
			break;
		case TokenType::GEQ:
			cmpToken = eatToken(TokenType::GEQ);
			break;
		case TokenType::GRE:
			cmpToken = eatToken(TokenType::GRE);
			break;
		case TokenType::EQL:
			cmpToken = eatToken(TokenType::EQL);
			break;
		case TokenType::NEQ:
			cmpToken = eatToken(TokenType::NEQ);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		return cmpToken;
	}

	//字符串常量
	inline string Parser2::strConst() {
		token strTok;
		switch (lookToken()) {
		case TokenType::STRCON:
			strTok = eatToken(TokenType::STRCON);
			break;
		default:
			error(Error::OTHERS);
			break;
		}
		output("<字符串>");
		return strTok.val;
	}

	inline void Parser2::error(Error::errorType eType) {
		err.errorHandle(tokenToError(eType));
		skip(eType);
	}

	inline Error::errorStruct Parser2::tokenToError(Error::errorType eType) const {
		Error::errorStruct e;
		e.tok = tok;
		e.line = curLine;
		e.col = tok.col;
		e.type = eType;;
		return e;
	}

	inline void Parser2::skip(Error::errorType eType) {
		if (eType == Error::ERROR_TYPE_TO_CONST) {
			eatToken(lookToken());
		}
		//TODO skip
		// vector<tokenType> skipList = Error::ErrorSkipSet[eType];
		// tokenType temp = lookToken();
		// while (checkFind(skipList.begin(), skipList.end(), temp) == skipList.end()) {
		// 	eatToken(temp);
		// 	temp = lookToken();
		// }
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

	inline Symbol::SymbolType Parser2::tokenType2SymbolType(TokenType::tokenType tType) {
		if (tType == TokenType::INTTK) {
			return Symbol::INT;
		}
		else if (tType == TokenType::CHARTK) {
			return Symbol::CHAR;
		}
		else {
			//error
			return Symbol::VOID;
		}
	}

	inline string Parser2::symbolType2String(Symbol::SymbolType type) {
		if(type == Symbol::INT) {
			return "int";
		}
		else if (type == Symbol::CHAR) {
			return "char";
		}
		else {
			//error
			return "void,maybe error";
		}
	}

	inline vector<Symbol::SymbolType> Parser2::SymbolItemList2SymbolTypeList(vector<Symbol::SymbolItem> paraList) {
		std::vector<Symbol::SymbolType> symbolTypeList;
		for (int i = 0; i < paraList.size(); i++) {
			symbolTypeList.push_back(paraList[i].getType());
		}
		return symbolTypeList;
	}

	void Parser2::pushRegToStack(string reg_name) {
		midCodes.pushRegInstr(reg_name);
		// symbolTable.addScopeOffset(4);
	}

	void Parser2::popRegFromStack(string reg_name) {
		midCodes.popRegInstr(reg_name);
		// symbolTable.subScopeOffset(4);
	}

	void Parser2::assignToIdent(string ans_reg, string ident) {
		if (symbolTable.checkSymbolIsGlobal(ident)) {
			midCodes.addInstr(
				{ MidIR::MidInstr::SAVE_GLOBAL, ans_reg, symbolTable.getGlobalOffsetBytesByIdent(ident) });
		}
		else {
			midCodes.addInstr(
				{ MidIR::MidInstr::SAVE_STACK, ans_reg, symbolTable.getStackOffsetBytesByIdent(ident) });
		}
	}

	void Parser2::exprPushIdent(token token) {
		if(symbolTable.checkSymbolIsConst(token.val)) {
			if (symbolTable.checkSymbolIsChar(token.val)) {
				midCodes.exprPushObj_ImmInt(int(symbolTable.getConstSymbolValue(token.val)[0]));
			}
			else {
				midCodes.exprPushObj_ImmInt(stoi(symbolTable.getConstSymbolValue(token.val)));
			}
		} else if (symbolTable.checkSymbolIsGlobal(token.val)) {
			midCodes.exprPushObj_GlobalVar(token.val,symbolTable.getGlobalOffsetBytesByIdent(token.val));
			midCodes.showVarName(token.val);
		} else {
			midCodes.exprPushObj_StackVar(token.val, symbolTable.getStackOffsetBytesByIdent(token.val));
			midCodes.showVarName(token.val);
		}
	}

	void Parser2::scanf2midInstr(Symbol::SymbolItem symbol)  {
		if (symbol.isGlobal) {
			if (symbol.getType() == Symbol::SymbolType::INT) {
				midCodes.addInstr({ MidIR::MidInstr::SCAN_GLOBAL_INT,symbolTable.getGlobalOffsetBytesByIdent(symbol.getName()) });
			}
			else if (symbol.getType() == Symbol::SymbolType::CHAR) {
				midCodes.addInstr({ MidIR::MidInstr::SCAN_GLOBAL_CHAR,symbolTable.getGlobalOffsetBytesByIdent(symbol.getName()) });
			}
		} else {
			if (symbol.getType() == Symbol::SymbolType::INT) {
				midCodes.addInstr({ MidIR::MidInstr::SCAN_INT,symbolTable.getStackOffsetBytesByIdent(symbol.getName()) });
			}
			else if (symbol.getType() == Symbol::SymbolType::CHAR) {
				midCodes.addInstr({ MidIR::MidInstr::SCAN_CHAR,symbolTable.getStackOffsetBytesByIdent(symbol.getName()) });
			}
		}
		midCodes.showVarName(symbol.getName());
	}

	// inline string Parser2::getArrAddr(string arr_name, string expr_reg) {
	// 	midCodes.addInstr({ MidIR::MidInstr::MOVE, "$k1", expr_reg });
	// 	midCodes.addInstr({ MidIR::MidInstr::LI, "$k0", "4" });
	// 	midCodes.addInstr({ MidIR::MidInstr::MUL, "$k1", "$k1", "$k0" });
	// 	if (symbolTable.checkSymbolIsGlobal(arr_name)) {
	// 		// midCodes.addInstr({ MidIR::MidInstr::LA, "$k0", midCodes.getGlobalArrLabel(arr_name) });
	// 		midCodes.addInstr({MidIR::MidInstr::LI, "$k0",  symbolTable.getGlobalOffsetBytesByIdent(arr_name) });
	// 		midCodes.addInstr({MidIR::MidInstr::ADD, "$k0", "$k0", "$k1" });
	// 		midCodes.addInstr({MidIR::MidInstr::MOVE, "$k1",  "$gp" });
	// 	} else {
	// 		midCodes.addInstr({ MidIR::MidInstr::LI, "$k0", symbolTable.getStackOffsetBytesByIdent(arr_name) });
	// 		midCodes.addInstr({ MidIR::MidInstr::SUB, "$k0", "$k0", "$k1" });
	// 		midCodes.addInstr({ MidIR::MidInstr::MOVE, "$k1",  "$fp" });
	// 	}
	// 	midCodes.addInstr({ MidIR::MidInstr::ADD, "$k0", "$k0", "$k1" });
	//
	// 	return "$k0";
	// }

	void Parser2::pushArr(string arr_name, string sub_reg) {
		if (symbolTable.checkSymbolIsGlobal(arr_name)) {
			midCodes.exprPushObj_GLOBAL_Arr(symbolTable.getGlobalOffsetBytesByIdent(arr_name), sub_reg);
		}
		else {
			midCodes.exprPushObj_Stack_Arr(symbolTable.getStackOffsetBytesByIdent(arr_name), sub_reg);
		}
	}

	 void Parser2::assignToArr(string arr_name, string sub_reg, string expr_ans) {
		// arr[sub] = expr_ans
		// sw expr_ans arr_offset($gp+sub_reg>>4)
		if (symbolTable.checkSymbolIsGlobal(arr_name)) {
			midCodes.addInstr({ MidIR::MidInstr::SAVE_GLOBAL_ARR,expr_ans, symbolTable.getGlobalOffsetBytesByIdent(arr_name),sub_reg });
		}
		else {
			midCodes.addInstr({ MidIR::MidInstr::SAVE_STACK_ARR,expr_ans, symbolTable.getStackOffsetBytesByIdent(arr_name),sub_reg });
		}
	}


	inline void Parser2::midShow(string show) {
		midCodes.addInstr({ MidIR::MidInstr::MID_SHOW, show });
	}

}
