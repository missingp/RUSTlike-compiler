#include "Parser.h"
#include <fstream>
#include<qdebug.h>
#include<qmessagebox.h>
#include<qfiledialog.h>
#include<fstream>
#include<cstdio>
#include<sstream>
#include<qfile.h>
#include<cstdlib>
#include<stdio.h>
#include<Windows.h>
#include<ctime>
//#include"Show.h"


#define DEFAULT_GRAMMAR "Grammar.txt"
#define CODE_FILE "Input_Code.txt"
//#define QUESTION_PATH "question.txt"

//lexer
#define SUCCESS 1
#define CODE_FILE_OPEN_ERROR 2
#define LEXICAL_ANALYSIS_ERROR 3

//grammar
#define GRAMMAR_FILE_OPEN_ERROR 2
#define GRAMMAR_ERROR 3
#define GRAMMAR_ERROR_FILE_OPEN_ERROR 4

//lr1
#define ACTION_ERROR 2
#define GOTO_ERROR  3
#define REDUCTION_PROCESS_FILE_OPEN_ERROR 4
#define PARSE_ERROR_FILE_OPEN_ERROR 5
#define TREE_DOT_FILE_OPEN_ERROR 6


/**
* @func:   Parser构造函数
* @para:
* @return:
*/
Parser::Parser(QWidget* parent) : QMainWindow(parent)
{

	this->ui = new Ui::ParserClass;
	this->ui->setupUi(this);
	//resize(800, 600);
	this->compileFlag = 0;
	this->loadGrammarFlag = 0;
	this->treeFlag = 0;
	this->lexerFlag = 0;
	this->errorFlag = 0;
	this->errorLine = 0;
	this->dfaFlag = 0;


	//this->lr1Grammar=new lr1Grammar(DEFAULT_GRAMMAR);//先加载初始语法文件
	this->codeFile = "";//codefile为空
	this->pageShow = new Show;

	this->my_lr1Grammar = new lr1Grammar;

	this->grammarFile = DEFAULT_GRAMMAR;


	int ans = readGrammar();
	if (ans == 0) {
		QString tip = "编译失败!!!";
		this->ui->textBrowser->append(tip);
	}




	//导入代码或者文法
	connect(this->ui->action_Code, &QAction::triggered, this, &Parser::loadCode);//记得在输出窗口print结果,状态转换
	connect(this->ui->action_Grammar, &QAction::triggered, this, &Parser::loadGrammar);//写到my_grammar.txt里

	//编译或者重置
	connect(this->ui->action_Run, &QAction::triggered, [=]()
		{
			if (this->grammarFlag == 0) {
				QMessageBox::critical(this, "错误", "没有支持编译的有效文法");
			}
			else {
				//文法文件
				if (this->loadGrammarFlag == 0) {
					if (QMessageBox::No == QMessageBox::question(this, "警告", "当前还没有导入文法文件，确定使用默认文件" + QString::fromLocal8Bit(DEFAULT_GRAMMAR), QMessageBox::Yes | QMessageBox::No)) {
						return;
					}
				}
				this->compile();
			}
		});//记得在输出窗口print结果,状态转换
	connect(this->ui->action_Restart, &QAction::triggered, this, &Parser::restart);//写到my_grammar.txt里 

	//展示词法dfa
	connect(this->ui->action_DFA, &QAction::triggered, [=]() {
		// 显示站点修改页面 展示页面
		if (this->lexerFlag == 0) {
			QMessageBox::critical(this, "错误", "请先完成编译");
		}
		else {
			this->pageShow->showDFA();
			this->pageShow->show();
		}
		});


	// 展示action goto表
	connect(this->ui->action_Table, &QAction::triggered, [=]() {
		// 显示站点修改页面 展示页面
		if (this->compileFlag == 0) {
			QMessageBox::critical(this, "错误", "请先完成编译");
		}
		else {
			this->pageShow->showTable();
			//this->pageShow->showloading();
			this->pageShow->show();
		}
		});


	// 展示词法token
	connect(this->ui->action_Token, &QAction::triggered, [=]() {
		// 显示站点修改页面 展示页面
		if (this->lexerFlag == 0) {
			QMessageBox::critical(this, "错误", "请先完成编译");
		}
		else {
			this->pageShow->showToken();
			this->pageShow->show();
		}
		});

	// 展示规约过程
	connect(this->ui->action_Process, &QAction::triggered, [=]() {
		// 显示站点修改页面 展示页面
		if (this->compileFlag == 0) {
			QMessageBox::critical(this, "错误", "请先完成编译");
		}
		else {
			this->pageShow->showProcess();
			//this->pageShow->showloading();
			this->pageShow->show();
		}
		});


	// 展示语法分析树
	connect(this->ui->action_Tree, &QAction::triggered, [=]() {
		// 显示站点修改页面 展示页面
		if (this->compileFlag == 0) {
			QMessageBox::critical(this, "错误", "请先完成编译");
		}
		else if (this->treeFlag == 0) {
			QMessageBox::critical(this, "错误", "请先安装graphviz");
		}
		else {
			this->pageShow->showTree();
			this->pageShow->show();
		}
		});

	// 展示语法分析树
	connect(this->ui->actionLR_1_DFA, &QAction::triggered, [=]() {
		// 显示站点修改页面 展示页面
		if (this->compileFlag == 0) {
			QMessageBox::critical(this, "错误", "请先完成编译");
		}
		else if (this->dfaFlag == 0) {
			QMessageBox::critical(this, "错误", "语法DFA还未生成");
		}
		else {
			this->pageShow->showParseDFA();
			this->pageShow->show();
		}
		});

	// 展示中间代码生成的四元式
	connect(this->ui->action_Quaternion, &QAction::triggered, [=]() {
		// 显示站点修改页面 展示页面
		if (this->compileFlag == 0) {
			QMessageBox::critical(this, "错误", "请先完成编译");
		}
		else {
			this->pageShow->showQuaternion();
			this->pageShow->show();
		}
		});





	QString startTip = "*************************\n";
	startTip += "LR(1)词法语法分析器启动\n";
	startTip += "*************************\n";
	startTip += "请导入文法文件和并输入/导入源代码";


	this->ui->textBrowser->setText(startTip);





}

