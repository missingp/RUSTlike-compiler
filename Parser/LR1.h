#pragma once
#include "grammar.h"
#include "lexer.h"
#include <vector>
#include <unordered_map>
#include <queue>
#include <fstream>
#include <iomanip>


#define Table_FILE  "results/ACTION_GOTO_Table.csv" //输出表地址
#define REDUCTION_PROCESS_FILE "results/Reduction_Process.txt" //输出归约地址
#define TREE_DOT_FILE  "results/Parse_Tree.dot"   //画图dot文件地址
#define TREE_PNG_FILE   "picture/Parse_Tree.png" //语法树图片地址
#define PARSE_ERROR_FILE "error_log/Parse_Error.txt"
#define DFA_DOT_FILE   "results/Parse_DFA.dot"


// 分别计算出内置类型的 Hash Value 然后对它们进行 Combine 得到一个哈希值
// 一般直接采用移位加异或（XOR）得到哈希值
struct HashFunc
{
	template<typename T, typename U>
	size_t operator()(const std::pair<T, U>& p) const {
		return std::hash<T>()(p.first) ^ std::hash<U>()(p.second);
	}
};

// 键值比较，哈希碰撞的比较定义，需要直到两个自定义对象是否相等
struct EqualKey {
	template<typename T, typename U>
	bool operator ()(const std::pair<T, U>& p1, const std::pair<T, U>& p2) const {
		return p1.first == p2.first && p1.second == p2.second;
	}
};



//LR1的每一个推导式
class lr1Item
{
public:
	int left;//左侧符号序号编号
	vector<int> right;//右侧符号序号编号
	int dot_pos;//中心点的位置
	int forward;//向前看的符号编号 
	int grammar_index;//这个LR1项是哪个产生式出来的,其实是有冗余，有这个index就已经有了left和right
public:
	
	lr1Item() { left = 0; dot_pos = 0; forward = 0; grammar_index = 0; right.push_back(0); };
	lr1Item(int l, vector<int>& r, int ds, int fw, int gi);
	bool operator==(const lr1Item& item);
	void lr1ItemInit(int l, vector<int>& r, int ds, int fw, int gi);
};


//LR(1)闭包
class lr1Closure
{
public:
	vector<lr1Item> closure;//项目闭包
public:
	bool isIn(lr1Item item);//返回该项目是否在该闭包中
	bool operator==(lr1Closure& clos);
	unordered_map<int, vector<int>> getShiftinSymbol();//得到可移进的字符以及项目在闭包中的位置
	vector<pair<int, int>> getReduceSymbol();//得到可以归约的符号和对应的产生式的序号
	
};


//ACTION表中可以存在的动作
enum actionOption
{
	SHIFT_IN,//移进
	REDUCE,//归约
	ACCEPT,//接受
	REJECT//拒绝
};

//GOTO表中可以存在的动作
enum gotoOption
{
	GO,
	REJECT_
};

class ACTIONItem
{
public:
	actionOption op;
	int serial;//编号，根据op是移进，归约或接受而有不同的含义
	ACTIONItem(actionOption ac_op, int num) {
		this->op = ac_op;
		this->serial = num;
	}
	ACTIONItem(){}
	
};

class GOTOItem
{
public:
	gotoOption op;
	int serial;//编号，在GOTO表里只有转移编号，就是闭包集中的编号
	GOTOItem(gotoOption goto_op, int num) {
	   this->op = goto_op;
	   this->serial = num;
	}
	GOTOItem(){}
	
};

//LR(1)文法总过程
class lr1Grammar :public grammar
{
public:
	int error_line;

	vector<lr1Item> item_sum;//存所有的项目，set没有编号
	vector<lr1Closure> closure_sum;//所有可能出现的闭包，相当于编个号
	unordered_map<pair<int, int>, int,HashFunc,EqualKey> DFA;//map<<closure的编号，符号的编号>，目标closure编号>
	
	unordered_map<pair<int, int>, ACTIONItem, HashFunc, EqualKey> ACTION;//ACTION表
	unordered_map<pair<int, int>, GOTOItem, HashFunc, EqualKey> GOTO;

	lr1Item start_item; //初始项目
	lr1Closure start_closure; //初始项目闭包
public:
	lr1Grammar() {};

	void initClosure0(); //从grammar继承的productions，从开始产生式开始，使得项目集中第一个是闭包
	lr1Closure generateClosure(vector<lr1Item>);//给定项目计算闭包
	int getClosureIndex(lr1Closure& clos);//判断闭包集合中是否有该闭包，若有返回序号，若没有返回-1
	void generateDFA();//得到所有闭包，初始闭包是0号闭包，过程中同时确定DFA


	void generateACTION_GOTO();//计算ACTION表和GOTO表
	void printACTION_GOTO();//打印ACTION和GOTO表为csv文件
    
	int reduction(vector<elem>& lexical_res);//进行归约，在过程中进行打印

	int printParseDFA();//绘制DFA
};