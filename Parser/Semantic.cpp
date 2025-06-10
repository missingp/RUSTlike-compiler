#include "semantic.h"

using namespace std;

/**
 * @brief： 构造四元式对象。
 * @param： int seqNum 代表四元式的序列号，用于唯一标识每个四元式。
 * @param： string op 表示四元式中的操作类型，如加法、赋值等。
 * @param： string op1 表示操作的第一个操作数，根据操作类型可以是变量、常量或其他。
 * @param： string op2 表示操作的第二个操作数，对于一元操作此字段可能为空。
 * @param： string res 表示操作的结果或目标变量。
 * @return  不返回值，但构造一个初始化的四元式对象。
 * */
Quaternion::Quaternion(int seqNum, string op, string op1, string op2, string res)
{
	this->sequenceNumber = seqNum;
	this->operation = op;
	this->operand1 = op1;
	this->operand2 = op2;
	this->result = res;
}

/**
 * @brief： 构造一个新的符号表对象。
 * @param： const SymbolTableCategory tableCategory 符号表的类别（如全局表、函数表等）。
 * @param： const string tableName 符号表的名称，用于标识和定位符号表。
 * @return  不返回值，但构造一个初始化的符号表对象。
 * */
SemanticSymbolTable::SemanticSymbolTable(const SymbolTableCategory tableCategory, const string tableName)
{
	this->tableCategory = tableCategory;
	this->tableName = tableName;
}

/**
 * @brief： 在符号表中查找指定名称的符号。
 * @param： const string id_name 要查找的符号的名称。
 * @return  如果找到符号，则返回符号在符号表中的索引；否则返回 -1。
 * */
int SemanticSymbolTable::FindSymbol(const string id_name)
{
	for (int i = 0; i < this->symbols.size(); i++)
	{
		if (id_name == this->symbols[i].identifierName)
			return i;
	}
	return -1;
}

/**
 * @brief： 向符号表中插入一个新的符号。
 * @param： const IdentifierInfo ident 要插入的符号信息。
 * @return  如果成功插入新符号，则返回新符号在符号表中的索引；如果符号已存在，则返回 -1。
 * */
int SemanticSymbolTable::InsertSymbol(const IdentifierInfo ident)
{
	int symbolIndex = this->FindSymbol(ident.identifierName);
	if (symbolIndex == -1)
	{
		this->symbols.push_back(ident);
		return this->symbols.size() - 1;
	}
	else
		return -1;
}

/**********************************************/
/**
 * @brief： 构造SemanticAnalysis类的实例。
 *          初始化全局符号表、临时变量表，设置四元式标号起始值，以及初始化其他相关成员。
 * @param：
 * @return
 */
SemanticAnalysis::SemanticAnalysis()
	: nextQuaternionIndex(1), // 从标号1开始生成四元式标号；0号用于 (j, -, -, main_address)
	mainFunctionIndex(-1),  // main函数的标号先置为-1
	backpatchingLevel(0),	  // 初始回填层次为0，表示不需要回填
	tempVarCounter(0)		  // 临时变量计数器归零
{
	// 初始化映射表
	translationMap["Program"] = &SemanticAnalysis::TranslateProgram;
	translationMap["ExtDef"] = &SemanticAnalysis::TranslateExtDef;
	translationMap["VarSpecifier"] = &SemanticAnalysis::TranslateVarSpecifier;
	translationMap["FunSpecifier"] = &SemanticAnalysis::TranslateFunSpecifier;
	translationMap["IfStmt"] = &SemanticAnalysis::TranslateIfStmt;
	translationMap["IfStmt_m1"] = &SemanticAnalysis::TranslateIfStmt_m1;
	translationMap["IfStmt_m2"] = &SemanticAnalysis::TranslateIfStmt_m2;
	translationMap["IfNext"] = &SemanticAnalysis::TranslateIfNext;
	translationMap["IfStmt_next"] = &SemanticAnalysis::TranslateIfStmt_next;
	translationMap["FunDec"] = &SemanticAnalysis::TranslateFunDec;
	translationMap["FunHead"] = &SemanticAnalysis::TranslateFunHead;
	translationMap["FunHead"] = &SemanticAnalysis::TranslateFunHead;
	translationMap["ParamDec"] = &SemanticAnalysis::TranslateParamDec;
	translationMap["Block"] = &SemanticAnalysis::TranslateBlock;
	translationMap["Def"] = &SemanticAnalysis::TranslateDef;
	translationMap["WhileStmt"] = &SemanticAnalysis::TranslateWhileStmt;
	translationMap["WhileStmt_m1"] = &SemanticAnalysis::TranslateWhileStmt_m1;
	translationMap["WhileStmt_m2"] = &SemanticAnalysis::TranslateWhileStmt_m2;
	translationMap["VarDeclAssign"] = &SemanticAnalysis::TranslateVarDeclAssign;
	translationMap["VarDeclAssign"] = &SemanticAnalysis::TranslateVarDeclAssign;
	translationMap["Exp"] = &SemanticAnalysis::TranslateExp;
	translationMap["AddSubExp"] = &SemanticAnalysis::TranslateAddSubExp;
	translationMap["Item"] = &SemanticAnalysis::TranslateItem;
	translationMap["Factor"] = &SemanticAnalysis::TranslateFactor;
	translationMap["CallStmt"] = &SemanticAnalysis::TranslateCallStmt;
	translationMap["CallFunCheck"] = &SemanticAnalysis::TranslateCallFunCheck;
	translationMap["Args"] = &SemanticAnalysis::TranslateArgs;
	translationMap["ReturnStmt"] = &SemanticAnalysis::TranslateReturnStmt;
	translationMap["Relop"] = &SemanticAnalysis::TranslateRelop;
	translationMap["Element"] = &SemanticAnalysis::TranslateElement;
	translationMap["MulDivExp"] = &SemanticAnalysis::TranslateMulDivExp;
	translationMap["VarDeclInner"] = &SemanticAnalysis::TranslateVarDeclInner;
	translationMap["AssignStmt"] = &SemanticAnalysis::TranslateAssignStmt;
	translationMap["Type"] = &SemanticAnalysis::TranslateType;
	translationMap["VarMut"] = &SemanticAnalysis::TranslateVarMut;
	translationMap["Param"] = &SemanticAnalysis::TranslateParam;
	translationMap["Assignable"] = &SemanticAnalysis::TranslateAssignable;
	translationMap["AddSubOp"] = &SemanticAnalysis::TranslateAddSubOp;
	translationMap["MulDivOp"] = &SemanticAnalysis::TranslateAddSubOp;
	translationMap["CompOp"] = &SemanticAnalysis::TranslateCompOp;
	translationMap["ArgList"] = &SemanticAnalysis::TranslateArgList;
	translationMap["BreakStmt"] = &SemanticAnalysis::TranslateBreakStmt;
	translationMap["ContinueStmt"] = &SemanticAnalysis::TranslateContinueStmt;
	translationMap["LoopStmt"] = &SemanticAnalysis::TranslateLoopStmt;
	translationMap["LoopStmt_m"] = &SemanticAnalysis::TranslateLoopStmt_m;
	translationMap["ForStmt"] = &SemanticAnalysis::TranslateForStmt; 
	translationMap["ForStmt_m1"] = &SemanticAnalysis::TranslateForStmt_m1;
	translationMap["ForStmt_m2"] = &SemanticAnalysis::TranslateForStmt_m2;
	// 创建全局的符号表
	symbolTables.push_back(SemanticSymbolTable(SemanticSymbolTable::GlobalTable, "global table"));
	// 当前作用域为全局作用域
	scopeStack.push_back(0);
	// 创建临时变量表
	symbolTables.push_back(SemanticSymbolTable(SemanticSymbolTable::TempTable, "temp variable table"));
}

/**
 * @brief： 将一个符号添加到符号列表。
 *          该列表用于存储语义分析过程中遇到的符号。
 * @param： const SemanticSymbol symbol 要添加的符号。
 * @return  无
 */
void SemanticAnalysis::AddSymbolToList(const SemanticSymbol symbol)
{
	symbolList.push_back(symbol);
}

/**
 * @brief： 检查标识符是否已在当前或上层作用域的符号表中定义。
 *          从当前作用域开始，逐层向上查找标识符。
 * @param： SemanticSymbol identifier 要检查的标识符。
 * @param： int* tb_index 输出参数，返回找到标识符的符号表在表集合中的索引。
 * @param： int* tb_index_index 输出参数，返回找到标识符在符号表中的索引。
 * @return  bool 表示是否找到标识符。如果找到返回true，否则返回false。
 */
bool SemanticAnalysis::CheckIdDefine(SemanticSymbol identifier, int* tb_index, int* tb_index_index)
{
	// 从当前层向上遍历
	for (int now_layer = scopeStack.size() - 1; now_layer >= 0; now_layer--)
	{
		SemanticSymbolTable  now_table = symbolTables[scopeStack[now_layer]]; // 用引用可以优化开销
		int indexInTable = now_table.FindSymbol(identifier.value);

		if (indexInTable != -1) // 找到了标识符
		{
			*tb_index = scopeStack[now_layer];
			*tb_index_index = indexInTable;
			return true;
		}
	}

	// 如果没有找到标识符
	return false;
}

/**
 * @brief： 检查函数调用的参数数量是否与声明一致。
 *          比较给定标识符的参数数量与提供的值。
 * @param： SemanticSymbol check 要检查的函数标识符。
 * @param： int* value 输出参数，提供的参数数量。
 * @return  int 返回值表示参数数量的检查结果：
 *              - 0 表示参数数量一致；
 *              - 1 表示提供的参数数量多于声明；
 *              - 2 表示提供的参数数量少于声明。
 */
