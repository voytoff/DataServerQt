#include "archivechartseries.h"
#include "timechartview.h"

#include <QColor>
#include <QLegendMarker>
#include <QLegend>
#include <QChart>

#include <algorithm>
#include <cmath>
#include <limits>

namespace qds
{

ArchiveChartSeries::ArchiveChartSeries(
  TimeChartView* view)
  : QObject(view),
  m_view(view),
  m_valueAxis(view->valueChartAxis())
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

ArchiveChartSeries::~ArchiveChartSeries()
{
  for (auto* series : m_lines)
  {
    m_view->chart()->removeSeries(series);
    delete series;
  }

  for (auto* series : m_points)
  {
    m_view->chart()->removeSeries(series);
    delete series;
  }

  if (m_legendSeries)
  {
    m_view->chart()->removeSeries(m_legendSeries);
    delete m_legendSeries;
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

        QPen pen = series->pen();
        pen.setColor(m_color);
        pen.setWidth(m_lineWidth);
        series->setPen(pen);

        m_view->addSeries(series);

        if (m_valueAxis &&
            m_valueAxis != m_view->valueChartAxis())
        {
          series->detachAxis(m_view->valueChartAxis());
          series->attachAxis(m_valueAxis);
        }

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

        if (m_valueAxis &&
            m_valueAxis != m_view->valueChartAxis())
        {
          series->detachAxis(m_view->valueChartAxis());
          series->attachAxis(m_valueAxis);
        }

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

void ArchiveChartSeries::setValueAxis(
  QValueAxis* axis)
{
  if (!axis || m_valueAxis == axis)
    return;

  if (!m_view->chart()->axes(Qt::Vertical).contains(axis))
    return;

  m_valueAxis = axis;

  for (auto* series : m_lines)
  {
    for (auto* attached : series->attachedAxes())
    {
      if (attached->orientation() == Qt::Vertical)
        series->detachAxis(attached);
    }

    series->attachAxis(axis);
  }

  for (auto* series : m_points)
  {
    for (auto* attached : series->attachedAxes())
    {
      if (attached->orientation() == Qt::Vertical)
        series->detachAxis(attached);
    }

    series->attachAxis(axis);
  }

  m_view->autoScaleY();
}

QValueAxis* ArchiveChartSeries::valueAxis() const noexcept
{
  return m_valueAxis;
}

std::optional<double> ArchiveChartSeries::valueAt(
  double x) const
{
  if (!m_visible || !std::isfinite(x))
    return std::nullopt;

  // Сначала проверяем непрерывные участки.
  for (const auto* series : m_lines)
  {
    if (!series->isVisible())
      continue;

    const auto points = series->points();

    if (points.size() < 2)
      continue;

    if (x < points.first().x() ||
        x > points.last().x())
      continue;

    const auto it = std::lower_bound(
      points.cbegin(),
      points.cend(),
      x,
      [](const QPointF& point, double value)
      {
        return point.x() < value;
      });

    if (it == points.cend())
      continue;

    if (it->x() == x)
    {
      if (std::isfinite(it->y()))
        return it->y();

      continue;
    }

    if (it == points.cbegin())
      continue;

    const auto& right = *it;
    const auto& left = *(it - 1);

    const double dx = right.x() - left.x();

    if (dx <= 0.0 ||
        !std::isfinite(left.y()) ||
        !std::isfinite(right.y()))
    {
      continue;
    }

    const double ratio =
      (x - left.x()) / dx;

    const double value =
      left.y() +
      ratio * (right.y() - left.y());

    if (std::isfinite(value))
      return value;
  }

  // Отдельные одиночные точки.
  for (const auto* series : m_points)
  {
    if (!series->isVisible())
      continue;

    const auto points = series->points();

    for (const auto& point : points)
    {
      if (point.x() == x &&
          std::isfinite(point.y()))
      {
        return point.y();
      }
    }
  }

  return std::nullopt;
}

std::optional<QPointF> ArchiveChartSeries::positionAt(
  double x) const
{
  const auto value = valueAt(x);

  if (!value)
    return std::nullopt;

  QXYSeries* reference = nullptr;

  for (auto* series : m_lines)
  {
    if (series->isVisible() &&
        !series->points().empty())
    {
      reference = series;
      break;
    }
  }

  if (!reference)
  {
    for (auto* series : m_points)
    {
      if (series->isVisible() &&
          !series->points().empty())
      {
        reference = series;
        break;
      }
    }
  }

  if (!reference)
    return std::nullopt;

  return m_view->chart()->mapToPosition(
    QPointF(x, *value),
    reference);
}

std::optional<QPointF>
ArchiveChartSeries::positionAt(
  const QPointF& point) const
{
  QXYSeries* reference = nullptr;

  for (auto* series : m_lines)
  {
    if (series->isVisible() &&
        !series->points().empty())
    {
      reference = series;
      break;
    }
  }

  if (!reference)
  {
    for (auto* series : m_points)
    {
      if (series->isVisible() &&
          !series->points().empty())
      {
        reference = series;
        break;
      }
    }
  }

  if (!reference)
    return std::nullopt;

  return m_view->chart()->mapToPosition(
    point,
    reference);
}

QColor ArchiveChartSeries::color() const noexcept
{
  return m_color;
}

void ArchiveChartSeries::refreshAppearance()
{
  QPen legendPen = m_legendSeries->pen();
  legendPen.setColor(m_color);
  legendPen.setWidth(m_lineWidth);
  m_legendSeries->setPen(legendPen);

  for (auto* series : m_lines)
  {
    QPen pen = series->pen();
    pen.setColor(m_color);
    pen.setWidth(m_lineWidth);
    series->setPen(pen);
  }

  for (auto* series : m_points)
    series->setColor(m_color);

  updateLegend();
}

void ArchiveChartSeries::setLineWidth(int width)
{
  width = std::clamp(width, 1, 3);

  if (m_lineWidth == width)
    return;

  m_lineWidth = width;

  for (auto* series : m_lines)
  {
    QPen pen = series->pen();
    pen.setWidth(m_lineWidth);
    series->setPen(pen);
  }

  QPen pen = m_legendSeries->pen();
  pen.setWidth(m_lineWidth);
  m_legendSeries->setPen(pen);
}

int ArchiveChartSeries::lineWidth() const noexcept
{
  return m_lineWidth;
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

std::optional<QPointF>
ArchiveChartSeries::nearestSample(double x) const
{
  if (!m_visible || !std::isfinite(x))
    return std::nullopt;

  std::optional<QPointF> nearest;
  double minDistance =
    std::numeric_limits<double>::infinity();

  const auto consider =
    [&](const QPointF& point)
  {
    if (!std::isfinite(point.x()) ||
        !std::isfinite(point.y()))
    {
      return;
    }

    const double distance =
      std::abs(point.x() - x);

    if (distance < minDistance)
    {
      minDistance = distance;
      nearest = point;
    }
  };

  const auto search =
    [&](const auto& series)
  {
    if (!series->isVisible())
      return;

    const auto points = series->points();

    if (points.empty())
      return;

    const auto it = std::lower_bound(
      points.cbegin(),
      points.cend(),
      x,
      [](const QPointF& point, double value)
      {
        return point.x() < value;
      });

    if (it != points.cend())
      consider(*it);

    if (it != points.cbegin())
      consider(*(it - 1));
  };

  for (const auto* series : m_lines)
    search(series);

  for (const auto* series : m_points)
    search(series);

  return nearest;
}

}
