#pragma once
#include <iostream>
#include "Lexer.hpp"
#include <map>

namespace Parser {

	map<string, bool> funcRetMap;

	class Parser {
	public:
		Parser(Lexer::Lexer& lexer, ofstream& fout) : lexer(lexer), fout(fout) {
		}

		void program();
		void error();
	private:
		token tok;
		tokenType tokType;
		Lexer::Lexer& lexer;
		ofstream& fout;

		int constDecFlag = 0;
		int varDecFlag = 0;
		int statBlockFlag = 0;
		void resetFlag(bool a = true, bool b = true, bool c = false);

		tokenType lookToken(int preN = 0);
		void eatToken(tokenType);

		void plusOp();
		void mulOp();
		void relationalOp();
		void unsignedInt();
		void integer();
		void strConst();

		void constDec(); //常量声明
		void constDef(); //常量定义
		void constDefInt(); //常量定义int
		void constDefIntRep(); //常量定义int组(重复)
		void constDefChar(); //常量定义char
		void constDefCharRep(); //常量定义char组
		void identType(); //类型标识符
		void varDec(); //变量声明
		void varDef(); //变量定义
		void varDefName(); //变量名定义
		void varDefArrayName(); //变量名定义带数组
		void varDefNameRep(); //变量名定义组
		void funcDefRep(); //函数定义组
		void funcDef(); //函数定义
		void headDec(); //声明头部
		void addFuncName(bool);	//功能：添加函数名与后无返回值的对应
		void retFuncDef(); //有返回值函数定义
		void nonRetFuncDef(); //无返回值函数定义
		void mainFunc(); //主函数
		void compStatement(); //复合语句
		void paraList(); //参数表
		void paraListRep(); //参数表组
		void exprOp(); //表达式前缀
		void expr(); //表达式
		void exprRep(); //表达式组
		void term(); //项
		void termRep(); //项组
		void factor(); //因子
		void factorStartWithIdent(); //因子以标识符开始
		void statement(); //语句
		void assignStat(); //赋值语句
		void assignStatAfterIdent(); //赋值语句标识符后
		void condStatement(); //条件语句
		void elseCondStat(); //条件语句else
		void condition(); //条件
		void condAfterFirstExpr(); //条件表达式后
		void cycleStatement(); //循环语句
		void step(); //步长
		void funcCallStatement(); //函数调用语句
		void funcCallInputParaList(); //值参数表
		void funcCallInputParaListRep(); //值参数表组
		void statementBlock(); //语句块
		void readStatement(); //读语句
		void readIdentRep(); //读语句标识符组
		void writeStatement(); //写语句
		void writeStatContent(); //写语句内容
		void wirteStatAfterStr(); //写语句字符串后
		void retStatement(); //返回语句
		void retStatContent(); //返回语句内容

	};

	inline void Parser::resetFlag(bool a, bool b, bool c) {
		if (a) {
			constDecFlag = 0;
		}
		if (b) {
			varDecFlag = 0;
		}
		if (c) {
			statBlockFlag = 0;
		}
	}

	inline tokenType Parser::lookToken(int preN) {
		if (preN == 0) {
			tok = lexer.getToken(preN);
			tokType = tok.type;
			return tokType;
		} else {
			tokenType temp;
			for (int i = 0; i < preN; i++) {
				temp = lexer.getToken(preN).type;
			}
			return temp;
		}
	}

	inline void Parser::eatToken(tokenType oriType) {
		tok = lexer.getToken();
		tokType = tok.type;
		if (tokType == oriType) {
			fout << tok.toString() << endl;
			lexer.eatToken();
			//lookToken();
		} else {
			error();
		}
	}

	inline void Parser::error() {
		fout << "error" << endl;
	}

	// "程序": [["常量说明", "变量说明", "函数定义组", "主函数"]],
	inline void Parser::program() {
		switch (lookToken()) {
		case CONSTTK:
		case INTTK:
		case CHARTK:
		case VOIDTK:
			constDec();
			varDec();
			funcDefRep();
			mainFunc();
			break;
		default:
			error();
		}
		fout << "<程序>" << endl;
	}

	// "常量说明": [
	// 		[CONSTTK, "常量定义", SEMICN, "常量说明"],
	// 		EMPTY,
	// ]
	inline void Parser::constDec() {
		switch (lookToken()) {
		case IDENFR:
			return;
		case CONSTTK:
			eatToken(CONSTTK);
			constDef();
			eatToken(SEMICN);
			constDec();
			break;
		case INTTK:
		case CHARTK:
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
			return;
		default:
			error();
		}
		if (constDecFlag == 0) {
			fout << "<常量说明>" << endl;
			constDecFlag = 1;
		}
		resetFlag(false, true, false);
	}

