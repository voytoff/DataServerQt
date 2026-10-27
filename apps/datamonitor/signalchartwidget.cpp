#include "signalchartwidget.h"

#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QVBoxLayout>
#include <QPainter>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QLegend>
#include <QLegendMarker>
#include <QColor>

#include <algorithm>
#include <cmath>
#include <limits>

namespace qds
{

SignalChartWidget::SignalChartWidget(
  QWidget* parent)
  : QWidget(parent)
{
  m_chart = new QChart();

  m_chart->setTitle(
    tr("Мониторинг сигналов"));

  m_chart->legend()->setVisible(true);

  m_axisX = new QValueAxis(m_chart);
  m_axisY = new QValueAxis(m_chart);

  m_axisX->setTitleText(tr("Время, с"));
  m_axisX->setLabelFormat("%.1f");
  m_axisX->setRange(0.0, m_windowSeconds);

  m_axisY->setTitleText(tr("Значение"));
  m_axisY->setLabelFormat("%.4g");
  m_axisY->setRange(-1.0, 1.0);

  m_chart->addAxis(
    m_axisX,
    Qt::AlignBottom);

  m_chart->addAxis(
    m_axisY,
    Qt::AlignLeft);

  m_chartView =
    new QChartView(m_chart, this);

  m_chartView->setRenderHint(
    QPainter::Antialiasing);



  auto* layout = new QVBoxLayout(this);

  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  // Панель управления графиком.
  auto* controls = new QHBoxLayout();

  controls->setContentsMargins(8, 4, 8, 0);

  controls->addStretch();

  controls->addWidget(
    new QLabel(tr("Интервал:"), this));

  m_intervalCombo = new QComboBox(this);

  m_intervalCombo->addItem(
    tr("5 с"), 5);

  m_intervalCombo->addItem(
    tr("10 с"), 10);

  m_intervalCombo->addItem(
    tr("30 с"), 30);

  m_intervalCombo->addItem(
    tr("60 с"), 60);

  m_intervalCombo->setCurrentIndex(
    m_intervalCombo->findData(m_windowSeconds));

  controls->addWidget(m_intervalCombo);

  layout->addLayout(controls);
  layout->addWidget(m_chartView);

  connect(
    m_intervalCombo,
    &QComboBox::currentIndexChanged,
    this,
    [this](int index)
    {
      if (index < 0)
        return;

      setWindowSeconds(
        m_intervalCombo->itemData(index).toInt());
    });
}

void SignalChartWidget::setSignals(
  std::span<const SignalId> ids,
  const std::vector<QString>& names)
{
  clear();

  for (auto* series : m_series)
  {
    m_chart->removeSeries(series);
    delete series;
  }

  m_series.clear();

  if (ids.size() != names.size())
    return;

  m_series.reserve(ids.size());

  m_signalIds.assign(
    ids.begin(),
    ids.end());

  for (std::size_t i = 0; i < ids.size(); ++i)
  {
    auto* series = new QLineSeries();

    series->setName(names[i]);

    m_chart->addSeries(series);

    series->attachAxis(m_axisX);
    series->attachAxis(m_axisY);

    m_series.push_back(series);
  }

  setupLegend();
  updateSeriesVisibility();
}

void SignalChartWidget::appendData(
  uint64_t timestamp,
  std::span<const double> values)
{
  if (values.size() != m_series.size() ||
      values.empty())
  {
    return;
  }

  if (!m_hasFirstTimestamp)
  {
    m_firstTimestamp = timestamp;
    m_lastTimestamp = timestamp;
    m_hasFirstTimestamp = true;
  }
  else
  {
    if (timestamp < m_lastTimestamp)
      return;

    m_lastTimestamp = timestamp;
  }

  const double currentTime =
    static_cast<double>(
      timestamp - m_firstTimestamp) /
    1'000'000.0;

  const double minimumTime =
    std::max(
      0.0,
      currentTime - m_windowSeconds);

  for (std::size_t i = 0; i < values.size(); ++i)
  {
    auto* series = m_series[i];

    if (std::isfinite(values[i]))
      series->append(currentTime, values[i]);

    const auto& points = series->points();

    qsizetype removeCount = 0;

    while (removeCount < points.size() &&
           points[removeCount].x() < minimumTime)
    {
      ++removeCount;
    }

    if (removeCount > 0)
    {
      series->removePoints(
        0,
        removeCount);
    }
  }

  updateAxes(currentTime);
}

void SignalChartWidget::updateAxes(
  double currentTime)
{
  const double minimumTime =
    std::max(
      0.0,
      currentTime - m_windowSeconds);

  const double maximumTime =
    minimumTime + m_windowSeconds;

  m_axisX->setRange(
    minimumTime,
    maximumTime);

  double minimumValue =
    std::numeric_limits<double>::max();

  double maximumValue =
    std::numeric_limits<double>::lowest();

  bool hasValues = false;

  for (const auto* series : m_series)
  {
    if (!series->isVisible())
      continue;

    for (const QPointF& point : series->points())
    {
      minimumValue =
        std::min(minimumValue, point.y());

      maximumValue =
        std::max(maximumValue, point.y());

      hasValues = true;
    }
  }

  if (!hasValues)
  {
    m_axisY->setRange(-1.0, 1.0);
    return;
  }

  double range =
    maximumValue - minimumValue;

  if (range <= 0.0)
  {
    range = std::max(
      std::abs(maximumValue) * 0.1,
      1.0);
  }

  const double margin = range * 0.1;

  m_axisY->setRange(
    minimumValue - margin,
    maximumValue + margin);
}

void SignalChartWidget::setupLegend()
{
  for (QLegendMarker* marker :
       m_chart->legend()->markers())
  {
    marker->setVisible(true);

    connect(
      marker,
      &QLegendMarker::clicked,
      this,
      [this, marker]()
      {
        const auto it = std::find(
          m_series.begin(),
          m_series.end(),
          marker->series());

        if (it == m_series.end())
          return;

        const std::size_t index =
          static_cast<std::size_t>(
            std::distance(m_series.begin(), it));

        const SignalId id = m_signalIds[index];

        if (m_hiddenSignals.contains(id))
          m_hiddenSignals.erase(id);
        else
          m_hiddenSignals.insert(id);

        updateSeriesVisibility();

        emit hiddenSignalsChanged();
      });
  }
}

void SignalChartWidget::clear()
{
  for (auto* series : m_series)
    series->clear();

  m_firstTimestamp = 0;
  m_lastTimestamp = 0;
  m_hasFirstTimestamp = false;

  m_axisX->setRange(
    0.0,
    m_windowSeconds);

  m_axisY->setRange(
    -1.0,
    1.0);
}

void SignalChartWidget::setWindowSeconds(
  int seconds)
{
  if (seconds != 5 &&
      seconds != 10 &&
      seconds != 30 &&
      seconds != 60)
  {
    return;
  }

  if (m_windowSeconds == seconds)
    return;

  m_windowSeconds = seconds;

  const int index =
    m_intervalCombo->findData(seconds);

  if (index >= 0)
  {
    QSignalBlocker blocker(m_intervalCombo);

    m_intervalCombo->setCurrentIndex(index);
  }

  if (m_hasFirstTimestamp)
  {
    const double currentTime =
      static_cast<double>(
        m_lastTimestamp - m_firstTimestamp) /
      1'000'000.0;

    const double minimumTime =
      std::max(
        0.0,
        currentTime - m_windowSeconds);

    for (auto* series : m_series)
    {
      const auto points = series->points();

      qsizetype removeCount = 0;

      while (removeCount < points.size() &&
             points[removeCount].x() < minimumTime)
      {
        ++removeCount;
      }

      if (removeCount > 0)
      {
        series->removePoints(
          0,
          removeCount);
      }
    }

    updateAxes(currentTime);
  }
  else
  {
    m_axisX->setRange(
      0.0,
      m_windowSeconds);
  }

  emit windowSecondsChanged(seconds);
}

int SignalChartWidget::windowSeconds() const noexcept
{
  return m_windowSeconds;
}

void SignalChartWidget::setHiddenSignals(
  std::span<const SignalId> ids)
{
  m_hiddenSignals.clear();

  for (SignalId id : ids)
    m_hiddenSignals.insert(id);

  updateSeriesVisibility();
}

std::vector<SignalId>
SignalChartWidget::hiddenSignals() const
{
  std::vector<SignalId> result;

  result.reserve(m_hiddenSignals.size());

  for (SignalId id : m_hiddenSignals)
    result.push_back(id);

  return result;
}

void SignalChartWidget::updateSeriesVisibility()
{
  for (std::size_t i = 0; i < m_series.size(); ++i)
  {
    auto* series = m_series[i];

    const bool visible =
      !m_hiddenSignals.contains(m_signalIds[i]);

    series->setVisible(visible);

    for (QLegendMarker* marker :
         m_chart->legend()->markers(series))
    {
      // Маркер должен оставаться доступным,
      // даже когда серия скрыта.
      marker->setVisible(true);

      // Полупрозрачная легенда для скрытой линии.
      QColor color =
        marker->labelBrush().color();

      color.setAlpha(visible ? 255 : 90);

      marker->setLabelBrush(QBrush(color));
    }
  }

  if (m_hasFirstTimestamp)
  {
    const double currentTime =
      static_cast<double>(
        m_lastTimestamp - m_firstTimestamp) /
      1'000'000.0;

    updateAxes(currentTime);
  }
}

}