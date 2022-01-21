#ifndef MAIN_H
#define MAIN_H

#include <QApplication>

namespace bdmstuff {
    class glue;
}

class glue
{
public:
    void DisplayDebug(int index);
    void ECUIndexLogic(int index);
    void SignalShutdown();
    void CastMessage(const char *msg);
    void CastProgress(uint32_t prog);
};

#endif // MAIN_H
