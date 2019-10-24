#pragma once
#include <iostream>
#include "Lexer.hpp"
#include "deque"
#include <map>

namespace Parser2 {

	map<string, bool> funcRetMap;

	class Parser2 {
	public:
		Parser2(Lexer::Lexer& lexer, ofstream& fout) : lexer(lexer), fout(fout) {
		}

		void program(); //程序
		void error();
	private:
		token tok;
		tokenType tokType;
		deque<token> tokQueue;
		Lexer::Lexer& lexer;
		ofstream& fout;

		tokenType lookToken(int preN = 0);
		void eatToken(tokenType);

		void plusOp();	//＜加法运算符＞
		void mulOp();	//＜乘法运算符＞ 
		bool isMulOp(tokenType token);	//增加函数，判断token是否是乘法运算符
		void relationalOp();	//＜关系运算符＞
		void strConst();	//字符串常量

		void constDec(); //常量说明
		void constDef(); //常量定义
		void integer(); //整数
		void unsignedInt(); //无符号整数
		void varDec(); //变量说明
		void varDef(); //变量定义
		void varDefName(); //增加函数，处理变量名
		void funcDef(); //函数定义
		void retfuncDef(); //有返回值函数定义
		void nonRetfuncDef(); //无返回值函数定义
		void headDec(); //声明头部
		void paraList(); //参数表
		void identType(); //类型标识符
		void compStatement(); //复合语句
		void statementBlock(); //语句块
		void statement(); //语句
		void funcCallStatement(); //函数调用语句
		void funcCallInputParaList(); //值参数表
		void assignStat(); //赋值语句
		void expr(); //表达式
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
		void addFuncName(bool ret);	//功能：添加函数头与有无返回值的对应
	};

	inline void Parser2::error() {
		fout << "error" << endl;
	}

	inline tokenType Parser2::lookToken(int preN) {
		while (tokQueue.size() <= preN) {
			tokQueue.push_back(lexer.getToken(preN));
		}
		tok = tokQueue[preN];
		tokType = tok.type;
		return tokType;
	}

