#include "mainwindow.h"
#include "main.h"
#include "ui_mainwindow.h"
#include "worker.h"
#include "debugwindow.h"
#include "../../../core/core.h"

#include <QTime>

static std::mutex textmutex;
static std::mutex bartmutex;

static glue glue_;
static Worker worker_;
// static debugwindow debugwindow_;

// Stupid glue
static void MessagePoint(const char *msg)
{   glue_.CastMessage(msg); }

//////////////////////////////////////////////
/// Triggers
void MainWindow::ECUIndexChange(int index)
{
    ECUIndexLogic(index);

    ui->textBrowser->clear();

    if (index > 0)
    {
        core_InstallMessage(  reinterpret_cast<void*>(&MessagePoint)  );
        core_PrintInfo(static_cast<uint>(index));
    }
    else
        glue_.CastMessage(core_VersionString());
}

void MainWindow::ECUIndexLogic(int index)
{
    EnableAll(false);
    EnableECUlist(true);

    if (index > 0)
    {
        if (core_TargetSizeFLASH(static_cast<uint>(index)))
            EnableFlash(true);
        if (core_TargetSizeEEPROM(static_cast<uint>(index)))
            EnableEeprom(true);
        if (core_TargetHasDebug(static_cast<uint>(index)))
            EnableDebug(true);
    }
}

void MainWindow::btnDumpEepromClick()
{
    EnableAll(false);
    worker_.StartEepromDump(GetECUIndex());
}

void MainWindow::btnFlashEepromClick()
{
    EnableAll(false);
    worker_.StartEepromFlash(GetECUIndex());
}

void MainWindow::btnDumpClick()
{
    EnableAll(false);
    worker_.StartDump(GetECUIndex());
}

void MainWindow::btnFlashClick()
{
    EnableAll(false);
    worker_.StartFlash(GetECUIndex());
}

void MainWindow::btnDebugClick()
{
    // EnableAll(false);
    // glue_.CastMessage("Debug clicked");
    // debugwindow d;
    // d.show();

    glue_.DisplayDebug(GetECUIndex());
}

void MainWindow::PushTextMessage(const char *message)
{
    QString msg = QString::fromUtf8(message);
    QTime time = QTime::currentTime();

    if (msg.length() > 1)
        msg = "<" + time.toString() + ":" + QString().sprintf("%03d", time.msec()) + "> " + message;

    textmutex.lock();
    ui->textBrowser->append(msg);
    textmutex.unlock();
}

void MainWindow::PushProgress(uint progress)
{
    int prog = static_cast<int>(progress);

    prog = prog > 100 ? 100 : prog;

/*  // Some stupid performance wasting.. why not?
    int r = static_cast<int>( (100-prog)*2 );
    int g = static_cast<int>( prog*2 );

    QPalette pal = palette();
    pal.setColor(QPalette::Highlight, QColor(r,g,50));
    bar->setPalette(pal); */

    bartmutex.lock();
    ui->progressBar->setValue(prog);
    bartmutex.unlock();
}

void MainWindow::ClearECUList()
{
   ui->comboBox->clear();
}

void MainWindow::PushECUItem(const char *name)
{
    ui->comboBox->addItem(name);
}

int MainWindow::GetECUIndex()
{
    return ui->comboBox->currentIndex();
}

void MainWindow::EnableECUlist(bool enable)
{
    ui->comboBox->setEnabled(enable);
}

void MainWindow::EnableFlash(bool enable)
{
    ui->btnFlash->setEnabled(enable);
    ui->btnDump->setEnabled(enable);
    ui->lblFlash->setEnabled(enable);
}

void MainWindow::EnableEeprom(bool enable)
{
    ui->btnWriteEEP->setEnabled(enable);
    ui->btnReadEEP->setEnabled(enable);
    ui->lblEep->setEnabled(enable);
}

void MainWindow::EnableDebug(bool enable)
{
    ui->btnDebug->setEnabled(enable);
    ui->lblDebug->setEnabled(enable);
}

void MainWindow::EnableAll(bool enable)
{
    EnableECUlist(enable);
    EnableEeprom(enable);
    EnableFlash(enable);
    EnableDebug(enable);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Switch to a nerdier font
    QFont myfont ("Monospace");
    myfont.setStyleHint(QFont::Monospace);
    ui->textBrowser->setFont(myfont);

    // Reset list, clear progress etc
    ClearECUList();
    PushProgress(0);

    // Enable triggers for buttons
    connect(ui->btnReadEEP , SIGNAL (released()),this, SLOT (btnDumpEepromClick()));
    connect(ui->btnWriteEEP, SIGNAL (released()),this, SLOT (btnFlashEepromClick()));
    connect(ui->btnDump    , SIGNAL (released()),this, SLOT (btnDumpClick()));
    connect(ui->btnFlash   , SIGNAL (released()),this, SLOT (btnFlashClick()));
    connect(ui->comboBox   , SIGNAL (currentIndexChanged(int)),this, SLOT (ECUIndexChange(int)));
    connect(ui->btnDebug   , SIGNAL (released()),this, SLOT (btnDebugClick()));
}

MainWindow::~MainWindow()
{   delete ui; }

void MainWindow::closeEvent(QCloseEvent *event)
{
    glue_.SignalShutdown();
    event->accept();
}
