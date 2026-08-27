#pragma once

#include <strongidhash.h>
#include "calibration.h"
#include "datatypes.h"
#include <unordered_map>

namespace  qds
{

class CalibrationRepository
{
public:

  bool addBySignal(
    SignalId signalId,
    Calibration calibration);

  bool addBySignalType(
    SignalTypeId signalTypeId,
    Calibration calibration);

  bool calibrateBySignal(
    SignalId signalId,
    double x,
    double& result) const;

  bool calibrateBySignalType(
    SignalTypeId signalTypeId,
    double x,
    double& result) const;

private:

  std::unordered_map<
    SignalId,
    Calibration> m_bySignal;

  std::unordered_map<
    SignalTypeId,
    Calibration> m_bySignalType;
};

}