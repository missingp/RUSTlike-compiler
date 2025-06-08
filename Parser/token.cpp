#include"token.h"


token::~token()
{
}

/**
* @func:   判断字符类型
* @para:   char c     输入的字符
* @return: 定义的字符类型
*/
int token::toState(char c) {
    if (isalpha(c))
        return 1;
    else if (isdigit(c))
        return 2;
    else if (c == '$' || c == '_')
        return 3;
    else if (c == '\\')//解决转义问题
        return 4;
    else if (c == '"')
        return 5;
    else if (c == '\'')
        return 6;
    //要结束的字符
    else if (c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == ';' || c == ',' || c == '.')//这里需要注意.符号，若是在数字后面就需要考虑浮点数，这个单独考虑
        return 7;
    else if (c == ' ')
        return 10;
    return 8;
}

/**
* @func:   判断字符是不是界符
* @para:   char c     输入的字符
* @return: 是否为界符
*/
int token::isDelimiter(char c) {

    if (Delimiter == c)
        return 1;
    return 0;
}


int token::isDelimiter(string c) {
    if (c.length() == 1)
    {
        if (Delimiter == c[0])
            return 1;
    }
    return 0;
}


/**
* @func:   判断字符是不是界符
* @para:   char c     输入的字符
* @return: 是否为界符
* @note:   单边判断，认为有可能组成双目运算符的算符身边空格不可删除，但是像f = 5这种空格也不会删掉了
*          需要添加=，避免有= =出现
*/
bool token::spaceRemovable(char c)
{
    if (c == '=' || c == '+' || c == '-' || c == '*' || c == '/' || c == '>' || c == '<' || c == '!')
        return false;
    if ((c > 'z' || (c < 'a' && c > 'Z') || (c < 'A' && c > '9') || (c < '0')) && c != '_' && c != '$')
        return true;
    return false;
}


/**
* @func:   判断字符串是不是可以表示整数
* @para:   char *str     输入的字符串
* @return:  返回值为0 表示 输入的字符串不是一个有效的整数。
*           返回值为1 表示 是一个有效的整数。
*           返回值为2 表示 几乎是一个有效的整数，但存在一些命名错误。
*/

int token::isInt(string str) {
    // const int OCT = 8;
    // const int DEC = 10;
    // const int HEX = 16;
    // int intType = DEC;
    // int len = 0;
    // string t;
    // if (str[0] == '+' || str[0] == '-')
    //     t = str.substr(1);
    // else
    //     t = str;
    // len = t.length();
    // // 判断数的进制
    // if (isdigit(t[0])) {
    //     if (t[0] == '0' && t[1] != '\0') {  //判断是八进制还是十六进制，当首位是0且数字不止一位时考虑下一位，
    //         if (isdigit(t[1]))
    //             intType = OCT;
    //         else if (t[1] == 'x' || t[1] == 'X')
    //             intType = HEX;
    //         else
    //             return 0;
    //     }
    //     else
    //         intType = DEC;
    // }
    // else
    //     return 0;
    // //检查数的格式是否正确
    // switch (intType)
    // {
    //     case OCT:
    //         for (int i = 1; i < len; i++) {//八进制数首位为0，长度至少为2.
    //             if (isdigit(t[i])) {
    //                 if (t[i] >= '0' && t[i] <= '7')
    //                     ;
    //                 else
    //                     return 2;
    //             }
    //             else
    //                 return 0;
    //         }
    //         return 1;
    //         break;
    //     case DEC:
    //         for (int i = 0; i < len; i++) {
    //             if (isdigit(t[i]))
    //                 ;
    //             else
    //                 return 0;
    //         }
    //         return 1;
    //         break;
    //     case HEX:
    //         for (int i = 2; i < len; i++) {  // 十六进制数前两位为0x\0X，长度至少为3.
    //             if (isdigit(t[i]) || (t[i] >= 'a' && t[i] <= 'f') || (t[i] >= 'A' && t[i] <= 'F'))
    //                 ;
    //             else {
    //                 if (!isalpha(t[i]))//十六进制数中出现非字母
    //                     return 0;
    //                 else
    //                     return 2;
    //             }
    //         }
    //         return 1;
    //         break;
    // }
    // return 0;




    if (str.empty()) return 0;
    for (char c : str) {
        if (!isdigit(c)) return 0; // 必须全是数字
    }
    return 1; // 符合数值规则
}




//判断xx.xx的情况，无小数点视为非小数，否则无法与整数区分
int token::hasDot(string str)
{
    int len = 0;
    int dotAppearNum = 0;
    string t;
    if (str[0] == '+' || str[0] == '-')
        t=str.substr(1);
    else
        t=str;
    len = t.length();
    for (int i = 0; i < len; i++)
    {
        if (len > 0)//确保有首位和最后一位
        {
            if (i == 0)//判断首位
            {
                if (!isdigit(t[i]))//不是数字
                {
                    if (t[i] == '.')
                        return 2;
                    else
                        return 0;
                }
            }
            if (i < len - 1)//判断非最后一位且不是首位
            {
                if (!isdigit(t[i]) && t[i] != '.')//既不是数字也不是小数点
                    return 0;
                if (t[i] == '.')//如果是小数点
                {
                    dotAppearNum++;
                    if (dotAppearNum > 1)//出现多个小数点的情况
                        return 2;
                }
            }
            if (i == len - 1)//判断末位
            {
                //既不是数字也不是f\F、d\D
                if (!isdigit(t[i]) && t[i] != 'f' && t[i] != 'F' && t[i] != 'd' && t[i] != 'D')
                    return 0;
            }
        }
    }
    if (dotAppearNum < 1)
        return 0;
    return 1;
}