Parser::~Parser()
{

}

/**
* @func:   导入文法
* @para:
* @return:
*/
void Parser::loadGrammar()
{
	if (QMessageBox::Yes == QMessageBox::question(this, "提示", "导入文法时间可能较长，请耐心等待？", QMessageBox::Yes | QMessageBox::No))
	{
		QString fileName = QFileDialog::getOpenFileName(this, "导入文件", ".", "files(*.txt)");
		if (!fileName.isEmpty()) {




			this->grammarFile = fileName;
			this->loadGrammarFlag = 1;
			this->ui->textBrowser->append("\n文法文件导入成功");
			delete this->my_lr1Grammar;
			this->my_lr1Grammar = new lr1Grammar;
			int ans = readGrammar();
			if (ans == 0) {
				QString tip = "编译失败!!!";
				this->ui->textBrowser->append(tip);
			}
			QMessageBox::information(this, "提示", "导入文法文件成功");

		}
		else {
			this->ui->textBrowser->append("\n空文件，导入失败");
			QMessageBox::critical(this, "错误", "导入文法文件失败");
		}


	}

}

/**
* @func:   导入代码到文本框中
* @para:
* @return:
*/
void Parser::loadCode()
{

	QString fileName = QFileDialog::getOpenFileName(this, "导入文件", ".", "files(*.txt *.cpp *.c)");
	if (!fileName.isEmpty()) {
		this->codeFile = fileName;
		fstream fcode;
		fcode.open((string)fileName.toLocal8Bit());
		if (fcode.is_open() == 0) {
			QString tip = "\n源码文件" + fileName + "无法打开";
			this->ui->textBrowser->append(tip);
			QMessageBox::critical(this, "错误", "源码文件无法打开");
			return;
		}
		//清空
		this->ui->textEdit->clear();
		while (!fcode.eof() && fcode) {
			string line;
			getline(fcode, line);
			this->ui->textEdit->append(QString::fromLocal8Bit(line.c_str()));
		}
		fcode.close();
		this->ui->textBrowser->append("\n成功导入源码文件");
	}
	else {
		this->ui->textBrowser->append("\n空文件，导入失败");
		QMessageBox::critical(this, "错误", "导入源代码文件失败");
	}
}

