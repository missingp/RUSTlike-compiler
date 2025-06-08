#include "grammar.h"
#include <fstream>



/**
 * @brief： 构造函数
 * @param： 
 * @return 
 * */
grammar::grammar()
{
}

grammar::~grammar() {

}


/**
 * @brief： 文法初始化，转化为拓展文法
 * @param： ofstream  输出文法错误信息
 * @return  输入的文法文件是否正确
 * */
bool  grammar::checkGrammar(ofstream &ferr)
{
	if (symbolIndex(EndToken) == -1)
	{
		symbols.push_back(symbol(symbolType::final, EndToken));
		terminals.insert(symbols.size() - 1);
	}
	if (symbolIndex(StartToken) == -1)
	{
		ferr << "该文法没有开始符号!\n";
		return false;
	}
	if (terminals.size()==0)
	{
		ferr << "该文法没有没有终结符！\n";
		return false;
	}
	//生成拓展起始符
	if (symbolIndex(ExtendStartToken) == -1)
	{
		int left_symbol;
		vector<int>right_symbol_index;
		symbols.push_back(symbol(symbolType::nonterminal, ExtendStartToken));
		left_symbol = symbols.size() - 1;
		int right = symbolIndex(StartToken);
		right_symbol_index.push_back(right);
		this->productions.push_back(production(left_symbol, right_symbol_index));
	}
	return true;
}


/**
 * @brief： 读取文法文件并构建first集
 * @param： const string file_path 读入的文法文件
 * @return  读入文法文件的错误类型
 * */
int grammar::ReadGrammar(const string file_path) {

	ifstream fgrammar;//读入的文法文件
	fgrammar.open(file_path, ios::in);
	if (!fgrammar.is_open())
	{
		return GRAMMAR_FILE_OPEN_ERROR;
	}

	ofstream ferr;
	ferr.open(GRAMMAR_ERROR_FILE, ios::out);
	{
		if (!ferr.is_open())
		{
			return GRAMMAR_ERROR_FILE_OPEN_ERROR;
		}
	}

	symbols.push_back(symbol(symbolType::epsilon, EpsilonToken));

	int production_index = 0;
	int row = 0;
	string production_str;

	while (getline(fgrammar, production_str, '\n'))
	{
		row++;
		if (production_str.empty())
			continue;
		Trimmed(production_str);// 去掉空行
		//注释处理
		if (production_str.find("$") != production_str.npos)
			production_str.erase(production_str.find_first_of("$"));//从$第一次出现的位置 删到结尾
		if (production_str.empty())
			continue;

		//处理产生式
		vector<string> process = split(production_str, ProToken);
		if (process.size() != 2) {
			ferr<<"第"<<row<<"行的产生式格式有误（每行应有且只有一个\""<<ProToken<<"\"符号）\n";
			ferr<<process[0]<<process[1]<<"\n";
			ferr<<process.size()<<production_str;
			return GRAMMAR_ERROR;
		}
		string left = process[0];
		string right = process[1];
	
		Trimmed(left);
		int left_symbol = -1;
		if (left != AllTerminalToken)
		{
			left_symbol = symbolIndex(left);
			if (left_symbol == -1)//没找到
			{
				Trimmed(left);
				symbols.push_back(symbol(symbolType::nonterminal, left));
				left_symbol = symbols.size() - 1;
				non_terminals.insert(left_symbol);
			}
		}
		//左部非终结符处理完成 
		//完成右部的分割
		vector<string>right_part = split(right, SplitToken);
		if (right_part.size() == 0)
		{
			ferr<<"第"<<row<<"行的产生式格式有误（没有文法号）\n";
			return GRAMMAR_ERROR;
		}

		for (auto it = right_part.begin(); it != right_part.end(); it++)
		{
			if (left_symbol == -1)
			{
				Trimmed(*it);
				symbols.push_back(symbol(symbolType::terminal, *it));
				terminals.insert(symbols.size() - 1);
			}
			else {
				vector<int>right_symbol_index;
				//分割右部
				vector<string>right_symbol_name = split(*it, " ");
				for (auto right_it = right_symbol_name.begin(); right_it != right_symbol_name.end(); right_it++)
				{
					int right_index_now = symbolIndex(*right_it);
					if (right_index_now == -1) {
						Trimmed(*right_it);
						if ((*right_it).length() == 0)
							continue;
						symbols.push_back(symbol(symbolType::nonterminal, *right_it));
						right_index_now = symbols.size() - 1;
						non_terminals.insert(right_index_now);
					}

					right_symbol_index.push_back(right_index_now);
				}
				this->productions.push_back(production(left_symbol, right_symbol_index));
				//拓展文法产生式在production中的位置
				if (symbols[left_symbol].name == ExtendStartToken)
				{
					start_index = productions.size() - 1;
				}

			}
		}
	}

    //生成拓展文法
	if (!checkGrammar(ferr)) {
		return GRAMMAR_ERROR;
	}
	//将拓展文法输出到文件中
	printExtendedGrammar();

	//初始化first集
	initFirst();
	fgrammar.close();
	ferr.close();
	return SUCCESS;
}

