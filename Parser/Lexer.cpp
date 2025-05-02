#include "lexer.h"
#include<qdebug.h>


#define SUCCESS 1
#define CODE_FILE_OPEN_ERROR 2
#define LEXICAL_ANALYSIS_ERROR 3


/**
 * @brief： elem的构造函数
 * @param： string tp elem的类型
 *          string v  elem的值
 * @return 
 */
elem::elem(string tp, string v,int line_count)
{
    this->type = tp;//类型
    this->value = v;//值
    this->line = line_count;//所在行数
}

/**
 * @brief： lexer的构造函数
 * @param： 
 * @return
 */
lexer::lexer()
{
    this->current_buffer = 0;
    this->comment_flag = 0;
}

lexer::~lexer()
{

}



/**
 * @brief： 语法分析器
 * @param： string file_name  源代码文件
 * @return  分析结果
 */
int lexer::lexicalAnalysis(string file_name) {
  
    if ((this->fcode = fopen(file_name.c_str(), "r")) == NULL)
    {
        return  CODE_FILE_OPEN_ERROR;
    }

    this->ftoken = fopen(TOKEN_LIST_FILE, "w");//存储词法分析结果
    fprintf(this->ftoken, "Token类型    Token值      所在行数\n");
    this->ferror = fopen(ERROR_LIST_FILE, "w");
    
    char c = '\0';
    int buffer_flag = 0;//缓冲区是否需要轮转
    int line_count = 0;//记录当前行数

    while (1)
    {
        c = fgetc(this->fcode);
        if (c == EOF)//代码读取结束
        {
            line_count++;
            this->deleComments(line_count);// 删除注释
            this->deleSpaces(line_count); // 删除多余空格
            if (this->double_buffer[this->current_buffer].count > 0)
            {
                strcpy(this->final_buffer.buffer, this->double_buffer[this->current_buffer].buffer);
                this->final_buffer.count = this->double_buffer[this->current_buffer].count;
                //进入状态机处理 
                this->double_buffer[current_buffer].count = 0;
                if (!this->DFA(line_count)) {
                    fclose(fcode);
                    fclose(ftoken);
                    fclose(ferror);
                    this->error_line = line_count;
                    return LEXICAL_ANALYSIS_ERROR;
                }
            }
            break;
        }
        //缓冲池满了
        if (this->double_buffer[this->current_buffer].count == BUFFER_SIZE - 2)
        {
            this->double_buffer[this->current_buffer].buffer[this->double_buffer[this->current_buffer].count] = c;
            int i;
            for (i = 0; i < this->double_buffer[this->current_buffer].count; i++)
            {
                if (isDelimiter(double_buffer[this->current_buffer].buffer[i]))
                {
                    int j;//分界点
                    int k;
                    // 转移缓冲区内容
                    for (j = 0, k = i + 1; k <= double_buffer[current_buffer].count; k++, j++)
                    {
                        double_buffer[1 - current_buffer].buffer[j] = double_buffer[current_buffer].buffer[k];
                    }
                    //count大小重新设置
                    double_buffer[1 - current_buffer].count = j;
                    double_buffer[current_buffer].count = i;
                    //设置终结点
                    double_buffer[1 - current_buffer].buffer[j] = '\0';
                    double_buffer[current_buffer].buffer[i + 1] = '\0';
                    buffer_flag = 1; // 设置缓冲区轮转标志
                    break;
                }
            }
        }
        else if (c == '\n')
        {
            line_count++;
            double_buffer[current_buffer].buffer[double_buffer[current_buffer].count] = '\0';
        }
        else {
            double_buffer[current_buffer].buffer[double_buffer[current_buffer].count++] = c;
            continue;//继续
        }
        //继续处理换行后/缓冲池满后的处理
        deleComments(line_count);
        deleSpaces(line_count);

        if (double_buffer[current_buffer].count > 0)
        {
            strcpy(final_buffer.buffer, double_buffer[current_buffer].buffer);
            final_buffer.count = double_buffer[current_buffer].count;
            double_buffer[current_buffer].count = 0;
            if (!this->DFA(line_count)) {
                fclose(fcode);
                fclose(ftoken);
                fclose(ferror);
                this->error_line = line_count;
                return LEXICAL_ANALYSIS_ERROR;
            }
        }

        if (buffer_flag == 1)
        {
            //下一次 缓冲区轮转
            double_buffer[current_buffer].count = 0;
            current_buffer = 1 - current_buffer;
            buffer_flag = 0;
        }
    }
    fclose(fcode);
    fclose(ftoken);
    fclose(ferror);
    return SUCCESS;
}


