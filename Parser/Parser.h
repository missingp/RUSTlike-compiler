#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Parser.h"
#include<qevent.h>
#include <string>

#include "Show.h"
#include "LR1.h"
#include"grammar.h"
#include<qcolor.h>




class Parser : public QMainWindow
{
    Q_OBJECT

public:
    Parser(QWidget *parent = nullptr);
    ~Parser();
private:
    Ui::ParserClass* ui;
    QString codeFile;//源码文件名
    QString grammarFile;//文法文件名
    bool errorFlag;
    int errorLine;
    bool loadGrammarFlag;
    bool lexerFlag;
    bool compileFlag;
    bool treeFlag;
    bool dfaFlag;

    Show* pageShow;
    lr1Grammar *my_lr1Grammar;
    lexer *my_lexer;
    void loadCode();//导入源码
    void loadGrammar();//导入文法
    void restart(); //重启编译程序
    void compile(); //开始编译
    void showErrorLine(int n,QColor color);//展示错误行
};