int SemanticAnalysis::CheckParNum(SemanticSymbol check, int* value)
{
	// 直接引用参数数量
	int declaredParamCount = symbolTables[check.tableIndex].symbols[check.symbolIndex].parameterCount;

	if (declaredParamCount > *value) // 提供的参数数量多于声明
		return 1;
	else if (declaredParamCount < *value) // 提供的参数数量少于声明
		return 2;
	return 0; // 参数数量一致
}

/**
 * @brief： 处理语义分析中遇到的各种错误。
 *          根据错误类型，执行不同的检查和错误报告。
 * @param： SemanticSymbol identifier 要检查的标识符。
 * @param： int* tb_index 指向符号所在的符号表索引的指针。
 * @param： int* tb_index_index 指向符号在符号表中的索引的指针。
 * @param： ErrorProcess type 错误类型。
 * @return  无
 */
// int SemanticAnalysis::ProcessError(SemanticSymbol identifier, int* tb_index, int* tb_index_index, ErrorProcess type, int pos)
int SemanticAnalysis::ProcessError(SemanticSymbol identifier, int* tb_index, int* tb_index_index, ErrorProcess type, int pos)
{
	fstream ferr(SEMANTIC_ERROR_FILE, ios::out);
	if (!ferr.is_open()) {
		//文件无法打开
		//文件无法打开
		return SEMANTIC_ERROR_FILE_NOT_OPEN;
	}




	if (type == UNDEFINED_IDENTIFIER) {
		if (!CheckIdDefine(identifier, tb_index, tb_index_index))
		{

			ferr << "语义分析中发生错误：\"" << identifier.value << "\"未定义" << ",位于第" << pos << "行" << endl;
			return SEMANTIC_ERROR_UNDEFINED;
		}
	}
	else if (type == INVALID_PARAMETER_COUNT)
	{
		int res = CheckParNum(identifier, tb_index);
		if (res != 0)
		{
			string error_msg = res == 2 ? "过多" : "过少";
			ferr << "语义分析中发生错误：函数调用参数" << error_msg
				<< "，出错函数名为 " << identifier.value << ",位于第" << pos << "行" << endl;
			return SEMANTIC_ERROR_PARAMETER_NUM;
		}
	}
	else if (type == FUNCTION_REDEFINED || type == PARAMETER_REDEFINED || type == IDENTIFIER_REDEFINED)
	{
		SemanticSymbolTable table = type == FUNCTION_REDEFINED ? symbolTables[0] : symbolTables[*tb_index];
		if (table.FindSymbol(identifier.value) != -1)
		{
			string error_type = (type == FUNCTION_REDEFINED) ? "函数" : (type == PARAMETER_REDEFINED) ? "函数参数"
				: "变量";
			ferr << "语义分析中发生错误：" << error_type << identifier.value << "重定义, 位于第" << pos << "行" << endl;
			return SEMANTIC_ERROR_REDEFINED;
		}
	}
	else if (type == INVALID_PARAMETER_COUNT)
	{
		int res = CheckParNum(identifier, tb_index);
		if (res != 0)
		{
			string error_msg = res == 2 ? "过多" : "过少";
			ferr << "语义分析中发生错误：函数调用参数" << error_msg
				<< "，出错函数名为 " << identifier.value << ",位于第" << pos << "行" << endl;
			return SEMANTIC_ERROR_PARAMETER_NUM;
		}
	}
	else if (type == FUNCTION_REDEFINED || type == PARAMETER_REDEFINED || type == IDENTIFIER_REDEFINED)
	{
		SemanticSymbolTable table = type == FUNCTION_REDEFINED ? symbolTables[0] : symbolTables[*tb_index];
		if (table.FindSymbol(identifier.value) != -1)
		{
			string error_type = (type == FUNCTION_REDEFINED) ? "函数" : (type == PARAMETER_REDEFINED) ? "函数参数"
				: "变量";
			ferr << "语义分析中发生错误：" << error_type << identifier.value << "重定义, 位于第" << pos << "行" << endl;
			return SEMANTIC_ERROR_REDEFINED;
		}
	}
	else if (type == MAIN_FUNCTION_MISSING)

	{
		ferr << "语义分析中发生错误：main函数未定义" << "，位于第" << pos << "行" << endl;
		ferr << "语义分析中发生错误：main函数未定义" << "，位于第" << pos << "行" << endl;
		return SEMANTIC_ERROR_MAIN_UNDEFINED;
	}
	else if (type == NO_RETURN) {
		ferr << "没有返回值, 位于第" << pos << "行" << endl;
		ferr << "没有返回值, 位于第" << pos << "行" << endl;
		return SEMANTIC_ERROR_NO_RETURN;
	}
	else if (type == CALL_FUNCTION_UNDIFINED) {
		ferr << "调用的函数" << identifier.value << "未定义,位于第" << pos << "行" << endl;
		return SEMANTIC_ERROR_CALL_FUNCTION_FAIL;
	}
	else if (type == CALL_NOT_FUNCTION) {
		ferr << identifier.value << "不是函数,无法调用,位于第" << pos << "行" << endl;
		return SEMANTIC_ERROR_CALL_FUNCTION_FAIL;
	}
	return 1;
}

/**
 * @brief： 从符号列表的末尾移除指定数量的符号。
 *          用于回退语义分析过程中加入的符号，通常在处理局部作用域时使用。
 * @param： int count 要移除的符号数量。
 * @return  无
 */
void SemanticAnalysis::PopSymbolList(int cnt)
{
	while (cnt--)
		symbolList.pop_back();
}

/**
 * @brief： 根据语法产生式的左部选择相应的翻译方法进行语义分析。
 *          该函数根据不同的产生式应用不同的翻译规则，生成四元式或执行其他语义动作。
 * @param： const string production_left 产生式的左部，决定采用哪种翻译方法。
 * @param： const vector<string> production_right 产生式的右部，提供翻译所需的具体元素。
 * @return  无
 */
// int SemanticAnalysis::Analysis(const string production_left, const vector<string> production_right, int pos)
int SemanticAnalysis::Analysis(const string production_left, const vector<string> production_right, int pos)
{
	cout << production_left << " | ";
	for (int i = 0; i < production_right.size(); i++)
	{
		cout << production_right[i] << " ";
	}
	cout << endl <<"------------------------------" << endl;
	cout << production_left << " | ";
	for (int i = 0; i < production_right.size(); i++)
	{
		cout << production_right[i] << " ";
	}
	cout << endl <<"------------------------------" << endl;
	auto it = translationMap.find(production_left);
	//cout << 444;
	int res = 1;
	int res = 1;
	if (it != translationMap.end())
	{
		// 调用相应的翻译函数
		res = (this->*(it->second))(production_left, production_right, pos);
		res = (this->*(it->second))(production_left, production_right, pos);
	}
	else
	{
		if (production_right[0] != "@")
		{
			int count = production_right.size();
			while (count--)
				symbolList.pop_back();
		}
		symbolList.push_back({ production_left, "", -1, -1 });
	}
	return res;
}

/**
 * @brief： 将四元式列表输出到指定文件。
 *          遍历四元式列表，将每个四元式的内容格式化输出到文件。
 * @param： const string file_path 输出文件的路径。
 * @return  无
 */
int SemanticAnalysis::PrintQuaternion(const string file_path)
{
	ofstream ofs(file_path, ios::out);
	if (!ofs.is_open()) {
		return QUATERNION_FILE_NOT_OPEN;
	}

	for (const Quaternion& temp : quaternionList)
	{
		ofs << temp.sequenceNumber << "(";
		ofs << temp.operation << "," << temp.operand1 << "," << temp.operand2 << "," << temp.result << ")" << endl;
	}

	ofs.close();
	return 1;
}

/**
 * @brief： 翻译程序的入口点。
 *          检查 main 函数是否定义，插入初始跳转四元式，并更新符号流。
 * @param： const string production_left 产生式的左部，代表当前语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 */
// int SemanticAnalysis::TranslateProgram(const string production_left, const vector<string> production_right, int pos)
int SemanticAnalysis::TranslateProgram(const string production_left, const vector<string> production_right, int pos)
{
	int res = 1;
	if (mainFunctionIndex == -1)
		res = ProcessError({ "", "", -1, -1 }, nullptr, nullptr, MAIN_FUNCTION_MISSING, pos);
		res = ProcessError({ "", "", -1, -1 }, nullptr, nullptr, MAIN_FUNCTION_MISSING, pos);

	// 插入初始跳转四元式
	quaternionList.insert(quaternionList.begin(), { 0, "j", "-", "-", std::to_string(mainFunctionIndex) });

	// 修改符号流
	PopSymbolList(production_right.size());

	symbolList.emplace_back(production_left, "", -1, -1); // 使用emplace_back代替push_back（提高效率）
	return res;
}

/**
 * @brief： 翻译外部定义，包括变量和函数的定义。
 *          根据产生式右部的最后一个元素判断是变量定义还是函数定义。
 *          对于变量定义，将其添加到符号表；对于函数定义，处理相应的作用域变化。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 * @term    ExtDef ::= VarSpecifier <ID> ; | FunSpecifier FunDec Block
 */
int SemanticAnalysis::TranslateExtDef(const string production_left, const vector<string> production_right, int pos)
{
	int res = 1;
	int res = 1;
	if (production_right.back() == ";") // 变量定义
	{
		// 获取变量名和类型
		SemanticSymbol varName = symbolList[symbolList.size() - 2];
		SemanticSymbol varType = symbolList[symbolList.size() - 3];

		// 处理可能的重定义错误
		res = ProcessError(varName, &(scopeStack.back()), nullptr, IDENTIFIER_REDEFINED, pos);
		res = ProcessError(varName, &(scopeStack.back()), nullptr, IDENTIFIER_REDEFINED, pos);

		// 之前没有定义，添加变量到符号表
		IdentifierInfo variable{ IdentifierInfo::Variable, varType.value, varName.value };
		symbolTables[scopeStack.back()].InsertSymbol(variable);

		// 更新符号流
		PopSymbolList(production_right.size());
		symbolList.emplace_back(production_left, varName.value, -1, -1);
	}
	else // 函数定义
	{
		SemanticSymbol funcName = symbolList[symbolList.size() - 2];

		// 弹出当前函数作用域
		scopeStack.pop_back();

		// 更新符号流
		PopSymbolList(production_right.size());
		symbolList.emplace_back(production_left, funcName.value, -1, -1);
	}
	return res;
}
/* 	注意！！！
	下面两（三）个函数是一样的，可以考虑合并，一个变量的，一个函数的
*/
/**
 * @brief： 翻译变量声明的类型指定部分。
 *          从符号流中提取类型信息，并替换右侧的产生式为左侧的语法结构。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 * @term    VarSpecifier ::= int
 */
