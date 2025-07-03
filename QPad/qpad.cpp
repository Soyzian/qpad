// qpad.cpp
#include "qpad.h"
//#include "exportworker.h"
#include "ui_qpad.h"
#include "qsettings.h"
#include "qpad_config.h"
#include "winblur.h"

#include <QTextCharFormat>
#include <QFont>
#include <QMessageBox>
#include <QTextEdit>
#include <QThread>
#include <QVBoxLayout>
#include <QTextCursor>
#include <QFileDialog>
#include <QProcess>
#include <QTextDocumentWriter>
#include <QTimer>
#include <QTextStream>
#include <QFileInfo>
#include <QShortcut>
#include <QRegularExpression>
#include <QMouseEvent>
#include <windows.h>
#include <richedit.h>
#include <richole.h>
#include <QString>
#include <QByteArray>
#include <QDebug>



// Callback para recoger el RTF en memoria
DWORD CALLBACK StreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
    QByteArray *output = reinterpret_cast<QByteArray*>(dwCookie);
    output->append(reinterpret_cast<char*>(pbBuff), cb);
    *pcb = cb;
    return 0;
}

QString convertHtmlToRtf(const QString &html)
{
    // 1) Cargar la DLL de RichEdit 4.1
    LoadLibraryW(L"Msftedit.dll");

    // 2) Crear el control en memoria
    HWND hwnd = CreateWindowExW(0, MSFTEDIT_CLASS, L"",
                                WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL,
                                0, 0, 0, 0, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    if (!hwnd) {
        qDebug() << "No se pudo crear el RichEdit";
        return {};
    }

    // 3) Enviar el HTML al control
    QString wrappedHtml = "<html><body>" + html + "</body></html>";
    std::wstring whtml = wrappedHtml.toStdWString();

    SETTEXTEX stex = { ST_DEFAULT, 1200 }; // CP_UTF-8
    SendMessageW(hwnd, EM_SETTEXTEX, (WPARAM)&stex, (LPARAM)whtml.c_str());

    // 4) Preparar stream para extraer RTF
    QByteArray result;
    EDITSTREAM es;
    es.dwCookie = (DWORD_PTR)&result;
    es.dwError = 0;
    es.pfnCallback = StreamOutCallback;

    SendMessage(hwnd, EM_STREAMOUT, SF_RTF, (LPARAM)&es);

    DestroyWindow(hwnd);

    return QString::fromUtf8(result);
}

qpad::qpad(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::qpad)
    /*
    , m_dragging(false)
    */
{
    // 1) QMainWindow sin marco y transparente
    //setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    ui->setupUi(this);
    /*
    // Asegurarnos de que los widgets clave sean también translúcidos
    ui->centralwidget->setAttribute(Qt::WA_TranslucentBackground);
    ui->centralwidget->setAutoFillBackground(false);
    ui->titleBar->setAttribute(Qt::WA_TranslucentBackground);
    ui->titleBar->setAutoFillBackground(false);
    */
    // 2) Activar blur acrílico de Windows 11
    HWND hwnd = reinterpret_cast<HWND>(winId());
    enableAcrylicBlur(hwnd, QColor(30, 30, 30, 160));
    // Atajos de formato
    // Formato de texto
    (new QShortcut(QKeySequence("Ctrl+B"), this, SLOT(toggleBold())));
    (new QShortcut(QKeySequence("Ctrl+I"), this, SLOT(toggleItalic())));
    (new QShortcut(QKeySequence("Ctrl+U"), this, SLOT(toggleUnderline())));
    (new QShortcut(QKeySequence("Ctrl+Shift+S"), this, SLOT(toggleStrikeOut())));

    // Tamaño de fuente
    (new QShortcut(QKeySequence("Ctrl++"), this, SLOT(aumentarFuente())));
    (new QShortcut(QKeySequence("Ctrl+="), this, SLOT(aumentarFuente()))); // por si acaso
    (new QShortcut(QKeySequence("Ctrl+-"), this, SLOT(disminuirFuente())));

    // HTML Viewer
    (new QShortcut(QKeySequence("Ctrl+R"), this, SLOT(on_HTML_switch_clicked())));

    // Acciones comunes
    (new QShortcut(QKeySequence("Ctrl+S"), this, SLOT(guardarArchivo())));
    (new QShortcut(QKeySequence("Ctrl+A"), this, SLOT(abrirArchivo())));
    (new QShortcut(QKeySequence("Ctrl+E"), this, SLOT(exportarHTML2RTF())));
    (new QShortcut(QKeySequence("Ctrl+N"), this, SLOT(nuevoDocumento())));

    // Inicialización de controles
    ui->spinBox->setValue(12);
    ui->fontComboBox->setCurrentFont(QFont("Calibri"));
    ui->comboBox->setCurrentIndex(0);
    ui->pushButton->setCheckable(true);   // Negrita
    ui->pushButton_2->setCheckable(true); // Cursiva
    ui->pushButton_3->setCheckable(true); // Subrayado
    ui->pushButton_4->setCheckable(true); // Tachado
    ui->spinBox->setMinimum(6);    // mínimo tamaño fuente válido (ejemplo)
    ui->spinBox->setMaximum(72);   // máximo tamaño fuente válido (ejemplo)
    ui->spinBox->setValue(12);     // valor inicial, por ejemplo 12 pt

    applyDarkMode(false);

    // Conexiones de acciones

    connect(ui->actionNuevo,   &QAction::triggered, this, [=]() { nuevoDocumento(); });
    connect(ui->actionAbrir,   &QAction::triggered, this, &qpad::abrirArchivo);
    connect(ui->actionGuardar, &QAction::triggered, this, &qpad::guardarArchivo);
    connect(ui->HTML_switch,   &QPushButton::clicked, this, &qpad::on_HTML_switch_clicked);
    connect(ui->HTML_run,      &QPushButton::clicked, this, &qpad::on_HTML_run_clicked);

    // Botones de chrome
    /*
    connect(ui->actionCerrar,     &QAction::triggered, this, &QMainWindow::close);
    connect(ui->actionMinimizar,  &QAction::triggered, this, &QMainWindow::showMinimized);
    connect(ui->actionUpdate,     &QAction::triggered, [this](){
        if (isMaximized()) {
            showNormal();
            ui->actionUpdate->setText("Maximizar");
        } else {
            showMaximized();
            ui->actionUpdate->setText("Restaurar");
        }
    });
    connect(ui->btnCERR, &QPushButton::clicked, this, &QMainWindow::close);
    connect(ui->btnMIN, &QPushButton::clicked, this, &QMainWindow::showMinimized);
    connect(ui->btnMAX, &QPushButton::clicked, this, [=]() {
        if (isMaximized()) {
            showNormal();
            ui->btnMAX->setText("⬜"); // Cambia al ícono de maximizar
        } else {
            showMaximized();
            ui->btnMAX->setText("❐"); // Cambia al ícono de restaurar
        }
    });
    */

    nuevoDocumento("Nuevo.rtf");  
}

qpad::~qpad()
{
    delete ui;
}

QTextEdit* qpad::currentTextEdit()
{
    QWidget* tab = ui->tabWidget->currentWidget();
    if (auto te = qobject_cast<QTextEdit*>(tab))
        return te;
    return tab ? tab->findChild<QTextEdit*>() : nullptr;
}

QString qpad::htmlSemantico(const QString &html)
{
    QString out = html;

    // 1) Negrita
    out.replace(
        QRegularExpression(R"(<span[^>]*font-weight\s*:\s*700;?[^>]*>(.*?)</span>)",
                           QRegularExpression::DotMatchesEverythingOption),
        "<b>\\1</b>");

    // 2) Cursiva
    out.replace(
        QRegularExpression(R"(<span[^>]*font-style\s*:\s*italic;?[^>]*>(.*?)</span>)",
                           QRegularExpression::DotMatchesEverythingOption),
        "<i>\\1</i>");

    // 3) Subrayado
    out.replace(
        QRegularExpression(R"(<span[^>]*text-decoration\s*:\s*underline;?[^>]*>(.*?)</span>)",
                           QRegularExpression::DotMatchesEverythingOption),
        "<u>\\1</u>");

    // 4) Tachado (si lo tuvieras con text-decoration:line-through)
    out.replace(
        QRegularExpression(R"(<span[^>]*text-decoration\s*:\s*line-through;?[^>]*>(.*?)</span>)",
                           QRegularExpression::DotMatchesEverythingOption),
        "<s>\\1</s>");

    return out;
}

void qpad::applyCharFormat(const QTextCharFormat &fmt)
{
    if (auto te = currentTextEdit()) {
        QTextCursor cursor = te->textCursor();
        if (!cursor.hasSelection())
            cursor.select(QTextCursor::WordUnderCursor);
        cursor.mergeCharFormat(fmt);
        te->mergeCurrentCharFormat(fmt);
    }
}
void qpad::on_pushButton_clicked() // Negrita
{
    if (auto te = currentTextEdit()) {
        QTextCharFormat fmt;
        fmt.setFontWeight(ui->pushButton->isChecked() ? QFont::Bold : QFont::Normal);

        QTextCursor cursor = te->textCursor();
        if (cursor.hasSelection()) {
            cursor.mergeCharFormat(fmt);
        }
        te->mergeCurrentCharFormat(fmt);
    }
}

void qpad::on_pushButton_2_clicked() // Cursiva
{
    if (auto te = currentTextEdit()) {
        QTextCharFormat fmt;
        fmt.setFontItalic(ui->pushButton_2->isChecked());

        QTextCursor cursor = te->textCursor();
        if (cursor.hasSelection()) {
            cursor.mergeCharFormat(fmt);
        }
        te->mergeCurrentCharFormat(fmt);
    }
}

void qpad::on_pushButton_3_clicked() // Subrayado
{
    if (auto te = currentTextEdit()) {
        QTextCharFormat fmt;
        fmt.setFontUnderline(ui->pushButton_3->isChecked());

        QTextCursor cursor = te->textCursor();
        if (cursor.hasSelection()) {
            cursor.mergeCharFormat(fmt);
        }
        te->mergeCurrentCharFormat(fmt);
    }
}

void qpad::on_pushButton_4_clicked() // Tachado
{
    if (auto te = currentTextEdit()) {
        QTextCharFormat fmt;
        fmt.setFontStrikeOut(ui->pushButton_4->isChecked());

        QTextCursor cursor = te->textCursor();
        if (cursor.hasSelection()) {
            cursor.mergeCharFormat(fmt);
        }
        te->mergeCurrentCharFormat(fmt);
    }
}

void qpad::toggleBold() {
    ui->pushButton->toggle();
    on_pushButton_clicked();
}

void qpad::toggleItalic() {
    ui->pushButton_2->toggle();
    on_pushButton_2_clicked();
}

void qpad::toggleUnderline() {
    ui->pushButton_3->toggle();
    on_pushButton_3_clicked();
}

void qpad::toggleStrikeOut() {
    ui->pushButton_4->toggle();
    on_pushButton_4_clicked();
}

void qpad::aumentarFuente() {
    if (auto te = currentTextEdit()) {
        QTextCursor cursor = te->textCursor();
        QTextCharFormat format = cursor.charFormat();
        int newSize = format.fontPointSize() > 0 ? format.fontPointSize() + 2 : 14;
        format.setFontPointSize(newSize);
        applyCharFormat(format);
        ui->spinBox->setValue(newSize);
    }
}

void qpad::disminuirFuente() {
    if (auto te = currentTextEdit()) {
        QTextCursor cursor = te->textCursor();
        QTextCharFormat format = cursor.charFormat();
        int newSize = format.fontPointSize() > 2 ? format.fontPointSize() - 2 : 2;
        format.setFontPointSize(newSize);
        applyCharFormat(format);
        ui->spinBox->setValue(newSize);
    }
}

#include <QTextDocumentWriter>

void qpad::exportarHTML2RTF()
{
    auto *ed = currentTextEdit();
    if (!ed) {
        QMessageBox::warning(this, "Error", "No hay documento abierto.");
        return;
    }

    // Carpeta de exportación
    QString baseDir = QFileDialog::getExistingDirectory(
        this, "Selecciona carpeta de exportación",
        QCoreApplication::applicationDirPath() + "/qexports");
    if (baseDir.isEmpty()) return;

    // Nombre de base
    QString rawName  = ui->tabWidget->tabText(ui->tabWidget->currentIndex());
    QString baseName = QFileInfo(rawName).completeBaseName();

    // Rutas
    QString projectDir = QDir(baseDir).filePath(baseName);
    QString oldDir     = QDir(projectDir).filePath("old");
    QString rtfPath    = QDir(projectDir).filePath(baseName + ".rtf");

    if (!QDir().mkpath(oldDir)) {
        QMessageBox::critical(this, "Error", "No se pudo crear:\n" + oldDir);
        return;
    }

    // (Opcional) guardar html raw
    QString htmlPath = QDir(oldDir).filePath(baseName + ".html");
    {
        QFile f(htmlPath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            f.write(ed->toHtml().toUtf8());
            f.close();
        }
    }

    // Convertir vía RichEdit Win32
    QString rtf = convertHtmlToRtf(ed->toHtml());
    if (rtf.isEmpty()) {
        QMessageBox::critical(this, "Error", "No se pudo convertir HTML a RTF.");
        return;
    }

    // Guardar RTF resultante
    QFile out(rtfPath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error",
                              "No se pudo crear RTF:\n" + rtfPath + "\n" + out.errorString());
        return;
    }
    out.write(rtf.toUtf8());
    out.close();

    QMessageBox::information(this, "Éxito", "RTF exportado en:\n" + rtfPath);
}

