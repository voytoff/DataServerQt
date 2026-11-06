#include "timechartview.h"

#include <QChart>
#include <QDateTime>
#include <QTimeZone>
#include <QValueAxis>
#include <QXYSeries>
#include <QMouseEvent>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace qds
{

namespace
{

constexpr double MicrosecondsPerSecond = 1'000'000.0;
constexpr int TickCount = 6;
// отступ 10% от диапазона значений, когда минимум и максимум различаются.
constexpr double YRangePaddingFactor = 0.10;
// отступ 10% от абсолютного значения, когда все точки одинаковые.
constexpr double YConstantPaddingFactor = 0.10;
// минимальный отступ 0.00001, чтобы график не схлопывался около нуля.
constexpr double YMinimumPadding = 1e-5;

}

TimeChartView::TimeChartView(QWidget* parent)
  : QChartView(new QChart(), parent)
{
  setRenderHint(QPainter::Antialiasing);

  m_axisX = new QCategoryAxis();
  m_axisY = new QValueAxis();

  m_axisX->setLabelsPosition(
    QCategoryAxis::AxisLabelsPositionOnValue);

  m_axisX->setTitleText(tr("Время"));
  m_axisY->setTitleText(tr("Значение"));

  chart()->addAxis(m_axisX, Qt::AlignBottom);
  chart()->addAxis(m_axisY, Qt::AlignLeft);

  chart()->legend()->setVisible(true);
  chart()->legend()->hide();

  setTimeRange(0.0, 10.0);
  setValueRange(-1.0, 1.0);

  chart()->setPlotAreaBackgroundVisible(true);

  chart()->setPlotAreaBackgroundBrush(Qt::NoBrush);

  chart()->setPlotAreaBackgroundPen(
    QPen(QColor(210, 210, 210), 1.0));
}

void TimeChartView::setTimeAxis(TimeAxis* timeAxis)
{
  if (m_timeAxis == timeAxis)
    return;

  if (m_timeAxis)
  {
    disconnect(
      m_timeAxis,
      nullptr,
      this,
      nullptr);
  }

  m_timeAxis = timeAxis;

  if (m_timeAxis)
  {
    connect(
      m_timeAxis,
      &TimeAxis::changed,
      this,
      &TimeChartView::updateTimeLabels);

    connect(
      m_timeAxis,
      &QObject::destroyed,
      this,
      [this]()
      {
        m_timeAxis = nullptr;
        updateTimeLabels();
      });
  }

  updateTimeLabels();
}

TimeAxis* TimeChartView::timeAxis() const noexcept
{
  return m_timeAxis;
}

void TimeChartView::setWallClockReference(
  Timestamp timestamp,
  WallClockTime wallTime)
{
  m_referenceTimestamp = timestamp;
  m_referenceWallTime = wallTime;
  m_hasWallClockReference = true;

  updateTimeLabels();
}

void TimeChartView::addSeries(QXYSeries *series)
{
  if (!series)
    return;

  chart()->addSeries(series);

  series->attachAxis(m_axisX);
  series->attachAxis(m_axisY);
}

void TimeChartView::clearSeries()
{
  chart()->removeAllSeries();
}

void TimeChartView::setTimeRange(
  double minimum,
  double maximum)
{
  if (!std::isfinite(minimum) ||
      !std::isfinite(maximum) ||
      minimum >= maximum)
  {
    return;
  }

  m_initialTimeMinimum = minimum;
  m_initialTimeMaximum = maximum;

  applyTimeRange(minimum, maximum);
}

void TimeChartView::setValueRange(
  double minimum,
  double maximum)
{
  if (!std::isfinite(minimum) ||
      !std::isfinite(maximum) ||
      minimum >= maximum)
  {
    return;
  }

  m_axisY->setRange(minimum, maximum);
}

QCategoryAxis* TimeChartView::timeChartAxis() const noexcept
{
  return m_axisX;
}

QValueAxis* TimeChartView::valueChartAxis() const noexcept
{
  return m_axisY;
}

void TimeChartView::autoScaleY()
{
  double minimum = std::numeric_limits<double>::max();
  double maximum = std::numeric_limits<double>::lowest();

  bool hasValues = false;

  for (auto* series : chart()->series())
  {
    auto* xySeries = qobject_cast<QXYSeries*>(series);

    if (!xySeries || !xySeries->isVisible())
      continue;

    const auto points = xySeries->points();

    for (const auto& point : points)
    {
      const double value = point.y();

      if (!std::isfinite(value))
        continue;

      minimum = std::min(minimum, value);
      maximum = std::max(maximum, value);

      hasValues = true;
    }
  }

  if (!hasValues)
  {
    setValueRange(-1.0, 1.0);
    return;
  }

  if (minimum == maximum)
  {
    const double padding = std::max(
      std::abs(minimum) * YConstantPaddingFactor,
      YMinimumPadding);

    setValueRange(
      minimum - padding,
      maximum + padding);

    return;
  }

  const double padding =
    (maximum - minimum) * YRangePaddingFactor;

  setValueRange(
    minimum - padding,
    maximum + padding);
}

void TimeChartView::resetTimeRange()
{
  applyTimeRange(
    m_initialTimeMinimum,
    m_initialTimeMaximum);
}

std::pair<double, double>
TimeChartView::visibleTimeRange() const noexcept
{
  return {
    m_axisX->min(),
    m_axisX->max()
  };
}

void TimeChartView::setVisibleTimeRange(
  double minimum,
  double maximum)
{
  applyTimeRange(minimum, maximum);
}

void TimeChartView::wheelEvent(QWheelEvent* event)
{
  const QRectF plot = chart()->plotArea();

  const QPointF position = event->position();

  if (!plot.contains(position))
  {
    QChartView::wheelEvent(event);
    return;
  }

  const QPoint angle = event->angleDelta();

  if (angle.y() == 0)
  {
    event->ignore();
    return;
  }

  const double minimum = m_axisX->min();
  const double maximum = m_axisX->max();

  const double range = maximum - minimum;

  if (range <= 0.0)
    return;

  constexpr double zoomFactor = 1.2;

  const double steps =
    static_cast<double>(angle.y()) / 120.0;

  const double factor =
    std::pow(zoomFactor, -steps);

  //const double newRange = range * factor;
  const double initialRange =
    m_initialTimeMaximum - m_initialTimeMinimum;

  const double newRange =
    std::min(range * factor, initialRange);

  constexpr double minimumRange = 0.001;

  if (newRange < minimumRange)
  {
    event->accept();
    return;
  }

  const double ratio =
    (position.x() - plot.left()) / plot.width();

  const double anchor =
    minimum + range * ratio;

  const double newMinimum =
    anchor - newRange * ratio;

  const double newMaximum =
    newMinimum + newRange;

  applyTimeRange(newMinimum, newMaximum);

  event->accept();
}

void TimeChartView::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton &&
      chart()->plotArea().contains(event->position()))
  {
    m_panning = true;
    m_lastMousePosition = event->position();

    setCursor(Qt::ClosedHandCursor);

    event->accept();
    return;
  }

  QChartView::mousePressEvent(event);
}

