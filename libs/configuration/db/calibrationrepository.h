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
    const SignalId &signalId,
    const Calibration &calibration);

  bool addBySignalType(
    const SignalTypeId &signalTypeId,
    const Calibration &calibration);

  bool calibrateBySignal(
    const SignalId &signalId,
    double x,
    double& result) const;

  bool calibrateBySignalType(
    const SignalTypeId &signalTypeId,
    double x,
    double& result) const;

  [[nodiscard]]
  std::size_t sizeSignals() const noexcept;

  [[nodiscard]]
  std::size_t sizeSignalTypes() const noexcept;

  void clear() noexcept;

private:

  std::unordered_map<
    SignalId,
    Calibration> m_bySignal;

  std::unordered_map<
    SignalTypeId,
    Calibration> m_bySignalType;
};

}