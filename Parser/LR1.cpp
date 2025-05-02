#include "LR1.h"

#define SUCESS 1
#define ACTION_ERROR 2
#define GOTO_ERROR  3
#define REDUCTION_PROCESS_FILE_OPEN_ERROR 4
#define PARSE_ERROR_FILE_OPEN_ERROR 5
#define TREE_DOT_FILE_OPEN_ERROR 6
#define DFA_DOT_FILE_OPEN_ERROR 7


/*********************************LR1 Item********************************/

/**
 * @brief： lr1Item构造函数
 * @param：  int l   产生式左边的符号
             vector<int> &r   产生式右边的符号
			 int ds     点符的位置
			 int fw     向前看的符号
			 int gi      产生该Item的文法式序号

 * @return 
 * */
lr1Item::lr1Item(int l, vector<int>& r, int ds, int fw, int gi)
{
	this->left = l;
	this->right = r;
	this->dot_pos = ds;
	this->forward = fw;
	this->grammar_index = gi;
}

/**
 * @brief： lr1Item初始化函数
 * @param：  int l   产生式左边的符号
			 vector<int> &r   产生式右边的符号
			 int ds     点符的位置
			 int fw     向前看的符号
			 int gi      产生该Item的文法式序号

 * @return
 * */
void lr1Item::lr1ItemInit(int l, vector<int>& r, int ds, int fw, int gi)
{
	this->left = l;
	this->right = r;
	this->dot_pos = ds;
	this->forward = fw;
	this->grammar_index = gi;
}

/**
 * @brief：   运算符重载判断lr1Item等价
 * @param：   const lr1Item& item
 * @return   判断lr1Item是否等价
 * */
bool lr1Item::operator==(const lr1Item& item)
{
	if (this->grammar_index == item.grammar_index && this->dot_pos == item.dot_pos && this->forward == item.forward)
		return true;
	return false;
}


/*********************************LR1 Closure********************************/
/**
 * @brief：   item是否在当前闭包
 * @param：   lr1Item item
 * @return   item是否在当前闭包
 * */
bool lr1Closure::isIn(lr1Item item)
{
	for (int i = 0; i < this->closure.size(); i++)
	{
		if (item == this->closure[i])
			return true;
	}
	return false;
}

/**
 * @brief：   运算符重载判断lr1Closure等价
 * @param：   lr1Closure clos
 * @return   判断lr1Closure是否等价
 * */
bool lr1Closure::operator==(lr1Closure& clos)
{
	if (this->closure.size() != clos.closure.size())
		return false;

    //两个size是相等的才会到这里
	for (int i = 0; i < clos.closure.size(); i++)
	{
		if (!this->isIn(clos.closure[i]))
			return false;
	}
	return true;
}

/**
 * @brief：   得到可移进的字符以及项目在闭包中的位置
 * @param：  
 * @return   
 * */
unordered_map<int, vector<int>> lr1Closure::getShiftinSymbol()
{
	unordered_map<int, vector<int>> MAP;
	for (int i = 0; i < this->closure.size(); i++)
	{
		lr1Item temp = this->closure[i];
		if (temp.dot_pos >= temp.right.size())//点到头了，是归约项目
			continue;
		int present_symbol = temp.right[temp.dot_pos];//当前点后面的字符
		//存在该字符
		if (MAP.find(present_symbol) != MAP.end())
			MAP[present_symbol].push_back(i);//插入项目的序号
		else
		{
			vector<int> target;
			target.push_back(i);
			MAP[present_symbol] = target;//插入
		}
	}
	return MAP;
}

/**
 * @brief：   得到可以归约的符号和对应的产生式的序号
 * @param：
 * @return
 * */
vector<pair<int, int>> lr1Closure::getReduceSymbol()
{
	vector<pair<int, int>> a;
	for (int i = 0; i < this->closure.size(); i++)
	{
		lr1Item temp = this->closure[i];
		if (temp.dot_pos >= temp.right.size())//点到头了，是归约项目
		{
			a.push_back(pair<int, int>(temp.forward, temp.grammar_index));
		}
		else
			continue;
	}
	return a;
}


/*********************************LR1 Grammar********************************/


/**
 * @brief：   判断闭包集合中是否有该闭包，
 * @param：    lr1Closure& clos闭包
 * @return   若有返回序号，若没有返回-1
 * */
