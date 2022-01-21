#ifndef WORKER_H
#define WORKER_H

#include <QThread>
#include <QObject>
#include <QMainWindow>
#include <thread>
#include <QErrorMessage>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QDebug>

namespace Workers {
    class Worker;
}

class Worker : public QObject {
    Q_OBJECT

public:
    Worker();
    ~Worker();
    void WrkMsg_inter(const char *msg);
    void WrkProg_inter(uint prog);

    void StartEepromFlash(int index);
    void StartEepromDump(int index);
    void StartDump(int index);
    void StartFlash(int index);

signals:
    void finished();
    void WrkMsg_emit(QString msg);
    void WrkProg_emit(uint prog);

private slots:
    void WrkMsg_push(QString msg);
    void WrkProg_push(uint prog);
    void FlashEepromProcess();
    void DumpEepromProcess();
    void DumpProcess();
    void FlashProcess();
    void WorkerDone();

private:
    void PrepareThread(int index, QString fname, const char*slot);
    bool InitUSB();
    void DeInitUSB();
};

class FileDialog : public QWidget
{
public:
    QString fname = nullptr;
    void SaveDialog();
    void OpenDialog();
};



#endif // WORKER_H