/**
* @func:   开始编译
* @para:
* @return:
*/
void Parser::compile()
{
	time_t start, end;
	start = clock();

	this->compileFlag = 0;
	this->treeFlag = 0;
	this->lexerFlag = 0;
	if (this->errorFlag == 1) {
		this->showErrorLine(this->errorLine, Qt::white);
	}

	//读文本输入框的内容
	QString codeInput = this->ui->textEdit->toPlainText();
	if (codeInput.size() > 0) {
		// 数据提取
		QFile file(CODE_FILE);
		if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QTextStream out(&file);
			out << codeInput;
			file.close();
			this->ui->textBrowser->append("\n源代码已写入到文件Input_Code.txt中");
		}
		else {
			QMessageBox::warning(this, "警告", "代码保存失败");
		}
	}
	else {
		this->ui->textBrowser->append("\n当前没有导入或者输入源代码");
		QMessageBox::critical(this, "错误", "没有导入或输入代码");
		return;
	}


	//词法分析
	QString tip = "正在进行词法分析，请稍后...\n";
	this->ui->textBrowser->append(tip);
	this->my_lexer = new lexer;

	int lexer_error = this->my_lexer->lexicalAnalysis(CODE_FILE);

	if (lexer_error == SUCCESS) {
		tip = "词法分析无误\n";
	}
	else if (lexer_error == LEXICAL_ANALYSIS_ERROR) {
		end = clock();

		tip = "词法分析错误,错误日志文件内容如下\n";
		this->ui->textBrowser->append(tip);
		this->showErrorLine(this->my_lexer->error_line, Qt::red);
		this->errorLine = this->my_lexer->error_line;
		this->errorFlag = 1;
		//把记录错误的文件读出来
		QFile file(ERROR_LIST_FILE);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream in(&file);

			// 读取文件内容并写入 TextBrowser
			this->ui->textBrowser->append(in.readAll());
			file.close();
		}
		else {
			this->ui->textBrowser->append("Error: Unable to open the file.");
			return;
		}
		printTime(int(end - start));
		QMessageBox::critical(this, "错误", "词法分析失败，编译失败!");
		tip += "编译失败!!!";
		this->ui->textBrowser->append(tip);

		return;
	}
	else {
		this->ui->textBrowser->append("\n源代码文件导入失败");
		QMessageBox::critical(this, "错误", "导入源代码文件失败");
		return;
	}
	tip += "词法分析已结束，可在当前目录下查看\n词法分析结果文件results/Token_List.txt\n";
	this->ui->textBrowser->append(tip);
	this->lexerFlag = 1;



	tip = "\n正在进行语法分析，请稍后...\n";
	this->ui->textBrowser->append(tip);

	int lr1_error = my_lr1Grammar->reduction(my_lexer->lexer_res);
	int ferror = my_lr1Grammar->semantic_analysis.PrintQuaternion(QUATERNION_FILE);
	end = clock();

	if (ferror != 1) {
		tip = "存储语义分析结果的文件无法打开\n";
		this->ui->textBrowser->append(tip);
		QMessageBox::critical(this, "警告", "存储语义分析结果的文件无法打开，存储失败");
		return;
	}
	if (lr1_error == ACTION_ERROR) {
		tip = "文法分析过程中ACTION表查找出错\n";
		this->ui->textBrowser->append(tip);
		QMessageBox::critical(this, "错误", "ACTION表查找失败");
	}
	else if (lr1_error == GOTO_ERROR) {
		tip = "文法分析过程中GOTO表查找出错\n";
		this->ui->textBrowser->append(tip);
		QMessageBox::critical(this, "错误", "GOTO表查找失败");
	}
	else if (lr1_error == REDUCTION_PROCESS_FILE_OPEN_ERROR) {
		this->ui->textBrowser->append("\n规约过程记录失败");
		QMessageBox::warning(this, "警告", "规约过程记录失败");
		return;
	}
	else if (lr1_error == PARSE_ERROR_FILE_OPEN_ERROR) {
		this->ui->textBrowser->append("\n语法分析错误记录失败");
		QMessageBox::warning(this, "警告", "语法分析错误记录失败");
		return;
	}
	else if (lr1_error == SEMANTIC_ERROR_FILE_NOT_OPEN) {
		this->ui->textBrowser->append("\n语义分析错误记录失败");
		QMessageBox::warning(this, "警告", "语义分析错误记录失败");
		return;
	}
	else if (lr1_error > 10) {
		tip = "语义分析出错\n";
		this->ui->textBrowser->append(tip);
		QMessageBox::critical(this, "错误", "语义分析出现错误");
	}
	else {

		tip = "语法分析已结束，可在当前目录下查看：\n文法归约过程文件results/Reduction_Process.txt\n\n";
		printTime(int(end - start));
		tip += "编译成功！！！\n";
		this->ui->textBrowser->append(tip);

		this->compileFlag = 1;
		this->errorFlag = 0;

		if (lr1_error == TREE_DOT_FILE_OPEN_ERROR) {
			this->ui->textBrowser->append("\n语法树保存失败");
			QMessageBox::warning(this, "警告", "语法树保存失败");
			return;
		}
		else {
			const char* command = "dot --version";
			// 使用 popen 执行命令并获取输出
			FILE* fp = _popen(command, "r");
			if (fp == nullptr) {
				QMessageBox::warning(this, "警告", "没有安装graphviz,无法生成语法分析树");
				return;
			}
			// 读取输出并打印
			const int bufferSize = 128;
			char buffer[bufferSize];
			while (fgets(buffer, bufferSize, fp) != nullptr) {

			}
			if (buffer[0] != 'd') {
				QMessageBox::warning(this, "警告", "没有安装graphviz,无法生成语法分析树");
				return;
			}

			//cout << "***";

			_pclose(fp);

			system("dot -Tpng results/Parse_Tree.dot -o picture/Parse_Tree.png");
			tip = "语法分析树已生成，可查看picture/Parse_Tree.png。";
			this->ui->textBrowser->append(tip);
			this->treeFlag = 1;
		}

		if (my_lr1Grammar->printParseDFA() != SUCCESS) {
			this->ui->textBrowser->append("\nLR(1)文法DFA保存失败");
			QMessageBox::warning(this, "警告", "LR(1)文法DFA保存失败");
		}
		else {

			if (QMessageBox::Yes == QMessageBox::question(this, "提示", "语法DFA生成时间较长，确定继续生成？", QMessageBox::Yes | QMessageBox::No))
			{

				tip = "正在生成DFA...";
				this->ui->textBrowser->append(tip);
				system("bin/dot -Tpng results/Parse_DFA.dot -o picture/Parse_DFA.png");

				tip = "语法分析DFA已生成，可查看picture/Parse_DFA.png。";
				this->ui->textBrowser->append(tip);
				this->dfaFlag = 1;
			}
		}

	}
	//std：：cout << "lr1" << lr1_error;
	if (lr1_error != SUCCESS) {
		this->showErrorLine(this->my_lr1Grammar->error_line, Qt::red);
		this->errorLine = this->my_lr1Grammar->error_line;
		this->errorFlag = 1;
		QFile file(PARSE_ERROR_FILE);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream in(&file);
			// 读取文件内容并写入 TextBrowser
			this->ui->textBrowser->append(in.readAll());
			file.close();
		}
		else {

			this->ui->textBrowser->append("Error: Unable to open the file.");
			return;
		}
		if (lr1_error > 10) {
			QFile file2(SEMANTIC_ERROR_FILE);

			if (file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
				QTextStream in(&file2);
				// 读取文件内容并写入 TextBrowser
				this->ui->textBrowser->append(in.readAll());
				file2.close();
			}
			else {
				this->ui->textBrowser->append("Error: Unable to open the file.");
				return;
			}
		}
		printTime(int(end - start));
		tip = "编译失败!!!";
		this->ui->textBrowser->append(tip);

		return;
	}


}
int Parser::readGrammar()
{
	this->grammarFlag = 0;
	//文法分析
	QString tip = "正在读取文法文件，请稍后...\n";
	this->ui->textBrowser->append(tip);

	int grammar_error = this->my_lr1Grammar->ReadGrammar((string)this->grammarFile.toLocal8Bit());

	if (grammar_error == SUCCESS) {
		tip = "文法文件无误\n";
		this->ui->textBrowser->append(tip);
	}
	else if (grammar_error == GRAMMAR_ERROR) {
		tip = "文法错误,错误日志文件内容如下\n";
		this->ui->textBrowser->append(tip);
		//把记录错误的文件读出来
		QFile file(GRAMMAR_ERROR_FILE);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream in(&file);
			// 读取文件内容并写入 TextBrowser
			this->ui->textBrowser->append(in.readAll());
			file.close();
		}
		else {
			this->ui->textBrowser->append("Error: Unable to open the file.");
			return 0;
		}
		QMessageBox::critical(this, "错误", "文法错误!");
		return 0;
	}
	else if (grammar_error == GRAMMAR_FILE_OPEN_ERROR) {
		this->ui->textBrowser->append("\n文法错误信息记录失败");
		QMessageBox::warning(this, "错误", "文法错误信息记录失败");
		return 0;
	}
	else {
		this->ui->textBrowser->append("\n文法文件读入失败");
		QMessageBox::critical(this, "错误", "读入文法文件失败");
		return 0;
	}
	this->my_lr1Grammar->initClosure0();
	this->my_lr1Grammar->generateDFA();
	this->my_lr1Grammar->generateACTION_GOTO();
	this->my_lr1Grammar->printACTION_GOTO();

	tip = "文法已读入，可在当前目录下查看：\n1.拓广文法results/Extended_Grammar.txt\n2.文法的Fisrt集results/First_Set.txt\n3.Action Goto表results/Tables.csv";
	this->ui->textBrowser->append(tip);
	this->grammarFlag = 1;
	return 1;
}



