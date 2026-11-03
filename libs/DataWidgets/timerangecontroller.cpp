#include "timerangecontroller.h"
#include "timechartview.h"

#include <algorithm>
#include <cmath>

namespace qds
{

TimeRangeController::TimeRangeController(QObject* parent)
  : QObject(parent)
{
}

double TimeRangeController::minimum() const noexcept
{
  return m_minimum;
}

double TimeRangeController::maximum() const noexcept
{
  return m_maximum;
}

void TimeRangeController::setRange(
  double minimum,
  double maximum)
{
  if (!std::isfinite(minimum) ||
      !std::isfinite(maximum) ||
      minimum >= maximum)
  {
    return;
  }

  if (m_minimum == minimum &&
      m_maximum == maximum)
  {
    return;
  }

  m_minimum = minimum;
  m_maximum = maximum;

  emit rangeChanged(minimum, maximum);
}

void TimeRangeController::addView(TimeChartView* view)
{
  if (!view)
    return;

  connect(
    this,
    &TimeRangeController::rangeChanged,
    view,
    &TimeChartView::setVisibleTimeRange,
    Qt::UniqueConnection);

  connect(
    view,
    &TimeChartView::visibleTimeRangeChanged,
    this,
    &TimeRangeController::setRange,
    Qt::UniqueConnection);

  view->setVisibleTimeRange(m_minimum, m_maximum);
}

void TimeRangeController::removeView(TimeChartView* view)
{
  if (!view)
    return;

  disconnect(
    this,
    &TimeRangeController::rangeChanged,
    view,
    &TimeChartView::setVisibleTimeRange);

  disconnect(
    view,
    &TimeChartView::visibleTimeRangeChanged,
    this,
    &TimeRangeController::setRange);
}

}