/**
 * @brief： 识别注释
 * @param： int line_count  当前行数
 * @return  
 * */
void lexer::deleComments(int line_count) {
    char comment[BUFFER_SIZE];
    int comment_count = 0;
    bool flag_qoute = 0;
    //状态机 读到非“”包含的/进入循环
    for (int i = 0; double_buffer[current_buffer].buffer[i] != '\0'; i++)
    {
        if (double_buffer[current_buffer].buffer[i] == '"')
        {
            flag_qoute = 1 - flag_qoute;
            if (comment_flag != 1)
                continue;
        }
        if (flag_qoute == 1)
            if (comment_flag != 1)
                continue;
        if (double_buffer[current_buffer].buffer[i] == '/' || comment_flag == 1)
        {
            if (double_buffer[current_buffer].buffer[i + 1] == '\0')
            {
                continue;
            }
            if (double_buffer[current_buffer].buffer[i + 1] == '/' && !comment_flag)
            {
                //进入 //状态 直到\0停止
                int j;

                for (j = i; double_buffer[current_buffer].buffer[j] != '\0'; j++)
                {
                    comment[comment_count++] = double_buffer[current_buffer].buffer[j];
                    double_buffer[current_buffer].buffer[j] = '\0';
                }
                comment[comment_count] = '\0';
                fprintf(ftoken, "注释         %s     %d\n", comment,line_count);
                double_buffer[current_buffer].count -= comment_count;
                comment_count = 0;
                break;
            }
            if (double_buffer[current_buffer].buffer[i + 1] == '*' || comment_flag == 1)
            {
                //进入/* 状态 
                comment_flag = 1;
                int j;
                for (j = i + 2 * (1 - comment_flag); double_buffer[current_buffer].buffer[j] != '\0'; j++)
                {

                    comment[comment_count++] = double_buffer[current_buffer].buffer[j];
                    if (!flag_qoute && double_buffer[current_buffer].buffer[j] == '*' && double_buffer[current_buffer].buffer[j + 1] == '/')
                    {
                        comment_flag = 0;
                        comment[comment_count++] = '/';
                        comment[comment_count] = '\0';
                        fprintf(ftoken, "注释         %s    %d\n", comment,line_count);
                        double_buffer[current_buffer].count -= comment_count;
                        comment_count = 0;
                        break;
                    }
                }

                if (comment_flag == 0)
                    j = j + 2;

                //开始前移
                for (; double_buffer[current_buffer].buffer[j] != '\0'; j++, i++)
                {
                    if (double_buffer[current_buffer].buffer[j] == '\n')
                    {
                        i--;
                        continue;
                    }
                    double_buffer[current_buffer].buffer[i] = double_buffer[current_buffer].buffer[j];
                }

                if (comment_flag) {
                    //意味着多行注释，直接printf
                    comment[comment_count] = '\0';
                    fprintf(ftoken, "注释         %s   %d\n", comment,line_count);
                    double_buffer[current_buffer].buffer[i] = '\0';
                    double_buffer[current_buffer].count -= comment_count;
                    break;
                }
                double_buffer[current_buffer].buffer[i] = '\0';
            }

        }
    }
}



/**
 * @brief： 删除空格
 * @param： int line_count  当前行数
 * @return
 * */
