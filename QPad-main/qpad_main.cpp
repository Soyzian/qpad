#include "qpad_main.h"
#include <QVBoxLayout>
#include <QDir>
#include <QCoreApplication>
#include <QProcess>
#include <QDebug>

QPadMain::QPadMain(QWidget *parent)
    : QMainWindow(parent)
{
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    listWidget = new QListWidget(this);
    launchButton = new QPushButton("Iniciar QPad + Servicios", this);

    layout->addWidget(listWidget);
    layout->addWidget(launchButton);
    setCentralWidget(central);
    setWindowTitle("QPad Service Loader");

    connect(launchButton, &QPushButton::clicked, this, &QPadMain::launchSelected);

    loadServices();
}

void QPadMain::loadServices()
{
    QDir binDir(getExecutablePath("bin"));
    QStringList exeList = binDir.entryList(QStringList() << "*.exe", QDir::Files);

    for (const QString &exe : std::as_const(exeList)) {
        auto *item = new QListWidgetItem(exe);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        listWidget->addItem(item);
    }
}

void QPadMain::launchSelected()
{
    QString qpadPath = getExecutablePath("QPad-new.exe");
    if (!QProcess::startDetached(qpadPath)) {
        qWarning() << "No se pudo iniciar QPad-new.exe";
    }

    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            QString servicePath = getExecutablePath("bin/" + item->text());
            QProcess::startDetached(servicePath);
        }
    }

    close();  // Cierra el iniciador después de lanzar los servicios
}

QString QPadMain::getExecutablePath(const QString &relativePath)
{
    return QCoreApplication::applicationDirPath() + "/" + relativePath;
}
