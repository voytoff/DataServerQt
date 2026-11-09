#include "timechartview.h"

#include <QChart>
#include <QDateTime>
#include <QTimeZone>
#include <QValueAxis>
#include <QXYSeries>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QPen>
#include <QStyleHints>
#include <QApplication>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace qds
{

namespace
{

// минимальный временной диапазон (1 мс), согласованный с ограничением масштабирования колёсиком
constexpr double MinimumVisibleTimeRange = 0.001;
// минимальная ширина выделения на экране
constexpr qreal MinimumSelectionWidth = 5.0;

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

  auto* chr = chart();

  chr->addAxis(m_axisX, Qt::AlignBottom);
  chr->addAxis(m_axisY, Qt::AlignLeft);

  chr->legend()->setVisible(true);

  setTimeRange(0.0, 10.0);
  setValueRange(-1.0, 1.0);

  connect(
    QGuiApplication::styleHints(),
    &QStyleHints::colorSchemeChanged,
    this,
    [this](Qt::ColorScheme)
    {
      updateChartTheme();
    });

  TimeChartView::updateChartTheme();

  // Отслеживаем движение мыши без нажатых кнопок.
  setMouseTracking(true);
  viewport()->setMouseTracking(true);
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
  autoScaleY(m_axisY);

  if (m_yAxisMode == YAxisMode::Multiple)
  {
    for (auto* axis : m_additionalAxesY)
      autoScaleY(axis);
  }
}

void TimeChartView::autoScaleY(
  QValueAxis* axis)
{
  if (!axis)
    return;

  const double first = m_axisX->min();
  const double last = m_axisX->max();

  double minimum = std::numeric_limits<double>::max();
  double maximum = std::numeric_limits<double>::lowest();

  bool hasValues = false;

  auto includeValue = [&](double value)
  {
    if (!std::isfinite(value))
      return;

    minimum = std::min(minimum, value);
    maximum = std::max(maximum, value);
    hasValues = true;
  };

  for (auto* series : chart()->series())
  {
    auto* xySeries = qobject_cast<QXYSeries*>(series);

    if (!xySeries || !xySeries->isVisible())
      continue;

    if (!xySeries->attachedAxes().contains(axis))
      continue;

    const auto points = xySeries->points();

    // Учитываем отсчёты внутри видимого диапазона.
    for (const auto& point : points)
    {
      if (point.x() >= first && point.x() <= last)
        includeValue(point.y());
    }

    // Интерполяция нужна только для линейных серий.
    if (!qobject_cast<QLineSeries*>(xySeries))
      continue;

    for (qsizetype i = 1; i < points.size(); ++i)
    {
      const auto& p0 = points[i - 1];
      const auto& p1 = points[i];

      const double x0 = p0.x();
      const double x1 = p1.x();

      if (!std::isfinite(x0) ||
          !std::isfinite(x1) ||
          !std::isfinite(p0.y()) ||
          !std::isfinite(p1.y()))
        continue;

      if (x1 <= x0)
        continue;

      // Отрезок не пересекает видимое окно.
      if (x1 < first || x0 > last)
        continue;

      const double left = std::max(first, x0);
      const double right = std::min(last, x1);

      const double dx = x1 - x0;
      const double dy = p1.y() - p0.y();

      const double leftValue =
        p0.y() + (left - x0) / dx * dy;

      const double rightValue =
        p0.y() + (right - x0) / dx * dy;

      includeValue(leftValue);
      includeValue(rightValue);
    }
  }

  if (!hasValues)
    return;

  const double padding =
    minimum == maximum
      ? std::max(
          std::abs(minimum) * YConstantPaddingFactor,
          YMinimumPadding)
      : (maximum - minimum) * YRangePaddingFactor;

  axis->setRange(
    minimum - padding,
    maximum + padding);
}

void TimeChartView::resetTimeRange()
{
  applyTimeRange(
    m_initialTimeMinimum,
    m_initialTimeMaximum);
}

void TimeChartView::clearTimeRange()
{
  m_panning = false;
  m_selectingTimeRange = false;

  m_initialTimeMinimum = 0.0;
  m_initialTimeMaximum = 10.0;

  m_axisX->setRange(
    m_initialTimeMinimum,
    m_initialTimeMaximum);

  updateTimeLabels();
  viewport()->update();
}

std::pair<double, double>
TimeChartView::visibleTimeRange() const noexcept
{
  return {
    m_axisX->min(),
    m_axisX->max()
  };
}

void TimeChartView::setYAxisMode(YAxisMode mode)
{
  if (m_yAxisMode == mode)
    return;

  m_yAxisMode = mode;

  autoScaleY();
}

YAxisMode TimeChartView::yAxisMode() const noexcept
{
  return m_yAxisMode;
}

QValueAxis* TimeChartView::addValueAxis(
  Qt::Alignment alignment)
{
  if (alignment != Qt::AlignLeft &&
      alignment != Qt::AlignRight)
  {
    return nullptr;
  }

  auto* axis = new QValueAxis();

  axis->setTitleText(tr("Значение"));
  axis->setRange(-1.0, 1.0);

  chart()->addAxis(axis, alignment);

  m_additionalAxesY.push_back(axis);

  return axis;
}

void TimeChartView::removeValueAxis(
  QValueAxis* axis)
{
  if (!axis || axis == m_axisY)
    return;

  const auto it = std::find(
    m_additionalAxesY.begin(),
    m_additionalAxesY.end(),
    axis);

  if (it == m_additionalAxesY.end())
    return;

  for (auto* series : chart()->series())
  {
    if (series->attachedAxes().contains(axis))
    {
      series->detachAxis(axis);
      series->attachAxis(m_axisY);
    }
  }

  m_additionalAxesY.erase(it);

  chart()->removeAxis(axis);
  delete axis;

  autoScaleY();
}

bool TimeChartView::setValueAxisAlignment(
  QValueAxis* axis,
  Qt::Alignment alignment)
{
  if (!axis ||
      (alignment != Qt::AlignLeft &&
       alignment != Qt::AlignRight))
    return false;

  if (axis == m_axisY)
    return alignment == Qt::AlignLeft;

  if (std::find(
        m_additionalAxesY.begin(),
        m_additionalAxesY.end(),
        axis) == m_additionalAxesY.end())
    return false;

  if (axis->alignment() == alignment)
    return true;

  QList<QAbstractSeries*> attached;

  for (auto* series : chart()->series())
  {
    if (series->attachedAxes().contains(axis))
      attached.append(series);
  }

  chart()->removeAxis(axis);
  chart()->addAxis(axis, alignment);

  for (auto* series : attached)
    series->attachAxis(axis);

  return true;
}

void TimeChartView::setAvailableTimeRange(
  double minimum,
  double maximum)
{
  if (!std::isfinite(minimum) ||
      !std::isfinite(maximum) ||
      minimum >= maximum)
  {
    return;
  }

  const auto [visibleMinimum, visibleMaximum] =
    visibleTimeRange();

  m_initialTimeMinimum = minimum;
  m_initialTimeMaximum = maximum;

  const auto [clampedMinimum, clampedMaximum] =
    clampTimeRange(visibleMinimum, visibleMaximum);

  applyTimeRange(clampedMinimum, clampedMaximum);
}

void TimeChartView::updateChartTheme()
{
  const auto scheme =
    QGuiApplication::styleHints()->colorScheme();

  chart()->setTheme(
    scheme == Qt::ColorScheme::Dark
      ? QChart::ChartThemeDark
      : QChart::ChartThemeLight);

  chart()->setPlotAreaBackgroundVisible(true);
  chart()->setPlotAreaBackgroundBrush(Qt::NoBrush);

  chart()->setPlotAreaBackgroundPen(
    QPen(QColor(210, 210, 210), 1.0));

  viewport()->update();
}

void TimeChartView::setMeasurementCursorLocked(bool locked)
{
  if (m_measurementCursorLocked == locked)
    return;

  m_measurementCursorLocked = locked;

  if (!locked)
    updateMeasurementCursor(
      mapFromGlobal(QCursor::pos()));
}

bool TimeChartView::isMeasurementCursorLocked() const noexcept
{
  return m_measurementCursorLocked;
}

double TimeChartView::chartTimeAt(
  qreal positionX) const noexcept
{
  const QRectF plot = chart()->plotArea();

  if (plot.width() <= 0.0)
    return m_axisX->min();

  const qreal x = std::clamp(
    positionX,
    plot.left(),
    plot.right());

  const double ratio =
    (x - plot.left()) / plot.width();

  return m_axisX->min() +
         ratio * (m_axisX->max() - m_axisX->min());
}

void TimeChartView::updateTimeSelection(
  const QPointF& position)
{
  if (!m_selectingTimeRange)
    return;

  m_selectionEndX =
    chartTimeAt(position.x());

  viewport()->update();
}

void TimeChartView::setMeasurementCursorEnabled(bool enabled)
{
  if (m_measurementCursorEnabled == enabled)
    return;

  m_measurementCursorEnabled = enabled;

  if (!enabled)
  {
    setMeasurementCursorLocked(false);
    hideMeasurementCursor();
  }

  viewport()->update();
}

void TimeChartView::setMeasurementCursorPosition(
  const QPointF& position)
{
  if (!m_measurementCursorEnabled)
    return;

  if (!m_measurementCursorLocked)
    updateMeasurementCursor(position);
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

  if (newRange < MinimumVisibleTimeRange)
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
      event->modifiers().testFlag(Qt::ShiftModifier))
  {
    const QRectF plot = chart()->plotArea();

    if (plot.contains(event->position()))
    {
      m_selectingTimeRange = true;

      m_selectionStartPosition = event->position();

      m_selectionStartX =
        chartTimeAt(event->position().x());

      m_selectionEndX = m_selectionStartX;

      hideMeasurementCursor();

      viewport()->update();

      event->accept();
      return;
    }
  }

  if (event->button() == Qt::LeftButton &&
      chart()->plotArea().contains(event->position()))
  {
    hideMeasurementCursor();

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
  if (m_selectingTimeRange)
  {
    updateTimeSelection(event->position());

    event->accept();
    return;
  }

  if (!m_panning)
  {
    if (m_measurementCursorEnabled)
      updateMeasurementCursor(event->position());

    QChartView::mouseMoveEvent(event);
    return;
  }

  hideMeasurementCursor();

  const QRectF plot = chart()->plotArea();

  if (plot.width() <= 0.0)
  {
    event->accept();
    return;
  }

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
  if (m_selectingTimeRange &&
      event->button() == Qt::LeftButton)
  {
    updateTimeSelection(event->position());

    m_selectingTimeRange = false;

    const double minimum = std::min(
      m_selectionStartX,
      m_selectionEndX);

    const double maximum = std::max(
      m_selectionStartX,
      m_selectionEndX);

    const QRectF plot = chart()->plotArea();

    const qreal startX = std::clamp(
      m_selectionStartPosition.x(),
      plot.left(),
      plot.right());

    const qreal endX = std::clamp(
      event->position().x(),
      plot.left(),
      plot.right());

    const qreal distance = std::abs(endX - startX);

    if (distance >= MinimumSelectionWidth &&
        maximum - minimum >= MinimumVisibleTimeRange)
    {
      setVisibleTimeRange(minimum, maximum);
    }

    viewport()->update();

    event->accept();
    return;
  }

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

void TimeChartView::leaveEvent(QEvent* event)
{
  hideMeasurementCursor();

  QChartView::leaveEvent(event);
}

void TimeChartView::drawForeground(
  QPainter* painter,
  const QRectF& rect)
{
  QChartView::drawForeground(painter, rect);

  if (m_selectingTimeRange)
  {
    const QRectF plot = chart()->plotArea();

    const double range =
      m_axisX->max() - m_axisX->min();

    if (plot.width() > 0.0 && range > 0.0)
    {
      const auto toPixelX =
        [&](double time)
      {
        return plot.left() +
               (time - m_axisX->min()) *
                 plot.width() / range;
      };

      const qreal x1 = toPixelX(m_selectionStartX);
      const qreal x2 = toPixelX(m_selectionEndX);

      const QRectF selection(
        QPointF(std::min(x1, x2), plot.top()),
        QPointF(std::max(x1, x2), plot.bottom()));

      painter->save();

      painter->setPen(
        QPen(QColor(65, 130, 220, 190), 1.0));

      painter->setBrush(
        QColor(65, 130, 220, 50));

      painter->drawRect(selection);

      painter->restore();
    }
  }

  if (!m_measurementCursorVisible)
    return;

  const QRectF plot = chart()->plotArea();

  const double minimum = m_axisX->min();
  const double maximum = m_axisX->max();

  if (plot.width() <= 0.0 ||
      maximum <= minimum)
    return;

  const double ratio =
    (m_measurementCursorX - minimum) /
    (maximum - minimum);

  if (ratio < 0.0 || ratio > 1.0)
    return;

  const double x =
    plot.left() + ratio * plot.width();

  painter->save();

  const bool dark =
    QGuiApplication::styleHints()->colorScheme() ==
    Qt::ColorScheme::Dark;

  const QColor cursorColor =
    dark
      ? QColor(200, 200, 200, 170)
      : QColor(100, 100, 100, 180);

  painter->setPen(
    QPen(cursorColor, 1.0, Qt::DashLine));

  painter->drawLine(
    QPointF(x, plot.top()),
    QPointF(x, plot.bottom()));

  painter->restore();
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

  viewport()->update();
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

void TimeChartView::updateMeasurementCursor(
  const QPointF& position)
{
  if (!m_measurementCursorEnabled)
  {
    hideMeasurementCursor();
    return;
  }

  if (m_measurementCursorLocked)
    return;

  const QRectF plot = chart()->plotArea();

  if (!plot.contains(position) ||
      plot.width() <= 0.0)
  {
    hideMeasurementCursor();
    return;
  }

  const double minimum = m_axisX->min();
  const double maximum = m_axisX->max();

  const double ratio =
    (position.x() - plot.left()) /
    plot.width();

  m_measurementCursorX =
    minimum + ratio * (maximum - minimum);

  m_measurementCursorVisible = true;

  viewport()->update();

  emit measurementCursorMoved(m_measurementCursorX);
}

void TimeChartView::hideMeasurementCursor()
{
  if (m_measurementCursorLocked)
    return;

  if (!m_measurementCursorVisible)
    return;

  m_measurementCursorVisible = false;

  viewport()->update();

  emit measurementCursorHidden();
}

void TimeChartView::updateTimeLabels()
{
  if (!m_axisX)
    return;

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