// BUAA_Compiler_Wazaki.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "src/meow/meow.hpp"
#include "src/token.hpp"
#include "src/Error.hpp"
#include "src/SymbolTable.hpp"
#include "src/Lexer.hpp"
#include "src/Parser2.hpp"
#include "src/MidCode/MidCode.hpp"
#include "src/MidCode/MidOutput.hpp"
#include "src/MidCode/Optimizer.hpp"
#include "src/GenObject/GenMips.hpp"


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

	string input = readFileIntoString("testfile.txt");
	ofstream fout("output.txt");
	ofstream errout("error.txt");
	string midpath = "mid.txt";
	string optpath = "opt.txt";
	ofstream mipsout{ "mips.txt" };
	// string input = readFileIntoString("../../../testfile.txt");
	// ofstream fout("../../../output.txt");
	// ofstream errout("../../../error.txt");
	// string midpath = "../../../mid.txt";
	// string optpath = "../../../opt.txt";
	// ofstream mipsout{ "../../../mips.txt" };

	string out_setting = "semo";
	 
	Error::Error err{ errout, out_setting };
	Lexer :: Lexer lexer{ input,err };
	Parser2::Parser2 parser2{lexer,fout,err,out_setting };
	
	MidIR::MidCode midCodes = parser2.parser();

	MidIR::MidOutput midOutput(midCodes, midpath, out_setting);
	midOutput.output();

	MidIR::Optimizer optimizer(midCodes);
	MidIR::MidCode opt_MidCodes = optimizer.optimize();

	MidIR::MidOutput midOutput_opt(opt_MidCodes, optpath, out_setting);
	midOutput_opt.output();

	GenObject::GenMips genMips{ opt_MidCodes,mipsout,out_setting };
	genMips.gen();
	
	return 0;
}