/**
* @func:   判断字符串是不是表示浮点数
* @para:   char *str     输入的字符串
* @return: 返回值为0 意味着 压根不是
*          返回值为1 意味着 是 包括xx.xx和xx e\E xx两种情况
*          返回值为2 意味着 是但float命名错误 即：出现 0.0.1
*/
int token::isFloat(string str) {
    int len = 0;
    int dotAppearNum = 0;
    len = str.length();
    string t;
    //判断78e\E56的情况
    int pos = -1;
    for (int i = 0; i < len; i++)
    {
        if (str[i] == 'e' || str[i] == 'E')
        {
            pos = i;
            break;
        }
    }
    if (pos != -1)//出现了e/E
    {
        int j = 0;
        if (pos == 0 || pos == len - 1)//e在首位或末尾
            return 0;
        if (str[0] == '+' || str[0] == '-')
        {
            if (str[1] == 'e' || str[1] == 'E')//首位是符号位时保证e不在符号位之后
                return 0;
        }
        t=str.substr(j, pos + 1);//最后一位不是尾零
        t[pos] = '\0';
        if (hasDot(t) != 1 && isInt(t) != 1)
            return 0;
        j = pos + 1;//数字的起始位        
        t=str.substr(j, len - pos);//最后一位是尾零
        if (hasDot(t) != 1 && isInt(t) != 1)
            return 0;
        return 1;
    }
    return hasDot(str);
}


/**
* @func:   判断字符串是不是可以表示标识符
* @para:   char *str     输入的字符串
* @return:  返回值为0 意味着 压根不是
*           返回值为1 意味着 是
*           返回值为2 意味着 是但标识符命名错误
*/

int token::isSignWord(string str) {

    // if (str[0] == '_' || isalpha(str[0]) || str[0] == '$')
    // {
    //     for (int i = 0; str[i] != '\0'; i++)
    //     {
    //         if (str[i] != '_' && str[i] != '$' && !isdigit(str[i]) && !isalpha(str[i]))
    //             return 2;
    //     }
    //     return 1;
    // }
    // return 0;

    if (isKey_Word(str)) return 0; // 不能与关键字相同
    if (str.empty() || !(isalpha(str[0]) || str[0] == '_')) return 0; // 首字符必须是字母或下划线
    for (size_t i = 1; i < str.size(); i++) {
        if (!(isalnum(str[i]) || str[i] == '_')) return 0; // 后续字符必须是字母、数字或下划线
    }
    return 1; // 符合标识符规则
}



/**
*@func:   判断字符串是不是关键字
*@para :  char* str     输入的字符串
*@return: 1 是，0不是
*/
int token::isKey_Word(string str) {

    for (int i = 0; i < KEY_WORD_SIZE; i++)
    {
        if (str==rust_keywords[i])
            return 1;
    }
    return 0;
}

/**
*@func:   判断字符串是不是双目运算符
*@para :  char* str     输入的字符串
*@return: 1 是，0不是
*/
int token::isBinocularOperator(string str) {
    for (int i = 0; i < BINOCULAR_OPERATOR_SIZE; i++)
    {
        if (str==Binocular_Operator[i])
            return 1;
    }
    return 0;
}


/**
*@func:   判断字符串是不是单目运算符
*@para :  char* str     输入的字符串
*@return: 1 是，0不是
*/
int token::isMonocularOperator(string str) {
    if (str.length() == 1) {
        for (int i = 0; i < MONOCULAR_OPERATOR_SIZE; i++)
        {
            if (str[0] == Monocular_Operator[i][0])
                return 1;
        }
    }
    return 0;
}

/**
*@func:   判断字符串是不是空格
*@para :  char* str     输入的字符串
*@return: 1 是，0不是
*/
int token::isBlank(string str)
{
    for (int i = 0;i < str.length();  i++)
    {
        if (str[i] != ' ')
            return 0;
    }
    return 1;
}



/**
 * @brief 判断输入字符是否为分隔符
 * @param: c 输入的字符
 * @return: int 1表示是分隔符，0表示不是
 */
int token::isSeparator(char c)
{
    if (Seprater == c)
        return 1;
    return 0;
}

/**
 * @brief: 判断输入字符类型是否为左括号
 * @param: c 输入的字符
 * @return: int 1表示是左括号，0表示不是
 */
int token::isBracketsLeft(char c)
{
    if (Brackets_Left == c)
        return 1;
    return 0;
}

/**
 * @brief 判断输入字符类型是否为右括号
 * @param c 输入的字符
 * @return int 1表示是右括号，0表示不是
 */
