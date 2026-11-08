
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
  QString name;

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

  for (const auto& signal : m_impl->entries)
  {
    if (signal->id == id)
      return false;
  }

  auto entry = std::make_unique<SignalContext>();
  entry->id = id;
  entry->name = name;

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

    entry->axisY = axis;
    series->setValueAxis(axis);
  }
  else
  {
    entry->axisY = valueChartAxis();
    entry->axisY->setTitleText(name);
  }

  entry->controller = controller;
  entry->series = series;

  auto* context = entry.get();

  m_impl->entries.push_back(std::move(entry));

  updateAxisLayout();

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

      const bool wasInitialized = m_impl->initialized;

      updateTimeRange();

      if (wasInitialized)
        requestSignalData(context);
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

  for (const auto& entry : m_impl->entries)
    requestSignalData(entry.get());
}

void ArchiveChartView::requestSignalData(
  SignalContext* context)
{
  if (!m_impl->initialized ||
      !context ||
      !context->ready)
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

  context->controller->requestPlot(
    context->id,
    first,
    last,
    maxPoints);
}

void ArchiveChartView::updateAxisLayout()
{
  const auto& entries = m_impl->entries;

  for (std::size_t i = 0; i < entries.size(); ++i)
  {
    auto* axis = entries[i]->axisY;

    const auto alignment =
      i % 2 == 0
        ? Qt::AlignLeft
        : Qt::AlignRight;

    setValueAxisAlignment(axis, alignment);
  }

  autoScaleY();
}

void ArchiveChartView::updateTimeRange()
{
  if (m_impl->entries.empty())
    return;

  bool found = false;

  Timestamp first{};
  Timestamp last{};

  for (const auto& entry : m_impl->entries)
  {
    if (!entry->ready)
      continue;

    if (!found)
    {
      first = entry->firstTimestamp;
      last = entry->lastTimestamp;
      found = true;
    }
    else
    {
      first = std::min(first, entry->firstTimestamp);
      last = std::max(last, entry->lastTimestamp);
    }
  }

  if (!found || last <= first)
    return;

  if (!m_impl->initialized)
  {
    // Первоначальная инициализация.
    for (const auto& entry : m_impl->entries)
    {
      if (!entry->ready)
        return;
    }

    initializeTimeRange();
    return;
  }

  // Начало координат сохраняется.
  const double minimum =
    m_impl->timeAxis->chartSeconds(first);

  const double maximum =
    m_impl->timeAxis->chartSeconds(last);

  setAvailableTimeRange(minimum, maximum);
}

std::size_t
ArchiveChartView::signalCount() const noexcept
{
  return m_impl->entries.size();
}

bool ArchiveChartView::removeSignal(SignalId id)
{
  auto& entries = m_impl->entries;

  const auto it = std::find_if(
    entries.begin(),
    entries.end(),
    [id](const auto& entry)
    {
      return entry->id == id;
    });

  if (it == entries.end())
    return false;

  auto& entry = *it;

  // Завершаем работу контроллера.
  delete entry->controller;
  entry->controller = nullptr;

  // Если удаляем сигнал с основной осью,
  // передаём её другому сигналу.
  if (entry->axisY == valueChartAxis())
  {
    const auto next = std::find_if(
      entries.begin(),
      entries.end(),
      [&entry](const auto& candidate)
      {
        return candidate.get() != entry.get();
      });

    if (next != entries.end())
    {
      auto* oldAxis = (*next)->axisY;

      (*next)->series->setValueAxis(valueChartAxis());
      (*next)->axisY = valueChartAxis();

      valueChartAxis()->setTitleText((*next)->name);

      if (oldAxis != valueChartAxis())
        removeValueAxis(oldAxis);
    }
  }

  // Удаляем графические серии.
  delete entry->series;
  entry->series = nullptr;

  // Удаляем дополнительную ось удаляемого сигнала.
  if (entry->axisY &&
      entry->axisY != valueChartAxis())
  {
    removeValueAxis(entry->axisY);
  }

  entries.erase(it);

  updateAxisLayout();

  if (entries.empty())
  {
    m_impl->initialized = false;
    valueChartAxis()->setTitleText(tr("Значение"));
  }
  else
  {
    updateTimeRange();
    autoScaleY();
  }

  return true;
}

}
