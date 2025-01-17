//
// Created by rudyc on 2019/9/6.
//

#pragma once
#include <cmath>

#define FORMAT meow::helper::format
using namespace std;
#define print(...)                \
    do {                            \
        std::cout << FORMAT(__VA_ARGS__);        \
    } while(0)

#define println(...)                \
    do {                            \
        std::cout << FORMAT(__VA_ARGS__) << std::endl;        \
    } while(0)

#define eprintf(...)            \
    do {        \
        std::cerr << FORMAT(__VA_ARGS__); \
    } while(0)

#define eprintln(...)   \
    do {    \
        eprintf(__VA_ARGS__);   \
        eprintf("\n");  \
    } while(0)  \

#define panic(...)                  \
    do {                            \
        eprintf("panic at {}:{} \n", __FILE__, __LINE__);   \
        eprintf("\t");              \
        eprintf(__VA_ARGS__);        \
        eprintf("\n");    \
		throw;	\
        exit(-1);        \
    } while(0)

#ifdef _DEBUG
#define debugln(...)      \
    do {        \
        eprintf("DEBUG[ {}:{} ]\t", __FILE__, __LINE__);  \
        eprintf(__VA_ARGS__);   \
        eprintf("\n");       \
    } while(0)
#else
#define debugln(...)
#endif

#define assert(expr)	\
	if (!(expr)) panic("assert failed {}", #expr)

#include "format.hpp"

using namespace std;

bool endWith(const std::string & str, const std::string & tail) {
	return str.compare(str.size() - tail.size(), tail.size(), tail) == 0;
}

bool startWith(const std::string & str, const std::string & head) {
	return str.compare(0, head.size(), head) == 0;
}

bool isNumber(std::string str) {
	if (isdigit(str[0]) || str[0] == '-') {
		return true;
	}
	return false;
}

int is_power_2(int num) {
	return (num & (num - 1)) == 0;
}

template<typename C, typename I>
bool notFound(C& container, I item) {
	return container.find(item) == container.end();
}

template<typename C, typename I>
bool Found(C& container, I item) {
	return !notFound(container, item);
}

template <typename T>
int getSubIndex(vector<T> container, T x) {
	for (int i = 0; i < container.size(); i++) {
		if (container[i] == x)
			return i;
	}
	return -1;
}

template<typename T>
set<T> setOr(const set<T>& a, const set<T>& b) {
	set<T> ans;
	for (auto obj : a) {
		ans.insert(obj);
	}
	for (auto obj : b) {
		ans.insert(obj);
	}
	return ans;
}

template<typename T>
set<T> setAnd(const set<T>& a, const set<T>& b) {
	set<T> ans;
	for (auto obj : a) {
		if(Found(b,obj)) {
			ans.insert(obj);
		}
	}
	for (auto obj : b) {
		if (Found(a, obj)) {
			ans.insert(obj);
		}
	}
	return ans;
}

template<typename T>
set<T> setSub(const set<T>& a, const set<T>& b) {
	set<T> ans;
	for (auto obj : a) {
	if (notFound(b, obj)) {
		ans.insert(obj);
	}
}
	return ans;
}

vector<string> globalRegs = {
			"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
			"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
			"$v1",
};
