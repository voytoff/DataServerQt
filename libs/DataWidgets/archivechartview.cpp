
#include "archivechartview.h"

#include "archivechartcontroller.h"
#include "archivechartseries.h"
#include "timeaxis.h"

#include <QChart>
#include <QDebug>
#include <QPainter>
#include <QFontMetrics>
#include <QPainterPath>
#include <QStyleHints>
#include <QMouseEvent>

#include <algorithm>
#include <cmath>
#include <cstdint>
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

void drawLockIcon(
  QPainter* painter,
  const QPointF& position,
  const QColor& color)
{
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  QPen pen(color, 1.4);
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);

  painter->setPen(pen);
  painter->setBrush(Qt::NoBrush);

  // Дужка замка.
  painter->drawArc(
    QRectF(position.x() + 3.0,
           position.y(),
           8.0,
           10.0),
    0,
    180 * 16);

  // Корпус замка.
  painter->setBrush(color);

  painter->drawRoundedRect(
    QRectF(position.x() + 1.5,
           position.y() + 6.0,
           11.0,
           8.0),
    1.5,
    1.5);

  painter->restore();
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

struct MeasurementValue
{
  SignalId id;
  std::optional<double> value;
};

struct ArchiveChartView::Impl
{
  std::filesystem::path archivePath;

  std::vector<std::unique_ptr<SignalContext>> entries;

  TimeAxis* timeAxis = nullptr;

  bool initialized = false;

  double cursorX = 0.0;
  bool cursorVisible = false;

  std::vector<MeasurementValue> lockedValues;
};

