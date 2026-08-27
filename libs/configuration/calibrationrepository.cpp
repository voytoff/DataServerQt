#include "calibrationrepository.h"

namespace qds
{

bool CalibrationRepository::addBySignal(SignalId signalId, Calibration calibration)
{
  auto exists = m_bySignal.find(signalId);

  if (exists != m_bySignal.end())
    return false;

  auto [it, inserted] =
    m_bySignal.emplace(
      signalId,
      calibration);

  return inserted;
}

bool CalibrationRepository::addBySignalType(SignalTypeId signalTypeId, Calibration calibration)
{
  auto exists = m_bySignalType.find(signalTypeId);

  if (exists != m_bySignalType.end())
    return false;

  auto [it, inserted] =
    m_bySignalType.emplace(
      signalTypeId,
      calibration);

  return inserted;
}

bool CalibrationRepository::calibrateBySignal(SignalId signalId, double x, double &result) const
{

}

bool CalibrationRepository::calibrateBySignalType(SignalTypeId signalTypeId, double x, double &result) const
{

}


}
