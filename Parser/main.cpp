#include "Parser.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Parser w;
    w.show();
    return a.exec();
}
