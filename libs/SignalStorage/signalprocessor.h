#ifndef SIGNALPROCESSOR_H
#define SIGNALPROCESSOR_H

#include "signalstorage.h"

namespace qds
{

class SignalProcessor
{
public:

  SignalProcessor(
    SignalStorage& storage);


  void update(
    SignalId id,
    uint64_t timestamp,
    double value);


private:

  SignalStorage& m_storage;

};

}

#endif // SIGNALPROCESSOR_H
