//定义token 单词符号，最小语法单位
#ifndef TOKEN_H
#define TOKEN_H

#include<string>

#define TOKEN_NUM 40		// actually 36 + 2
#define RESERVE_NUM 13	// actually 13
using namespace std;

static string tag[TOKEN_NUM] = {	//用于toString输出

	"IDENFR", // 标识符
	"INTCON", // 整型常量
	"CHARCON", // 字符常量
	"STRCON", // 字符串(常量)

	"RESERVE_START", //保留字开始标志
	"CONSTTK", // const
	"INTTK", // int
	"CHARTK", // char
	"VOIDTK", // void
	"MAINTK", // main
	"IFTK", // if
	"ELSETK", // else
	"DOTK", // do
	"WHILETK", // while
	"FORTK", // for
	"SCANFTK", // scanf
	"PRINTFTK", // printf
	"RETURNTK", // return
	"RESERVE_END", //保留字结束标志

	"PLUS", // +
	"MINU", // -
	"MULT", // *
	"DIV", // /
	"LSS", // < 
	"LEQ", // <=
	"GRE", // >
	"GEQ", // >=
	"EQL", //==
	"NEQ", //!=
	"ASSIGN", // =
	"SEMICN", // ;
	"COMMA", // ,
	"LPARENT", // (
	"RPARENT", // )
	"LBRACK", // [
	"RBRACK", // ]
	"LBRACE", // {
	"RBRACE" // }

};

string reserveList[RESERVE_NUM] = {	//用于保留字检查
	"const",
	"int",
	"char",
	"void",
	"main",
	"if",
	"else",
	"do",
	"while",
	"for",
	"scanf",
	"printf",
	"return"
};

enum tokenType {	//定义token类型

	IDENFR,
	// 标识符
	INTCON,
	// 整型常量
	CHARCON,
	// 字符常量
	STRCON,
	// 字符串(常量)

	RESERVE_START,
	//保留字开始标志
	CONSTTK,
	// const
	INTTK,
	// int
	CHARTK,
	// char
	VOIDTK,
	// void
	MAINTK,
	// main
	IFTK,
	// if
	ELSETK,
	// else
	DOTK,
	// do
	WHILETK,
	// while
	FORTK,
	// for
	SCANFTK,
	// scanf
	PRINTFTK,
	// printf
	RETURNTK,
	// return
	RESERVE_END,
	//保留字结束标志

	PLUS,
	// +
	MINU,
	// -
	MULT,
	// *
	DIV,
	// /
	LSS,
	// < 
	LEQ,
	// <=
	GRE,
	// >
	GEQ,
	// >=
	EQL,
	//==
	NEQ,
	//!=
	ASSIGN,
	// =
	SEMICN,
	// ;
	COMMA,
	// ,
	LPARENT,
	// (
	RPARENT,
	// )
	LBRACK,
	// [
	RBRACK,
	// ]
	LBRACE,
	// {
	RBRACE,
	// }

	FINISH // 文件结尾

};

struct token {
	tokenType type;
	string val;
	int line;
	int col;
	string typeToString() const;
	string toString() const;
};

inline string token::typeToString() const {
	return tag[type];
}

inline string token::toString() const {
	return typeToString() + ' ' + val;
}

#endif //TOKEN_H
