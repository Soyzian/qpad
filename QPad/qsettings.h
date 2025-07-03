#ifndef QSETTINGS_H
#define QSETTINGS_H

#include <QDialog>

namespace Ui {
class qsettings;
}

class qsettings : public QDialog
{
    Q_OBJECT

public:
    explicit qsettings(QWidget *parent = nullptr);
    ~qsettings();

private:
    Ui::qsettings *ui;
};

#endif // QSETTINGS_H
