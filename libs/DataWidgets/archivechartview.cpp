
#include "archivechartview.h"

#include "archivechartcontroller.h"
#include "archivechartseries.h"
#include "timeaxis.h"

#include <QChart>
#include <QDebug>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>

namespace qds
{

namespace
{

ArchiveChartSeries::Segments toChartSegments(
  const ArchiveSignalSegments& segments,
  const TimeAxis& timeAxis)
{
  ArchiveChartSeries::Segments result;
  result.reserve(segments.size());

  for (const auto& segment : segments)
  {
    QVector<QPointF> points;
    points.reserve(segment.points.size());

    for (const auto& sample : segment.points)
    {
      if (!std::isfinite(sample.value))
        continue;

      points.append(QPointF(
        timeAxis.chartSeconds(sample.timestamp),
        sample.value));
    }

    if (!points.empty())
      result.push_back(std::move(points));
  }

  return result;
}

Timestamp toTimestamp(
  double seconds,
  Timestamp origin)
{
  const auto microseconds =
    static_cast<int64_t>(
      std::llround(seconds * 1'000'000.0));

  if (microseconds < 0)
  {
    const auto delta =
      static_cast<uint64_t>(-(microseconds + 1)) + 1;

    return Timestamp{
      delta > origin.value
        ? 0
        : origin.value - delta
    };
  }

  const auto delta =
    static_cast<uint64_t>(microseconds);

  return Timestamp{
    delta > UINT64_MAX - origin.value
      ? UINT64_MAX
      : origin.value + delta
  };
}

}

struct ArchiveChartView::SignalContext
{
  SignalId id;

  ArchiveChartController* controller = nullptr;
  ArchiveChartSeries* series = nullptr;
  QValueAxis* axisY = nullptr;

  Timestamp firstTimestamp{};
  Timestamp lastTimestamp{};

  bool ready = false;
};

struct ArchiveChartView::Impl
{
  std::filesystem::path archivePath;

  std::vector<std::unique_ptr<SignalContext>> entries;

  TimeAxis* timeAxis = nullptr;

  bool initialized = false;
};

ArchiveChartView::ArchiveChartView(
  QWidget* parent)
  : TimeChartView(parent),
  m_impl(std::make_unique<Impl>())
{
  m_impl->timeAxis = new TimeAxis(this);
  setTimeAxis(m_impl->timeAxis);

  setYAxisMode(YAxisMode::Multiple);

  connect(
    this,
    &TimeChartView::visibleTimeRangeChanged,
    this,
    [this](double, double)
    {
      requestVisibleData();
    });
}

ArchiveChartView::~ArchiveChartView()
{
  clearSignals();
}

void ArchiveChartView::setArchivePath(
  const std::filesystem::path& path)
{
  if (!m_impl->entries.empty())
  {
    qWarning() <<
      "ArchiveChartView: clearSignals() required "
      "before changing archive path";
    return;
  }

  m_impl->archivePath = path;
}

bool ArchiveChartView::addSignal(
  SignalId id,
  const QString& name,
  const QColor& color)
{
  if (m_impl->archivePath.empty())
    return false;

  if (m_impl->initialized)
  {
    qWarning() <<
      "Adding signals after initialization "
      "is not supported yet";
    return false;
  }

  for (const auto& signal : m_impl->entries)
  {
    if (signal->id == id)
      return false;
  }

  auto signal = std::make_unique<SignalContext>();
  signal->id = id;

  auto* controller = new ArchiveChartController(
    m_impl->archivePath,
    this);

  auto* series = new ArchiveChartSeries(this);

  series->setName(name);
  series->setColor(color);

  if (!m_impl->entries.empty())
  {
    const auto index = m_impl->entries.size();

    auto* axis = addValueAxis(
      index % 2 == 0
        ? Qt::AlignLeft
        : Qt::AlignRight);

    axis->setTitleText(name);

    signal->axisY = axis;
    series->setValueAxis(axis);
  }
  else
  {
    valueChartAxis()->setTitleText(name);
  }

  signal->controller = controller;
  signal->series = series;

  auto* context = signal.get();

  m_impl->entries.push_back(std::move(signal));

  connect(
    controller,
    &ArchiveChartController::timeRangeReady,
    this,
    [this, context](
      bool success,
      Timestamp first,
      Timestamp last)
    {
      if (!success)
      {
        qWarning() <<
          "Failed to read signal time range";
        return;
      }

      context->firstTimestamp = first;
      context->lastTimestamp = last;
      context->ready = true;

      initializeTimeRange();
    });

  connect(
    controller,
    &ArchiveChartController::plotReady,
    this,
    [this, context](
      bool success,
      ArchiveSignalSegments segments)
    {
      if (!success)
      {
        qWarning() <<
          "Failed to read archive plot";
        return;
      }

      context->series->setSegments(
        toChartSegments(
          segments,
          *m_impl->timeAxis));
    });

  controller->requestTimeRange(id);

  return true;
}

void ArchiveChartView::clearSignals()
{
  for (auto& entry : m_impl->entries)
  {
    delete entry->controller;
    entry->controller = nullptr;
  }

  for (auto& entry : m_impl->entries)
  {
    delete entry->series;
    entry->series = nullptr;
  }

  for (auto& entry : m_impl->entries)
  {
    if (entry->axisY)
    {
      removeValueAxis(entry->axisY);
      entry->axisY = nullptr;
    }
  }

  m_impl->entries.clear();

  m_impl->initialized = false;
}

void ArchiveChartView::initializeTimeRange()
{
  if (m_impl->initialized ||
      m_impl->entries.empty())
    return;

  for (const auto& signal : m_impl->entries)
  {
    if (!signal->ready)
      return;
  }

  Timestamp first =
    m_impl->entries.front()->firstTimestamp;

  Timestamp last =
    m_impl->entries.front()->lastTimestamp;

  for (const auto& signal : m_impl->entries)
  {
    first = std::min(
      first,
      signal->firstTimestamp);

    last = std::max(
      last,
      signal->lastTimestamp);
  }

  if (last <= first)
    return;

  m_impl->initialized = true;

  m_impl->timeAxis->setOriginTimestamp(first);
  m_impl->timeAxis->setZeroTimestamp(first);

  const double duration =
    m_impl->timeAxis->chartSeconds(last);

  setTimeRange(0.0, duration);

  // setTimeRange() может не выдать сигнал,
  // если диапазон уже совпадает.
  requestVisibleData();
}

void ArchiveChartView::requestVisibleData()
{
  if (!m_impl->initialized)
    return;

  const auto [minimum, maximum] =
    visibleTimeRange();

  const auto origin =
    m_impl->timeAxis->originTimestamp();

  const auto first =
    toTimestamp(minimum, origin);

  const auto last =
    toTimestamp(maximum, origin);

  const double width =
    chart()->plotArea().width();

  const auto maxPoints =
    static_cast<std::size_t>(
      std::max(2.0, width * 2.0));

  for (const auto& signal : m_impl->entries)
  {
    signal->controller->requestPlot(
      signal->id,
      first,
      last,
      maxPoints);
  }
}

std::size_t
ArchiveChartView::signalCount() const noexcept
{
  return m_impl->entries.size();
}

}
