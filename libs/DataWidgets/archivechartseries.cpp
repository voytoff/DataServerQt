
#include "archivechartseries.h"
#include "timechartview.h"

#include <QColor>
#include <QLegendMarker>
#include <QLegend>

namespace qds
{

ArchiveChartSeries::ArchiveChartSeries(
  TimeChartView* view)
  : QObject(view),
  m_view(view)
{
  m_legendSeries = new QLineSeries();

  m_legendSeries->setColor(m_color);

  m_view->addSeries(m_legendSeries);

  auto* marker =
    m_view->chart()->legend()->markers(m_legendSeries).value(0);

  if (marker)
  {
    connect(
      marker,
      &QLegendMarker::clicked,
      this,
      [this]()
      {
        setVisible(!isVisible());
      });
  }
}

void ArchiveChartSeries::setName(
  const QString& name)
{
  m_legendSeries->setName(name);
}

void ArchiveChartSeries::setSegments(
  const Segments& segments)
{
  std::size_t lineIndex = 0;
  std::size_t pointIndex = 0;

  for (const auto& points : segments)
  {
    if (points.empty())
      continue;

    if (points.size() > 1)
    {
      if (lineIndex == m_lines.size())
      {
        auto* series = new QLineSeries();

        series->setColor(m_color);

        m_view->addSeries(series);

        for (auto* marker : m_view->chart()->legend()->markers(series))
          marker->setVisible(false);

        m_lines.push_back(series);
      }

      auto* series = m_lines[lineIndex++];

      series->replace(points);
      series->setVisible(m_visible);
    }
    else
    {
      if (pointIndex == m_points.size())
      {
        auto* series = new QScatterSeries();

        series->setColor(m_color);
        series->setMarkerSize(6.0);

        m_view->addSeries(series);

        for (auto* marker : m_view->chart()->legend()->markers(series))
          marker->setVisible(false);

        m_points.push_back(series);
      }

      auto* series = m_points[pointIndex++];

      series->replace(points);
      series->setVisible(m_visible);
    }
  }

  for (std::size_t i = lineIndex;
       i < m_lines.size();
       ++i)
  {
    m_lines[i]->clear();
    m_lines[i]->setVisible(false);
  }

  for (std::size_t i = pointIndex;
       i < m_points.size();
       ++i)
  {
    m_points[i]->clear();
    m_points[i]->setVisible(false);
  }

  updateLegend();

  m_view->autoScaleY();
}

void ArchiveChartSeries::setColor(
  const QColor& color)
{
  if (m_color == color)
    return;

  m_color = color;

  for (auto* series : m_lines)
    series->setColor(color);

  for (auto* series : m_points)
    series->setColor(color);

  m_legendSeries->setColor(color);
}

void ArchiveChartSeries::setVisible(
  bool visible)
{
  if (m_visible == visible)
    return;

  m_visible = visible;

  for (auto* series : m_lines)
  {
    series->setVisible(
      visible && !series->points().empty());
  }

  for (auto* series : m_points)
  {
    series->setVisible(
      visible && !series->points().empty());
  }

  for (auto* marker :
       m_view->chart()->legend()->markers(m_legendSeries))
  {
    QColor color = marker->labelBrush().color();
    color.setAlpha(visible ? 255 : 90);
    marker->setLabelBrush(QBrush(color));
  }

  updateLegend();

  m_view->autoScaleY();
}

bool ArchiveChartSeries::isVisible() const noexcept
{
  return m_visible;
}

void ArchiveChartSeries::updateLegend()
{
  auto* legend = m_view->chart()->legend();

  for (auto* marker : legend->markers(m_legendSeries))
    marker->setVisible(true);

  for (auto* series : m_lines)
  {
    for (auto* marker : legend->markers(series))
      marker->setVisible(false);
  }

  for (auto* series : m_points)
  {
    for (auto* marker : legend->markers(series))
      marker->setVisible(false);
  }
}

}