	// 常量定义": [
	// 		["常量定义int", "常量定义int组"],
	// 		["常量定义char", "常量定义char组"]
	// ]
	inline void Parser::constDef() {
		switch (lookToken()) {
		case INTTK:
			constDefInt();
			constDefIntRep();
			break;
		case CHARTK:
			constDefChar();
			constDefCharRep();
			break;
		default:
			error();
		}
		fout << "<常量定义>" << endl;
	}

	// "常量定义int": [[INTTK, IDENFR, ASSIGN, "整数"]]
	inline void Parser::constDefInt() {
		switch (lookToken()) {
		case INTTK:
			eatToken(INTTK);
			eatToken(IDENFR);
			eatToken(ASSIGN);
			integer();
			break;
		default:
			error();
		}
	}

	// "常量定义int组": [
	// 		[COMMA, IDENFR, ASSIGN, "整数", "常量定义int组"],
	// 		EMPTY
	// ]
	inline void Parser::constDefIntRep() {
		switch (lookToken()) {
		case SEMICN:
			return;
		case COMMA:
			eatToken(COMMA);
			eatToken(IDENFR);
			eatToken(ASSIGN);
			integer();
			constDefIntRep();
			break;
		default:
			error();
		}
	}

	// "常量定义char": [[CHARTK, IDENFR, ASSIGN, CHARCON]]
	inline void Parser::constDefChar() {
		switch (lookToken()) {
		case CHARTK:
			eatToken(CHARTK);
			eatToken(IDENFR);
			eatToken(ASSIGN);
			eatToken(CHARCON);
			break;
		default:
			error();
		}
	}

	// "常量定义char组": [
	// 		[COMMA, IDENFR, ASSIGN, CHARCON, "常量定义char组"],
	// 		EMPTY
	// ]
	inline void Parser::constDefCharRep() {
		switch (lookToken()) {
		case SEMICN:
			return;
		case COMMA:
			eatToken(COMMA);
			eatToken(IDENFR);
			eatToken(ASSIGN);
			eatToken(CHARCON);
			constDefCharRep();
			break;
		default:
			error();
		}
	}

	// "类型标识符": [[INTTK], [CHARTK]]
	inline void Parser::identType() {
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
		resetFlag();
	}

