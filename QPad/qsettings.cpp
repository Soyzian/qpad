#include "qsettings.h"
#include "ui_qsettings.h"

qsettings::qsettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::qsettings)
{
    ui->setupUi(this);
}

qsettings::~qsettings()
{
    delete ui;
}
