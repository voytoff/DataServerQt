#include "timeaxis.h"

#include <QDateTime>
#include <QTimeZone>

namespace qds
{

namespace
{

constexpr double MicrosecondsPerSecond = 1'000'000.0;

double secondsBetween(
  Timestamp timestamp,
  Timestamp origin) noexcept
{
  if (timestamp >= origin)
  {
    return static_cast<double>(timestamp.value - origin.value)
    / MicrosecondsPerSecond;
  }

  return -static_cast<double>(origin.value - timestamp.value)
         / MicrosecondsPerSecond;
}

}

TimeAxis::TimeAxis(QObject* parent)
  : QObject(parent)
{
}

TimeAxisMode TimeAxis::mode() const noexcept
{
  return m_mode;
}

void TimeAxis::setMode(TimeAxisMode mode)
{
  if (m_mode == mode)
    return;

  m_mode = mode;
  emit changed();
}

Timestamp TimeAxis::originTimestamp() const noexcept
{
  return m_originTimestamp;
}

void TimeAxis::setOriginTimestamp(Timestamp timestamp)
{
  if (m_originTimestamp == timestamp)
    return;

  m_originTimestamp = timestamp;
  emit changed();
}

Timestamp TimeAxis::zeroTimestamp() const noexcept
{
  return m_zeroTimestamp;
}

void TimeAxis::setZeroTimestamp(Timestamp timestamp)
{
  if (m_zeroTimestamp == timestamp)
    return;

  m_zeroTimestamp = timestamp;
  emit changed();
}

double TimeAxis::chartSeconds(
  Timestamp timestamp) const noexcept
{
  return secondsBetween(timestamp, m_originTimestamp);
}

double TimeAxis::relativeSeconds(
  Timestamp timestamp) const noexcept
{
  return secondsBetween(timestamp, m_zeroTimestamp);
}

QString TimeAxis::format(
  Timestamp timestamp,
  WallClockTime wallTime) const
{
  if (m_mode == TimeAxisMode::RelativeSeconds)
  {
    return QString::number(
      relativeSeconds(timestamp),
      'f',
      3);
  }

  const auto milliseconds =
    wallTime.unixMicroseconds / 1000;

  return QDateTime::fromMSecsSinceEpoch(
           milliseconds,
           QTimeZone::systemTimeZone()).toString(
      "dd.MM.yyyy HH:mm:ss.zzz");
}

}