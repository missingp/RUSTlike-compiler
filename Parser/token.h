#include<iostream>
#include<string>
#include<vector>
#include<Windows.h>
#define _CRT_SECURE_NO_WARNINGS
//#pragma warning(disable: 4996)

using namespace std;

//各种终结符的编号
#define my_Key_Word  1
#define my_SignWord 2
#define my_Integer 3
#define my_FloatPoint 4
#define my_MonocularOperator 5    // 单目运算符
#define my_BinocularOperator 6    // 双目运算符
#define my_Delimiter 7            // 界符
#define my_WrongWord 8            // 错误
#define my_Blank 9                // 空格
#define my_Separator 10           // 分隔符
#define my_BracketsLeft 11        // 左括号
#define my_BracketsRight 12       // 右括号
#define my_BracketsLeftBig 13     // 左大括号
#define my_BracketsRightBig 14    // 右大括号
#define my_End 15                 // 结束符
#define my_Str 16                 //字符串
#define my_Char 17                   //字符
#define my_Brackets_Left_Square 18   //左方括号
#define my_Brackets_Right_Square 19   //右方括号
#define my_Point_Arrow 20             //箭头
//#define my_Region 21                  //域符
//#define my_Region_Xigou 22            //析构符
#define my_Point 23                   //点符
#define my_Colon 24                   //冒号
//#define my_Question_Mark 25
#define my_DoublePoint 26             //双点符号

//特殊符号集大小
#define  KEY_WORD_SIZE  13
#define  MONOCULAR_OPERATOR_SIZE  5
#define  BINOCULAR_OPERATOR_SIZE  6

//定义各种token的种类   //关键字


// const string Key_Word[KEY_WORD_SIZE] = {"break","case","char","class","continue","do","default","double","define",
// "else","float","for","if","int","include","long","main","return","switch","typedef","void","unsigned","while","iostream" };//24个
const string  rust_keywords[] = {
    "i32" , "let" , "if" , "else" , "while" , "return" , "mut" , "fn" , "for" , "in" , 
    "loop" , "break" , "continue" 
    
};


const char Delimiter = ';' ; //界符
const char Seprater = ',';
const char Brackets_Left = '(';
const char Brackets_Right = ')';
const char Brackets_Left_Big = '{';
const char Brackets_Right_Big = '}';
const char End = '#';
const char Point = '.';
const char Brackets_Left_Square = '[';
const char Brackets_Right_Square = ']';
const char Point_Arrow[5] = "->";
const char Region[5] = "::";
const char Region_Xigou[5] = "::~";
const char Colon = ':';
const char Question_Mark = '?';


// const string Monocular_Operator[MONOCULAR_OPERATOR_SIZE] = {"+","-","*","/","!","%","~","&","|","^", "=" ,">","<"};   //单目运算符 13个
// const string Binocular_Operator[BINOCULAR_OPERATOR_SIZE]={"++","--","&&","||","<=","!=","==",">=","+=","-=","*=","/=","<<",">>"}; //双目运算符 14个


const string Monocular_Operator[MONOCULAR_OPERATOR_SIZE] = {"+", "-", "*", "/", "="};  //单目运算符 5个
const string Binocular_Operator[BINOCULAR_OPERATOR_SIZE] = {"==", ">", ">=", "<", "<=", "!="}; //双目运算符 6个


/****************************************
 * token 类
 * 完成终结符基本的类型判断
 * ***************************************/

class token {
public:
    
    virtual ~token();     // 虚析构函数，允许继承
    int toState(char c);// 判断当前DFA要转向的状态
    bool spaceRemovable(char c);//判断空格能否删除


    //符号类型判断
    int isDelimiter(char c);//界符
    int isDelimiter(string c);//界符(重载，判断字符数组)
    int isSeparator(char c);//分隔符
    int isBracketsLeft(char c);//左括号
    int isBracketsRight(char c);//右括号
    int isBracketsLeftBig(char c);//左大括号
    int isBracketsRightBig(char c);//右大括号
    int isPoint(char c);//.

    int isPointArrow(string str);//->
    int isDoublePoint(string str);//..

    int isBracketsLeftSquare(char c);//[
    int isBracketsRightSquare(char c);//]

    int isRegion(string str);//::
    int isRegionXigou(string str);//::~
    int isColon(char c);//:
    int isEnd(char c);//结束符
    int isStr(string str);//字符串
    int isChar(string str);//字串是字符
    int isInt(string str);//整型

    int isFloat(string str);//float 型 +-xx.xx e +-xx.xx
    int hasDot(string str);//float型 +-xx.xx
    
    int isSignWord(string str);// 标识符 
    int isKey_Word(string str);//保留字  关键字 


    int isBinocularOperator(string str);//判断双目运算符
    int isMonocularOperator(string str);//判断单目运算符

    int isBlank(string str);//判断空格
    int isQuestion_Mark(char c);
};