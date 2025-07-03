#include "exportworker.h"
#include <QCoreApplication>
#include <QFileInfo>

ExportWorker::ExportWorker(const QString &htmlPath,
                           const QString &rtfPath,
                           QObject *parent)
    : QObject(parent)
    , m_htmlPath(htmlPath)
    , m_rtfPath(rtfPath)
{}

ExportWorker::~ExportWorker()
{
    if (m_process) {
        m_process->kill();
        m_process->deleteLater();
    }
}

void ExportWorker::process()
{
    // Ruta a pandoc.exe en el mismo directorio de la app
    QString exePath = QCoreApplication::applicationDirPath() + "/pandoc.exe";
    if (!QFileInfo::exists(exePath)) {
        emit finished(false, "No se encontró pandoc.exe junto a la aplicación.");
        return;
    }

    // Argumentos: de HTML a RTF
    QStringList args = {
        "-f", "html",
        "-t", "rtf",
        "-o", m_rtfPath,
        m_htmlPath
    };

    m_process = new QProcess(this);
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &ExportWorker::onProcessFinished);

    m_process->start(exePath, args);
    if (!m_process->waitForStarted(2000)) {
        emit finished(false, "No se pudo iniciar el proceso de conversión (pandoc).");
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void ExportWorker::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // Leer error de conversión
    QString err = m_process->readAllStandardError().trimmed();
    bool success = (exitStatus == QProcess::NormalExit
                    && exitCode == 0
                    && err.isEmpty()
                    && QFileInfo::exists(m_rtfPath));

    QString message;
    if (success) {
        message = m_rtfPath;
    } else {
        message = err.isEmpty()
        ? "Error desconocido en la conversión."
        : err;
    }

    m_process->deleteLater();
    m_process = nullptr;
    emit finished(success, message);
}