/**
 * @brief： 输出当前的终结符和非终结符集合
 * @param： 
 * @return  
 * */
void grammar::printExtendedGrammar() {
	
	fstream file_open;
	file_open.open(EXTENDED_GRAMMAR_FILE, ios::out);
	file_open << symbols[0].name << " ";
	file_open << "终结符" << endl;
	for (set<int>::iterator i = terminals.begin(); i != terminals.end(); i++)
	{
		int index = *i;
		file_open << symbols[index].name << " ";
	}
	file_open << endl;
	file_open << "非终结符" << endl;
	for (set<int>::iterator i = non_terminals.begin(); i != non_terminals.end(); i++)
	{
		int index = *i;
		file_open << symbols[index].name << "  ";
	}
	file_open << endl;

	//输出所有的productions
	file_open << "语法规则" << endl;

	for (int i = 0; i < productions.size(); i++)
	{
		file_open << "production" << i << "  ";
		file_open << symbols[productions[i].left_symbol].name;
		file_open << "  --->  ";
		for (int j = 0; j < productions[i].right_symbol.size(); j++) {
			file_open << symbols[productions[i].right_symbol[j]].name << " ";
		}
		file_open << endl;
	}

}


/**
 * @brief：对应符号在symbol中的索引
 * @param： onst string token 词元符号
 * @return 返回对应符号在symbol中的索引，如果不存在，返回-1
 * */
int grammar::symbolIndex(const string token) {

	//遍历符号表 找到对应的位置
	for (int i = 0; i < symbols.size(); i++)
	{
		if (token == symbols[i].name)
		{
			return i;
		}
	}
	return -1;
}

/**
 * @brief：构造first集
 * @param： 
 * @return 
 * */
void grammar::initFirst() {

	initFirstTerm();
	initFirstNonTerm();
	printFirst();
}


/**
 * @brief：生成终结符的first集合
 * @param：
 * @return 
 * */
void grammar::initFirstTerm() {
	//如果是终结符，first集合就是他们本身
	for (set<int>::iterator i = terminals.begin(); i != terminals.end(); i++)
	{
		symbols[*i].first_set.insert(*i);
	}
}

/**
 * @brief：生成非终结符的first集合
 * @param：
 * @return
 * */
void grammar::initFirstNonTerm() {
	while (1) {
		//遍历非终结符
		bool useful_action = 0;;
		for (set<int>::iterator i = non_terminals.begin(); i != non_terminals.end(); i++)
		{
			//遍历production
			for (int j = 0; j < productions.size(); j++)
			{
				if (productions[j].left_symbol == *i) {

					if (symbols[productions[j].right_symbol[0]].type == symbolType::terminal || symbols[productions[j].right_symbol[0]].type == symbolType::epsilon)
					{
						useful_action = symbols[*i].first_set.insert(productions[j].right_symbol[0]).second || useful_action;
						continue;
					}
					//非终结符
					bool isAllNull = 1;
					int k = 0;
					for (k = 0; k < productions[j].right_symbol.size(); k++)
					{
						//如果右部是终结符
						if (symbols[productions[j].right_symbol[k]].type == symbolType::terminal)
						{
							useful_action = symbols[*i].first_set.insert(productions[j].right_symbol[k]).second || useful_action;

							isAllNull = false;
							break;
							//找下一条文法
						}
						else {
							//如果右部是非终结符
							useful_action = mergeSet(symbols[productions[j].right_symbol[k]].first_set, symbols[*i].first_set, symbolIndex(EpsilonToken)) || useful_action;
							isAllNull = symbols[productions[j].right_symbol[k]].first_set.count(symbolIndex(EpsilonToken)) && isAllNull;

							if (!isAllNull)
								break;
						}
					}
					if (k == productions[j].right_symbol.size() && isAllNull)
						useful_action = symbols[*i].first_set.insert(symbolIndex(EpsilonToken)).second || useful_action;
				}
			}

		}
		if (useful_action == 0)
			break;
	}
	
}

