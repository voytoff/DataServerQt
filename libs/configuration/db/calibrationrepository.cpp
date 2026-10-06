#include "calibrationrepository.h"

namespace qds
{

bool CalibrationRepository::addBySignal(const SignalId &signalId, const Calibration &calibration)
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

bool CalibrationRepository::addBySignalType(const SignalTypeId &signalTypeId, const Calibration &calibration)
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

bool CalibrationRepository::calibrateBySignal(const SignalId &signalId, double x, double &result) const
{
  auto it = m_bySignal.find(signalId);
  if (it == m_bySignal.end())
    return false;

  return it->second.apply(x, result);
}

bool CalibrationRepository::calibrateBySignalType(const SignalTypeId &signalTypeId, double x, double &result) const
{
  auto it = m_bySignalType.find(signalTypeId);
  if (it == m_bySignalType.end())
    return false;

  return it->second.apply(x, result);
}

std::size_t CalibrationRepository::sizeSignals() const noexcept
{
  return m_bySignal.size();
}

std::size_t CalibrationRepository::sizeSignalTypes() const noexcept
{
  return m_bySignalType.size();
}

void CalibrationRepository::clear() noexcept
{
  m_bySignal.clear();
  m_bySignalType.clear();
}

}
