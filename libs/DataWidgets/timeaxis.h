#pragma once

#include <QObject>
#include <QString>

#include <datatypes.h>

#include "datawidgets_export.h"

namespace qds
{

enum class TimeAxisMode
{
  LocalTime,
  RelativeSeconds
};

class DATAWIDGETS_EXPORT TimeAxis : public QObject
{
  Q_OBJECT

public:
  explicit TimeAxis(QObject* parent = nullptr);

  [[nodiscard]] TimeAxisMode mode() const noexcept;
  void setMode(TimeAxisMode mode);

  [[nodiscard]] Timestamp originTimestamp() const noexcept;
  void setOriginTimestamp(Timestamp timestamp);

  [[nodiscard]] Timestamp zeroTimestamp() const noexcept;
  void setZeroTimestamp(Timestamp timestamp);

  [[nodiscard]] double chartSeconds(
    Timestamp timestamp) const noexcept;

  [[nodiscard]] double relativeSeconds(
    Timestamp timestamp) const noexcept;

  [[nodiscard]] QString format(
    Timestamp timestamp,
    WallClockTime wallTime) const;

signals:
  void changed();

private:
  TimeAxisMode m_mode = TimeAxisMode::RelativeSeconds;

  Timestamp m_originTimestamp{};
  Timestamp m_zeroTimestamp{};
};

}