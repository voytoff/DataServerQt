#include "signalchartwidget.h"

#include <QGuiApplication>
#include <QStyleHints>
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

namespace
{

void updateChartTheme(QChart *chart) {
  // Получаем текущую схему (Qt::ColorScheme::Dark или Qt::ColorScheme::Light)
  Qt::ColorScheme currentScheme = QGuiApplication::styleHints()->colorScheme();

  if (currentScheme == Qt::ColorScheme::Dark) {
    chart->setTheme(QChart::ChartThemeDark); // Включаем темную тему
  } else {
    chart->setTheme(QChart::ChartThemeLight); // Включаем светлую тему (или ChartThemeQt)
  }
}

}

SignalChartWidget::SignalChartWidget(
  QWidget* parent)
  : QWidget(parent)
{
  m_chart = new QChart();

  updateChartTheme(m_chart);

  m_chart->setTitle(
    tr("Мониторинг сигналов"));

  m_chart->legend()->setVisible(true);

  m_axisX = new QValueAxis(m_chart);

  m_axisX->setTitleText(tr("Время, с"));
  m_axisX->setLabelFormat("%.1f");
  m_axisX->setRange(0.0, m_windowSeconds);

  m_chart->addAxis(
    m_axisX,
    Qt::AlignBottom);

  m_chartView =
    new QChartView(m_chart, this);

  m_chartView->setRenderHint(
    QPainter::Antialiasing);

  auto* layout = new QVBoxLayout(this);

  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  layout->addWidget(m_chartView);

  m_refreshTimer.setInterval(RefreshIntervalMs);

  connect(
    &m_refreshTimer,
    &QTimer::timeout,
    this,
    &SignalChartWidget::refreshChart);

  m_refreshTimer.start();
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

  for (auto* axis : m_axesY)
  {
    m_chart->removeAxis(axis);
    delete axis;
  }

  m_axesY.clear();
  m_signalIds.clear();
  m_pendingPoints.clear();

  m_activeAxisIndex = 0;

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

    auto* axisY = new QValueAxis(m_chart);

    axisY->setTitleText(names[i]);
    axisY->setLabelFormat("%.4g");
    axisY->setRange(-1.0, 1.0);

    m_chart->addAxis(
      axisY,
      Qt::AlignLeft);

    series->attachAxis(m_axisX);
    series->attachAxis(axisY);

    m_series.push_back(series);
    m_axesY.push_back(axisY);
  }

  m_pendingPoints.resize(m_series.size());

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

  for (std::size_t i = 0; i < values.size(); ++i)
  {
    if (!std::isfinite(values[i]))
      continue;

    m_pendingPoints[i].emplace_back(
      currentTime,
      values[i]);
  }
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

  for (std::size_t i = 0; i < m_series.size(); ++i)
  {
    const auto* series = m_series[i];
    auto* axisY = m_axesY[i];

    double minimumValue =
      std::numeric_limits<double>::max();

    double maximumValue =
      std::numeric_limits<double>::lowest();

    bool hasValues = false;

    for (const QPointF& point : series->points())
    {
      minimumValue =
        std::min(minimumValue, point.y());

      maximumValue =
        std::max(maximumValue, point.y());

      hasValues = true;
    }

    if (!hasValues)
    {
      axisY->setRange(-1.0, 1.0);
      continue;
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

    axisY->setRange(
      minimumValue - margin,
      maximumValue + margin);
  }
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

        // Ctrl + щелчок: переключение активной оси Y.
        if (QGuiApplication::keyboardModifiers() &
            Qt::ControlModifier)
        {
          if (m_series[index]->isVisible())
          {
            m_activeAxisIndex = index;

            updateAxisVisibility();
          }

          return;
        }

        // Обычный щелчок: скрытие или отображение линии.
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
  for (auto& points : m_pendingPoints)
    points.clear();

  for (auto* series : m_series)
    series->clear();

  m_firstTimestamp = 0;
  m_lastTimestamp = 0;
  m_hasFirstTimestamp = false;

  m_axisX->setRange(
    0.0,
    m_windowSeconds);

  for (auto* axisY : m_axesY)
    axisY->setRange(-1.0, 1.0);
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

  if (m_hasFirstTimestamp)
  {
    refreshChart();
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

void SignalChartWidget::setYAxisMode(
  YAxisMode mode)
{
  if (m_yAxisMode == mode)
    return;

  m_yAxisMode = mode;

  updateAxisVisibility();
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
      marker->setVisible(true);

      QColor color =
        marker->labelBrush().color();

      color.setAlpha(visible ? 255 : 90);

      marker->setLabelBrush(QBrush(color));
    }
  }

  updateAxisVisibility();

  if (m_hasFirstTimestamp)
  {
    const double currentTime =
      static_cast<double>(
        m_lastTimestamp - m_firstTimestamp) /
      1'000'000.0;

    updateAxes(currentTime);
  }
}

void SignalChartWidget::refreshChart()
{
  if (!m_hasFirstTimestamp)
    return;

  const double currentTime =
    static_cast<double>(
      m_lastTimestamp - m_firstTimestamp) /
    1'000'000.0;

  const double minimumTime =
    std::max(
      0.0,
      currentTime - m_windowSeconds);

  for (std::size_t i = 0; i < m_series.size(); ++i)
  {
    auto* series = m_series[i];
    auto& pending = m_pendingPoints[i];

    // Добавляем все накопленные измерения.
    if (!pending.empty())
    {
      const auto first = std::lower_bound(
        pending.begin(),
        pending.end(),
        minimumTime,
        [](const QPointF& point, double time)
        {
          return point.x() < time;
        });

      if (first != pending.end())
      {
        series->append(
          QList<QPointF>(
            first,
            pending.end()));
      }

      pending.clear();
    }

    // Удаляем точки за пределами окна.
    const auto points = series->points();

    const auto firstValid = std::lower_bound(
      points.begin(),
      points.end(),
      minimumTime,
      [](const QPointF& point, double time)
      {
        return point.x() < time;
      });

    const qsizetype removeCount =
      std::distance(
        points.begin(),
        firstValid);

    if (removeCount > 0)
    {
      series->removePoints(
        0,
        removeCount);
    }
  }

  updateAxes(currentTime);
}

void SignalChartWidget::updateAxisVisibility()
{
  if (m_axesY.empty())
    return;

  if (m_activeAxisIndex >= m_series.size() ||
      !m_series[m_activeAxisIndex]->isVisible())
  {
    m_activeAxisIndex = m_series.size();

    for (std::size_t i = 0; i < m_series.size(); ++i)
    {
      if (m_series[i]->isVisible())
      {
        m_activeAxisIndex = i;
        break;
      }
    }
  }

  for (std::size_t i = 0; i < m_axesY.size(); ++i)
  {
    const bool visible =
      m_series[i]->isVisible();

    const bool active =
      visible && i == m_activeAxisIndex;

    const bool showAxis =
      m_yAxisMode == YAxisMode::Multiple
        ? visible
        : active;

    m_axesY[i]->setVisible(showAxis);

    for (QLegendMarker* marker :
         m_chart->legend()->markers(m_series[i]))
    {
      QFont font = marker->font();

      if (font.bold() != active)
      {
        font.setBold(active);
        marker->setFont(font);
      }
    }
  }

  m_chart->legend()->setVisible(true);
}

}