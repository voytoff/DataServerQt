#include "archivechartseries.h"

namespace qds
{

namespace
{

QVector<QPointF> toChartPoints(
  const ArchiveSignalPoints& samples,
  const TimeAxis& timeAxis)
{
  QVector<QPointF> points;
  points.reserve(samples.size());

  for (const auto& sample : samples)
  {
    if (!std::isfinite(sample.value))
      continue;

    points.append(QPointF(
      timeAxis.chartSeconds(sample.timestamp),
      sample.value));
  }

  return points;
}

}

ArchiveChartSeries::ArchiveChartSeries(
  QObject *parent)
: QObject(parent)
{
}

void ArchiveChartSeries::setChartSegments(
  TimeChartView *view,
  const ArchiveSignalSegments &segments,
  const TimeAxis &timeAxis)
{
  std::size_t lineIndex = 0;
  std::size_t pointIndex = 0;

  for (const auto& segment : segments)
  {
    if (segment.points.empty())
      continue;

    const auto points =
      toChartPoints(segment.points, timeAxis);

    if (points.size() > 1)
    {
      if (lineIndex == m_lines.size())
      {
        auto* series = new QLineSeries();
        series->setColor(Qt::blue);

        view->addSeries(series);
        m_lines.push_back(series);
      }

      auto* series = m_lines[lineIndex++];
      series->replace(points);
      series->setVisible(true);
    }
    else if (points.size() == 1)
    {
      if (pointIndex == m_points.size())
      {
        auto* series = new QScatterSeries();
        series->setColor(Qt::blue);
        series->setMarkerSize(6.0);

        view->addSeries(series);
        m_points.push_back(series);
      }

      auto* series = m_points[pointIndex++];
      series->replace(points);
      series->setVisible(true);
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

  view->autoScaleY();
}

}
