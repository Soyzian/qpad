#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQuickControls2/QQuickStyle>
#include "qdir.h"
#include "qpad.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile f(":/styles/styles.qss");  // Supón que lo guardás en los recursos
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        QString style = f.readAll();
        qApp->setStyleSheet(style);
    }

    qpad w;
    w.show();

    return app.exec();  // ✅ Retornar la instancia correcta
}
