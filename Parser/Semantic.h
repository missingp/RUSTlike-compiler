#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <string.h>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>

using namespace std;

//文件处理
#define SEMANTIC_ERROR_FILE_NOT_OPEN  -11
#define  QUATERNION_FILE_NOT_OPEN  -21


#define SEMANTIC_ERROR_UNDEFINED 13
#define SEMANTIC_ERROR_PARAMETER_NUM 14
#define SEMANTIC_ERROR_NO_RETURN 15
#define SEMANTIC_ERROR_REDEFINED 16
#define SEMANTIC_ERROR_MAIN_UNDEFINED 17
#define SEMANTIC_ERROR_CALL_FUNCTION_FAIL 19


#define  QUATERNION_FILE  "results/Quaternion_List.txt"
#define SEMANTIC_ERROR_FILE "error_log/Semantic_Error_List.txt"

// 语义分析中的符号
struct SemanticSymbol
{
	string dataType; // 数据类型【貌似没用到】
	string value;	 // 符号的具体值
	int tableIndex;	 // 符号所在的符号表在所有表中的索引位置。
	int symbolIndex; // 符号在其所属符号表内的索引位置。
	SemanticSymbol(string d, string v, int t, int s) {
		dataType = d;
		value = v;
		tableIndex = t;
		symbolIndex = s;
	};
};

// 语义分析中可能遇到的错误类型
enum ErrorProcess
{
	UNDEFINED_IDENTIFIER,	 // 标识符未定义错误
	INVALID_PARAMETER_COUNT, // 错误的参数数量
	FUNCTION_REDEFINED,		 // 函数标识符重定义错误
	PARAMETER_REDEFINED,	 // 参数标识符重定义错误
	IDENTIFIER_REDEFINED,	 // 标识符重定义错误
	MAIN_FUNCTION_MISSING,	 // main函数未定义错误
	NO_RETURN, //没有返回值
	CALL_FUNCTION_UNDIFINED, //调用函数没定义
	CALL_NOT_FUNCTION //调用的不是函数
};

// 四元式
struct Quaternion
{
	int sequenceNumber; // 四元式在列表中的唯一标识符。
	string operation;	// 四元式中的操作类型
	string operand1;	// 操作数1
	string operand2;	// 操作数2
	string result;		// 操作的结果或目标变量。

	// 构造函数：用于初始化四元式的各个成员变量。
	Quaternion(int seqNum, string op, string op1, string op2, string res);
};

// 标识符信息，即函数、变量、临时变量、常量的具体信息
struct IdentifierInfo
{
	// IdentifierCategory 枚举定义了标识符的不同类别（函数、变量、临时变量、常量、返回值）
	enum IdentifierCategory
	{
		Function,	// 函数
		Variable,	// 变量
		Temporary,	// 临时变量【貌似没用到】
		Constant,	// 常量【貌似没用到】
		ReturnValue // 返回值
	};

	IdentifierCategory category; // 类别：标识符的类别（函数、变量等）。
	string specifierType;		 // 数据类型：变量或函数返回的数据类型。
	string identifierName;		 // 名称：标识符的名称或常量的值。
	int parameterCount;			 // 参数数量：函数的参数个数。
	int entryIndex;				 // 入口索引：函数入口的四元式标号。【貌似没用到】
	int tableIndex;				 // 表索引：函数符号表在全局符号表列表中的索引。【貌似没用到】
};

// 语义分析中的符号表
struct SemanticSymbolTable
{
	// SymbolTableCategory 枚举定义了符号表的不同类别（全局表、函数表、块级表、临时表）
	enum SymbolTableCategory
	{
		GlobalTable,
		FunctionTable,
		BlockTable, // 【貌似没用到】
		TempTable
	};

	SymbolTableCategory tableCategory; // 符号表的类别
	vector<IdentifierInfo> symbols;	   // 符号集：存储在此表中的所有标识符信息。
	string tableName;				   // 表名：符号表的名称，用于标识和定位。

	// 构造函数：初始化符号表的类别和名称。
	SemanticSymbolTable(const SymbolTableCategory category, const string tableName);

	// 根据符号名称在表中查找符号的索引位置。如果找到符号，则返回其在符号集中的索引；否则返回 -1。
	int FindSymbol(const string symbolName);

	// 将一个新的标识符信息加入到符号表中。返回加入标识符的索引位置。
	int InsertSymbol(const IdentifierInfo newIdentifier);
};

// SemanticAnalysis 类定义了C语言的语义分析器。
// 它包含了用于生成和管理四元式、符号表、以及进行错误检测的方法。
class SemanticAnalysis
{
private:
	// 定义函数指针类型
	using TranslationFunction = int (SemanticAnalysis::*)(const string, const vector<string>, int pos);