	// "变量说明": [
	//		["变量定义", SEMICN, "变量说明"],
	// 		EMPTY
	// ]
	inline void Parser::varDec() {
		switch (lookToken()) {
		case IDENFR:
			return;
		case INTTK:
			switch (lookToken(2)) {
			case SEMICN:
			case COMMA:
			case LBRACK:
				varDef();
				eatToken(SEMICN);
				varDec();
				break;
			case LPARENT:
				return;
			default:
				error();
			}
			break;
		case CHARTK:
			switch (lookToken(2)) {
			case SEMICN:
			case COMMA:
			case LBRACK:
				varDef();
				eatToken(SEMICN);
				varDec();
				break;
			case LPARENT:
				return;
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
			return;
		default:
			error();
		}
		if (varDecFlag == 0) {
			fout << "<变量说明>" << endl;
			varDecFlag = 1;
		}
		resetFlag(true, false, false);
	}

	// "变量定义": [["类型标识符", "变量名定义", "变量名定义组"]]
	inline void Parser::varDef() {
		switch (lookToken()) {
		case INTTK:
			identType();
			varDefName();
			varDefNameRep();
			break;
		case CHARTK:
			identType();
			varDefName();
			varDefNameRep();
			break;
		default:
			error();
		}
		fout << "<变量定义>" << endl;
	}

	// "变量名定义": [
	// 		[IDENFR, "变量名定义带数组"]
	// ]
	inline void Parser::varDefName() {
		switch (lookToken()) {
		case IDENFR:
			eatToken(IDENFR);
			varDefArrayName();
			break;
		default:
			error();
		}
	}

	// "变量名定义带数组": [
	// 		[LBRACK, "无符号整数", RBRACK],
	// 		EMPTY
	// ]
	inline void Parser::varDefArrayName() {
		switch (lookToken()) {
		case SEMICN:
		case COMMA:
			return;
		case LBRACK:
			eatToken(LBRACK);
			unsignedInt();
			eatToken(RBRACK);
			break;
		default:
			error();
		}
	}

	// "变量名定义组": [
	// 		[COMMA, "变量名定义", "变量名定义组"],
	// 		EMPTY
	// ]
	inline void Parser::varDefNameRep() {
		switch (lookToken()) {
		case SEMICN:
			return;
		case COMMA:
			eatToken(COMMA);
			varDefName();
			varDefNameRep();
			break;
		default:
			error();
		}
	}

	// "函数定义组": [
	// 		["函数定义", "函数定义组"],
	// 		EMPTY
	// ]
	inline void Parser::funcDefRep() {
		switch (lookToken()) {
		case INTTK:
		case CHARTK:
			funcDef();
			funcDefRep();
			break;
		case VOIDTK:
			switch (lookToken(1)) {
			case IDENFR:
				funcDef();
				funcDefRep();
				break;
			case MAINTK:
				return;
			default:
				error();
			}
			break;
		default:
			error();
		}
		resetFlag();
	}

	// "函数定义": [
	// 		["有返回值函数定义"],
	// 		["无返回值函数定义"]
	// ]
	inline void Parser::funcDef() {
		switch (lookToken()) {
		case INTTK:
		case CHARTK:
			retFuncDef();
			break;
		case VOIDTK:
			nonRetFuncDef();
			break;
		default:
			error();
		}
		resetFlag();
	}

	// "声明头部": [
	// 		[INTTK, IDENFR],
	// 		[CHARTK, IDENFR]
	// ]
	inline void Parser::headDec() {
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
		resetFlag();
	}

	inline void Parser::addFuncName(bool ret) {
		lookToken();
		funcRetMap.insert(pair<string, bool>(tok.val, ret));
	}

	//"有返回值函数定义": [["声明头部", LPARENT, "参数表", RPARENT, LBRACE, "复合语句", RBRACE]]
	inline void Parser::retFuncDef() {
		switch (lookToken()) {
		case INTTK:
		case CHARTK:
			headDec();
			eatToken(LPARENT);
			paraList();
			eatToken(RPARENT);
			eatToken(LBRACE);
			compStatement();
			eatToken(RBRACE);
			break;
		default:
			error();
		}
		fout << "<有返回值函数定义>" << endl;
		resetFlag();
	}

	//"无返回值函数定义": [[VOIDTK, IDENFR, LPARENT, "参数表", RPARENT, LBRACE, "复合语句", RBRACE]]
	inline void Parser::nonRetFuncDef() {
		switch (lookToken()) {
		case VOIDTK:
			eatToken(VOIDTK);
			// funcRetMap.insert(pair<string, bool>(tok.val, false));
			addFuncName(false);
			eatToken(IDENFR);
			eatToken(LPARENT);
			paraList();
			eatToken(RPARENT);
			eatToken(LBRACE);
			compStatement();
			eatToken(RBRACE);
			break;
		default:
			error();
		}
		fout << "<无返回值函数定义>" << endl;
		resetFlag();
	}

	//"主函数": [[VOIDTK, MAINTK, LPARENT, RPARENT, LBRACE, "复合语句", RBRACE]]
	inline void Parser::mainFunc() {
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
		resetFlag();
	}

	//"复合语句": [["常量说明", "变量说明", "语句列"]]
	inline void Parser::compStatement() {
		switch (lookToken()) {
		case IDENFR:
		case CONSTTK:
		case INTTK:
		case CHARTK:
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
			constDec();
			varDec();
			statementBlock();
			break;
		default:
			error();
		}
		resetFlag(true, true, true);
		fout << "<复合语句>" << endl;
	}

	//"参数表": [["类型标识符", IDENFR, "参数表组"], EMPTY]
	inline void Parser::paraList() {
		switch (lookToken()) {
		case INTTK:
		case CHARTK:
			identType();
			eatToken(IDENFR);
			paraListRep();
			break;
		case RPARENT:
			break;
		default:
			error();
		}
		fout << "<参数表>" << endl;
		resetFlag();
	}

	// "参数表组": [[COMMA, "类型标识符", IDENFR, "参数表组"], EMPTY]
	inline void Parser::paraListRep() {
		switch (lookToken()) {
		case COMMA:
			eatToken(COMMA);
			identType();
			eatToken(IDENFR);
			paraListRep();
			break;
		case RPARENT:
			return;
			break;
		default:
			error();
		}
		resetFlag();
	}

	// "表达式前缀": [[PLUS],[MINU],EMPTY]
	inline void Parser::exprOp() {
		switch (lookToken()) {
		case IDENFR:
		case INTCON:
		case CHARCON:
			return;
		case PLUS:
			eatToken(PLUS);
			break;
		case MINU:
			eatToken(MINU);
			break;
		case LPARENT:
			return;
		default:
			error();
		}
		resetFlag();
	}

	// "表达式": [["表达式前缀", "项", "表达式组"]]
	inline void Parser::expr() {
		switch (lookToken()) {
		case IDENFR:
		case INTCON:
		case CHARCON:
		case PLUS:
		case MINU:
		case LPARENT:
			exprOp();
			term();
			exprRep();
			break;
		default:
			error();
		}
		fout << "<表达式>" << endl;
		resetFlag();
	}

	// "表达式组": [
	// 		["加法运算符", "项", "表达式组"],
	// 		EMPTY
	// ]
	inline void Parser::exprRep() {
		switch (lookToken()) {
		case PLUS:
		case MINU:
			plusOp();
			term();
			exprRep();
			break;
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
			return;
		default:
			error();
		}
		resetFlag();
	}

	//"项": [["因子", "项组"]]
	inline void Parser::term() {
		switch (lookToken()) {
		case IDENFR:
		case INTCON:
		case CHARCON:
		case PLUS:
		case MINU:
		case LPARENT:
			factor();
			termRep();
			break;
		default:
			error();
		}
		fout << "<项>" << endl;
		resetFlag();
	}

	// "项组": [
	//		["乘法运算符", "因子", "项组"],
	// 		EMPTY
	// ]
	inline void Parser::termRep() {
		switch (lookToken()) {
		case PLUS:
		case MINU:
			return;
		case MULT:
		case DIV:
			mulOp();
			factor();
			termRep();
			break;
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
			return;
		default:
			error();
		}
		resetFlag();
	}

	// "因子": [
	// 		[IDENFR, "因子以标识符开始"],
	// 		[LPARENT, "表达式", RPARENT],
	// 		["整数"],
	// 		[CHARCON],
	// 		["函数调用语句"],
	// ]
	inline void Parser::factor() {
		switch (lookToken()) {
		case IDENFR:
			switch (lookToken(1)) {
			case LPARENT:
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
				eatToken(IDENFR);
				factorStartWithIdent();
				break;
			default:
				error();
			}
			break;
		case INTCON:
			integer();
			break;
		case CHARCON:
			eatToken(CHARCON);
			break;
		case PLUS:
			integer();
			break;
		case MINU:
			integer();
			break;
		case LPARENT:
			eatToken(LPARENT);
			expr();
			eatToken(RPARENT);
			break;
		default:
			error();
		}
		fout << "<因子>" << endl;
		resetFlag();
	}

	// "因子以标识符开始": [
	// 		[LBRACK, "表达式", RBRACK],
	// 		EMPTY
	// ]
	inline void Parser::factorStartWithIdent() {
		switch (lookToken()) {
		case PLUS:
		case MINU:
		case MULT:
		case DIV:
		case LSS:
		case LEQ:
		case GEQ:
		case GRE:
		case EQL:
		case NEQ:
		case SEMICN:
		case COMMA:
		case RPARENT:
			return;
		case LBRACK:
			eatToken(LBRACK);
			expr();
			eatToken(RBRACK);
			break;
		case RBRACK:
			return;
		default:
			error();
		}
		resetFlag();
	}

	// "语句": [
	// 		["条件语句"],
	// 		["循环语句"],
	// 		[LBRACE, "语句列", RBRACE],
	// 		["函数调用语句", SEMICN],
	// 		["赋值语句", SEMICN],
	// 		["读语句", SEMICN],
	// 		["写语句", SEMICN],
	// 		[SEMICN],
	// 		["返回语句", SEMICN]
	// ]
	inline void Parser::statement() {
		switch (lookToken()) {
		case IDENFR:
			switch (lookToken(1)) {
			case LPARENT:
				funcCallStatement();
				eatToken(SEMICN);
				break;
			case ASSIGN:
			case LBRACK:
				assignStat();
				eatToken(SEMICN);
				break;
			default:
				error();
			}
			break;
		case IFTK:
			condStatement();
			break;
		case DOTK:
			cycleStatement();
			break;
		case WHILETK:
			cycleStatement();
			break;
		case FORTK:
			cycleStatement();
			break;
		case SCANFTK:
			readStatement();
			eatToken(SEMICN);
			break;
		case PRINTFTK:
			writeStatement();
			eatToken(SEMICN);
			break;
		case RETURNTK:
			retStatement();
			eatToken(SEMICN);
			break;
		case SEMICN:
			eatToken(SEMICN);
			break;
		case LBRACE:
			eatToken(LBRACE);
			statementBlock();
			resetFlag(true, true, true);
			eatToken(RBRACE);
			break;
		default:
			error();
		}
		fout << "<语句>" << endl;
		resetFlag();
	}

	//"赋值语句": [[IDENFR, "赋值语句标识符后"]]
	inline void Parser::assignStat() {
		switch (lookToken()) {
		case IDENFR:
			eatToken(IDENFR);
			assignStatAfterIdent();
			break;
		default:
			error();
		}
		fout << "<赋值语句>" << endl;
		resetFlag();
	}

	// "赋值语句标识符后": [
	// 		[ASSIGN, "表达式"],
	// 		[LBRACK, "表达式", RBRACK, ASSIGN, "表达式"]
	// ]
	inline void Parser::assignStatAfterIdent() {
		switch (lookToken()) {
		case ASSIGN:
			eatToken(ASSIGN);
			expr();
			break;
		case LBRACK:
			eatToken(LBRACK);
			expr();
			eatToken(RBRACK);
			eatToken(ASSIGN);
			expr();
			break;
		default:
			error();
		}
		resetFlag();
	}

	//"条件语句": [[IFTK, LPARENT, "条件", RPARENT, "语句", "条件语句else"]]
	inline void Parser::condStatement() {
		switch (lookToken()) {
		case IFTK:
			eatToken(IFTK);
			eatToken(LPARENT);
			condition();
			eatToken(RPARENT);
			statement();
			elseCondStat();
			break;
		default:
			error();
		}
		fout << "<条件语句>" << endl;
		resetFlag();
	}

	// "条件语句else": [
	// 		[ELSETK, "语句"],
	// 		EMPTY
	// ]
	inline void Parser::elseCondStat() {
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
		resetFlag();
	}

	// "条件": [["表达式", "条件表达式后"]]
	inline void Parser::condition() {
		switch (lookToken()) {
		case IDENFR:
		case INTCON:
		case CHARCON:
		case PLUS:
		case MINU:
		case LPARENT:
			expr();
			condAfterFirstExpr();
			break;
		default:
			error();
		}
		fout << "<条件>" << endl;
		resetFlag();
	}

	// "条件表达式后": [
	// 		["关系运算符", "表达式"],
	// 		EMPTY,
	// ]
	inline void Parser::condAfterFirstExpr() {
		switch (lookToken()) {
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
			return;
		default:
			error();
		}
		resetFlag();
	}

	// "循环语句": [
	// 		[WHILETK, LPARENT, "条件", RPARENT, "语句"],
	// 		[DOTK, "语句", WHILETK, LPARENT, "条件", RPARENT], # FIXME:SEMICN ?
	// 		[FORTK, LPARENT, IDENFR, ASSIGN, "表达式", SEMICN, "条件", SEMICN, IDENFR, ASSIGN, IDENFR, "加法运算符", "步长", RPARENT,
	// 		"语句"],
	// ]
	inline void Parser::cycleStatement() {
		switch (lookToken()) {
		case DOTK:
			eatToken(DOTK);
			statement();
			eatToken(WHILETK);
			eatToken(LPARENT);
			condition();
			eatToken(RPARENT);
			break;
		case WHILETK:
			eatToken(WHILETK);
			eatToken(LPARENT);
			condition();
			eatToken(RPARENT);
			statement();
			break;
		case FORTK:
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
		resetFlag();
	}

	// "步长": [["无符号整数"]]
	inline void Parser::step() {
		switch (lookToken()) {
		case INTCON:
			unsignedInt();
			break;
		default:
			error();
		}
		fout << "<步长>" << endl;
		resetFlag();
	}

	// "函数调用语句": [[IDENFR, LPARENT, "值参数表", RPARENT]]
	inline void Parser::funcCallStatement() {
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
		resetFlag();
	}

	// "值参数表": [
	// 		["表达式", "值参数表组"],
	// 		EMPTY
	// ]
	inline void Parser::funcCallInputParaList() {
		switch (lookToken()) {
		case IDENFR:
		case INTCON:
		case CHARCON:
		case PLUS:
		case MINU:
		case LPARENT:
			expr();
			funcCallInputParaListRep();
			break;
		case RPARENT:
			break;
		default:
			error();
		}
		fout << "<值参数表>" << endl;
		resetFlag();
	}

	// "值参数表组": [
	// 		[COMMA, "表达式", "值参数表组"],
	// 		EMPTY
	// ]
	inline void Parser::funcCallInputParaListRep() {
		switch (lookToken()) {
		case COMMA:
			eatToken(COMMA);
			expr();
			funcCallInputParaListRep();
			break;
		case RPARENT:
			return;
			break;
		default:
			error();
		}
		resetFlag();
	}

	// "语句列": [
	// 		["语句", "语句列"],
	// 		EMPTY
	// ]
	inline void Parser::statementBlock() {
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
			statementBlock();
			break;
		case RBRACE:
			break;
		default:
			error();
		}
		if (statBlockFlag == 0) {
			fout << "<语句列>" << endl;
			statBlockFlag = 1;
		}
	}

	// "读语句": [[SCANFTK, LPARENT, IDENFR, "读语句标识符组", RPARENT]]
	inline void Parser::readStatement() {
		switch (lookToken()) {
		case SCANFTK:
			eatToken(SCANFTK);
			eatToken(LPARENT);
			eatToken(IDENFR);
			readIdentRep();
			eatToken(RPARENT);
			break;
		default:
			error();
		}
		fout << "<读语句>" << endl;
		resetFlag();
	}

	// "读语句标识符组": [
	// 		[COMMA, IDENFR, "读语句标识符组"],
	// 		EMPTY
	// ]
	inline void Parser::readIdentRep() {
		switch (lookToken()) {
		case COMMA:
			eatToken(COMMA);
			eatToken(IDENFR);
			readIdentRep();
			break;
		case RPARENT:
			return;
		default:
			error();
		}
		resetFlag();
	}

	// "写语句": [[PRINTFTK, LPARENT, "写语句内容"]]
	inline void Parser::writeStatement() {
		switch (lookToken()) {
		case PRINTFTK:
			eatToken(PRINTFTK);
			eatToken(LPARENT);
			writeStatContent();
			break;
		default:
			error();
		}
		fout << "<写语句>" << endl;
		resetFlag();
	}

	// "写语句内容": [
	//	["字符串", "写语句字符串后"],
	// 	["表达式", RPARENT],
	// ]
	inline void Parser::writeStatContent() {
		switch (lookToken()) {
		case STRCON:
			strConst();
			wirteStatAfterStr();
			break;
		case IDENFR:
		case INTCON:
		case CHARCON:
		case PLUS:
		case MINU:
		case LPARENT:
			expr();
			eatToken(RPARENT);
			break;
		default:
			error();
		}
		resetFlag();
	}

	// "写语句字符串后": [
	// 		[COMMA, "表达式", RPARENT],
	// 		[RPARENT],
	// ]
	inline void Parser::wirteStatAfterStr() {
		switch (lookToken()) {
		case COMMA:
			eatToken(COMMA);
			expr();
			eatToken(RPARENT);
			break;
		case RPARENT:
			eatToken(RPARENT);
			break;
		default:
			error();
		}
		resetFlag();
	}

	// "返回语句": [[RETURNTK, "返回语句内容"]]
	inline void Parser::retStatement() {
		switch (lookToken()) {
		case RETURNTK:
			eatToken(RETURNTK);
			retStatContent();
			break;
		default:
			error();
		}
		fout << "<返回语句>" << endl;
		resetFlag();
	}

	// "返回语句内容": [
	// 		[LPARENT, "表达式", RPARENT],
	// 		EMPTY
	// ]
	inline void Parser::retStatContent() {
		switch (lookToken()) {
		case SEMICN:
			return;
		case LPARENT:
			eatToken(LPARENT);
			expr();
			eatToken(RPARENT);
			break;
		default:
			error();
		}
		resetFlag();
	}

	inline void Parser::plusOp() {
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
		resetFlag();
	}

	inline void Parser::mulOp() {
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
		resetFlag();
	}

	inline void Parser::relationalOp() {
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
		resetFlag();
	}

	inline void Parser::unsignedInt() {
		switch (lookToken()) {
		case INTCON:
			eatToken(INTCON);
			break;
		default:
			error();
		}
		fout << "<无符号整数>" << endl;
		resetFlag();
	}

	inline void Parser::integer() {
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
		resetFlag();
	}

	inline void Parser::strConst() {
		switch (lookToken()) {
		case STRCON:
			eatToken(STRCON);
			break;
		default:
			error();
		}
		fout << "<字符串>" << endl;
		resetFlag();
	}

}
