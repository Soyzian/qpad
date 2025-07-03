#ifndef QPAD_CONFIG_H
#define QPAD_CONFIG_H

#include <QDialog>   // Cambiar QWidget por QDialog

class qpad_config : public QDialog
{
    Q_OBJECT
public:
    explicit qpad_config(QWidget *parent = nullptr);

signals:
};

#endif // QPAD_CONFIG_H