void TimeChartView::mouseMoveEvent(QMouseEvent* event)
{
  if (!m_panning)
  {
    QChartView::mouseMoveEvent(event);
    return;
  }

  const QRectF plot = chart()->plotArea();

  if (plot.width() <= 0.0)
    return;

  const double dx =
    event->position().x() - m_lastMousePosition.x();

  const double minimum = m_axisX->min();
  const double maximum = m_axisX->max();

  const double range = maximum - minimum;

  const double shift =
    dx * range / plot.width();

  applyTimeRange(
    minimum - shift,
    maximum - shift);

  m_lastMousePosition = event->position();

  event->accept();
}

void TimeChartView::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton && m_panning)
  {
    m_panning = false;

    unsetCursor();

    event->accept();
    return;
  }

  QChartView::mouseReleaseEvent(event);
}

void TimeChartView::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton &&
      chart()->plotArea().contains(event->position()))
  {
    m_panning = false;
    unsetCursor();

    resetTimeRange();

    event->accept();
    return;
  }

  QChartView::mouseDoubleClickEvent(event);
}

QString TimeChartView::formatTime(double chartSeconds) const
{
  if (!m_timeAxis)
    return QString::number(chartSeconds, 'f', 3);

  const double timestampMicroseconds =
    static_cast<double>(m_timeAxis->originTimestamp().value) +
    chartSeconds * MicrosecondsPerSecond;

  if (!std::isfinite(timestampMicroseconds) ||
      timestampMicroseconds < 0.0 ||
      timestampMicroseconds >=
        static_cast<double>(std::numeric_limits<uint64_t>::max()))
  {
    return {};
  }

  const Timestamp timestamp{
    static_cast<uint64_t>(std::llround(timestampMicroseconds))
  };

  if (m_timeAxis->mode() == TimeAxisMode::RelativeSeconds)
  {
    return QString::number(
      m_timeAxis->relativeSeconds(timestamp),
      'f',
      3);
  }

  if (!m_hasWallClockReference)
    return {};

  const double deltaMicroseconds =
    static_cast<double>(timestamp.value) -
    static_cast<double>(m_referenceTimestamp.value);

  const double unixMicroseconds =
    static_cast<double>(m_referenceWallTime.unixMicroseconds) +
    deltaMicroseconds;

  if (!std::isfinite(unixMicroseconds))
    return {};

  const qint64 milliseconds =
    static_cast<qint64>(std::llround(unixMicroseconds / 1000.0));

  return QDateTime::fromMSecsSinceEpoch(
           milliseconds,
           QTimeZone::systemTimeZone()).toString(
      "HH:mm:ss.zzz");
}

