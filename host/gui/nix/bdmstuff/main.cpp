#include <worker.h>
#include <signal.h>
#include <QCloseEvent>
#include <QMainWindow>

#include "mainwindow.h"
#include "main.h"
#include "debugwindow.h"

#include "../../../core/core.h"

// We intend to share class pointers etc between functions of different classes.. ~ish. I hate C++..
static MainWindow  *wptr;
static debugwindow *dptr;

static void Cleanup()
{
    dptr->close();
    wptr->close();
}

void signalhandler(int sig)
{
  if(sig == SIGINT)
  {
    Cleanup();
    qApp->quit();
  }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    wptr = &w;
    debugwindow d;
    dptr = &d;

    // Push list of targets to the combobox
    w.PushECUItem("Select target");
    const uint noTarg = core_NoTargets();
    for (uint i = 1; i <= noTarg; i++ )
        w.PushECUItem(core_TargetName(i));

    // Index 0 is used as a label. Do not enable any buttons
    w.EnableAll(false);
    w.EnableECUlist(true);

    w.show();

    // Execute QT's main loop
    return a.exec();
}

void glue::DisplayDebug(int index)
{
    dptr->Populate(index);
    dptr->show();
}

void glue::CastMessage(const char *msg)
{
    wptr->PushTextMessage(msg);
}

void glue::CastProgress(uint32_t prog)
{
    wptr->PushProgress(prog);
}

void glue::ECUIndexLogic(int index)
{
    wptr->ECUIndexLogic(index);
}

void glue::SignalShutdown()
{
    Cleanup();
}
