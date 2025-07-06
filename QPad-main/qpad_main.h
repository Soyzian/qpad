#ifndef QPAD_MAIN_H
#define QPAD_MAIN_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>

class QPadMain : public QMainWindow {
    Q_OBJECT

public:
    explicit QPadMain(QWidget *parent = nullptr);

private slots:
    void launchSelected();

private:
    QListWidget *listWidget;
    QPushButton *launchButton;

    void loadServices();  // Cargar los .exe en /bin
    QString getExecutablePath(const QString &relativePath);
};

#endif // QPAD_MAIN_H