void Parser::printTime(int ms)
{
	struct tm cur_time;
	time_t now;
	time(&now);
	localtime_s(&cur_time, &now);

	string allt = "compile time consumed: " + to_string(ms) + "ms" + "\ncompile moment: " + to_string((int)cur_time.tm_year % 100 + 2000) +
		"/" + to_string((int)(cur_time.tm_mon + 1)) + "/" + to_string((int)cur_time.tm_mday) + "   " + to_string((int)cur_time.tm_hour) + ":"
		+ to_string((int)cur_time.tm_min) + ":" + to_string((int)cur_time.tm_sec);
	//cout << allt;
	QString tip = QString::fromLocal8Bit(allt.c_str());
	this->ui->textBrowser->append(tip);
}


/**
* @func:   重启
* @para:
* @return:
*/
void Parser::restart()
{
	if (QMessageBox::No == QMessageBox::question(this, "警告", "重启时间较长(约10s)，确定重启？", QMessageBox::Yes | QMessageBox::No)) {
		return;
	}
	//清空
	this->ui->textBrowser->clear();
	this->ui->textEdit->clear();
	this->grammarFile = DEFAULT_GRAMMAR;
	this->loadGrammarFlag = 0;
	this->codeFile = "";
	this->compileFlag = 0;
	this->treeFlag = 0;
	this->lexerFlag = 0;
	this->errorFlag = 0;
	this->dfaFlag = 0;

	QString startTip = "*************************\n";
	startTip += "LR(1)词法语法分析器启动\n";
	startTip += "*************************\n";
	startTip += "请导入文法文件和并输入/导入源代码";

	this->ui->textBrowser->setText(startTip);
	delete this->my_lr1Grammar;
	this->my_lr1Grammar = new lr1Grammar;
	int ans = readGrammar();
	if (ans == 0) {
		QString tip = "编译失败!!!";
		this->ui->textBrowser->append(tip);
	}
}


/**
* @func:   展示错误的位置
* @para:   int n 行数
*          QColor color要展示的颜色
* @return:
*/
void Parser::showErrorLine(int n, QColor color)
{

	// 获取第三行的位置
	QTextCursor cursor = this->ui->textEdit->textCursor();
	cursor.movePosition(QTextCursor::Start);
	for (int i = 0; i < n - 1; ++i) {
		cursor.movePosition(QTextCursor::NextBlock);
	}

	QTextBlockFormat blockFormat = cursor.blockFormat();
	blockFormat.setBackground(color);

	// 应用格式到整行
	cursor.setBlockFormat(blockFormat);
}