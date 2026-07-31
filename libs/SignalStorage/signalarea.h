#ifndef SIGNALAREA_H
#define SIGNALAREA_H

#include "signalstorage.h"

namespace qds
{

class SignalArea
{
public:

  SignalStorage raw;

  SignalStorage calculated;
};

}

#endif // SIGNALAREA_H