int lr1Grammar::getClosureIndex(lr1Closure& clos)
{
	for (int i = 0; i < this->closure_sum.size(); i++)
	{
		if (clos == this->closure_sum[i])
			return i;
	}
	return -1;
}



/**
 * @brief：   生成0号项目集闭包
 * @param：   
 * @return
 * */
void lr1Grammar::initClosure0()
{
	start_item.lr1ItemInit(productions[0].left_symbol, productions[0].right_symbol, 0, symbolIndex(EndToken), start_index);
	vector<lr1Item>lr1;
	lr1.push_back(start_item);
	start_closure = generateClosure(lr1);
}

/**
 * @brief：   生成项目集闭包
 * @param：   vector<lr1Item> lr  已有的lr1产生式
 * @return    生成的闭包
 * */
lr1Closure lr1Grammar::generateClosure(vector<lr1Item> lr1)
{
	// 创建 LR(1) 闭包对象
	lr1Closure current_closure;
	current_closure.closure = lr1;

	// 遍历核心项
	for (int i = 0; i < current_closure.closure.size(); i++) {
		// 处理当前 LR(1) 项
		lr1Item item_now = current_closure.closure[i];

		// 如果·在最后一个位置，跳过处理
		if (item_now.dot_pos >= item_now.right.size()) {
			continue;
		}

		// 当前项的产生式中，·后面的符号对应的索引
		int dot_next_symbol_index = item_now.right[item_now.dot_pos];
		symbol dot_next_symbol = symbols[dot_next_symbol_index];

		// 如果·后面是空串，将·向前移动一位
		if (dot_next_symbol.type == symbolType::epsilon) {
			current_closure.closure[i].dot_pos++;
			continue;
		}

		// 如果·后面是终结符，跳过处理
		if (dot_next_symbol.type == symbolType::terminal) {
			continue;
		}

		// 获取βA，即·后面的符号及其之后的符号
		vector<int> BetaA(item_now.right.begin() + item_now.dot_pos + 1, item_now.right.end());
		BetaA.push_back(item_now.forward);

		// 获取βA 的 FIRST 集合
		set<int> betaA_first_set = getFirst(BetaA);

		// 对于产生式 A->α·Bβ,a
		// 查找符号 B 的产生式 B->XX ，计算其 FIRST(β,a)
		for (int j = 0; j < productions.size(); j++) {
			production production_now = productions[j];
			if (dot_next_symbol_index == production_now.left_symbol) {
				for (auto it = betaA_first_set.begin(); it != betaA_first_set.end(); it++) {
					// 检查闭包中是否已经存在该 LR(1) 项
					bool have_exist = false;
					bool is_epsilon = false;
					is_epsilon = (symbols[production_now.right_symbol[0]].type == symbolType::epsilon);

					for (auto temp = current_closure.closure.begin(); temp != current_closure.closure.end(); temp++) {
						if (*temp == lr1Item(production_now.left_symbol, production_now.right_symbol, 0, *it, j)) {
							have_exist = true;
							break;
						}
					}

					// 如果不存在，则添加到闭包中
					if (!have_exist) {
						current_closure.closure.push_back(lr1Item(production_now.left_symbol, production_now.right_symbol, is_epsilon, *it, j));
					}
				}
			}
		}
	}

	return current_closure;
}


/**
 * @brief：   生成DFA
 * @param：   
 * @return  
 * */
void lr1Grammar::generateDFA()
{
	// 将初始闭包加入闭包集合
	this->closure_sum.push_back(this->start_closure);

	// 使用队列循环求出链接情况
	queue<lr1Closure> q;
	q.push(this->start_closure);

	while (!q.empty())
	{
		// 取出队列中的一个闭包作为当前源闭包
		lr1Closure source = q.front();
		q.pop();

		// 获取源闭包的所有移进符号以及对应的项目集合
		unordered_map<int, vector<int>> shift_symbol = source.getShiftinSymbol();

		// 遍历所有移进符号及对应的项目集合
		for (auto it = shift_symbol.begin(); it != shift_symbol.end(); it++)
		{
			// 有了此时移进的字符
			// 需要求得点向后移动一位后的项目，然后组成 vector 求出闭包
			vector<lr1Item> next_closure_key;
			for (int j = 0; j < it->second.size(); j++)
			{
				lr1Item temp = source.closure[it->second[j]];
				temp.dot_pos++; // 点向后移动一位，其他均不变
				next_closure_key.push_back(temp); // 所有在该字符下可以移进的 LR1 项目
			}

			// 生成下一个闭包
			lr1Closure next_closure = generateClosure(next_closure_key);
			int next_closure_site = getClosureIndex(next_closure);

			// 如果下一个闭包不存在，将其加入队列和闭包集合
			if (next_closure_site == -1)
			{
				q.push(next_closure);
				this->closure_sum.push_back(next_closure);
			}

			// 不管在不在，DFA 都要加上边，记录从当前闭包通过移进符号到达下一个闭包
			this->DFA[pair<int, int>(getClosureIndex(source), it->first)] = getClosureIndex(next_closure);
		}
	}
}