	inline void Parser2::eatToken(tokenType type) {
		token temp;
		if (tokQueue.empty()) {
			tokQueue.push_back(lexer.getToken());
		}
		if ((temp = tokQueue.front()).type == type) {
			tokQueue.pop_front();
			lexer.eatToken();
			lookToken();	//可去除
		}
		fout << temp.toString() << endl;
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
			error();
		}
		fout << "<程序>" << endl;
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
		switch (lookToken()) {
		case INTTK:
			eatToken(INTTK);
			eatToken(IDENFR);
			eatToken(ASSIGN);
			integer();
			while (lookToken() == COMMA) {
				//{,＜标识符＞＝＜整数＞}
				eatToken(COMMA);
				eatToken(IDENFR);
				eatToken(ASSIGN);
				integer();
			}
			break;
		case CHARTK:
			eatToken(CHARTK);
			eatToken(IDENFR);
			eatToken(ASSIGN);
			eatToken(CHARCON);
			while (lookToken() == COMMA) {
				//{ ,＜标识符＞＝＜字符＞ }
				eatToken(COMMA);
				eatToken(IDENFR);
				eatToken(ASSIGN);
				eatToken(CHARCON);
			}
			break;
		default:
			error();
		}
		fout << "<常量定义>" << endl;
	}

	//＜整数＞ ::= [＋|－]＜无符号整数＞
	inline void Parser2::integer() {
		switch (lookToken()) {
		case INTCON:
			unsignedInt();
			break;
		case PLUS:
			eatToken(PLUS);
			unsignedInt();
			break;
		case MINU:
			eatToken(MINU);
			unsignedInt();
			break;
		default:
			error();
		}
		fout << "<整数>" << endl;
	}

	//＜无符号整数＞  ::= ＜非零数字＞｛＜数字＞｝| 0 := INTCON
	inline void Parser2::unsignedInt() {
		switch (lookToken()) {
		case INTCON:
			eatToken(INTCON);
			break;
		default:
			error();
		}
		fout << "<无符号整数>" << endl;
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
					error();
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
				error();
			}
		}
	VARDEFEND:
		if (hasVarDef) {
			fout << "<变量说明>" << endl;
		}

	}

	//＜变量定义＞  :: = ＜类型标识符＞ <变量名> { , <变量名> }
	inline void Parser2::varDef() {
		switch (lookToken()) {
		case INTTK:
		case CHARTK:
			eatToken(tokType);	//eat ＜类型标识符＞
			varDefName();	//<变量名>
			while (lookToken() == COMMA) {
				//{ , <变量名> }
				eatToken(COMMA);
				varDefName();
			}
			break;
		default:
			error();
		}
		fout << "<变量定义>" << endl;
	}

	//<变量名> ::= (＜标识符＞ | ＜标识符＞'['＜无符号整数＞']')
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
			error();
		}
	}

	//<函数定义> ::= ＜有返回值函数定义＞|＜无返回值函数定义＞
	inline void Parser2::funcDef() {
		while (true) {
			switch (lookToken()) {
			case INTTK:
			case CHARTK:
				retfuncDef();
				break;
			case VOIDTK:
				switch (lookToken(1)) {
				case IDENFR:
					nonRetfuncDef();
					break;
				case MAINTK:
					return; //跳出循环
				default:
					error();
				}
				break;
			default:
				error();
			}
		}
	}

	//＜有返回值函数定义＞  :: = ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'
	inline void Parser2::retfuncDef() {
		switch (lookToken()) {
		case INTTK:
		case CHARTK:
			// ＜声明头部＞
			headDec();
			// '('＜参数表＞')'
			eatToken(LPARENT);
			paraList();
			eatToken(RPARENT);
			// '{'＜复合语句＞'}'
			eatToken(LBRACE);
			compStatement();
			eatToken(RBRACE);
			break;
		default:
			error();
		}
		fout << "<有返回值函数定义>" << endl;
	}

	//＜无返回值函数定义＞  :: = void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}'
	inline void Parser2::nonRetfuncDef() {
		switch (lookToken()) {
		case VOIDTK:
			eatToken(VOIDTK);
			// funcRetMap.insert(pair<string, bool>(tok.val, false));
			addFuncName(false);
			eatToken(IDENFR);
			//'('＜参数表＞')'
			eatToken(LPARENT);
			paraList();
			eatToken(RPARENT);
			//'{'＜复合语句＞'}'
			eatToken(LBRACE);
			compStatement();
			eatToken(RBRACE);
			break;
		default:
			error();
		}
		fout << "<无返回值函数定义>" << endl;
	}

	//＜声明头部＞ ::=  int＜标识符＞ |char＜标识符＞
	inline void Parser2::headDec() {
		switch (lookToken()) {
		case INTTK:
			eatToken(INTTK);
			// funcRetMap.insert(pair<string, bool>(tok.val, true));
			addFuncName(true);
			eatToken(IDENFR);
			break;
		case CHARTK:
			eatToken(CHARTK);
			// funcRetMap.insert(pair<string, bool>(tok.val, true));
			addFuncName(true);
			eatToken(IDENFR);
			break;
		default:
			error();
		}
		fout << "<声明头部>" << endl;
	}

	//＜参数表＞ ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞
	inline void Parser2::paraList() {
		switch (lookToken()) {
		case INTTK:
		case CHARTK:
			identType();
			eatToken(IDENFR);
			break;
		case RPARENT:
			break; //为空由此跳出，去输出
		default:
			error();
		}
		while (lookToken() == COMMA) {
			// {, ＜类型标识符＞＜标识符＞}
			eatToken(COMMA);
			switch (lookToken()) {
			case INTTK:
			case CHARTK:
				identType();
				eatToken(IDENFR);
				break;
			case RPARENT:
				break;
			default:
				error();
			}
		}
		fout << "<参数表>" << endl;
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
			error();
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
				error();
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
				error();
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
			error();
		}
		fout << "<语句>" << endl;
	}

	//＜有/无返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'
	inline void Parser2::funcCallStatement() {
		bool ret = true;
		switch (lookToken()) {
		case IDENFR:
			ret = funcRetMap[tok.val];
			eatToken(IDENFR);
			eatToken(LPARENT);
			funcCallInputParaList();
			eatToken(RPARENT);
			break;
		default:
			error();
		}
		//区分有返回值函数调用语句和无返回值函数调用语句
		if (ret) {
			fout << "<有返回值函数调用语句>" << endl;
		} else {
			fout << "<无返回值函数调用语句>" << endl;
		}
	}

	// ＜值参数表＞ ::= ＜表达式＞{,＜表达式＞}｜＜空＞
	inline void Parser2::funcCallInputParaList() {
		switch (lookToken()) {
		case IDENFR:
		case INTCON:
		case CHARCON:
		case PLUS:
		case MINU:
		case LPARENT:
			expr();
			while (lookToken() == COMMA) {
				//{,＜表达式＞}
				eatToken(COMMA);
				expr();
			}
			break;
		case RPARENT:
			break; //()值参数表为空，跳出去输出
		default:
			error();
		}
		fout << "<值参数表>" << endl;
	}

	//＜赋值语句＞ ::= ＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞
	inline void Parser2::assignStat() {
		switch (lookToken()) {
		case IDENFR:
			eatToken(IDENFR);
			if (lookToken() == LBRACK) {
				//'['＜表达式＞']'
				eatToken(LBRACK);
				expr();
				eatToken(RBRACK);
			}
			eatToken(ASSIGN);
			expr();
			break;
		default:
			error();
		}
		fout << "<赋值语句>" << endl;
	}

	//＜表达式＞ ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞} 
	inline void Parser2::expr() {
		//TODO 改进
		bool first = true;
		while (true) {
			switch (lookToken()) {
			case PLUS:
			case MINU:
				if (first) {
					eatToken(tokType);
				}
			case IDENFR:
			case INTCON:
			case CHARCON:
			case LPARENT:
				if (!first) {
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
				error();
			}
		}
	EndExpr:
		fout << "<表达式>" << endl;
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
			error();
		}
		fout << "<项>" << endl;
	}

	//＜因子＞ ::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|'('＜表达式＞')'
	//				｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞
	inline void Parser2::factor() {
		switch (lookToken()) {
		case IDENFR:
			switch (lookToken(1)) {
			case LPARENT:	//＜有返回值函数调用语句＞
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
				eatToken(IDENFR);	//＜标识符＞｜＜标识符＞'['＜表达式＞']'
				if (lookToken() == LBRACK) {
					eatToken(LBRACK);
					expr();
					eatToken(RBRACK);
				}
				break;
			default:
				error();
			}
			break;
		case CHARCON:	//＜字符＞
			eatToken(CHARCON);
			break;
		case INTCON:
		case PLUS:
		case MINU:		//<整数>
			integer();
			break;
		case LPARENT:	//'('＜表达式＞')'
			eatToken(LPARENT);
			expr();
			eatToken(RPARENT);
			break;
		default:
			error();
		}
		fout << "<因子>" << endl;
	}

	//＜条件语句＞ ::= if '('＜条件＞')'＜语句＞［else＜语句＞］
	inline void Parser2::condStatement() {
		switch (lookToken()) {
		case IFTK:
			eatToken(IFTK);
			eatToken(LPARENT);
			condition();	//＜条件＞
			eatToken(RPARENT);
			statement();	//语句
			elseCondStat();	//else语句,一定进入else，但else可能是空，即不存在else
			break;
		default:
			error();
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
			error();
		}
	}

	//＜条件＞ ::=  ＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞
	inline void Parser2::condition() {
		switch (lookToken()) {
		case IDENFR:
		case INTCON:
		case CHARCON:
		case PLUS:
		case MINU:
		case LPARENT:
			expr();
			// 判断有无 逻辑运算符 
			switch (lookToken()) {	//＜关系运算符＞＜表达式＞
			case LSS:
			case LEQ:
			case GEQ:
			case GRE:
			case EQL:
			case NEQ:
				relationalOp();
				expr();
				break;
			case SEMICN:
			case RPARENT:
				goto EndCondition;
			default:
				error();
			}
			break;
		default:
			error();
		}
	EndCondition:
		fout << "<条件>" << endl;
	}

	// ＜循环语句＞   :: = while '('＜条件＞')'＜语句＞ | do＜语句＞while '('＜条件＞')'
	// | for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+| -)＜步长＞')'＜语句＞
	inline void Parser2::cycleStatement() {
		switch (lookToken()) {
		case DOTK:	//do＜语句＞while '('＜条件＞')'
			eatToken(DOTK);
			statement();
			eatToken(WHILETK);
			eatToken(LPARENT);
			condition();
			eatToken(RPARENT);
			break;
		case WHILETK:	//while '('＜条件＞')'＜语句＞
			eatToken(WHILETK);
			eatToken(LPARENT);
			condition();
			eatToken(RPARENT);
			statement();
			break;
		case FORTK:	//for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+| -)＜步长＞')'＜语句＞
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
			error();
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
			error();
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
			error();
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
			error();
		}
		fout << "<写语句>" << endl;
	}

	//<写语句内容> ::= ＜字符串＞,＜表达式＞  | ＜字符串＞ | ＜表达式＞
	inline void Parser2::writeStatContent() {
		switch (lookToken()) {
		case STRCON:
			strConst();	//＜字符串＞
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
			expr();	//＜表达式＞
			break;
		default:
			error();
		}
	}

	//＜返回语句＞ ::=  return['('＜表达式＞')']  
	inline void Parser2::retStatement() {
		switch (lookToken()) {
		case RETURNTK:
			eatToken(RETURNTK);
			switch (lookToken()) {	
			case SEMICN:
				break;
			case LPARENT:
				//'('＜表达式＞')'] 
				eatToken(LPARENT);
				expr();
				eatToken(RPARENT);
				break;
			default:
				error();
			}
			break;
		default:
			error();
		}
		fout << "<返回语句>" << endl;
	}

	//＜主函数＞ ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’
	inline void Parser2::mainFunc() {
		switch (lookToken()) {
		case VOIDTK:
			eatToken(VOIDTK);
			eatToken(MAINTK);
			eatToken(LPARENT);
			eatToken(RPARENT);
			eatToken(LBRACE);
			compStatement();
			eatToken(RBRACE);
			break;
		default:
			error();
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
			error();
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
			error();
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
			error();
		}
	}

	//字符串常量
	inline void Parser2::strConst() {
		switch (lookToken()) {
		case STRCON:
			eatToken(STRCON);
			break;
		default:
			error();
		}
		fout << "<字符串>" << endl;
	}

}
