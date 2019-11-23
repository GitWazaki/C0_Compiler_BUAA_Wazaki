//lexical analysis program
#ifndef LEX_ANALYSIS_H
#define LEX_ANALYSIS_H

#include <cctype>
#include "token.hpp"

namespace Lexer {

	class Lexer {

	public:
		Lexer(string s, Error::Error& err) : inputStr(s), err(err) {
			filesize = s.size();
			startLoc = nextLoc = 0;
			lineNum = 1;
			colNum = 1;
			tok = token{};
		}

		void init(string, ofstream&);
		token getToken(int preN = 0);
		TokenType::tokenType eatToken();

	private:
		string inputStr;
		Error::Error& err;
		long long int filesize;
		long long startLoc;
		long long nextLoc;
		int lineNum;
		int colNum;
		token tok;
		string sym;
		char ch;

		void error();

		void clean(bool restart = true);
		void getch();
		void retract();
		void getstr();
		void getSym();
		void catSym();
		void getDigit();

		bool isBlank();
		bool isChar();
		bool isletter() const;
		bool isDigit() const;
		bool isSingleQuot() const;
		bool isDoubleQuot() const;
		bool isPlus() const;
		bool isMinus() const;
		bool isMul() const;
		bool isDivi() const;
		bool isLess() const;
		bool isGreater() const;
		bool isEqual() const;
		bool isExclaim() const;
		bool isSemi() const;
		bool isComm() const;
		bool isLParent() const;
		bool isRParent() const;
		bool isLBrack() const;
		bool isRBrack() const;
		bool isLBrace() const;
		bool isRBrace() const;
		int checkReserve() const;
	};

	inline void Lexer::error() {
		err.errorHandle({tok,Error::LEX_ERROR, lineNum, colNum});
	}

	inline void Lexer::clean(bool restart) {
		if (restart) {
			nextLoc = startLoc;
		}
		tok = token{};
		sym = "";
		getch();
	}

	inline token Lexer::getToken(int preN) {
		clean(preN == 0);
		while (isBlank())
			getch();

		if (isletter()) {
			getSym();
			int r = checkReserve();
			if (r > 0) {
				//success, is reserved word  RESERVE_START < r <RESERVE_END
				tok.type = static_cast<TokenType::tokenType>(r);
				tok.val = sym;
			} else {
				//failed, is normal symbol
				tok.type = TokenType::IDENFR;
				tok.val = sym;
			}
			retract();
		} else if (isDigit()) {
			getDigit();
			retract();
			tok.type = TokenType::INTCON;
			tok.val = sym;
		} else if (isSingleQuot()) {
			tok.type = TokenType::CHARCON;
			getch(); //读取字符常量
			if(!isChar()) {
				error();
			}
			tok.val = ch;
			getch(); // 读右侧单引号
		} else if (isDoubleQuot()) {
			getstr();
			tok.type = TokenType::STRCON;
			tok.val = sym;
		} else if (isPlus()) {
			tok.type = TokenType::PLUS;
			tok.val = ch;
		} else if (isMinus()) {
			tok.type = TokenType::MINU;
			tok.val = ch;
		} else if (isMul()) {
			tok.type = TokenType::MULT;
			tok.val = ch;
		} else if (isDivi()) {
			tok.type = TokenType::DIV;
			tok.val = ch;
		} else if (isLess()) {
			getch();
			if (isEqual()) {
				tok.type = TokenType::LEQ;
				tok.val = "<=";
			} else {
				tok.type = TokenType::LSS;
				tok.val = "<";
				retract();
			}
		} else if (isGreater()) {
			getch();
			if (isEqual()) {
				tok.type = TokenType::GEQ;
				tok.val = ">=";
			} else {
				tok.type = TokenType::GRE;
				tok.val = ">";
				retract();
			}
		} else if (isEqual()) {
			getch();
			if (isEqual()) {
				tok.type = TokenType::EQL;
				tok.val = "==";
			} else {
				tok.type = TokenType::ASSIGN;
				tok.val = "=";
				retract();
			}
		} else if (isExclaim()) {
			getch();
			if (isEqual()) {
				tok.type = TokenType::NEQ;
				tok.val = "!=";
			} else {
				//TODO error
				error();
				retract();
			}
		} else if (isSemi()) {
			tok.type = TokenType::SEMICN;
			tok.val = ch;
		} else if (isComm()) {
			tok.type = TokenType::COMMA;
			tok.val = ch;
		} else if (isLParent()) {
			tok.type = TokenType::LPARENT;
			tok.val = ch;
		} else if (isRParent()) {
			tok.type = TokenType::RPARENT;
			tok.val = ch;
		} else if (isLBrack()) {
			tok.type = TokenType::LBRACK;
			tok.val = ch;
		} else if (isRBrack()) {
			tok.type = TokenType::RBRACK;
			tok.val = ch;
		} else if (isLBrace()) {
			tok.type = TokenType::LBRACE;
			tok.val = ch;
		} else if (isRBrace()) {
			tok.type = TokenType::RBRACE;
			tok.val = ch;
		} else {
			//TODO error
			error();
			getch();		//读过非法字符
		}
		tok.line = lineNum;
		tok.col = colNum;
		return tok;
	}

