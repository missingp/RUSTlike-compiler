#pragma once

#include<iostream>
#include<vector>
#include<set>
#include<string>
#include<fstream>
#include<algorithm>
#include<Windows.h>

using namespace std;

//文法分析错误
#define GRAMMAR_FILE_OPEN_ERROR 2
#define SUCCESS 1
#define GRAMMAR_ERROR 3
#define GRAMMAR_ERROR_FILE_OPEN_ERROR 4

//文件类型
#define GRAMMAR_ERROR_FILE "error_log/Grammar_Error_List.txt"
#define EXTENDED_GRAMMAR_FILE "results/Extended_Grammar.txt"
#define FIRST_SET_FILE "results/First_Set.txt"

//文法符号
const string EpsilonToken = "@";
const string SplitToken = " | ";// 产生式右部分隔符
const string ProToken = "::=";// 产生式左右部分隔符
const string EndToken = "#";// 尾token 终止符号
const string StartToken = "Program";// 文法起始符号
const string ExtendStartToken = "S";// 扩展文法起始符号
const string AllTerminalToken = "%token";//所有的终结符

//文法符号的类型
typedef enum
{
    unknown,   //未定义
    terminal,	   //终结符
    nonterminal,	   //非终结符
    epsilon,       //空
    end             //结束符
}symbolType;


//单个符号相关属性
typedef struct symbol {
    symbolType type;//文法符号种类
    set<int> first_set;//记录该symbol的first集,这里记录的是索引
    string name;//符号名
    symbol(symbolType type, const string name) {
        this->type = type;
        this->name = name;
    }
}symbol;

//单个产生式的结构
typedef struct production {
    int left_symbol;
    vector<int> right_symbol;//这里记录的是索引

    production(const int left, const vector<int>& right) {
        this->left_symbol = left;
        this->right_symbol = right;
    }
}production;



class grammar {
public:

    vector<symbol>symbols;//所有的符号表
    set<int>terminals;//终结符在symbol中的下标
    set<int>non_terminals;//非终结符在symbol中的下标
    vector<production>productions;//所有的产生式

    int start_index;//起始产生式在productions中的位置

    grammar();
    ~grammar();

    //从file中读入grammar
    int ReadGrammar(const string file_path);
    bool checkGrammar(ofstream& ferr); //生成拓广文法
    void printExtendedGrammar();   //打印拓展文法

    int symbolIndex(const string token);//找到该符号的索引

    //初始化first集合
    void initFirst();
    void initFirstTerm();
    void initFirstNonTerm();
    void printFirst();
   
    //返回一个符号串的first集合
    set<int>getFirst(const vector<int>& str);
};


//工具函数
vector<string> split(const string &str, const string& pattern);
void Trimmed(std::string& str);
int mergeSet(set<int>& src, set<int>& dst, int null_index);