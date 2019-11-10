// BUAA_Compiler_Wazaki.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <fstream>
#include <sstream>

#include "src/token.hpp"
#include "src/Error.hpp"
#include "src/Symbol.hpp"
#include "src/SymbolTable.hpp"
#include "src/Lexer.hpp"
#include "src/Parser2.hpp"



using namespace std;

string readFileIntoString(string filename) {
	ifstream ifile(filename);

	ostringstream buf;
	char ch;
	while (buf && ifile.get(ch))
		buf.put(ch);
	return buf.str();
}

int main() {
	ofstream fout("output.txt");
	ofstream errout("error.txt");
	// ofstream errout("../../../error.txt");
	Error::Error err{ errout };
	// ofstream fout("../../../output.txt");
	Lexer :: Lexer lexer{ readFileIntoString("testfile.txt"),fout,err };
	// Lexer :: Lexer lexer{ readFileIntoString("../../../testfile.txt"),fout,err };
	Parser2::Parser2 parser2{lexer,fout,err};
	
	
	parser2.program();
	
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
