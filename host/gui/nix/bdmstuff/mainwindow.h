#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTextBrowser>
#include <QCloseEvent>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void PushTextMessage(const char *message);
    void PushProgress(uint progress);
    void PushECUItem(const char *name);
    void ClearECUList();
    int  GetECUIndex();

    void ECUIndexLogic (int index);
    void EnableECUlist (bool enable);
    void EnableFlash   (bool enable);
    void EnableEeprom  (bool enable);
    void EnableDebug   (bool enable);
    void EnableAll     (bool enable);

    ~MainWindow() override;

private slots:
    void btnDumpEepromClick();
    void btnFlashEepromClick();
    void btnDumpClick();
    void btnFlashClick();
    void ECUIndexChange(int index);
    void btnDebugClick();

private:
    Ui::MainWindow *ui;

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
