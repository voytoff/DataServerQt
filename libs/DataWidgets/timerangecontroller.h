#pragma once

#include <QObject>
#include <QPointer>
#include <QVector>

#include "datawidgets_export.h"

namespace qds
{

class TimeChartView;

class DATAWIDGETS_EXPORT TimeRangeController : public QObject
{
  Q_OBJECT

public:
  explicit TimeRangeController(QObject* parent = nullptr);

  [[nodiscard]] double minimum() const noexcept;
  [[nodiscard]] double maximum() const noexcept;

  void setRange(double minimum, double maximum);

  void addView(TimeChartView* view);
  void removeView(TimeChartView* view);

signals:
  void rangeChanged(double minimum, double maximum);

private:
  double m_minimum = 0.0;
  double m_maximum = 10.0;

};

}