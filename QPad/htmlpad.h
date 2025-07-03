#ifndef HTMLPAD_H
#define HTMLPAD_H

#include <QDialog>

namespace Ui {
class HTMLpad;
}

class HTMLpad : public QDialog
{
    Q_OBJECT

public:
    explicit HTMLpad(QWidget *parent = nullptr);
    ~HTMLpad();

private:
    Ui::HTMLpad *ui;
};

#endif // HTMLPAD_H
