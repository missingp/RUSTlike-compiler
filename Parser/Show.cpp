#include "Show.h"
#include <qdebug.h>
#include"myQGraphicsView.h"
#include <qmessagebox.h>
#include<fstream>
#include<sstream>
#include<qfile.h>
#include<qtextbrowser.h>
#include<qtablewidget.h>
#include<qpixmap.h>




Show::Show(QWidget* parent) : QMainWindow(parent)
{
    this->layout =NULL;
    this->ui = new Ui::ShowWindow;
    this->ui->setupUi(this);
}


Show::~Show()
{
}

/**
* @func:   展示产生的token的列表
* @para:   
* @return: 
*/
void Show::showToken()
{

    this->ui->centralwidget->setWindowTitle("Token显示");

    QTextBrowser* textBrowser = new QTextBrowser(this->ui->centralwidget);
    // 设置 TextBrowser 的内容
    QFile file(TOKEN_LIST_FILE);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        // 读取文件内容并写入 TextBrowser
        textBrowser->setPlainText(in.readAll());
        file.close();
    }
    else {
        // 处理文件打开失败的情况
        textBrowser->setPlainText("Error: Unable to open the file.");
    }
    if (this->layout != NULL) {
        delete this->layout;
    }
    this->layout = new QVBoxLayout(this->ui->centralwidget);

    // 将 TextBrowser 添加到布局中
    this->layout->addWidget(textBrowser);
    // 设置主窗口的布局
    this->ui->centralwidget->setLayout(this->layout);
}
  
/**
* @func:   展示词法DFA
* @para:
* @return:
*/
void Show::showDFA()
{
    
    // 创建一个 QGraphicsView
    myQGraphicsView* view = new myQGraphicsView(this->ui->centralwidget);
    // 创建一个 QGraphicsScene
    QGraphicsScene* scene = new QGraphicsScene(this->ui->centralwidget);

    QPixmap pixmap(DFA_PNG_FILE);

    scene->addPixmap(pixmap);
    view->setDragMode(QGraphicsView::ScrollHandDrag);// 设置鼠标拖拽格式

    // 将场景设置给 QGraphicsView
    view->setScene(scene);

    if (this->layout != NULL) {
        delete this->layout;
    }
    // 创建一个垂直布局管理器
    this->layout = new QVBoxLayout(this->ui->centralwidget);

    // 将 QGraphicsView 添加到布局中
    this->layout->addWidget(view);

    // 设置主窗口的布局
    this->ui->centralwidget->setLayout(this->layout);
}

/**
* @func:   //展示Action goto表
* @para:
* @return:
*/
void Show::showTable()
{
    this->ui->centralwidget->setWindowTitle("Action Goto表显示");
    // 创建 QTableWidget 部件
    QTableWidget* tableWidget = new QTableWidget();

    if (this->layout != NULL) {
        delete this->layout;
    }
    this->layout = new QVBoxLayout(this->ui->centralwidget);
    


    // 打开CSV文件
    QFile file(Table_FILE);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);

        // 读取文件内容并解析为行
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(',');
            //qDebug() << line<<endl;

            // 在QTableWidget中插入一行
            int row = tableWidget->rowCount();
            tableWidget->insertRow(row);
           

            // 在当前行的每一列中插入数据
            tableWidget->setColumnCount(fields.size());
            for (int column = 0; column < fields.size(); column++) {
               // qDebug() << fields.at(column);
                QTableWidgetItem* item = new QTableWidgetItem(fields.at(column));
                tableWidget->setItem(row, column, item);
            }
           // tableWidget->resizeColumnsToContents();
        }
        tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

        file.close();
    }
    else {
        // 处理文件打开失败的情况
        QTableWidgetItem* errorItem = new QTableWidgetItem("Error: Unable to open the file.");
        tableWidget->setItem(0, 0, errorItem);
    }

    // 将 QTableWidget 添加到布局中
    this->layout->addWidget(tableWidget);

    // 设置主窗口的布局
    this->ui->centralwidget->setLayout(this->layout);
}

/**
* @func:   //展示语法分析树
* @para:
* @return:
*/
void Show::showTree()
{
    this->ui->centralwidget->setWindowTitle("Action Goto表显示");
    // 创建一个 QGraphicsView
    myQGraphicsView* view = new myQGraphicsView(this->ui->centralwidget);

    // 创建一个 QGraphicsScene
    QGraphicsScene* scene = new QGraphicsScene(this->ui->centralwidget);
   
    QPixmap pixmap(TREE_PNG_FILE);

    scene->addPixmap(pixmap);
    view->setDragMode(QGraphicsView::ScrollHandDrag);// 设置鼠标拖拽格式

    // 将场景设置给 QGraphicsView
    view->setScene(scene);

    if (this->layout != NULL) {
        delete this->layout;
    }
    // 创建一个垂直布局管理器
    this->layout = new QVBoxLayout(this->ui->centralwidget);

    // 将 QGraphicsView 添加到布局中
    this->layout->addWidget(view);

    // 设置主窗口的布局
    this->ui->centralwidget->setLayout(this->layout);

}

/**
* @func:   //展示语法DFA
* @para:
* @return:
*/
void Show::showParseDFA()
{
   
    
    this->ui->centralwidget->setWindowTitle("LR(1) DFA显示");
    // 创建一个 QGraphicsView
    myQGraphicsView* view = new myQGraphicsView(this->ui->centralwidget);

    // 创建一个 QGraphicsScene
    QGraphicsScene* scene = new QGraphicsScene(this->ui->centralwidget);

    QPixmap pixmap(PARSE_DFA_PNG_FILE);

    scene->addPixmap(pixmap);
    view->setDragMode(QGraphicsView::ScrollHandDrag);// 设置鼠标拖拽格式

    // 将场景设置给 QGraphicsView
    view->setScene(scene);

    if (this->layout != NULL) {
        delete this->layout;
    }
    // 创建一个垂直布局管理器
    this->layout = new QVBoxLayout(this->ui->centralwidget);

    // 将 QGraphicsView 添加到布局中
    this->layout->addWidget(view);

    // 设置主窗口的布局
    this->ui->centralwidget->setLayout(this->layout);

}

/**
* @func:   //展示归约过程
* @para:
* @return:
*/
void Show::showProcess()
{
    this->ui->centralwidget->setWindowTitle("归约过程显示");
    QTextBrowser* textBrowser = new QTextBrowser(this->ui->centralwidget);
    // 设置 TextBrowser 的内容
    QFile file(REDUCTION_PROCESS_FILE);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        // 读取文件内容并写入 TextBrowser
        textBrowser->setPlainText(in.readAll());
        file.close();
    }
    else {
        // 处理文件打开失败的情况
        textBrowser->setPlainText("Error: Unable to open the file.");
    }
    if (this->layout != NULL) {
        delete this->layout;
    }
    this->layout = new QVBoxLayout(this->ui->centralwidget);

    // 将 TextBrowser 添加到布局中
    this->layout->addWidget(textBrowser);

    // 设置主窗口的布局
    this->ui->centralwidget->setLayout(this->layout);

}