	inline TokenType::tokenType Lexer::eatToken() {
		//getToken();
		startLoc = nextLoc;
		return tok.type;
	}

	inline void Lexer::getch() {
		if (nextLoc == filesize) {
			tok.type = TokenType::FINISH;
		}
		colNum++;
		ch = inputStr[nextLoc++];
	}

	inline void Lexer::retract() {
		ch = inputStr[--nextLoc];
	}

	inline void Lexer::getstr() {
		getch();
		while (ch != '\"') {
			if (32 <= ch && ch <= 126 && ch != 34) {
				catSym();
				getch();
			} else {
				//TODO ERROR
				error();
			}
		}
	}

	inline void Lexer::getSym() {
		while (isletter() || isDigit()) {
			catSym();
			getch();
		}
	}

	inline void Lexer::catSym() {
		sym += ch;
	}

	inline void Lexer::getDigit() {
		while (isDigit()) {
			catSym();
			getch();
		}
	}

	inline bool Lexer::isBlank() {
		if (ch == '\n') {
			++lineNum;
			colNum = 0;
		}
		return ch == '\r' || ch == '\n' || ch == '\t' || ch == ' ';
	}

	inline bool Lexer::isChar() {
		return isletter() || isDigit() || isPlus() || isMinus() || isMul() || isDivi();
	}

	inline bool Lexer::isletter() const {
		return ch == '_' ||
			('a' <= ch && ch <= 'z') ||
			('A' <= ch && ch <= 'Z');
	}

	inline bool Lexer::isDigit() const {
		return ('0' <= ch && ch <= '9');
	}

	inline bool Lexer::isSingleQuot() const {
		return ch == '\'';
	}

	inline bool Lexer::isDoubleQuot() const {
		return ch == '\"';
	}

	inline bool Lexer::isPlus() const {
		return ch == '+';
	}

	inline bool Lexer::isMinus() const {
		return ch == '-';
	}

	inline bool Lexer::isMul() const {
		return ch == '*';
	}

	inline bool Lexer::isDivi() const {
		return ch == '/';
	}

	inline bool Lexer::isLess() const {
		return ch == '<';
	}

	inline bool Lexer::isGreater() const {
		return ch == '>';
	}

	inline bool Lexer::isEqual() const {
		return ch == '=';
	}

	inline bool Lexer::isExclaim() const {
		return ch == '!';
	}

	inline bool Lexer::isSemi() const {
		return ch == ';';
	}

	inline bool Lexer::isComm() const {
		return ch == ',';
	}

	inline bool Lexer::isLParent() const {
		return ch == '(';
	}

	inline bool Lexer::isRParent() const {
		return ch == ')';
	}

	inline bool Lexer::isLBrack() const {
		return ch == '[';
	}

	inline bool Lexer::isRBrack() const {
		return ch == ']';
	}

	inline bool Lexer::isLBrace() const {
		return ch == '{';
	}

	inline bool Lexer::isRBrace() const {
		return ch == '}';
	}

	inline int Lexer::checkReserve() const {
		for (int i = 0; i < RESERVE_NUM; i++) {
			if (sym == reserveList[i]) {
				return i + TokenType::RESERVE_START + 1;
			}
		}
		return -1;
	}

}

#endif // !LEX_ANALYSIS_H