void qpad::on_spinBox_valueChanged(int v)        { QTextCharFormat f; f.setFontPointSize(v);      applyCharFormat(f); }
void qpad::on_fontComboBox_currentFontChanged( const QFont &f) { QTextCharFormat fmt; fmt.setFontFamily(f.family()); applyCharFormat(fmt); }
void qpad::on_comboBox_currentIndexChanged(int i){ setAlignment(i); }

void qpad::updateFormatButtons()
{
    if (auto te = currentTextEdit()) {
        QTextCharFormat fmt = te->currentCharFormat();
        ui->pushButton->setChecked(fmt.fontWeight() == QFont::Bold);
        ui->pushButton_2->setChecked(fmt.fontItalic());
        ui->pushButton_3->setChecked(fmt.fontUnderline());
        ui->pushButton_4->setChecked(fmt.fontStrikeOut());

        // Fuente actual
        ui->fontComboBox->blockSignals(true); // Evita emitir currentFontChanged al setear
        ui->fontComboBox->setCurrentFont(fmt.font());
        ui->fontComboBox->blockSignals(false);

        // Tamaño de fuente actual
        int sz = fmt.fontPointSize();
        if (sz <= 0) sz = te->fontPointSize(); // fallback
        ui->spinBox->blockSignals(true);
        ui->spinBox->setValue(sz);
        ui->spinBox->blockSignals(false);
    }
}


