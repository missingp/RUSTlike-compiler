#include"token.h"
#include<map>
#include<vector>
#include<stdio.h>
#include<Windows.h>
using namespace std;

//缓冲区大小定义
#define BUFFER_SIZE 300

//词法分析相关文件
#define TOKEN_LIST_FILE "results/Token_List.txt" 
#define ERROR_LIST_FILE "error_log/Lexical_Error_List.txt"


//存储每个分析单元的类型和值
typedef struct elem
{
    string type;
    string value;
    int line;
    elem(string tp, string v, int line);//构造函数
}elem;


class Buffer {
public:
    char* buffer;
    int count;
    Buffer() {
        count = 0;
        buffer = new char[BUFFER_SIZE];
    }
    ~Buffer() {
        delete buffer;
    }
};



class lexer :public token {
protected:
    Buffer double_buffer[2];//双缓冲区
    Buffer final_buffer;//当前分析的buffer
    int current_buffer;//当前选中的缓冲区编号
    int comment_flag;//当前注释标志位


    FILE* fcode;
    FILE* ftoken;
    FILE* ferror;

public:
    vector<elem> lexer_res;//存储词法分析的结果
    int error_line;//记录错误行数

    lexer();
    virtual ~lexer();

    int lexicalAnalysis(string filename);//词法分析的主要函数
    void deleComments(int line_count);//清除注释 
    void deleSpaces(int line_count);//删除空格

    bool DFA(int line_count);//状态机 调用结构体的buffer变量 返回的int 转交给kindJudge函数
    bool tokenJudge(char* str,int line_count);//kindJudge函数 判断传入的 str 类型 并将结果转交给 print函数完成输出
    
    
    elem elemGenerate(int kind, char* str,int line_count);//生成语法分析单元，送给语法分析器
};