void TimeChartView::applyTimeRange(
  double minimum,
  double maximum)
{
  if (!std::isfinite(minimum) ||
      !std::isfinite(maximum) ||
      minimum >= maximum)
  {
    return;
  }

  auto [clampedMinimum, clampedMaximum] =
    clampTimeRange(minimum, maximum);

  minimum = clampedMinimum;
  maximum = clampedMaximum;

  if (m_axisX->min() == minimum &&
      m_axisX->max() == maximum)
  {
    return;
  }

  m_axisX->setRange(minimum, maximum);

  updateTimeLabels();

  emit visibleTimeRangeChanged(minimum, maximum);
}

std::pair<double, double> TimeChartView::clampTimeRange(
  double minimum,
  double maximum) const noexcept
{
  const double initialMinimum = m_initialTimeMinimum;
  const double initialMaximum = m_initialTimeMaximum;

  const double initialWidth =
    initialMaximum - initialMinimum;

  const double width = maximum - minimum;

  if (width >= initialWidth)
    return {initialMinimum, initialMaximum};

  if (minimum < initialMinimum)
  {
    minimum = initialMinimum;
    maximum = minimum + width;
  }

  if (maximum > initialMaximum)
  {
    maximum = initialMaximum;
    minimum = maximum - width;
  }

  return {minimum, maximum};
}

void TimeChartView::updateTimeLabels()
{
  const auto labels = m_axisX->categoriesLabels();

  for (const auto& label : labels)
    m_axisX->remove(label);

  const double minimum = m_axisX->min();
  const double maximum = m_axisX->max();

  if (!std::isfinite(minimum) ||
      !std::isfinite(maximum) ||
      minimum >= maximum)
  {
    return;
  }

  const double step =
    (maximum - minimum) / (TickCount - 1);

  // QCategoryAxis использует верхнюю границу категории.
  // Первую подпись помещаем в начало диапазона.
  for (int i = 0; i < TickCount; ++i)
  {
    const double value =
      minimum + step * i;

    const QString label = formatTime(value);

    m_axisX->append(
      label.isEmpty() ? QString(" ") : label,
      value);
  }
  /*
for (int i = 0; i < TickCount; ++i)
{
  const double value =
    minimum + step * i;

  const QString label = formatTime(value);

  if (i == 0)
  {
    m_axisX->setStartValue(minimum);
    continue;
  }

  m_axisX->append(
    label.isEmpty() ? QString(" ") : label,
    value);
}  */
}

}