void lexer::deleSpaces(int line_count) 
{
    bool inString= true, inChar = true;
    for (int i = 0; double_buffer[current_buffer].buffer[i] != '\0'; i++)
    {
        char cur_char = double_buffer[current_buffer].buffer[i];
        //不能删除字符串内的空格
        if ( cur_char== '"')
            inString = !inString;
        //不能删除空格字符
        if (cur_char == '\'')
            inChar = !inChar;
        if ((cur_char == ' ' || cur_char == '\t') && inString && inChar)
        {
            // 查找连续空格或制表符的结束位置
            int j = i + 1;
            for (; double_buffer[current_buffer].buffer[j] != '\0' && (double_buffer[current_buffer].buffer[j] == ' ' || double_buffer[current_buffer].buffer[j] == '\t'); j++)
            {
            }
            //如果空格或制表符在缓冲区末尾，则直接截断字符串
            if (double_buffer[current_buffer].buffer[j] == '\0')
            {
                double_buffer[current_buffer].buffer[i] = '\0';
                double_buffer[current_buffer].count -= (j - i);
                break;
            }
            // 如果空格或制表符在缓冲区开头，则全部删除
            if (i == 0)
            {
                int k = i;
                double_buffer[current_buffer].count -= (j - i);
                for (; double_buffer[current_buffer].buffer[j] != '\0'; j++, k++)
                    double_buffer[current_buffer].buffer[k] = double_buffer[current_buffer].buffer[j];
                double_buffer[current_buffer].buffer[k] = '\0';
                i--; // 回退一个位置
            }
            else
            {
                //如果之间有多个空格，先删到只有一个
                if (j - i >= 2)
                {
                    int k = i + 1;
                    double_buffer[current_buffer].count -= (j - (i + 1));
                    for (; double_buffer[current_buffer].buffer[j] != '\0'; j++, k++)
                        double_buffer[current_buffer].buffer[k] = double_buffer[current_buffer].buffer[j];
                    double_buffer[current_buffer].buffer[k] = '\0';
                    j = i + 1;
                }
                if (double_buffer[current_buffer].buffer[j] != '\0' && ((spaceRemovable(double_buffer[current_buffer].buffer[j]) || 
                    (i > 0 && spaceRemovable(double_buffer[current_buffer].buffer[i - 1])))))
                {
                    int k = i;
                    double_buffer[current_buffer].count -= (j - i);
                    for (; double_buffer[current_buffer].buffer[j] != '\0'; j++, k++)
                        double_buffer[current_buffer].buffer[k] = double_buffer[current_buffer].buffer[j];
                    double_buffer[current_buffer].buffer[k] = '\0';
                    i--;
                }

            }
        }

    }

}

/**
 * @brief： DFA状态机
 * @param： int line_count  当前行数
 * @return  DFA是否正常结束
 * */