ArchiveChartView::ArchiveChartView(
  QWidget* parent)
  : TimeChartView(parent),
  m_impl(std::make_unique<Impl>())
{
  m_impl->timeAxis = new TimeAxis(this);
  setTimeAxis(m_impl->timeAxis);

  setYAxisMode(YAxisMode::Multiple);
  setMeasurementCursorEnabled(false);

  connect(
    this,
    &TimeChartView::visibleTimeRangeChanged,
    this,
    [this](double, double)
    {
      requestVisibleData();
    });

  connect(
    this,
    &TimeChartView::measurementCursorMoved,
    this,
    [this](double x)
    {
      m_impl->cursorX = x;
      m_impl->cursorVisible = true;

      viewport()->update();
    });

  connect(
    this,
    &TimeChartView::measurementCursorHidden,
    this,
    [this]()
    {
      qDebug()
      << "measurementCursorHidden"
      << "this =" << this
      << "impl =" << m_impl.get();

      m_impl->cursorVisible = false;

      viewport()->update();
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
  setMeasurementCursorEnabled(true);

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
  // Завершаем работу асинхронных контроллеров.
  for (auto& entry : m_impl->entries)
  {
    delete entry->controller;
    entry->controller = nullptr;
  }

  // Удаляем графические серии.
  for (auto& entry : m_impl->entries)
  {
    delete entry->series;
    entry->series = nullptr;
  }

  // Удаляем только дополнительные оси Y.
  for (auto& entry : m_impl->entries)
  {
    if (entry->axisY &&
        entry->axisY != valueChartAxis())
    {
      removeValueAxis(entry->axisY);
    }

    entry->axisY = nullptr;
  }

  m_impl->entries.clear();
  m_impl->lockedValues.clear();

  m_impl->initialized = false;
  m_impl->cursorVisible = false;

  // Сбрасываем измерительный курсор.
  setMeasurementCursorEnabled(false);
  setMeasurementCursorLocked(false);
  hideMeasurementCursor();

  // Восстанавливаем основную ось Y.
  valueChartAxis()->setTitleText(tr("Значение"));
  valueChartAxis()->setRange(-1.0, 1.0);

  // Восстанавливаем временную ось.
  clearTimeRange();

  viewport()->update();
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

void ArchiveChartView::drawMeasurementPanel(
  QPainter* painter)
{
  if (!m_impl->cursorVisible)
    return;

  struct PanelRow
  {
    QString name;
    QString value;
    QColor color;
  };

  std::vector<PanelRow> rows;
  rows.reserve(m_impl->entries.size());

  for (const auto& entry : m_impl->entries)
  {
    if (!entry->series ||
        !entry->series->isVisible())
    {
      continue;
    }

    std::optional<double> value;

    if (isMeasurementCursorLocked())
    {
      const auto it = std::find_if(
        m_impl->lockedValues.begin(),
        m_impl->lockedValues.end(),
        [id = entry->id](const auto& item)
        {
          return item.id == id;
        });

      if (it != m_impl->lockedValues.end())
        value = it->value;
    }
    else
    {
      value = entry->series->valueAt(
        m_impl->cursorX);
    }

    rows.push_back({
      entry->name,
      value
        ? QString::number(*value, 'f', 3)
        : QStringLiteral("—"),
      entry->series->color()
    });
  }

  if (rows.empty())
    return;

  const bool dark =
    QGuiApplication::styleHints()->colorScheme() ==
    Qt::ColorScheme::Dark;

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  const QFontMetrics metrics(painter->font());

  constexpr qreal padding = 10.0;
  constexpr qreal markerSize = 8.0;
  constexpr qreal spacing = 8.0;
  constexpr qreal rowSpacing = 5.0;

  const qreal lineHeight = metrics.height();
  const qreal rowHeight = lineHeight + rowSpacing;

  const QString timeText =
    QStringLiteral("Время: %1 с")
      .arg(m_impl->cursorX, 0, 'f', 3);

  qreal nameWidth =
    metrics.horizontalAdvance(timeText) +
    (isMeasurementCursorLocked() ? 20.0 : 0.0);

  qreal valueWidth = 0.0;

  for (const auto& row : rows)
  {
    nameWidth = std::max(
      nameWidth,
      qreal(metrics.horizontalAdvance(row.name)));

    valueWidth = std::max(
      valueWidth,
      qreal(metrics.horizontalAdvance(row.value)));
  }

  const qreal panelWidth =
    padding * 2.0 +
    markerSize +
    spacing +
    nameWidth +
    spacing * 2.0 +
    valueWidth;

  const qreal panelHeight =
    padding * 2.0 +
    lineHeight +
    8.0 +
    rows.size() * rowHeight;

  const QRectF plot = chart()->plotArea();

  const QRectF panelRect(
    plot.right() - panelWidth - 10.0,
    plot.top() + 10.0,
    panelWidth,
    panelHeight);

  const QColor background =
    dark
      ? QColor(38, 40, 48, 190)
      : QColor(255, 255, 255, 205);

  const QColor border =
    dark
      ? QColor(150, 150, 160, 100)
      : QColor(160, 160, 160, 120);

  const QColor textColor =
    dark
      ? QColor(235, 235, 240)
      : QColor(45, 45, 45);

  // Полупрозрачный фон.
  painter->setPen(QPen(border, 1.0));
  painter->setBrush(background);

  painter->drawRoundedRect(
    panelRect,
    6.0,
    6.0);

  const qreal left = panelRect.left() + padding;
  const qreal right = panelRect.right() - padding;

  qreal baseline =
    panelRect.top() + padding + metrics.ascent();

  // Время.
  painter->setPen(textColor);

  if (isMeasurementCursorLocked())
  {
    drawLockIcon(
      painter,
      QPointF(left, baseline - metrics.ascent()),
      textColor);
  }

  painter->setPen(textColor);

  painter->drawText(
    QPointF(
      left + (isMeasurementCursorLocked() ? 20.0 : 0.0),
      baseline),
    timeText);

  baseline += lineHeight + 8.0;

  // Значения сигналов.
  for (const auto& row : rows)
  {
    const qreal markerY =
      baseline -
      metrics.ascent() / 2.0 -
      markerSize / 2.0;

    painter->setPen(Qt::NoPen);
    painter->setBrush(row.color);

    painter->drawEllipse(
      QRectF(
        left,
        markerY,
        markerSize,
        markerSize));

    painter->setPen(textColor);

    painter->drawText(
      QPointF(left + markerSize + spacing, baseline),
      row.name);

    painter->drawText(
      QPointF(
        right - metrics.horizontalAdvance(row.value),
        baseline),
      row.value);

    baseline += rowHeight;
  }

  painter->restore();
}

void ArchiveChartView::captureMeasurement()
{
  m_impl->lockedValues.clear();

  for (const auto& entry : m_impl->entries)
  {
    if (!entry->series ||
        !entry->series->isVisible())
    {
      continue;
    }

    m_impl->lockedValues.push_back({
      entry->id,
      entry->series->valueAt(m_impl->cursorX)
    });
  }
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

  // удаляем фиксированные измерения
  std::erase_if(
    m_impl->lockedValues,
    [id](const auto& item)
    {
      return item.id == id;
    });

  entries.erase(it);

  updateAxisLayout();
  setMeasurementCursorEnabled(!m_impl->entries.empty());

  if (entries.empty())
  {
    m_impl->initialized = false;
    valueChartAxis()->setTitleText(tr("Значение"));

    setMeasurementCursorLocked(false);
    m_impl->lockedValues.clear();

    m_impl->cursorVisible = false;
  }
  else
  {
    updateTimeRange();
    autoScaleY();
  }

  return true;
}

std::optional<double> ArchiveChartView::valueAt(
  SignalId id,
  double x) const
{
  const auto& entries = m_impl->entries;

  const auto it = std::find_if(
    entries.begin(),
    entries.end(),
    [id](const auto& entry)
    {
      return entry->id == id;
    });

  if (it == entries.end() ||
      !(*it)->series)
  {
    return std::nullopt;
  }

  return (*it)->series->valueAt(x);
}

void ArchiveChartView::updateChartTheme()
{
  TimeChartView::updateChartTheme();

  for (const auto& entry : m_impl->entries)
  {
    if (!entry->series)
      continue;

    entry->series->refreshAppearance();
  }

  viewport()->update();
}

void ArchiveChartView::drawForeground(
  QPainter* painter,
  const QRectF& rect)
{
  TimeChartView::drawForeground(painter, rect);

  if (!m_impl->cursorVisible)
    return;

  const QRectF plot = chart()->plotArea();

  painter->save();

  painter->setRenderHint(
    QPainter::Antialiasing,
    true);

  for (const auto& entry : m_impl->entries)
  {
    if (!entry->series ||
        !entry->series->isVisible())
    {
      continue;
    }

    const auto position =
      entry->series->positionAt(
        m_impl->cursorX);

    if (!position)
      continue;

    if (!plot.contains(*position))
      continue;

    const QColor color =
      entry->series->color();

    painter->setPen(
      QPen(Qt::white, 1.5));

    painter->setBrush(color);

    painter->drawEllipse(
      *position,
      4.5,
      4.5);
  }

  painter->restore();

  drawMeasurementPanel(painter);
}

void ArchiveChartView::mousePressEvent(
  QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton &&
      event->modifiers().testFlag(Qt::AltModifier))
  {
    // в самое начало обработчика Alt + ЛКМ добавим проверку наличия сигналов
    // так на пустом графике Alt + ЛКМ вообще не будет менять состояние измерений.
    if (m_impl->entries.empty())
    {
      event->accept();
      return;
    }

    if (isMeasurementCursorLocked())
    {
      setMeasurementCursorLocked(false);

      m_impl->lockedValues.clear();
    }
    else
    {
      setMeasurementCursorPosition(event->position());

      setMeasurementCursorLocked(true);

      captureMeasurement();
    }

    viewport()->update();

    event->accept();
    return;
  }

  TimeChartView::mousePressEvent(event);
}

}
