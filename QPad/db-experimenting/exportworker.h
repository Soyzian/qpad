#pragma once

#include <QObject>
#include <QString>
#include <QProcess>

class ExportWorker : public QObject
{
    Q_OBJECT

public:
    explicit ExportWorker(const QString &htmlPath,
                          const QString &rtfPath,
                          QObject *parent = nullptr);
    ~ExportWorker();

public slots:
    void process();

signals:
    void finished(bool success, const QString &message);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString    m_htmlPath;
    QString    m_rtfPath;
    QProcess  *m_process = nullptr;
};
