#include "qpad_main.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setStyle("Fusion");
    QApplication a(argc, argv);
    QPadMain w;
    w.show();
    return a.exec();
}