/**
 * @brief：   生成ACTION GOTO表
 * @param：  
 * @return   
 * */
void lr1Grammar::generateACTION_GOTO()
{
	//根据DFA得到移进动作
	for (auto it = this->DFA.begin(); it != this->DFA.end(); it++)
	{
		//转移符号是终结符，记录在ACTION表中
		if (terminals.find(it->first.second) != terminals.end())
		{
			ACTIONItem act_item(actionOption::SHIFT_IN, it->second);
			this->ACTION[pair<int, int>(it->first.first, it->first.second)] = act_item;
		}
		else
		{
			//非终结符记录在GOTO表中
			GOTOItem GOTOItem(gotoOption::GO, it->second);
			this->GOTO[pair<int, int>(it->first.first, it->first.second)] = GOTOItem;
		}
	}

	//根据闭包的归约项得到归约/接受动作
	for (int i = 0; i < this->closure_sum.size(); i++)
	{
		vector<pair<int, int>> reduce_line = this->closure_sum[i].getReduceSymbol();
		for (int j = 0; j < reduce_line.size(); j++)
		{
			if (reduce_line[j].second == start_index)
			{
				ACTIONItem act_item(actionOption::ACCEPT, reduce_line[j].second);
				this->ACTION[pair<int, int>(i, reduce_line[j].first)] = act_item;
			}
			else
			{
				ACTIONItem act_item(actionOption::REDUCE, reduce_line[j].second);
				this->ACTION[pair<int, int>(i, reduce_line[j].first)] = act_item;
			}
		}

	}
}

/**
 * @brief：   打印action goto表到csv文件中
 * @param：    
 * @return   
 * */
void lr1Grammar::printACTION_GOTO()
{
	const int width = 10;
	const int interval = 10;
	const int start_state = 5;
	const int state_action = 20;
	const int action_goto = 230;

	ofstream ofs(Table_FILE, ios::out);
	ofs.setf(std::ios::left);
	ofs << "STATE" << "," << "ACTION" << ",";
	for (int i = 0; i < terminals.size(); i++) {
		ofs << ",";
	}
	ofs << "GOTO" << endl;
	int rowNum = this->closure_sum.size();
	int columnNum = this->symbols.size();
	int actionNum = this->terminals.size();
	int gotoNum = this->non_terminals.size();
	vector<int> terminal_site;//记录终结符在symbols里面的序号
	vector<int> non_terminal_site;//记录非终结符在symbols里面的序号

	ofs << ",";

	for (int i = 0; i < columnNum; i++)
	{
		if (terminals.find(i) != terminals.end())
		{
			if (symbols[i].name == ",")
				ofs << "\"" << symbols[i].name << "\"" << ",";
			else
				ofs << symbols[i].name << ",";
			terminal_site.push_back(i);
		}
	}

	for (int i = 0; i < columnNum; i++)
	{
		if (non_terminals.find(i) != non_terminals.end())
		{

			ofs << symbols[i].name << ",";
			non_terminal_site.push_back(i);
		}
	}

	ofs << endl;

	for (int i = 0; i < rowNum; i++)
	{
		ofs << i << ",";
		for (int j = 0; j < actionNum; j++)
		{
			auto it = ACTION.find(pair<int, int>(i, terminal_site[j]));
			if (it != ACTION.end())
			{
				if (it->second.op == actionOption::SHIFT_IN)
					ofs << "s" << it->second.serial << ",";
				else if (it->second.op == actionOption::REDUCE)
					ofs << "r" << it->second.serial << ",";
				else if (it->second.op == actionOption::ACCEPT)
					ofs << "acc" << ",";
			}
			else
				ofs << ",";
		}
		for (int j = 0; j < gotoNum; j++)
		{
			auto it = GOTO.find(pair<int, int>(i, non_terminal_site[j]));
			if (it != GOTO.end())
				ofs << it->second.serial << ",";
			else
				ofs << ",";
		}

		ofs << endl;
	}
	ofs.close();
}