int token::isBracketsRight(char c)
{
    if (Brackets_Right == c)
        return 1;
    return 0;
}

/**
 * @brief 判断输入字符类型是否为左大括号
 * @param c 输入的字符
 * @return int 1表示是左大括号，0表示不是
 */
int token::isBracketsLeftBig(char c)
{
    if (Brackets_Left_Big == c)
        return 1;
    return 0;
}

/**
 * @brief 判断输入字符类型是否为右大括号
 * @param c 输入的字符
 * @return int 1表示是右大括号，0表示不是
 */
int token::isBracketsRightBig(char c)
{
    if (Brackets_Right_Big == c)
        return 1;
    return 0;
}

/**
 * @brief 判断输入字符类型是否为结束符
 * @param c 输入的字符
 * @return int 1表示是结束符，0表示不是
 */
int token::isEnd(char c)
{
    if (End == c)
        return 1;
    return 0;
}

/**
 * @brief 判断输入字串类型是否为字符串 "xxxxxx"
 * @param str 输入的字符串
 * @return int 1表示是字符串，0表示不是
 */
int token::isStr(string str)
{
    //小型状态机
    int i = 0;
    for (i = 0; i < str.length(); i++)
    {
        if (str[i] == ' ')
            continue;
        else
            break;
    }
    int flag = 0;
    //此时i为非' '的首字符
    if (str[i] == '"')
    {
        i++;
        flag = 1;//第一个"
    }
    if (flag == 1)
    {
        for (i; i < str.length(); i++)
        {
            if (str[i] == '"' && str[i - 1] != '\\')
                return 1;
        }
    }
    return 0;
}

/**
 * @brief 判断输入字串类型是否为字符 'x'
 * @param str 输入的字符串
 * @return int 1表示是字符，0表示不是
 */
int token::isChar(string str)
{
    //小型状态机
    int i = 0;
    for (i = 0; i < str.length(); i++)
    {
        if (str[i] == ' ')
            continue;
        else
            break;
    }
    int flag = 0;
    //此时i为非' '的首字符
    if (str[i] == '\'')
    {
        i++;
        flag = 1;//第一个"
    }
    if (flag == 1)
    {
        //只允许 '\x'
        if (str[i] == '\\')
        {
            if (isdigit(str[i]))
            {
                flag = 2;//10进制
            }
            else if (str[i] == 'x')
            {
                flag = 3;//16进制
            }
            else
            {
                if (str[i + 2] == '\'')
                    return 1;

            }
            if (flag == 2 && isInt(&str[i]) && strlen(&str[i]) == 3)
                return 1;
            if (flag == 3 && isInt(&str[i]) && strlen(&str[i]) == 5)
                return 1;

        }
        else {
            if (str[i + 1] == '\'')
                return 1;
            return 0;
        }

    }
    return 0;
}

/**
 * @brief 判断输入字符类型是否为字符 '.'
 * @param c 输入的字符
 * @return int 1表示是字符'.'，0表示不是
 */
int token::isPoint(char c)
{
    if (Point == c)
        return 1;
    return 0;
}

/**
 * @brief 判断输入字符类型是否为字符 '..'
 * @param c 输入的字符
 * @return int 1表示是字符'..'，0表示不是
 */
int token::isDoublePoint(string str)
{
    return str == "..";
}


/**
 * @brief 判断输入字符类型是否为字符 '['
 * @param c 输入的字符
 * @return int 1表示是字符'[',0表示不是
 */
int token::isBracketsLeftSquare(char c)
{
    if (Brackets_Left_Square == c)
        return 1;
    return 0;
}

/**
 * @brief 判断输入字符类型是否为字符 ']'
 * @param c 输入的字符
 * @return int 1表示是字符']'，0表示不是
 */
int token::isBracketsRightSquare(char c)
{
    if (Brackets_Right_Square == c)
        return 1;
    return 0;
}

/**
 * @brief 判断输入字串类型是否为字符 '->'
 * @param str 输入的字符串
 * @return int 1表示是字符'->'，0表示不是
 */
int token::isPointArrow(string str)
{
    if (str==Point_Arrow)
        return 1;
    return 0;
}

/**
 * @brief 判断输入字串类型是否为字符 '::'
 * @param str 输入的字符串
 * @return int 1表示是字符'::'，0表示不是
 */
int token::isRegion(string str)
{
    if (str==Region)
        return 1;
    return 0;
}

/**
 * @brief 判断输入字串类型是否为字符 '::~'
 * @param str 输入的字符串
 * @return int 1表示是字符'::~'，0表示不是
 */
int token::isRegionXigou(string str)
{
    if (str==Region_Xigou)
        return 1;
    return 0;
}

/**
 * @brief 判断输入字符类型是否为字符 ':'
 * @param c 输入的字符
 * @return int 1表示是字符':'，0表示不是
 */
int token::isColon(char c)
{
    if (c == Colon)
        return 1;
    return 0;
}


int token::isQuestion_Mark(char c) {
    if (c == Question_Mark)
        return 1;
    return 0;
}