void qpad::setAlignment(int idx)
{
    if (auto te = currentTextEdit()) {
        Qt::Alignment a = Qt::AlignLeft;
        switch(idx) {
        case 1: a = Qt::AlignHCenter; break;
        case 2: a = Qt::AlignRight;   break;
        case 3: a = Qt::AlignJustify; break;
        }
        te->setAlignment(a);
    }
}

void qpad::on_actionPropiedades_triggered() { qsettings dlg(this); dlg.exec(); }
void qpad::on_actionVersion_triggered()     { qpad_config dlg(this); dlg.exec(); }

void qpad::applyDarkMode(bool en)
{
    if (en) {
        QFile f(":/styles/styles.qss");
        if (f.open(QFile::ReadOnly))
            qApp->setStyleSheet(f.readAll());
    } else {
        qApp->setStyleSheet("");
    }
}
void qpad::nuevoDocumento(const QString &n)
{
    auto *editor = new QTextEdit(this);
    QFont baseFont("Calibri", 12);

    // Establecer la fuente global del documento y del widget
    editor->document()->setDefaultFont(baseFont);
    editor->setFont(baseFont);

    // Crear nuevo tab
    int i = ui->tabWidget->addTab(editor, n);
    ui->tabWidget->setCurrentIndex(i);

    // Establecer el formato después de que QTextEdit esté visible
    QTimer::singleShot(0, this, [=]() {
        QTextCursor cursor = editor->textCursor();
        QTextCharFormat fmt;
        fmt.setFontFamily("Calibri");
        fmt.setFontPointSize(12);
        cursor.setCharFormat(fmt);
        editor->setTextCursor(cursor);
        editor->mergeCurrentCharFormat(fmt);
        editor->setCurrentCharFormat(fmt);  // Fundamental

        // Actualizar controles
        ui->fontComboBox->setCurrentFont(baseFont);
        ui->spinBox->setValue(12);
    });

    connect(editor, &QTextEdit::cursorPositionChanged, this, &qpad::updateFormatButtons);
}