bool lexer::DFA(int line_count)
{
    char dfa_token[BUFFER_SIZE];
    int count = 0;//当前dfa_token中的字符个数
    bool finish = false;
    int state = 0;//初态，state为0就表示了在初态

    for (int i = 0; i <= final_buffer.count; i++)
    {
        switch (state)
        {
            //初态
            case 0:
                switch (toState(final_buffer.buffer[i]))
                {
                    case 1://字母
                        dfa_token[count++] = final_buffer.buffer[i];
                        state = 1;
                        break;
                    case 2://数字
                        dfa_token[count++] = final_buffer.buffer[i];
                        state = 2;
                        break;
                    case 3://$,_
                        dfa_token[count++] = final_buffer.buffer[i];
                        state = 3;
                        break;
                    case 4://转义符只会在字符串内部使用，否则作为一个字符单独出来
                        dfa_token[count++] = final_buffer.buffer[i];
                        state = 4;
                        break;
                    case 5:
                        dfa_token[count++] = final_buffer.buffer[i];
                        state = 5;
                        break;
                    case 6:
                        dfa_token[count++] = final_buffer.buffer[i];
                        state = 6;
                        break;
                    case 7:
                        dfa_token[count++] = final_buffer.buffer[i];
                        state = 7;
                        break;
                    case 8:
                        dfa_token[count++] = final_buffer.buffer[i];
                        state = 8;
                        break;
                    case 10:
                        dfa_token[count++] = final_buffer.buffer[i];
                        state = 10;
                        break;
                    default:
                        dfa_token[count++] = final_buffer.buffer[i];
                        break;
                }
                break;
            case 1:
                switch (toState(final_buffer.buffer[i]))
                {
                    case 1:case 2:case 3:
                        dfa_token[count++] = final_buffer.buffer[i];
                        break;
                    default:
                        dfa_token[count] = '\0';
                        i--;
                        finish = 1;
                        state = 9;//结束状态
                }
                break;
            case 2:
                switch (toState(final_buffer.buffer[i]))
                {
                    case 1:
                    case 2:
                        dfa_token[count++] = final_buffer.buffer[i];
                        break;
                    case 7:
                        if (final_buffer.buffer[i] == '.')
                        {
                            dfa_token[count++] = final_buffer.buffer[i];
                            break;
                        }
                        else
                        {
                            dfa_token[count] = '\0';
                            i--;
                            finish = 1;
                            state = 9;//结束状态
                        }
                        break;
                    case 8:
                        //现在是+-，前面是Ee
                        if ((final_buffer.buffer[i] == '+' || final_buffer.buffer[i] == '-') && (final_buffer.buffer[i - 1] == 'e' || final_buffer.buffer[i - 1] == 'E'))
                        {
                            dfa_token[count++] = final_buffer.buffer[i];
                            break;
                        }
                        else
                        {
                            dfa_token[count] = '\0';
                            i--;
                            finish = 1;
                            state = 9;//结束状态
                            break;
                        }
                    default:
                        dfa_token[count] = '\0';
                        i--;
                        finish = 1;
                        state = 9;//结束状态
                        break;
                }
                break;
            case 3://好像$_和字母是一样的操作
                switch (toState(final_buffer.buffer[i]))
                {
                    case 1:case 2:case 3:
                        dfa_token[count++] = final_buffer.buffer[i];
                        break;
                    default:
                        dfa_token[count] = '\0';
                        i--;
                        finish = 1;
                        state = 9;//结束状态
                        break;
                }
                break;
            case 4:
                //字符串内转义符的情况在5态内部处理，这里处理单独的'\'
                dfa_token[count] = '\0';
                i--;
                finish = 1;
                state = 9;//结束状态
                break;
            case 5:
                dfa_token[count++] = final_buffer.buffer[i];
                if (final_buffer.buffer[i] == '"')
                {
                    //此时一定不是初态，所以不需要判断i与1的关系
                    if (final_buffer.buffer[i - 1] == '\\')
                    {
                    }
                    else
                    {
                        dfa_token[count] = '\0';
                        finish = 1;
                        state = 9;
                    }
                }
                break;
            case 6:
                dfa_token[count++] = final_buffer.buffer[i];
                if (final_buffer.buffer[i] == '\'')
                {
                    //还有一种情况是'\''，还是得判断
                    if (final_buffer.buffer[i - 1] == '\\')
                    {
                    }
                    else
                    {
                        dfa_token[count] = '\0';
                        finish = 1;
                        state = 9;
                    }
                }
                break;
            case 7:
                //要结束的字符，直接结束
                dfa_token[count] = '\0';
                i--;
                finish = 1;
                state = 9;
                break;
            case 8:
                switch (toState(final_buffer.buffer[i]))
                {
                    case 8:case 11:
                        dfa_token[count++] = final_buffer.buffer[i];
                        break;
                    default:
                        dfa_token[count] = '\0';
                        i--;
                        finish = 1;
                        state = 9;
                        break;
                }
                break;
            case 9://结束态
                //此时dfa_token已经得到，并且最后以\0结尾，故状态换成初始状态
                state = 0;
                count = 0;
                finish = 0;
                i--;
                //cout << dfa_token<<endl;
                if (!tokenJudge(dfa_token, line_count)) {
                    return false;
                }
                break;
            case 10://空格另加
                switch (toState(final_buffer.buffer[i]))
                {
                    case 10:
                        dfa_token[count++] = final_buffer.buffer[i];
                        break;
                    default:
                        dfa_token[count] = '\0';
                        i--;
                        finish = 1;
                        state = 9;
                        break;
                }
                break;

            default:
                break;
        }
        //最后剩的
        if (final_buffer.buffer[i + 1] == '\0')
        {
            dfa_token[count] = '\0';
            if (!tokenJudge(dfa_token, line_count)) {
                return false;
            }
            break;
        }
    }
    return true;
}