/**
 * @brief：   归约过程
 * @param：   vector<elem>& lexical_result  词法分析的结果
 * @return   规约错误类型
 * */
int lr1Grammar::reduction(vector<elem>& lexical_result)
{
	// 初始化状态栈、符号栈、序号栈
	vector<int> status_stack;
	vector<int> symbol_stack;
	vector<int> serial_stack;
	int step = 0;
	int node_serial = 0;
	int err_code = 1;  // 错误码，默认为1

	// 在输入串的最后加上结束符号
	elem end(EndToken, EndToken, 0);
	lexical_result.push_back(end);

	// 初始状态栈和符号栈
	status_stack.push_back(0);  // 状态栈先压入状态0
	symbol_stack.push_back(symbolIndex(EndToken));  // 在符号栈中先放入结束符号

	// 打开文件及错误处理
	ofstream ofs(REDUCTION_PROCESS_FILE, ios::out);
	if (ofs.is_open() == 0) {
		err_code = REDUCTION_PROCESS_FILE_OPEN_ERROR;
	}

	ofstream fwrong(PARSE_ERROR_FILE, ios::out);
	if (fwrong.is_open() == 0) {
		err_code = PARSE_ERROR_FILE_OPEN_ERROR;
	}

	ofstream ftree(TREE_DOT_FILE, ios::out);
	if (!ftree.is_open()) {
		err_code = TREE_DOT_FILE_OPEN_ERROR;
	}
	ftree << "digraph mygraph {\n";

	// 输出第一行
	ofs << "STEP: " << step << endl;
	ofs << "STATUS_STACK: ";
	for (int t = 0; t < status_stack.size(); t++)
		ofs << " " << status_stack[t];
	ofs << endl;
	ofs << "SYMBOL_STACK: ";
	for (int t = 0; t < symbol_stack.size(); t++)
		ofs << symbols[symbol_stack[t]].name;
	ofs << endl << "INPUT: ";
	for (int t = 0; t < lexical_result.size(); t++)
		ofs << lexical_result[t].value;
	ofs << endl << endl;
	step++;

	int reduce_error_status = -1;
	int reduce_error_symbol = -1;

	// 开始进行语法分析
	for (int i = 0; i < lexical_result.size(); i++)
	{
		string present_terminal = lexical_result[i].type;
		int present_terminal_serial = symbolIndex(present_terminal);
		int present_status = status_stack.back();
		auto it = ACTION.find(pair<int, int>(present_status, present_terminal_serial));

		// 不存在，即REJECT，错误退出
		if (it == ACTION.end())
		{
			err_code = ACTION_ERROR;
		}
		else
		{
			switch (it->second.op)
			{
				case actionOption::SHIFT_IN:
				{
					// 移进
					status_stack.push_back(it->second.serial);  // 新状态入栈
					symbol_stack.push_back(present_terminal_serial);  // 读入的终结符压栈
					serial_stack.push_back(node_serial);
					ftree << "n" << node_serial++ << "[label=\"" << lexical_result[i].value << "\",color=red];" << endl;
					break;
				}
				case actionOption::REDUCE:
				{
					// 归约，要归约则当前输入串不加一！！
					i--;
					production production_need = productions[it->second.serial];  // 要使用的产生式
					int right_length = production_need.right_symbol.size();  // 要归约掉的长度
					if (right_length == 1)
					{
						// 特判 epsilon，因为存的 size 是1，但实际 length 是0
						if (production_need.right_symbol[0] == symbolIndex("@"))
							right_length = 0;
					}
					vector<int> drop;
					for (int k = 0; k < right_length; k++)
					{
						status_stack.pop_back();  // 状态栈移出
						symbol_stack.pop_back();  // 符号栈移出
						drop.push_back(serial_stack.back());
						serial_stack.pop_back();
					}
					symbol_stack.push_back(production_need.left_symbol);  // 符号栈压入非终结符
					int temp_status = status_stack.back();

					// 归约之后查看 GOTO 表
					auto goto_it = GOTO.find(pair<int, int>(temp_status, production_need.left_symbol));
					if (goto_it == GOTO.end())  // 不存在转移，则应退出 GOTO，编译错误
					{
						err_code = GOTO_ERROR;
						reduce_error_status = temp_status;
						reduce_error_symbol = production_need.left_symbol;
						break;
					}
					else
					{
						if (goto_it->second.op == gotoOption::GO)
						{
							status_stack.push_back(goto_it->second.serial);  // 将新状态压栈
							serial_stack.push_back(node_serial);
						}
						else  // 不会出现
						{
							err_code = GOTO_ERROR;
							break;
						}
					}

					ftree << "n" << node_serial++ << "[label=\"" << symbols[production_need.left_symbol].name << "\"];\n";
					if (right_length == 0)
					{
						ftree << "e" << node_serial << "[label=\"@\"];\n";
						ftree << "n" << node_serial - 1 << " -> " << "e" << node_serial << ";\n";
					}
					else
					{
						for (auto t = drop.begin(); t != drop.end(); t++)
							ftree << "n" << node_serial - 1 << " -> " << "n" << *t << ";\n";
					}
					break;
				}
				case actionOption::ACCEPT:
				{
					ftree << "}";
					ftree.close();
					// 接受状态，直接返回
					ofs << endl << "Parse successfully!" << endl;
					ofs.close();
					fwrong.close();
					return SUCESS;
				}
				case actionOption::REJECT:
				{
					err_code = ACTION_ERROR;
					break;
				}
				default:
					break;
			}
		}

		// 有 error，直接退出
		if (err_code == ACTION_ERROR)
		{
			fwrong << "找不到" << lexical_result[i].value << "的转移"
				<< ", 该符号类型为" << lexical_result[i].type << ",错误位于源代码第" << lexical_result[i].line << "行" << endl;
			this->error_line = lexical_result[i].line;
			ofs << endl << "语法分析错误:不存在当前的ACTION!" << endl;
			ofs << "当前状态: " << present_status << endl;
			ofs << "当前终结符类型: " << present_terminal << endl;
			break;
		}
		else if (err_code == GOTO_ERROR)
		{
			fwrong << "找不到" << lexical_result[i].value << "的转移"
				<< ", 该符号类型为" << lexical_result[i].type << ",错误位于源代码第" << lexical_result[i].line << "行" << endl;
			this->error_line = lexical_result[i].line;
			ofs << endl << "语法分析错误:不存在当前的GOTO!" << endl;
			ofs << "当前状态: " << reduce_error_status << endl;
			ofs << "当前终结符类型: " << symbols[reduce_error_symbol].name << endl;
			break;
		}

		// 输出这一行
		ofs << "STEP: " << step << endl;
		ofs << "STATUS_STACK: ";
		for (int t = 0; t < status_stack.size(); t++)
		{
			ofs << " " << status_stack[t];
		}
		ofs << endl << "STATUS_SYMBOL: ";
		for (int t = 0; t < symbol_stack.size(); t++)
		{
			ofs << symbols[symbol_stack[t]].name;
		}
		ofs << endl << "INPUT: ";
		for (int t = i + 1; t < lexical_result.size(); t++)
		{
			ofs << lexical_result[t].type;
		}
		ofs << endl << endl;
		step++;
	}

	ofs.close();
	fwrong.close();
	ftree.close();
	return err_code;
}




int lr1Grammar::printParseDFA() {


	ofstream fdot(DFA_DOT_FILE,ios::out);
	if (!fdot.is_open()) {
		return DFA_DOT_FILE_OPEN_ERROR;
	}
	
	fdot << "digraph G {\n";
	fdot << "node [shape=rectangle, fontname=\"SimSun\"];\n";
    
	for (const auto& entry : DFA) {
		fdot << "  \"I" << entry.first.first << "\" -> \"I" << entry.second << "\" [label=\"";
		// 此处假设符号可以直接输出，实际可能需要转换
		fdot << symbols[entry.first.second].name << "\"];\n";
	}

	fdot << "}\n";
	fdot.close();
	return SUCESS;
}