int SemanticAnalysis::TranslateVarSpecifier(const string production_left, const vector<string> production_right, int pos)
{
	// 直接引用最后一个符号，避免复制
	SemanticSymbol lastC = symbolList.back();

	// 更新符号流
	PopSymbolList(production_right.size());
	symbolList.emplace_back(production_left, lastC.value, -1, -1);
	return 1;
}

/**
 * @brief： 翻译函数声明的类型指定部分。
 *          从符号流中提取函数类型信息，并替换右侧的产生式为左侧的语法结构。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 * @term    FunSpecifier ::= void | int
 */
int SemanticAnalysis::TranslateFunSpecifier(const string production_left, const vector<string> production_right, int pos)
{
	// 直接引用最后一个符号，避免复制
	SemanticSymbol lastC = symbolList.back();

	// 更新符号流
	PopSymbolList(production_right.size());
	symbolList.emplace_back(production_left, lastC.value, -1, -1);
	return 1;
}

/**
 * @brief： 处理函数返回类型的语义翻译。
 *          提取symbolList中最后一个元素作为函数的返回类型，并更新符号流，
 *          将右侧产生式的符号替换为左侧产生式的符号。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 */
int SemanticAnalysis::TranslateFunDec(const string production_left, const vector<string> production_right, int pos)
{
	// 直接引用最后一个符号，避免复制
	SemanticSymbol lastType = symbolList[symbolList.size() - 1];

	// 更新符号流
	PopSymbolList(production_right.size());
	symbolList.emplace_back(production_left, lastType.value, -1, -1);
	return 1;
}

/**
 * @brief： 在语义分析中创建并处理函数的符号表。
 *          从符号流中提取函数名和参数信息，检查函数是否重定义，
 *          并创建新的函数符号表和相应的函数入口四元式。
 * @param： const string production_left 产生式的左部，代表当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 */
// int SemanticAnalysis::TranslateFunHead(const string production_left, const vector<string> production_right, int pos)
int SemanticAnalysis::TranslateFunHead(const string production_left, const vector<string> production_right, int pos)
{
	int res = 1;
	// 创建函数表：funcName函数名，funcPara函数参数
	SemanticSymbol funcName = symbolList[symbolList.size() - 4];
	SemanticSymbol funcName = symbolList[symbolList.size() - 4];
	SemanticSymbol funcPara = symbolList[symbolList.size() - 2];
	SemanticSymbol funType = symbolList[symbolList.size() - 1];

	if (production_right.size() == 7)
	{
		funcName = symbolList[symbolList.size() - 6];
		funcPara = symbolList[symbolList.size() - 4];
	}
	else
	{
		funType.value = "";
	}
	

	// 判断函数名是否重定义
	res = ProcessError(funcName, nullptr, nullptr, FUNCTION_REDEFINED, pos);
	res = ProcessError(funcName, nullptr, nullptr, FUNCTION_REDEFINED, pos);

	// 在全局符号表创建当前函数的符号项（这里参数个数和入口地址会进行回填）
	symbolTables.emplace_back(SemanticSymbolTable::FunctionTable, funcName.value);
	symbolTables[0].InsertSymbol({ IdentifierInfo::Function, funType.value, funcName.value, 0, 0, int(symbolTables.size() - 1) });
	
	// 函数表压入栈
	scopeStack.push_back(symbolTables.size() - 1);

	//找参数，不完善
	for (int i = 0; i < symbolTables[0].symbols.size(); i++)
	{
		if (symbolTables[0].symbols[i].category == IdentifierInfo::Variable)
		{
			int new_position = symbolTables[scopeStack.back()].InsertSymbol({IdentifierInfo::Variable, symbolTables[0].symbols[i].specifierType, symbolTables[0].symbols[i].identifierName, -1, -1, -1});
			int table_position = symbolTables[0].FindSymbol(funcName.value);
			symbolTables[0].symbols[table_position].parameterCount++;
			symbolTables[0].symbols[i].category = IdentifierInfo::Destroy;

		}
	}

	// 返回值
	IdentifierInfo return_value({ IdentifierInfo::ReturnValue, funcName.value + "_return_value", funcPara.value });
	if (funcName.value == "main")
		mainFunctionIndex = nextQuaternionIndex;
	quaternionList.emplace_back(nextQuaternionIndex++, funcName.value, "-", "-", "-");
	symbolTables[scopeStack.back()].InsertSymbol(return_value);
	symbolList.emplace_back(production_left, funcName.value, 0, int(symbolTables[0].symbols.size() - 1));
	return res;
}

/**
 * @brief： 翻译函数的参数声明。
 *          从符号流中提取参数名称和类型，检查参数是否重定义，
 *          并将参数信息添加到当前函数的符号表中。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 */
int SemanticAnalysis::TranslateParamDec(const string production_left, const vector<string> production_right, int pos)
{
	int res = 1;
	SemanticSymbol name = symbolList.back();
	SemanticSymbol type = symbolList[symbolList.size() - 2];

	SemanticSymbolTable& function_table = symbolTables[scopeStack.back()];
	SemanticSymbolTable& function_table = symbolTables[scopeStack.back()];

	res = ProcessError(name, &(scopeStack.back()), nullptr, PARAMETER_REDEFINED, pos);
	res = ProcessError(name, &(scopeStack.back()), nullptr, PARAMETER_REDEFINED, pos);

	int new_position = function_table.InsertSymbol({ IdentifierInfo::Variable, type.value, name.value, -1, -1, -1 });
	int table_position = symbolTables[0].FindSymbol(function_table.tableName);
	symbolTables[0].symbols[table_position].parameterCount++;

	quaternionList.emplace_back(nextQuaternionIndex++, "defpar", "-", "-", name.value);

	PopSymbolList(production_right.size());
	symbolList.emplace_back(production_left, name.value, scopeStack.back(), new_position);
	return res;
}

/**
 * @brief： 翻译代码块。
 *          更新符号流，将代码块的处理结果加入符号流中。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 */
int SemanticAnalysis::TranslateBlock(const string production_left, const vector<string> production_right, int pos)
{
	// 直接引用符号流的最后一个元素
	SemanticSymbol type = symbolList.back();

	// 更新符号流
	PopSymbolList(production_right.size());
	symbolList.emplace_back(production_left, std::to_string(nextQuaternionIndex), -1, -1);
	return 1;
}

/**
 * @brief： 翻译变量定义。
 *          从符号流中提取变量名和类型，检查变量是否重定义，并将变量信息添加到当前作用域的符号表中。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 */
int SemanticAnalysis::TranslateDef(const string production_left, const vector<string> production_right, int pos)
{
	int res = 1;
	SemanticSymbol name = symbolList[symbolList.size() - 2];
	SemanticSymbol type = symbolList[symbolList.size() - 3];
	SemanticSymbolTable& current_table = symbolTables[scopeStack.back()];

	res = ProcessError(name, &(scopeStack.back()), nullptr, IDENTIFIER_REDEFINED, pos);
	res = ProcessError(name, &(scopeStack.back()), nullptr, IDENTIFIER_REDEFINED, pos);

	current_table.InsertSymbol({ IdentifierInfo::Variable, type.value, name.value, -1, -1, -1 });

	// 更新符号流
	PopSymbolList(production_right.size());
	symbolList.emplace_back(production_left, name.value, scopeStack.back(), int(current_table.symbols.size() - 1));
	return res;
}

/**
 * @brief： 翻译赋值语句。
 *          从符号流中提取赋值语句的组成部分，检查标识符是否已定义，并生成相应的四元式。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 * @ term   AssignStmt ::= <ID> = Exp
 */
// int SemanticAnalysis::TranslateVarDeclAssign(const string production_left, const vector<string> production_right, int pos)
int SemanticAnalysis::TranslateVarDeclAssign(const string production_left, const vector<string> production_right, int pos)
{
	int res = 1;
	SemanticSymbol exp = symbolList[symbolList.size() - 2];
	SemanticSymbol idtf = symbolList[symbolList.size() - 4];
	if (production_right.size() == 7)
	{
		idtf = symbolList[symbolList.size() - 6];
	}
	if (production_right.size() == 3)
	{
		exp = symbolList[symbolList.size() - 1];
		idtf = symbolList[symbolList.size() - 3];
	}
	// 检查标识符是否已定义
	int tbIndex = -1, tbIndexIndex = -1;
	res = ProcessError(idtf, &tbIndex, &tbIndexIndex, UNDEFINED_IDENTIFIER, pos);
	res = ProcessError(idtf, &tbIndex, &tbIndexIndex, UNDEFINED_IDENTIFIER, pos);

	quaternionList.emplace_back(nextQuaternionIndex++, "=", exp.value, "-", idtf.value);

	// 更新符号流
	PopSymbolList(production_right.size());
	symbolList.emplace_back(production_left, idtf.value, tbIndex, tbIndexIndex);
	symbolList[symbolList.size()-1].ext = exp.value;
	return res;
}

/**
 * @brief： 翻译表达式。
 *          根据表达式的右部分长度来确定是简单表达式还是包含关系运算符的复合表达式。
 *          对于简单表达式，直接从符号流中取出；对于复合表达式，生成相应的四元式。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 * @term    Exp ::= AddSubExp | Exp Relop AddSubExp
 */