	// 创建一个从产生式左部到处理函数的映射表
	unordered_map<string, TranslationFunction> translationMap;

public:
	vector<Quaternion> quaternionList; // 四元式表
	int mainFunctionIndex;			   // main函数对应的四元式标号
	int backpatchingLevel;			   // 回填层次

	vector<int> backpatchingList; // 待回填的四元式在四元式表quaternion_list中的序号
	int nextQuaternionIndex;	  // 当前位置下一个四元式标号
	int tempVarCounter;			  // 临时变量计数

	vector<SemanticSymbol> symbolList;		  // 语义分析过程的符号流，可以理解为在语义分析中的符号栈，跟着语法分析中的要求走
	vector<SemanticSymbolTable> symbolTables; // 程序所有符号表
	vector<int> scopeStack;					  // 当前作用域对应的符号表索引栈

	// 构造函数
	SemanticAnalysis();
	// 将所有的符号信息放入symbol_list
	void AddSymbolToList(const SemanticSymbol symbol);
	// 分析过程
	int Analysis(const string production_left, const vector<string> production_right, int pos);
	// 打印四元式表
	int PrintQuaternion(const string file_path);
	//=====================tool======================================
	bool CheckIdDefine(SemanticSymbol identifier, int* tb_index, int* tb_index_index);
	int CheckParNum(SemanticSymbol check, int* value);
	int ProcessError(SemanticSymbol identifier, int* tb_index, int* tb_index_index, ErrorProcess type, int pos);
	void PopSymbolList(int count);

private:
	// 每次翻译都对应的语法分析中的一个归约式
	int TranslateProgram(const string production_left, const vector<string> production_right, int pos);
	int TranslateExtDef(const string production_left, const vector<string> production_right, int pos);
	int TranslateVarSpecifier(const string production_left, const vector<string> production_right, int pos);
	int TranslateFunSpecifier(const string production_left, const vector<string> production_right, int pos);
	int TranslateIfStmt(const string production_left, const vector<string> production_right, int pos);
	int TranslateIfStmt_m1(const string production_left, const vector<string> production_right, int pos);
	int TranslateIfStmt_m2(const string production_left, const vector<string> production_right, int pos);
	int TranslateIfNext(const string production_left, const vector<string> production_right, int pos);
	int TranslateIfStmt_next(const string production_left, const vector<string> production_right, int pos);

	//===============================================================
	int TranslateFunDec(const string production_left, const vector<string> production_right, int pos);
	int TranslateFunHead(const string production_left, const vector<string> production_right, int pos);
	int TranslateParamDec(const string production_left, const vector<string> production_right, int pos);
	int TranslateBlock(const string production_left, const vector<string> production_right, int pos);
	int TranslateDef(const string production_left, const vector<string> production_right, int pos);
	int TranslateWhileStmt(const string production_left, const vector<string> production_right, int pos);
	int TranslateWhileStmt_m1(const string production_left, const vector<string> production_right, int pos);
	int TranslateWhileStmt_m2(const string production_left, const vector<string> production_right, int pos);

	//==============================================================
	int TranslateVarDeclAssign(const string production_left, const vector<string> production_right, int pos);
	int TranslateExp(const string production_left, const vector<string> production_right, int pos);
	int TranslateAddSubExp(const string production_left, const vector<string> production_right, int pos);
	int TranslateItem(const string production_left, const vector<string> production_right, int pos);
	int TranslateFactor(const string production_left, const vector<string> production_right, int pos);
	int TranslateCallStmt(const string production_left, const vector<string> production_right, int pos);
	int TranslateCallFunCheck(const string production_left, const vector<string> production_right, int pos);
	int TranslateArgs(const string production_left, const vector<string> production_right, int pos);
	int TranslateReturnStmt(const string production_left, const vector<string> production_right, int pos);
	int TranslateRelop(const string production_left, const vector<string> production_right, int pos);
	//==============================================================
	int TranslateElement(const string production_left, const vector<string> production_right, int pos);
	int TranslateMulDivExp(const string production_left, const vector<string> production_right, int pos);
	int TranslateVarDeclInner(const string production_left, const vector<string> production_right, int pos);
	int TranslateType(const string production_left, const vector<string> production_right, int pos);
	int TranslateAssignable(const string production_left, const vector<string> production_right, int pos);
	int TranslateParam(const string production_left, const vector<string> production_right, int pos);
	int TranslateVarMut(const string production_left, const vector<string> production_right, int pos);
	int TranslateAssignStmt(const string production_left, const vector<string> production_right, int pos);
};