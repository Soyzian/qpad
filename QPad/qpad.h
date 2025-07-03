#ifndef QPAD_H
#define QPAD_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPoint>

QT_BEGIN_NAMESPACE
namespace Ui { class qpad; }
QT_END_NAMESPACE

class qpad : public QMainWindow
{
    Q_OBJECT

public:
    explicit qpad(QWidget *parent = nullptr);
    ~qpad();

private slots:
    void on_pushButton_clicked();         // Negrita
    void on_pushButton_2_clicked();       // Cursiva
    void on_pushButton_3_clicked();       // Subrayado
    void on_pushButton_4_clicked();       // Tachado

    void on_spinBox_valueChanged(int value);
    void on_fontComboBox_currentFontChanged(const QFont &f);
    void on_comboBox_currentIndexChanged(int index);

    void nuevoDocumento(const QString& nombre = "Nuevo.txt");
    void abrirArchivo();
    void guardarArchivo();

    void on_HTML_switch_clicked();
    void on_HTML_run_clicked();

    void on_actionPropiedades_triggered();
    void on_actionVersion_triggered();
    void updateFormatButtons();
    void toggleBold();
    void toggleItalic();
    void toggleUnderline();
    void toggleStrikeOut();
    void aumentarFuente();
    void disminuirFuente();

    void mostrarHtmlColoreado(const QString &html);
    void exportarHTML2RTF();
    QString htmlSemantico(const QString  &html);
/*
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
*/
private:
    Ui::qpad *ui;

    QTextEdit* currentTextEdit();
    void applyCharFormat(const QTextCharFormat &fmt);
    void setAlignment(int index);
    void applyDarkMode(bool enabled);

};

#endif // QPAD_H