int SemanticAnalysis::TranslateExp(const string production_left, const vector<string> production_right, int pos)
{
	if (production_right.size() == 1)
	{
		SemanticSymbol add_sub_exp = symbolList.back();
		PopSymbolList(1);
		symbolList.emplace_back(production_left, add_sub_exp.value, add_sub_exp.tableIndex, add_sub_exp.symbolIndex);
	}
	else
	{
		SemanticSymbol exp1 = symbolList[symbolList.size() - 3];
		SemanticSymbol op = symbolList[symbolList.size() - 2];
		SemanticSymbol  exp2 = symbolList.back();
		string var = "T" + to_string(tempVarCounter++);
		int lable_num_next = nextQuaternionIndex++;

		quaternionList.emplace_back(lable_num_next, "j" + op.value, exp1.value, exp2.value, to_string(lable_num_next + 3));
		quaternionList.emplace_back(nextQuaternionIndex++, "=", "0", "-", var);
		quaternionList.emplace_back(nextQuaternionIndex++, "j", "-", "-", to_string(lable_num_next + 4));
		quaternionList.emplace_back(nextQuaternionIndex++, "=", "1", "-", var);
		PopSymbolList(production_right.size());
		symbolList.emplace_back(production_left, var, -1, -1);
	}
	return 1;
}

/**
 * @brief： 翻译加减表达式。
 *          根据产生式右部的长度来确定是单个项还是包含加法或减法运算符的表达式。
 *          对于单个项，直接从符号流中取出；对于包含运算符的表达式，生成相应的四元式。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 * @term    AddSubExp ::= Item | Item + Item | Item - Item
 */
int SemanticAnalysis::TranslateAddSubExp(const string production_left, const vector<string> production_right, int pos)
{
	if (production_right.size() == 1)
	{
		SemanticSymbol add_sub_exp = symbolList.back();
		PopSymbolList(1);
		symbolList.emplace_back(production_left, add_sub_exp.value, add_sub_exp.tableIndex, add_sub_exp.symbolIndex);
	}
	else
	{
		SemanticSymbol exp1 = symbolList[symbolList.size() - 3];
		SemanticSymbol  op = symbolList[symbolList.size() - 2];
		SemanticSymbol exp2 = symbolList.back();
		string var = "T" + to_string(tempVarCounter++);

		quaternionList.emplace_back(nextQuaternionIndex++, op.value, exp1.value, exp2.value, var);
		PopSymbolList(production_right.size());
		symbolList.emplace_back(production_left, var, -1, -1);
	}
	return 1;
}

/**
 * @brief： 翻译乘除表达式项。
 *          根据产生式右部的长度确定是单个因子还是包含乘法或除法运算符的表达式。
 *          对于单个因子，直接从符号流中取出；对于包含运算符的表达式，生成相应的四元式。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 * @term    Item ::= Factor | Factor * Factor | Factor / Factor
 */
int SemanticAnalysis::TranslateItem(const string production_left, const vector<string> production_right, int pos)
{
	if (production_right.size() == 1)
	{
		SemanticSymbol item_exp = symbolList.back();
		PopSymbolList(1);
		symbolList.emplace_back(production_left, item_exp.value, item_exp.tableIndex, item_exp.symbolIndex);
	}
	else {

		SemanticSymbol exp1 = symbolList[symbolList.size() - 3];
		SemanticSymbol op = symbolList[symbolList.size() - 2];
		SemanticSymbol exp2 = symbolList.back();
		string var = "T" + to_string(tempVarCounter++);

		quaternionList.emplace_back(nextQuaternionIndex++, op.value, exp1.value, exp2.value, var);
		PopSymbolList(production_right.size());
		symbolList.emplace_back(production_left, var, -1, -1);
	}
	return 1;
}

/**
 * @brief： 翻译因子。
 *          根据产生式右部的内容来处理不同类型的因子，例如整数、括号内表达式、标识符或函数调用。
 *          对于标识符，检查其是否已定义。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 * @term    Factor ::= <INT> | ( Exp ) | <ID> | CallStmt
 */