/**
 * @brief： 词元类型判断
 * @param： int line_count  当前行数
 * @return  是否是无法识别的字符
 * @note:   判断从自动机输出的token并输出到文件，类似<类型,原值,行数>，同时完成lexer_res的赋值
 * */
bool lexer::tokenJudge(char* str, int line_count)
{
    int kind = 0;
    if (isKey_Word(str) == 1) //判断是否为关键字
    {
        kind = my_Key_Word;
        fprintf(ftoken, "关键字       %-13s%d\n", str, line_count);
    }
    else if (isSignWord(str) == 1)  //判断是否为标识符
    {
        kind = my_SignWord;
        fprintf(ftoken, "标识符       %-13s%d\n", str, line_count);
    }
    else if (isInt(str) == 1)//判断是否为整数
    {
        kind = my_Integer;
        fprintf(ftoken, "整数         %-13s%d\n", str, line_count);
    }
    else if (isFloat(str) == 1)//判断是否为浮点数
    {
        kind = my_FloatPoint;
        fprintf(ftoken, "浮点数       %-13s%d\n", str, line_count);
    }
    else if (isMonocularOperator(str) == 1)//判断是否为单目运算符
    {
        kind = my_MonocularOperator;
        fprintf(ftoken, "单目运算符   %-13s%d\n", str, line_count);
    }
    else if (isBinocularOperator(str) == 1)//判断是否为双目运算符
    {
        kind = my_BinocularOperator;
        fprintf(ftoken, "双目运算符   %-13s%d\n", str, line_count);
    }
    else if (isDelimiter(str) == 1)//判断是否为界符
    {
        kind = my_Delimiter;
        fprintf(ftoken, "界符         %-13s%d\n", str, line_count);
    }
    else if (isBlank(str) == 1)//判断是否是空格
    {
        kind = my_Blank;

        // fprintf(ftoken, "空格         %-13s%d\n", str, line_count);
    }
    else if (isSeparator(str[0]) == 1 && strlen(str) == 1)//分隔符
    {
        kind = my_Separator;
        fprintf(ftoken, "分隔符       %-13s%d\n", str, line_count);
    }
    else if (isBracketsLeft(str[0]) == 1 && strlen(str) == 1) {
        kind = my_BracketsLeft;
        fprintf(ftoken, "左括号       %-13s%d\n", str, line_count);
    }
    else if (isBracketsRight(str[0]) == 1 && strlen(str) == 1) {
        kind = my_BracketsRight;
        fprintf(ftoken, "右括号       %-13s%d\n", str, line_count);
    }
    else if (isBracketsLeftBig(str[0]) == 1 && strlen(str) == 1) {
        kind = my_BracketsLeftBig;
        fprintf(ftoken, "左大括号     %-13s%d\n", str, line_count);
    }
    else if (isBracketsRightBig(str[0]) == 1 && strlen(str) == 1) {
        kind = my_BracketsRightBig;
        fprintf(ftoken, "左大括号     %-13s%d\n", str, line_count);
    }
    else if (isEnd(str[0]) == 1 && strlen(str) == 1) {
        kind = my_End;
        fprintf(ftoken, "结束符       %-13s%d\n", str, line_count);
    }
    else if (isBracketsLeftSquare(str[0]) == 1 && strlen(str) == 1) {
        kind = my_Brackets_Left_Square;
        fprintf(ftoken, "左方括号     %-13s%d\n", str, line_count);
    }
    else if (isBracketsRightSquare(str[0]) == 1 && strlen(str) == 1) {
        kind = my_Brackets_Right_Square;
        fprintf(ftoken, "右方括号     %-13s%d\n", str, line_count);
    }
    else if (isPointArrow(str) == 1) {
        kind = my_Point_Arrow;
        fprintf(ftoken, "箭头         %-13s%d\n", str, line_count);
    }
   /* else if (isRegion(str) == 1) {
        kind = my_Region;
        fprintf(ftoken, "域符         %-13s%d\n", str, line_count);
    }*/
    //else if (isRegionXigou(str) == 1) {
    //    kind = my_Region_Xigou;
    //    fprintf(ftoken, "析构符       %-13s%d\n", str, line_count);
    //}
    else if (strlen(str) == 1 && isColon(str[0]) == 1) {
        kind = my_Colon;
        fprintf(ftoken, "冒号         %-13s%d\n", str, line_count);
    }
    else if (isPoint(str[0]) == 1 && strlen(str) == 1) {
        kind = my_Point;
        fprintf(ftoken, "点符         %-13s%d\n", str, line_count);
    }
    else if (isDoublePoint(str) == 1) {
        kind = my_DoublePoint;
        fprintf(ftoken, "双点符         %-13s%d\n", str, line_count);
    }
    else if (isStr(str)) {
        kind = my_Str;
        fprintf(ftoken, "字符串       %-13s%d\n", str, line_count);
    }
    else if (isChar(str)) {
        kind = my_Char;
        fprintf(ftoken, "字符         %-13s%d\n", str, line_count);
    }
  /*  else if (strlen(str) == 1 && isQuestion_Mark(str[0]) == 1) {
           kind = my_Question_Mark;
           fprintf(ftoken, "问号         %-13s%d\n", str, line_count);
    }*/
    else {
        kind = my_WrongWord;
        fprintf(ftoken, "错误         %-13s%d\n", str, line_count);
        fprintf(ferror, "无法识别%s---------源代码第%d行\n", str, line_count);
    }
    if (kind != my_Blank)//送到语法分析器中的中间结果不含空格
    {
        elem t = elemGenerate(kind, str, line_count);
        lexer_res.push_back(t);
    }

    return (kind != my_WrongWord);
}


/**
 * @brief： 生成词元信息，保存到数组中
 * @param： int line_count  当前行数
 *          char* str 字符串
 *          int  kind 要保存的词元类型 
 * @return  生成的词元
 * */
elem lexer::elemGenerate(int kind, char* str,int line_count) {
    //判断是否是数据
    if (kind == my_Integer)
    {
        elem a("<INT>", str, line_count);
        return a;
    }
    else if (kind == my_FloatPoint)
    {
        elem a("<FLOAT>", str,line_count);
        return a;
    }
    else if (kind == my_Str)
    {
        elem a("<STRING>", str,line_count);
        return a;
    }
    else if (kind == my_Char)
    {
        elem a("<Char>", str,line_count);
        return a;
    }
    //判断是否是标识符
    else if (kind == my_SignWord)
    {
        elem a("<ID>", str,line_count);
        return a;
    }
    //判断是否出错
    else if (kind == my_WrongWord)
    {
        elem a("<WRONG>", str, line_count);
        return a;
    }
    //特殊处理main
    else if (kind == my_Key_Word && !strcmp(str, "main"))
    {
        elem a("<ID>", str,line_count);
        return a;
    }
    //处理各种符号
    else
    {
        elem a(str, str,line_count);
        return a;
    }
}


