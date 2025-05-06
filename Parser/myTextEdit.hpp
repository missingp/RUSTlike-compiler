#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QKeyEvent>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QColor>
#include <QFont>
class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QTextDocument* parent)
        : QSyntaxHighlighter(parent)
    {
        HighlightingRule rule;

        // 定义 Rust 关键字格式
        keywordFormat.setForeground(Qt::darkBlue);
        keywordFormat.setFontWeight(QFont::Bold);
        QStringList keywordPatterns;
        keywordPatterns << "\\blet\\b" << "\\bfn\\b" << "\\bif\\b" << "\\belse\\b"
            << "\\bfor\\b" << "\\bwhile\\b" << "\\bloop\\b" << "\\bmatch\\b"
            << "\\bstruct\\b" << "\\benum\\b" << "\\btrait\\b" << "\\bimpl\\b"
            << "\\buse\\b" << "\\bconst\\b" << "\\bstatic\\b" << "\\bmut\\b"
            << "\\breturn\\b";
        foreach(const QString & pattern, keywordPatterns) {
            rule.pattern = QRegularExpression(pattern);
            rule.format = keywordFormat;
            highlightingRules.append(rule);
        }

        // 定义 Rust 类型格式
        typeFormat.setForeground(Qt::darkGreen);
        typeFormat.setFontWeight(QFont::Bold);
        QStringList typePatterns;
        typePatterns << "\\b(i8|i16|i32|i64|i128|u8|u16|u32|u64|u128|f32|f64|bool|char|str)\\b"
            << "\\bVec\\b" << "\\bHashMap\\b" << "\\bResult\\b" << "\\bOption\\b";
        foreach(const QString & pattern, typePatterns) {
            rule.pattern = QRegularExpression(pattern);
            rule.format = typeFormat;
            highlightingRules.append(rule);
        }

        // 单行注释格式，设置为绿色
        singleLineCommentFormat.setForeground(Qt::darkGreen);
        rule.pattern = QRegularExpression("//[^\n]*");
        rule.format = singleLineCommentFormat;
        highlightingRules.append(rule);

        // 多行注释开始和结束正则表达式
        commentStartExpression = QRegularExpression("/\\*");
        commentEndExpression = QRegularExpression("\\*/");
        // 多行注释格式，设置为绿色
        multiLineCommentFormat.setForeground(Qt::darkGreen);

        // 字符串格式，设置为绿色
        stringFormat.setForeground(Qt::darkGreen);
        rule.pattern = QRegularExpression("\".*?\"");
        rule.format = stringFormat;
        highlightingRules.append(rule);

        // 函数名格式，设置为黄色
        functionFormat.setFontItalic(true);
        functionFormat.setForeground(Qt::darkYellow);
        rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
        rule.format = functionFormat;
        highlightingRules.append(rule);
    }
protected:
    void highlightBlock(const QString& text)
    {
        foreach(const HighlightingRule & rule, highlightingRules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }

        // 处理多行注释
        setCurrentBlockState(0);

        int startIndex = 0;
        if (previousBlockState() != 1)
            startIndex = text.indexOf(commentStartExpression);

        while (startIndex >= 0) {
            QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
            int endIndex = match.capturedStart();
            int commentLength = 0;
            if (endIndex == -1) {
                setCurrentBlockState(1);
                commentLength = text.length() - startIndex;
            }
            else {
                commentLength = endIndex - startIndex
                    + match.capturedLength();
            }
            setFormat(startIndex, commentLength, multiLineCommentFormat);
            startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
        }
    }

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat typeFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat functionFormat;
};
class MyTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    MyTextEdit(QWidget* parent = nullptr) : QTextEdit(parent) {
        highlighter = new Highlighter(this->document());
    }
    ~MyTextEdit() {
        delete highlighter;
    }

protected:
    void keyPressEvent(QKeyEvent* event) override
    {
        QTextCursor cursor = textCursor();
        int position = cursor.position();
        QString text = toPlainText();

        switch (event->key()) {
            case Qt::Key_ParenLeft:
                insertMatchingPair(")", cursor);
                break;
            case Qt::Key_BracketLeft:
                insertMatchingPair("]", cursor);
                break;
            case Qt::Key_BraceLeft:
                insertMatchingPair("}", cursor);
                break;
            case Qt::Key_QuoteDbl:
                insertMatchingPair("\"", cursor);
                break;
            case Qt::Key_Apostrophe:
                insertMatchingPair("'", cursor);
                break;
            case Qt::Key_ParenRight:
                if (shouldSkipClosing('(', ')', position, text)) {
                    cursor.movePosition(QTextCursor::Right);
                    setTextCursor(cursor);
                    return;
                }
                break;
            case Qt::Key_BracketRight:
                if (shouldSkipClosing('[', ']', position, text)) {
                    cursor.movePosition(QTextCursor::Right);
                    setTextCursor(cursor);
                    return;
                }
                break;
            case Qt::Key_BraceRight:
                if (shouldSkipClosing('{', '}', position, text)) {
                    cursor.movePosition(QTextCursor::Right);
                    setTextCursor(cursor);
                    return;
                }
                break;
            default:
                break;
        }
        if (event->key() == Qt::Key_Tab) {
            // 插入4个空格
            insertPlainText("    ");
        }
        else
            QTextEdit::keyPressEvent(event);
    }

private:
    void insertMatchingPair(const QString& pair, QTextCursor& cursor)
    {
        cursor.insertText(pair);
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
    }

    bool shouldSkipClosing(QChar opening, QChar closing, int position, const QString& text)
    {
        if (position < text.length() && text[position] == closing) {
            int i = position - 1;
            int openCount = 0;
            while (i >= 0) {
                if (text[i] == closing) {
                    openCount++;
                }
                else if (text[i] == opening) {
                    if (openCount == 0) {
                        return true;
                    }
                    openCount--;
                }
                i--;
            }
        }
        return false;
    }
    Highlighter* highlighter;
};