int SemanticAnalysis::TranslateFactor(const string production_left, const vector<string> production_right, int pos)
{
#if 0
#if 0
	int res = 1;
	if (production_right.size() == 1)
	{
		SemanticSymbol exp = symbolList.back();
		if ("<ID>" == production_right[0])
		{
			int tb_index = -1, tb_index_index = -1;
			res = ProcessError(exp, &tb_index, &tb_index_index, UNDEFINED_IDENTIFIER, pos);
			res = ProcessError(exp, &tb_index, &tb_index_index, UNDEFINED_IDENTIFIER, pos);
		}
		PopSymbolList(production_right.size());
		symbolList.emplace_back(production_left, exp.value, exp.tableIndex, exp.symbolIndex);
	}
	else
	{
		SemanticSymbol  exp = symbolList[symbolList.size() - 2];
		PopSymbolList(production_right.size());
		symbolList.emplace_back(production_left, exp.value, exp.tableIndex, exp.symbolIndex);
	}
	return res;
#endif
	 int res = 1;
    int rightSize = production_right.size();

    // Factor -> Element
    if (rightSize == 1 && production_right[0] == "Element") {
        SemanticSymbol element = symbolList.back();
        PopSymbolList(1);
        symbolList.emplace_back(production_left, element.value, element.tableIndex, element.symbolIndex);
    }
    
    // Factor -> * Factor (解引用)
    else if (rightSize == 2 && production_right[0] == "*") {
        SemanticSymbol factor = symbolList.back();
        string tempVar = "T" + to_string(tempVarCounter++);
        
        // 生成解引用四元式
        quaternionList.emplace_back(nextQuaternionIndex++, "deref", factor.value, "-", tempVar);
        
        PopSymbolList(2);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
    
    // Factor -> & mut Factor (可变引用)
    else if (rightSize == 3 && production_right[0] == "&" && production_right[1] == "mut") {
        SemanticSymbol factor = symbolList.back();
        string tempVar = "T" + to_string(tempVarCounter++);
        
        // 生成可变引用四元式
        quaternionList.emplace_back(nextQuaternionIndex++, "ref_mut", factor.value, "-", tempVar);
        
        PopSymbolList(3);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
    
    // Factor -> & Factor (不可变引用)
    else if (rightSize == 2 && production_right[0] == "&") {
        SemanticSymbol factor = symbolList.back();
        string tempVar = "T" + to_string(tempVarCounter++);
        
        // 生成不可变引用四元式
        quaternionList.emplace_back(nextQuaternionIndex++, "ref", factor.value, "-", tempVar);
        
        PopSymbolList(2);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
    
    // Factor -> [ ArrayElementList ] (数组)
    else if (rightSize == 3 && production_right[0] == "[" && production_right[2] == "]") {
        SemanticSymbol arrayList = symbolList[symbolList.size() - 2];
        string tempVar = "T" + to_string(tempVarCounter++);
        
        // 生成数组创建四元式
        quaternionList.emplace_back(nextQuaternionIndex++, "array", arrayList.value, "-", tempVar);
        
        PopSymbolList(3);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
    
    // Factor -> ( TupleAssignInner ) (元组)
    else if (rightSize == 3 && production_right[0] == "(" && production_right[2] == ")") {
        SemanticSymbol tupleInner = symbolList[symbolList.size() - 2];
        string tempVar = "T" + to_string(tempVarCounter++);
        
        // 生成元组创建四元式
        quaternionList.emplace_back(nextQuaternionIndex++, "tuple", tupleInner.value, "-", tempVar);
        
        PopSymbolList(3);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
	}
	return res;
#endif
	 int res = 1;
    int rightSize = production_right.size();

    // Factor -> Element
    if (rightSize == 1 && production_right[0] == "Element") {
        SemanticSymbol element = symbolList.back();
        PopSymbolList(1);
        symbolList.emplace_back(production_left, element.value, element.tableIndex, element.symbolIndex);
    }
    
    // Factor -> * Factor (解引用)
    else if (rightSize == 2 && production_right[0] == "*") {
        SemanticSymbol factor = symbolList.back();
        string tempVar = "T" + to_string(tempVarCounter++);
        
        // 生成解引用四元式
        quaternionList.emplace_back(nextQuaternionIndex++, "deref", factor.value, "-", tempVar);
        
        PopSymbolList(2);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
    
    // Factor -> & mut Factor (可变引用)
    else if (rightSize == 3 && production_right[0] == "&" && production_right[1] == "mut") {
        SemanticSymbol factor = symbolList.back();
        string tempVar = "T" + to_string(tempVarCounter++);
        
        // 生成可变引用四元式
        quaternionList.emplace_back(nextQuaternionIndex++, "ref_mut", factor.value, "-", tempVar);
        
        PopSymbolList(3);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
    
    // Factor -> & Factor (不可变引用)
    else if (rightSize == 2 && production_right[0] == "&") {
        SemanticSymbol factor = symbolList.back();
        string tempVar = "T" + to_string(tempVarCounter++);
        
        // 生成不可变引用四元式
        quaternionList.emplace_back(nextQuaternionIndex++, "ref", factor.value, "-", tempVar);
        
        PopSymbolList(2);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
    
    // Factor -> [ ArrayElementList ] (数组)
    else if (rightSize == 3 && production_right[0] == "[" && production_right[2] == "]") {
        SemanticSymbol arrayList = symbolList[symbolList.size() - 2];
        string tempVar = "T" + to_string(tempVarCounter++);
        
        // 生成数组创建四元式
        quaternionList.emplace_back(nextQuaternionIndex++, "array", arrayList.value, "-", tempVar);
        
        PopSymbolList(3);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
    
    // Factor -> ( TupleAssignInner ) (元组)
    else if (rightSize == 3 && production_right[0] == "(" && production_right[2] == ")") {
        SemanticSymbol tupleInner = symbolList[symbolList.size() - 2];
        string tempVar = "T" + to_string(tempVarCounter++);
        
        // 生成元组创建四元式
        quaternionList.emplace_back(nextQuaternionIndex++, "tuple", tupleInner.value, "-", tempVar);
        
        PopSymbolList(3);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
	return res;
}

/**
 * @brief： 翻译函数调用语句。
 *          检查被调用的函数是否已定义以及参数数量是否正确。
 *          生成对应的四元式来处理函数调用，并更新符号流。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 * @term	CallStmt ::= <ID> ( CallFunCheck Args )
 */
int SemanticAnalysis::TranslateCallStmt(const string production_left, const vector<string> production_right, int pos)
{
	int res = 1;
	SemanticSymbol functionId = symbolList[symbolList.size() - 5];
	SemanticSymbol check = symbolList[symbolList.size() - 3];
	SemanticSymbol args = symbolList[symbolList.size() - 2];

	int argCount = stoi(args.value);
	int dummy = -1;
	res = ProcessError(check, &argCount, &dummy, INVALID_PARAMETER_COUNT, pos);
	res = ProcessError(check, &argCount, &dummy, INVALID_PARAMETER_COUNT, pos);

	string tempVar = "T" + to_string(tempVarCounter++);
	quaternionList.emplace_back(nextQuaternionIndex++, "call", functionId.value, "-", tempVar);

	PopSymbolList(production_right.size());
	symbolList.emplace_back(production_left, tempVar, -1, -1);
	return res;
}

/**
 * @brief： 检查函数调用的正确性。
 *          确认函数标识符是否已定义且类型正确（是否为函数类型）。
 *          如果检查失败，抛出相应的语义错误。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，通常是一个空白的确认标记。
 * @return  无
 */
int SemanticAnalysis::TranslateCallFunCheck(const string production_left, const vector<string> production_right, int pos)
{
	int res = 1;
	SemanticSymbol  functionId = symbolList[symbolList.size() - 2];
	int functionPosition = symbolTables[0].FindSymbol(functionId.value);

	if (functionPosition == -1)
	{
		//cout << "未定义的函数名" << endl;
		//throw(SEMANTIC_ERROR_UNDEFINED);
		int invalid = 0;
		res = ProcessError(functionId, &invalid, &invalid, CALL_FUNCTION_UNDIFINED, pos);
		res = ProcessError(functionId, &invalid, &invalid, CALL_FUNCTION_UNDIFINED, pos);
	}
	else if (symbolTables[0].symbols[functionPosition].category != IdentifierInfo::Function)
	{
		//cout << "ID定义，但类型不是函数" << endl;
		//throw(SEMANTIC_ERROR_UNDEFINED);
		int invalid = 0;
		res = ProcessError(functionId, &invalid, &invalid, CALL_NOT_FUNCTION, pos);

	}

	symbolList.emplace_back(production_left, functionId.value, 0, functionPosition);
	return res;
}

/**
 * @brief： 翻译函数调用的参数列表。
 *          处理参数列表中每个表达式，并计算参数数量。
 *          对于每个参数，生成相应的四元式，并更新符号流中的参数数量。
 *          根据参数列表的长度来区分不同的处理方式：单个参数、多个参数或无参数。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 * @term    Args ::= Exp , Args | Exp | @
 */
int SemanticAnalysis::TranslateArgs(const string production_left, const vector<string> production_right, int pos)
{
	int symbol_list_size = symbolList.size();
	int right_size = production_right.size();
	if (right_size == 3)
	{
		int args_num = stoi(symbolList[symbol_list_size - 1].value) + 1;
		SemanticSymbol test_exp = symbolList[symbol_list_size - 3];
		PopSymbolList(right_size);
		quaternionList.push_back({ nextQuaternionIndex++, "param", test_exp.value, "-", "-" });
		symbolList.push_back({ production_left, to_string(args_num), -1, -1 });
	}
	else
	{
		if (production_right[0] == "Exp")
		{
			SemanticSymbol exp = symbolList[symbol_list_size - 1];
			PopSymbolList(right_size);
			quaternionList.push_back({ nextQuaternionIndex++, "param", exp.value, "-", "-" });
			symbolList.push_back({ production_left, "1", -1, -1 });
		}
		if (production_right[0] == "@")
		{
			symbolList.push_back({ production_left, "0", -1, -1 });
		}
	}
	return 1;
}


/**
 * @brief： 翻译返回语句。
 *          根据返回语句的类型（有无返回值）进行处理。
 *          如果函数声明为非void类型且缺少返回值，则抛出错误。
 *          生成相应的四元式来处理返回值或直接返回。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含具体的语法单元。
 * @return  无
 * @term    ReturnStmt ::= return Exp | return
 */
int SemanticAnalysis::TranslateReturnStmt(const string production_left, const vector<string> production_right, int pos)
{
	int res = 1;
	int right_size = production_right.size();
	int symbol_list_size = symbolList.size();
	SemanticSymbolTable symbol_table = symbolTables[scopeStack.back()];

	if (right_size == 1)
	{
		auto type = symbolTables[0].symbols[symbolTables[0].FindSymbol(symbol_table.tableName)].specifierType;
		if (type != "void")
		{
			//cout << "缺少返回值" << endl;
			int invalid = 0;
			res = ProcessError(symbolList[0], &invalid, &invalid, NO_RETURN, pos);
			res = ProcessError(symbolList[0], &invalid, &invalid, NO_RETURN, pos);
		}
		PopSymbolList(right_size);
		quaternionList.push_back({ nextQuaternionIndex++, "return", "-", "-", symbol_table.tableName });
		symbolList.push_back({ production_left, "", -1, -1 });
		symbolList.push_back({ production_left, "", -1, -1 });
	}
	else
	{
		auto type = symbolTables[0].symbols[symbolTables[0].FindSymbol(symbol_table.tableName)].specifierType;
		SemanticSymbol exp = symbolList[symbol_list_size - 2];

		quaternionList.push_back({ nextQuaternionIndex++, "=", exp.value, "-", type });
		quaternionList.push_back({ nextQuaternionIndex++, "return", type, "-", symbol_table.tableName });

		PopSymbolList(right_size);
		symbolList.push_back({ production_left, exp.value, -1, -1 });
	}
	return res;
}


/**
 * @brief： 翻译关系运算符。
 *          将关系运算符从符号流中提取出来，并更新符号流。
 *          不涉及错误检查，因为关系运算符的合法性由语法分析保证。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含关系运算符。
 * @return  无
 * @term    Relop ::= > | < | >= | <= | == | !=
 */
int  SemanticAnalysis::TranslateRelop(const string production_left, const vector<string> production_right, int pos)
{
	int right_size = production_right.size();
	int symbol_list_size = symbolList.size();
	SemanticSymbol exp = symbolList[symbol_list_size - 1];
	PopSymbolList(right_size);
	symbolList.push_back({ production_left, exp.value, -1, -1 });
	return 1;
}

/**
 * @brief： 翻译if语句。
 *          处理if语句的语义动作，包括回填跳转地址。
 *          根据if语句是否有else部分，回填不同数量的四元式。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，包含if语句的各个组成部分。
 * @return  无
 * @term    IfStmt ::= if IfStmt_m1 ( Exp ) IfStmt_m2 Block IfNext
 */
int SemanticAnalysis::TranslateIfStmt(const string production_left, const vector<string> production_right, int pos)
{
	SemanticSymbol IfStmt_m2 = symbolList[symbolList.size() - 3];
	SemanticSymbol IfNext = symbolList[symbolList.size() - 1];
	SemanticSymbol condition = symbolList[symbolList.size() - 3];
	SemanticSymbol block = symbolList[symbolList.size() - 2];
	SemanticSymbol elsePart = symbolList.back();

	// 如果没有else，只需要回填IfStmt_m2的一个四元式
	if (IfNext.value.empty())
	{
		// 回填假出口，跳出if，这里的next_quaternary_index是整体的下一条
		quaternionList[backpatchingList.back()].result = to_string(nextQuaternionIndex);
		backpatchingList.pop_back();
	}
	else
		// 有else，需要回填两个四元式
	{
		// 回填if块出口，填的是整体出口，对应Block执行完的无条件jump的跳转目标
		quaternionList[backpatchingList.back()].result = to_string(nextQuaternionIndex);
		backpatchingList.pop_back();

		// 回填if假出口
		quaternionList[backpatchingList.back()].result = IfNext.value;
		backpatchingList.pop_back();
	}

	// 修改symbol list
	PopSymbolList(production_right.size());
	symbolList.push_back({ production_left, "", -1, -1 });
	return 1;
#if 0
	int res = 1;

	// 获取条件表达式和主体块
	SemanticSymbol condition = symbolList[symbolList.size() - 3];
	SemanticSymbol block = symbolList[symbolList.size() - 2];
	SemanticSymbol elsePart = symbolList.back();

	// 生成唯一标签
	int falseLabel = nextQuaternionIndex;
	int endLabel = nextQuaternionIndex + 1;

	// 生成条件判断四元式
	quaternionList.emplace_back(nextQuaternionIndex++,
		"jz",           // 如果条件为假则跳转
		condition.value,
		"-",
		to_string(falseLabel));

	// if 块的代码在语法分析时已生成

	// 如果有else部分，需要在if块结束时跳过else块
	if (!elsePart.value.empty()) {
		quaternionList.emplace_back(nextQuaternionIndex++,
			"j",        // 无条件跳转
			"-",
			"-",
			to_string(endLabel));
	}

	// 生成false标签
	quaternionList.emplace_back(nextQuaternionIndex++,
		"label",
		"-",
		"-",
		to_string(falseLabel));

	// else部分的代码在语法分析时已生成

	// 如果有else部分，生成结束标签
	if (!elsePart.value.empty()) {
		quaternionList.emplace_back(nextQuaternionIndex++,
			"label",
			"-",
			"-",
			to_string(endLabel));
	}

	// 处理结果值
	string resultVar;
	if (block.value != "" || elsePart.value != "") {
		// if作为表达式时，需要一个临时变量存储结果
		resultVar = "T" + to_string(tempVarCounter++);
		if (block.value != "") {
			quaternionList.emplace_back(nextQuaternionIndex++,
				"=",
				block.value,
				"-",
				resultVar);
		}
		if (elsePart.value != "") {
			quaternionList.emplace_back(nextQuaternionIndex++,
				"=",
				elsePart.value,
				"-",
				resultVar);
		}
	}

	// 更新符号流
	PopSymbolList(4); // if Exp Block ElsePart
	symbolList.emplace_back(production_left, resultVar, -1, -1);

	return res;
#endif
}

/**
 * @brief： 翻译if语句的标记部分（IfStmt_m1）。
 *          该部分主要用作语法结构的占位符，不涉及具体的语义动作。
 *          因其右侧是空，所以无需从符号流中pop元素。
 *          记录当前下一条四元式的位置，但实际上此位置信息在此上下文中不被使用。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，此处为空。
 * @return  无
 * @term    IfStmt_m1 ::= @
 */
int SemanticAnalysis::TranslateIfStmt_m1(const string production_left, const vector<string> production_right, int pos)
{
	//因为右侧是空，所以不用pop
	symbolList.push_back({ production_left, to_string(nextQuaternionIndex), -1, -1 });
	return 1;
}

/**
 * @brief： 翻译if语句的第二个标记部分（IfStmt_m2）。
 *          用于处理if控制语句中布尔表达式的真假出口逻辑。
 *          生成两个四元式：一个用于布尔表达式假出口的待回填四元式，另一个是真出口的直接跳转。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，通常为空。
 * @return  无
 * @term    IfStmt_m2 ::= @
 */
int SemanticAnalysis::TranslateIfStmt_m2(const string production_left, const vector<string> production_right, int pos)
{
	SemanticSymbol expression = symbolList[symbolList.size() - 1];

	quaternionList.push_back({ nextQuaternionIndex++, "j=", expression.value, "0", "" });
	backpatchingList.push_back(quaternionList.size() - 1);



	quaternionList.push_back({ nextQuaternionIndex++, "j=", "-", "-", to_string(nextQuaternionIndex) });

	// 修改symbol list
	symbolList.push_back({ production_left, to_string(nextQuaternionIndex), -1, -1 });
	return 1;
}

/**
 * @brief： 翻译if语句的else部分。
 *          根据是否存在else部分（由产生式右部长度判断），进行相应的符号流处理。
 *          对于有else部分的情况，更新符号流以反映跳转逻辑；对于无else部分的情况，仅作为占位符。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，可能包含else部分的相关信息。
 * @return  无
 * @term    IfNext ::= @ | IfStmt_next else Block
 */
int SemanticAnalysis::TranslateIfNext(const string production_left, const vector<string> production_right, int pos)
{
	if (production_right.size() == 3)
	{
		SemanticSymbol IfStmt_next = symbolList[symbolList.size() - 3];
		// 修改symbol list
		PopSymbolList(production_right.size());
		symbolList.push_back({ production_left, IfStmt_next.value, -1, -1 });
	}
	else // 是空的情况
	{
		symbolList.push_back({ production_left, "", -1, -1 });
	}
	return 1;
}

/**
*@brief： 翻译if语句第一段执行完之后的下一步操作。
* 添加一个无条件跳转的四元式，其跳转位置在后续步骤中回填。
* 用于处理if语句中的分支跳转逻辑。
* @param： const string production_left 产生式的左部，表示当前处理的语法结构。
* @param： const vector<string> production_right 产生式的右部，此处通常为空。
* @return  无
* @term    IfStmt_next :: = @
*/
int SemanticAnalysis::TranslateIfStmt_next(const string production_left, const vector<string> production_right, int pos)
{
	// 无条件跳转语句
	quaternionList.push_back({ nextQuaternionIndex++, "j", "-", "-", "" });
	backpatchingList.push_back(quaternionList.size() - 1);

	// 修改symbol list,存当前情况下下一条四元式位置
	symbolList.push_back({ production_left, to_string(nextQuaternionIndex), -1, -1 });
	return 1;
}

int SemanticAnalysis::TranslateForStmt(const string production_left, const vector<string> production_right, int pos)
{
	
	SemanticSymbol forstmt_m1 = symbolList[symbolList.size() - 6];
	SemanticSymbol forstmt_m2 = symbolList[symbolList.size() - 2];
	SemanticSymbol exp = symbolList[symbolList.size() - 3];
	SemanticSymbol var = symbolList[symbolList.size() - 5];
	
	// 默认升序
	quaternionList.push_back({ nextQuaternionIndex++, "add", var.value , "1", "T"+to_string(tempVarCounter) });
	quaternionList.push_back({ nextQuaternionIndex++, "=", "T"+to_string(tempVarCounter++), "-", var.value });
	// 无条件跳转到 for 的条件判断语句处
	quaternionList.push_back({ nextQuaternionIndex++, "j", "-", "-", forstmt_m1.value });

	// 回填真出口
	quaternionList[backpatchingList.back()].result = forstmt_m2.value;
	backpatchingList.pop_back();

	// 回填假出口
	quaternionList[backpatchingList.back()].result = to_string(nextQuaternionIndex);
	quaternionList[backpatchingList.back()].operand1 = var.value; 
	quaternionList[backpatchingList.back()].operand2 = exp.value; 
	backpatchingList.pop_back();

	// 回填break
	while (breakList.size() > 0)
	{
		quaternionList[breakList.back()].result = to_string(nextQuaternionIndex);
		breakList.pop_back();
	}

	// 回填continue
	while (continueList.size() > 0)
	{
		quaternionList[continueList.back()].result = forstmt_m1.value;
		continueList.pop_back();
	}

	backpatchingLevel--;

	PopSymbolList(production_right.size());

	symbolList.push_back({ production_left, "", -1, -1 });
	return 1;
}

int SemanticAnalysis::TranslateForStmt_m1(const string production_left, const vector<string> production_right, int pos)
{
	// ForStmt_m1 ::= @
	backpatchingLevel++;
	symbolList.push_back({ production_left, to_string(nextQuaternionIndex), -1, -1 });
	return 1;
}

int SemanticAnalysis::TranslateForStmt_m2(const string production_left, const vector<string> production_right, int pos)
{
	// // ForStmt_m2 ::= @
	// // 假出口
	// SemanticSymbol for_exp = symbolList[symbolList.size() - 1];

	// // 假出口
	// quaternionList.push_back({ nextQuaternionIndex++, "j=", for_exp.value, "0", "" });
	// backpatchingList.push_back(quaternionList.size() - 1);

	// // 修改symbol list
	// symbolList.push_back({ production_left, to_string(nextQuaternionIndex), -1, -1 });
	// return 1;



	// 假出口
	SemanticSymbol for_exp = symbolList[symbolList.size() - 1];

	// 假出口
	quaternionList.push_back({ nextQuaternionIndex++, "j=", for_exp.value, "0", "" });
	backpatchingList.push_back(quaternionList.size() - 1);
	// 真出口
	quaternionList.push_back({ nextQuaternionIndex++, "j", "-", "-", "" });
	backpatchingList.push_back(quaternionList.size() - 1);

	symbolList.push_back({ production_left, to_string(nextQuaternionIndex), -1, -1 });
	return 1;
}


int SemanticAnalysis::TranslateLoopStmt(const string production_left, const vector<string> production_right, int pos)
{
	// LoopStmt ::= LoopStmt_m1 Exp LoopStmt_m2 Block
	SemanticSymbol loopstmt_m = symbolList[symbolList.size() - 2];
	// SemanticSymbol loopstmt_m2 = symbolList[symbolList.size() - 2];

	// 无条件跳转到 Loop 的条件判断语句处
	quaternionList.push_back({ nextQuaternionIndex++, "j", "-", "-", loopstmt_m.value });

	// 回填break
	while (breakList.size() > 0)
	{
		quaternionList[breakList.back()].result = to_string(nextQuaternionIndex);
		breakList.pop_back();
	}

	// 回填continue
	while (continueList.size() > 0)
	{
		quaternionList[continueList.back()].result = loopstmt_m.value;
		continueList.pop_back();
	}

	backpatchingLevel--;

	PopSymbolList(production_right.size());

	symbolList.push_back({ production_left, "", -1, -1 });
	return 1;
}

int SemanticAnalysis::TranslateLoopStmt_m(const string production_left, const vector<string> production_right, int pos)
{
	// LoopStmt_m1 ::= @
	backpatchingLevel++;
	symbolList.push_back({ production_left, to_string(nextQuaternionIndex), -1, -1 });
	return 1;
}


/**
*@brief： 翻译while语句。
* 在while循环结束时添加无条件跳转，返回到循环的条件判断部分。
* 回填循环体的真出口和假出口，以确保循环的正确执行。
* @param： const string production_left 产生式的左部，表示当前处理的语法结构。
* @param： const vector<string> production_right 产生式的右部，包含while语句的各个组成部分。
* @return  无
* @term    WhileStmt :: = while WhileStmt_m1(Exp) WhileStmt_m2 Block
*/
int SemanticAnalysis::TranslateWhileStmt(const string production_left, const vector<string> production_right, int pos)
{
	SemanticSymbol whilestmt_m1 = symbolList[symbolList.size() - 4];
	SemanticSymbol whilestmt_m2 = symbolList[symbolList.size() - 2];

	// 无条件跳转到 while 的条件判断语句处
	quaternionList.push_back({ nextQuaternionIndex++, "j", "-", "-", whilestmt_m1.value });

	// 回填真出口
	quaternionList[backpatchingList.back()].result = whilestmt_m2.value;
	backpatchingList.pop_back();

	// 回填假出口
	quaternionList[backpatchingList.back()].result = to_string(nextQuaternionIndex);
	backpatchingList.pop_back();

	// 回填break
	while (breakList.size() > 0)
	{
		quaternionList[breakList.back()].result = to_string(nextQuaternionIndex);
		breakList.pop_back();
	}

	// 回填continue
	while (continueList.size() > 0)
	{
		quaternionList[continueList.back()].result = whilestmt_m1.value;
		continueList.pop_back();
	}

	backpatchingLevel--;

	PopSymbolList(production_right.size());

	symbolList.push_back({ production_left, "", -1, -1 });
	return 1;
}

/**
 * @brief： 翻译while语句的第一个标记部分（WhileStmt_m1）。
 *          主要用于在进入while循环时，记录当前四元式的索引，以便在循环结束后跳回到循环开始。
 *          同时，增加回填层级以处理循环内部的跳转逻辑。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，此处通常为空。
 * @return  无
 * @term    WhileStmt_m1 ::= @
 */
int SemanticAnalysis::TranslateWhileStmt_m1(const string production_left, const vector<string> production_right, int pos)
{
	backpatchingLevel++;
	symbolList.push_back({ production_left, to_string(nextQuaternionIndex), -1, -1 });
	return 1;
}

/**
 * @brief： 翻译while语句的第二个标记部分（WhileStmt_m2）。
 *          用于处理while循环的条件判断逻辑。
 *          生成两个四元式：一个用于条件为假时的跳转（假出口），另一个是循环体结束后的无条件跳转。
 *          这些四元式的目标地址将在循环结束后进行回填。
 * @param： const string production_left 产生式的左部，表示当前处理的语法结构。
 * @param： const vector<string> production_right 产生式的右部，通常为空。
 * @return  无
 * @term    WhileStmt_m2 ::= @
 */
int SemanticAnalysis::TranslateWhileStmt_m2(const string production_left, const vector<string> production_right, int pos)
{
	// 假出口
	SemanticSymbol while_exp = symbolList[symbolList.size() - 1];

	// 假出口
	quaternionList.push_back({ nextQuaternionIndex++, "j=", while_exp.value, "0", "" });
	backpatchingList.push_back(quaternionList.size() - 1);
	// 真出口
	quaternionList.push_back({ nextQuaternionIndex++, "j", "-", "-", "" });
	backpatchingList.push_back(quaternionList.size() - 1);

	symbolList.push_back({ production_left, to_string(nextQuaternionIndex), -1, -1 });
	return 1;
}
int SemanticAnalysis::TranslateBreakStmt(const string production_left, const vector<string> production_right, int pos)
{
	// break出口
	quaternionList.push_back({ nextQuaternionIndex++, "j", "-", "-", "" });
	//backpatchingList.push_back(quaternionList.size() - 1);
	breakList.push_back(quaternionList.size() - 1);

	PopSymbolList(production_right.size());  

	symbolList.push_back({ production_left, to_string(nextQuaternionIndex), -1, -1 });
	return 1;
}
int SemanticAnalysis::TranslateContinueStmt(const string production_left, const vector<string> production_right, int pos)
{
	// continue出口
	quaternionList.push_back({ nextQuaternionIndex++, "j", "-", "-", "" });
	
	continueList.push_back(quaternionList.size() - 1);

	PopSymbolList(production_right.size());

	symbolList.push_back({ production_left, to_string(nextQuaternionIndex), -1, -1 });
	return 1;
}




int SemanticAnalysis::TranslateElement(const string production_left, const vector<string> production_right, int pos)
{
    int res = 1;
    int rightSize = production_right.size();
    
    // Element -> <INT>
    if (rightSize == 1 && production_right[0] == "<INT>") {
        SemanticSymbol intValue = symbolList.back();
        PopSymbolList(1);
        symbolList.emplace_back(production_left, intValue.value, -1, -1);
    }
    // Element -> <ID>
    else if (rightSize == 1 && production_right[0] == "<ID>") {
        SemanticSymbol id = symbolList.back();
        int tbIndex = -1, tbIndexIndex = -1;
        // 检查变量是否定义
        res = ProcessError(id, &tbIndex, &tbIndexIndex, UNDEFINED_IDENTIFIER, pos);
        PopSymbolList(1);
        symbolList.emplace_back(production_left, id.value, tbIndex, tbIndexIndex);
    }
    // Element -> (Exp)
    else if (rightSize == 3 && production_right[0] == "(" && production_right[2] == ")") {
        SemanticSymbol exp = symbolList[symbolList.size() - 2];
        PopSymbolList(3);
        symbolList.emplace_back(production_left, exp.value, exp.tableIndex, exp.symbolIndex);
    }
    // Element -> <ID> (ArgList)
    else if (rightSize == 4 && production_right[0] == "<ID>") {
        SemanticSymbol funcName = symbolList[symbolList.size() - 4];
        SemanticSymbol argList = symbolList[symbolList.size() - 2];
        
        //// 检查函数是否定义
        
        //
        //
        //// 检查是否是函数类型
        //if (res == 1 && symbolTables[tbIndex].symbols[tbIndexIndex].category != IdentifierInfo::Function) {
        //    res = ProcessError(funcName, &tbIndex, &tbIndexIndex, CALL_NOT_FUNCTION, pos);
        //}
		int tbIndex = -1, tbIndexIndex = -1;
		funcName.tableIndex = 0;
		int i = this->symbolTables[0].FindSymbol(funcName.value);
		if (i == -1)
			res = ProcessError(funcName, &tbIndex, &tbIndexIndex, CALL_FUNCTION_UNDIFINED, pos);
		else
			funcName.symbolIndex = i;
        // 检查参数数量
        if (res == 1) {
			int argCount;
			if (argList.value == "")
				argCount = 0;
			else
				argCount = stoi(argList.value);
            //int argCount = stoi(argList.value);
            int dummy = -1;
            res = ProcessError(funcName, &argCount, &dummy, INVALID_PARAMETER_COUNT, pos);
        }
		
        
        // 生成函数调用的四元式
        string tempVar = "T" + to_string(tempVarCounter++);
        quaternionList.emplace_back(nextQuaternionIndex++, "call", funcName.value, "-", tempVar);
        
        PopSymbolList(4);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
    // Element -> Assignable
    else if (rightSize == 1 && production_right[0] == "Assignable") {
        SemanticSymbol assignable = symbolList.back();
        PopSymbolList(1);
        symbolList.emplace_back(production_left, assignable.value, assignable.tableIndex, assignable.symbolIndex);
    }
    
    return res;
}



int SemanticAnalysis::TranslateMulDivExp(const string production_left, const vector<string> production_right, int pos)
{
	int res = 1;

	// MulDivExp -> Item
	if (production_right.size() == 1) {
		SemanticSymbol item = symbolList.back();
		PopSymbolList(1);
		symbolList.emplace_back(production_left, item.value, item.tableIndex, item.symbolIndex);
	}
	// MulDivExp -> MulDivExp MulDivOp Item
	else if (production_right.size() == 3) {
		SemanticSymbol exp = symbolList[symbolList.size() - 3];  // 第一个操作数
		SemanticSymbol op = symbolList[symbolList.size() - 2];   // 运算符
		SemanticSymbol item = symbolList.back();                 // 第二个操作数

		// 生成临时变量
		string tempVar = "T" + to_string(tempVarCounter++);

		// 根据运算符生成对应的四元式
		string operation;
		if (op.value == "*") {
			operation = "mul";
		}
		else if (op.value == "/") {
			operation = "div";
		}

		// 生成四元式
		quaternionList.emplace_back(nextQuaternionIndex++,
			operation,
			exp.value,
			item.value,
			tempVar);

		// 弹出原符号，压入新符号
		PopSymbolList(3);
		symbolList.emplace_back(production_left, tempVar, -1, -1);
	}

	return res;
}

int SemanticAnalysis::TranslateVarDeclInner(const string production_left, const vector<string> production_right, int pos)
{
	int res = 1;

	// VarDeclInner -> mut <ID>
	if (production_right.size() == 2) {
		SemanticSymbol id = symbolList.back();
		string varName = id.value;

		// 检查变量是否在当前作用域重定义
		//int tbIndex = symbolTables[scopeStack.back()].FindSymbol(varName);
		//if (tbIndex != -1) {
		//	res = ProcessError(id, nullptr, nullptr, IDENTIFIER_REDEFINED, pos);
		//}
		IdentifierInfo variable{ IdentifierInfo::Variable, id.value, id.value };
		symbolTables[scopeStack.back()].InsertSymbol(variable);
		// 创建可变变量的标识符信息
		//IdentifierInfo varInfo(IdentifierInfo::Variable,
		//	"mut",  // 标记为可变类型
		//	varName);

		PopSymbolList(2);  // 弹出 mut 和 <ID>
		symbolList.emplace_back(production_left, varName, scopeStack.back(), -1);
	}

	// VarDeclInner -> <ID>
	else if (production_right.size() == 1) {
		SemanticSymbol id = symbolList.back();
		string varName = id.value;

		//// 检查变量是否在当前作用域重定义
		// 
		//res = ProcessError(varName, &(scopeStack.back()), nullptr, IDENTIFIER_REDEFINED, pos);

		// 之前没有定义，添加变量到符号表
		IdentifierInfo variable{ IdentifierInfo::Variable, id.value, id.value };
		symbolTables[scopeStack.back()].InsertSymbol(variable);
		//int tbIndex = symbolTables[scopeStack.back()].FindSymbol(varName);
		//if (tbIndex != -1) {
		//	res = ProcessError(id, nullptr, nullptr, IDENTIFIER_REDEFINED, pos);
		//}

		// 创建不可变变量的标识符信息
		//IdentifierInfo varInfo(IdentifierInfo::Variable,
		//	"immut",  // 标记为不可变类型
		//	varName);

		PopSymbolList(1);  // 弹出 <ID>
		symbolList.emplace_back(production_left, varName, scopeStack.back(), symbolTables[scopeStack.back()].symbols.back().Variable);
	}

	return res;
}
#if 1
int SemanticAnalysis::TranslateAssignStmt(const string production_left, const vector<string> production_right, int pos)
{
	int res = 1;

	// 获取被赋值的变量和表达式
	SemanticSymbol assignable = symbolList[symbolList.size() - 4]; // Assignable
	SemanticSymbol exp = symbolList[symbolList.size() - 2];        // Exp

	// 检查左值是否已定义
	int tbIndex, tbIndexIndex;
	if (!CheckIdDefine(assignable, &tbIndex, &tbIndexIndex)) {
		res = ProcessError(assignable, nullptr, nullptr, UNDEFINED_IDENTIFIER, pos);
	}
	else {
		// 检查是否为不可变变量
		//IdentifierInfo& varInfo = symbolTables[tbIndex].symbols[tbIndexIndex];
		//if (varInfo.specifierType == "immut") {
		//	// 不可变变量不能被赋值
		//	res = ProcessError(assignable, nullptr, nullptr, SEMANTIC_ERROR_REDEFINED, pos);
		//}
		//else {
			// 生成赋值四元式
		//symbolTables[scopeStack.back()].symbols[pos].Variable=exp.value;
		//变量的值没有修改
			quaternionList.emplace_back(nextQuaternionIndex++,
				"=",
				exp.value,
				"-",
				assignable.value);
		//}
	}

	// 弹出所有相关符号并压入新符号
	PopSymbolList(production_right.size());
	symbolList.emplace_back(production_left, assignable.value, -1, -1);

	return res;
}
#endif

int SemanticAnalysis::TranslateType(const string production_left, const vector<string> production_right, int pos)
{
    int res = 1;
    string typeStr;
    
    // Type -> i32
    if (production_right.size() == 1 && production_right[0] == "i32") {
        typeStr = "i32";
        PopSymbolList(1);
    }
    
    // Type -> [ Type ; <INT> ]
    else if (production_right.size() == 5 && production_right[0] == "[") {
        SemanticSymbol elementType = symbolList[symbolList.size() - 4];
        SemanticSymbol size = symbolList[symbolList.size() - 2];
        typeStr = "[" + elementType.value + ";" + size.value + "]";
        PopSymbolList(5);
    }
    
    // Type -> ( TupleTypeInner )
    else if (production_right.size() == 3 && production_right[0] == "(") {
        SemanticSymbol tupleInner = symbolList[symbolList.size() - 2];
        typeStr = "(" + tupleInner.value + ")";
        PopSymbolList(3);
    }
    
    // Type -> & mut Type
    else if (production_right.size() == 3 && production_right[0] == "&" && production_right[1] == "mut") {
        SemanticSymbol innerType = symbolList.back();
        typeStr = "&mut " + innerType.value;
        PopSymbolList(3);
    }
    
    // Type -> & Type
    else if (production_right.size() == 2 && production_right[0] == "&") {
        SemanticSymbol innerType = symbolList.back();
        typeStr = "&" + innerType.value;
        PopSymbolList(2);
    }
    
    // 将类型信息压入符号栈
    symbolList.emplace_back(production_left, typeStr, -1, -1);
    return res;
}

int SemanticAnalysis::TranslateVarMut(const string production_left, const vector<string> production_right, int pos)
{
    // VarMut -> mut
    if (production_right.size() == 1 && production_right[0] == "mut") {
        PopSymbolList(1);
        symbolList.emplace_back(production_left, "mut", -1, -1);
    }
    // VarMut -> @
    else if (production_right.size() == 1 && production_right[0] == "@") {
        symbolList.emplace_back(production_left, "immut", -1, -1);
    }

    return 1;
}

int SemanticAnalysis::TranslateParam(const string production_left, const vector<string> production_right, int pos)
{
    int res = 1;
    
    // 从符号流中获取参数的各个组成部分
    SemanticSymbol varMut = symbolList[symbolList.size() - 4];  // 可变性
    SemanticSymbol name = symbolList[symbolList.size() - 3];      // 参数名
    SemanticSymbol type = symbolList.back();                    // 参数类型
    
    SemanticSymbolTable& function_table = symbolTables[scopeStack.back()];

	res = ProcessError(name, &(scopeStack.back()), nullptr, PARAMETER_REDEFINED, pos);

	int new_position = function_table.InsertSymbol({ IdentifierInfo::Variable, type.value, name.value, -1, -1, -1 });
	int table_position = symbolTables[0].FindSymbol(function_table.tableName);
	symbolTables[0].symbols[table_position].parameterCount++;

	quaternionList.emplace_back(nextQuaternionIndex++, "defpar", "-", "-", name.value);

	PopSymbolList(production_right.size());
	symbolList.emplace_back(production_left, name.value, scopeStack.back(), new_position);
	return res;
}

int SemanticAnalysis::TranslateAssignable(const string production_left, const vector<string> production_right, int pos)
{
    int res = 1;
    
    // Assignable -> <ID>
    if (production_right.size() == 1 && production_right[0] == "<ID>") {
        SemanticSymbol id = symbolList.back();
        // 检查变量是否定义
        int tbIndex = -1, tbIndexIndex = -1;
        if (!CheckIdDefine(id, &tbIndex, &tbIndexIndex)) {
            res = ProcessError(id, nullptr, nullptr, UNDEFINED_IDENTIFIER, pos);
        }
        
        PopSymbolList(1);
        symbolList.emplace_back(production_left, id.value, tbIndex, tbIndexIndex);
    }
    
    // Assignable -> Element [ Exp ]
    else if (production_right.size() == 4 && production_right[1] == "[") {
        SemanticSymbol array = symbolList[symbolList.size() - 4];
        SemanticSymbol index = symbolList[symbolList.size() - 2];
        
        // 生成数组访问的临时变量
        string tempVar = "T" + to_string(tempVarCounter++);
        quaternionList.emplace_back(nextQuaternionIndex++, 
                                  "array_access", 
                                  array.value, 
                                  index.value, 
                                  tempVar);
        
        PopSymbolList(4);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
    
    // Assignable -> Factor <INT>
    else if (production_right.size() == 2 && production_right[1] == "<INT>") {
        SemanticSymbol factor = symbolList[symbolList.size() - 2];
        SemanticSymbol index = symbolList.back();
        
        string tempVar = "T" + to_string(tempVarCounter++);
        quaternionList.emplace_back(nextQuaternionIndex++, 
                                  "tuple_access", 
                                  factor.value, 
                                  index.value, 
                                  tempVar);
        
        PopSymbolList(2);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
    
    // Assignable -> Factor . <INT>
    else if (production_right.size() == 3 && production_right[1] == ".") {
        SemanticSymbol factor = symbolList[symbolList.size() - 3];
        SemanticSymbol index = symbolList.back();
        
        string tempVar = "T" + to_string(tempVarCounter++);
        quaternionList.emplace_back(nextQuaternionIndex++, 
                                  "tuple_dot_access", 
                                  factor.value, 
                                  index.value, 
                                  tempVar);
        
        PopSymbolList(3);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
    
    // Assignable -> * Factor
    else if (production_right.size() == 2 && production_right[0] == "*") {
        SemanticSymbol factor = symbolList.back();
        
        string tempVar = "T" + to_string(tempVarCounter++);
        quaternionList.emplace_back(nextQuaternionIndex++, 
                                  "deref", 
                                  factor.value, 
                                  "-", 
                                  tempVar);
        
        PopSymbolList(2);
        symbolList.emplace_back(production_left, tempVar, -1, -1);
    }
    
    return res;
}

int SemanticAnalysis::TranslateAddSubOp(const string production_left, const vector<string> production_right, int pos)
{
    // 获取运算符类型
    SemanticSymbol opType = symbolList.back();
	if (opType.value == "+") {
		opType.value = "add";
	}
	else if (opType.value == "-") {
		opType.value = "sub";
	}
    // 更新符号流
    PopSymbolList(1);  // 弹出 + 或 -
    symbolList.emplace_back(production_left, opType.value, -1, -1);
    
    return 1;
}

int SemanticAnalysis::TranslateMulDivOp(const string production_left, const vector<string> production_right, int pos)
{
	// 获取运算符类型
	SemanticSymbol opType = symbolList.back();
	// 更新符号流
	PopSymbolList(1);  // 弹出 + 或 -
	symbolList.emplace_back(production_left, opType.value, -1, -1);

	return 1;
}

int SemanticAnalysis::TranslateCompOp(const string production_left, const vector<string> production_right, int pos)
{

	// 根据比较运算符类型设置操作码
	SemanticSymbol opType = symbolList.back();

	// 更新符号流
	PopSymbolList(1);
	symbolList.emplace_back(production_left, opType.value, -1, -1);

	return 1;
}

int SemanticAnalysis::TranslateArgList(const string production_left, const vector<string> production_right, int pos)
{
	int res = 1;
	string argListStr;
	int argCount = 0;

	// ArgList -> @
	if (production_right.size() == 1 && production_right[0] == "@") {
		argListStr = "";
	}

	// ArgList -> Exp
	else if (production_right.size() == 1 && production_right[0] == "Exp") {
		SemanticSymbol exp = symbolList.back();
		
		// 生成参数传递四元式
		quaternionList.push_back({ nextQuaternionIndex++, "param", exp.value, "-", "-" });
		argCount = 1;
		PopSymbolList(1);
	}

	// ArgList -> Exp , ArgList
	else if (production_right.size() == 3) {
		SemanticSymbol exp = symbolList[symbolList.size() - 3];
		SemanticSymbol restArgs = symbolList.back();

		// 获取剩余参数数量
		int restArgCount = restArgs.value.empty() ? 0 : stoi(restArgs.value);
		argCount = restArgCount + 1;

		// 生成参数传递四元式
		quaternionList.push_back({ nextQuaternionIndex++, "param", exp.value, "-", "-" });

		PopSymbolList(3);
	}

	// 将参数个数作为值传递给上层
	symbolList.emplace_back(production_left, to_string(argCount), -1, -1);

	return res;
}