void qpad::abrirArchivo()
{
    QString fn = QFileDialog::getOpenFileName(this, "Abrir archivo", "", "Todos los archivos (*.*)");
    if (fn.isEmpty()) return;
    QFile f(fn);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString c = QTextStream(&f).readAll();
        f.close();
        auto *ed = new QTextEdit(this);
        ed->setPlainText(c);
        int i = ui->tabWidget->addTab(ed, QFileInfo(fn).fileName());
        ui->tabWidget->setCurrentIndex(i);
    } else {
        QMessageBox::warning(this, "Error", "No se pudo abrir el archivo.");
    }
}

void qpad::guardarArchivo()
{
    if (auto ed = currentTextEdit()) {
        QString fn = QFileDialog::getSaveFileName(this, "Guardar archivo", "", "Todos los archivos (*.*)");
        if (fn.isEmpty()) return;
        QFile f(fn);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream(&f) << ed->toPlainText();
            f.close();
            ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), QFileInfo(fn).fileName());
        } else {
            QMessageBox::warning(this, "Error", "No se pudo guardar el archivo.");
        }
    }
}

void qpad::mostrarHtmlColoreado(const QString &html)
{
    QDialog dlg(this);
    dlg.setWindowTitle("Vista HTML coloreada");
    dlg.resize(700, 400);
    QVBoxLayout L(&dlg);
    QTextEdit view(&dlg);
    view.setReadOnly(true);
    L.addWidget(&view);

    QTextDocument doc(&view);
    QTextCursor cursor(&doc);

    QRegularExpression tagRx(R"(<[^>]+>)"),
        strRx(R"("[^"]*")"),
        numRx(R"(\b\d+\b)");

    int last = 0;
    auto it = tagRx.globalMatch(html);
    while (it.hasNext()) {
        auto m = it.next();
        cursor.insertText(html.mid(last, m.capturedStart() - last));
        QTextCharFormat tf; tf.setForeground(Qt::blue);
        cursor.insertText(m.captured(0), tf);
        last = m.capturedEnd();
    }
    cursor.insertText(html.mid(last));

    auto highlight = [&](auto &rx, const QColor &col){
        QTextCursor tc(&doc);
        QTextCharFormat fmt; fmt.setForeground(col);
        auto it2 = rx.globalMatch(doc.toPlainText());
        while (it2.hasNext()) {
            auto mm = it2.next();
            tc.setPosition(mm.capturedStart());
            tc.setPosition(mm.capturedEnd(), QTextCursor::KeepAnchor);
            tc.setCharFormat(fmt);
        }
    };

    highlight(strRx, Qt::darkGreen);
    highlight(numRx, Qt::red);

    view.setDocument(&doc);
    dlg.exec();
}

void qpad::on_HTML_switch_clicked() {
    if (auto ed = currentTextEdit()) mostrarHtmlColoreado(ed->toHtml());
}

void qpad::on_HTML_run_clicked() {
    if (auto ed = currentTextEdit()) {
        ed->setHtml(ed->toPlainText());
    }
}
/*
// —— Movimiento de la ventana ——
void qpad::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Asegurarse que el clic está dentro del titleBar
        if (ui->titleBar->geometry().contains(event->pos())) {
            m_dragging = true;
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        }
    }
}

void qpad::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        move(event->globalPos() - m_dragPosition);
    }
}

void qpad::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_dragging = false;
}
*/