/**
 * @brief：打印first集
 * @param： 
 * @return 
 * */
void grammar::printFirst()
{
	fstream file_open;
	file_open.open(FIRST_SET_FILE, ios::out);
	//输出每个symbol的first集合
	for (set<int>::iterator it = terminals.begin(); it != terminals.end(); it++)
	{
		
		file_open << symbols[*it].name;
		file_open << ": first set: {";

		for (set<int>::iterator i = symbols[*it].first_set.begin(); i != symbols[*it].first_set.end(); i++)
		{
			file_open << symbols[*i].name << " ";
		}
		file_open <<"}"<<endl;
	}
	for (set<int>::iterator it = non_terminals.begin(); it != non_terminals.end(); it++)
	{
		
		file_open << symbols[*it].name;
		file_open << ": first set: {";

		for (set<int>::iterator i = symbols[*it].first_set.begin(); i != symbols[*it].first_set.end(); i++)
		{
			file_open << symbols[*i].name << " ";
		}
		file_open <<"}"<< endl;
	}
}


/**
 * @brief：获取first集
 * @param：const vector<int>& str  输入串
 * @return 返回他们整体的first集
 * */
set<int>grammar::getFirst(const vector<int>& str) {
	set<int>first_set;
	
	if (str.empty()) {
		return first_set;
	}
	bool is_epsilon = true;
	int empty_location = symbolIndex(EpsilonToken);
	for (auto i = str.begin(); i != str.end(); i++)
	{
		if (symbols[*i].type == symbolType::terminal)
		{
			is_epsilon = false;
			mergeSet(symbols[*i].first_set, first_set, empty_location);

			break;
		}
		
		mergeSet(symbols[*i].first_set, first_set, empty_location);
		is_epsilon = symbols[*i].first_set.count(empty_location) && is_epsilon;
		if (!is_epsilon)
			break;
	}
	if (is_epsilon)
		first_set.insert(empty_location);
	return first_set;
}



//工具函数
//字符串分割函数  
vector<string> split(const string& str, const string& pattern) {
	vector<string> res;
	if (str == "")
		return res;
	//在字符串末尾也加入分隔符，方便截取最后一段
	string strs = str + pattern;
	size_t pos = strs.find(pattern);

	while (pos != strs.npos)
	{
		string temp = strs.substr(0, pos);
		res.push_back(temp);
		//去掉已分割的字符串,在剩下的字符串中进行分割
		strs = strs.substr(pos + pattern.length(), strs.size());
		pos = strs.find(pattern);
	}
	return res;
}


//去掉首尾 空格白
void Trimmed(std::string& str) {
	str.erase(0, str.find_first_not_of(" \r\n\t\v\f"));
	str.erase(str.find_last_not_of(" \r\n\t\v\f") + 1);
}

int mergeSet(set<int>& src, set<int>& dst, int null_index)
{
	set<int>temp;
	set_difference(src.begin(), src.end(), dst.begin(), dst.end(), inserter(temp, temp.begin()));
	//如果dst里没有空串，把插入后空串的删除
	bool isNullExist = dst.find(null_index) != dst.end();
	int pre_size = dst.size();
	dst.insert(temp.begin(), temp.end());
	if (!isNullExist)
	{
		dst.erase(null_index);
	}
	return pre_size < dst.size();
}