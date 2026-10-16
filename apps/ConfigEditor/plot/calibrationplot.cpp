#include "calibrationplot.h"

#include "qcustomplot.h"

#include <QVBoxLayout>
#include <QVector>
#include <QPen>
#include <QColor>

#include <algorithm>
#include <cmath>

namespace qds
{

CalibrationPlot::CalibrationPlot(
  QWidget* parent)
  : QWidget(parent)
{
  m_plot = new QCustomPlot(this);

  auto* layout = new QVBoxLayout(this);

  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_plot);

  m_plot->xAxis->setLabel("X");
  m_plot->yAxis->setLabel("Y");

  m_plot->setInteractions(
    QCP::iRangeDrag |
    QCP::iRangeZoom);

  m_plot->axisRect()->setupFullAxesBox(true);

  m_plot->xAxis->setRange(0, 1);
  m_plot->yAxis->setRange(0, 1);
}

void CalibrationPlot::setCalibration(
  const Calibration& calibration)
{
  clear();

  switch (calibration.kind)
  {
  case CalibrationKind::Table:
    updateTable(calibration);
    break;

  case CalibrationKind::Polynomial:
    updatePolynomial(calibration);
    break;
  }

  if (m_plot->graphCount() > 0)
    m_plot->rescaleAxes();

  m_plot->replot();
}

void CalibrationPlot::updateTable(
  const Calibration& calibration)
{
  QVector<double> x;
  QVector<double> y;

  x.reserve(calibration.points.size());
  y.reserve(calibration.points.size());

  for (const auto& point : calibration.points)
  {
    if (!std::isfinite(point.x) ||
        !std::isfinite(point.y))
    {
      return;
    }

    x.append(point.x);
    y.append(point.y);
  }

  if (x.isEmpty())
    return;

  // QCustomPlot ожидает данные, упорядоченные по X.
  // При редактировании таблицы порядок может нарушаться.
  QVector<QPair<double, double>> points;

  points.reserve(x.size());

  for (qsizetype i = 0; i < x.size(); ++i)
    points.append({x[i], y[i]});

  std::sort(
    points.begin(),
    points.end(),
    [](const auto& a, const auto& b)
    {
      return a.first < b.first;
    });

  for (qsizetype i = 0; i < points.size(); ++i)
  {
    x[i] = points[i].first;
    y[i] = points[i].second;
  }

  auto* graph = m_plot->addGraph();

  graph->setData(x, y);

  graph->setLineStyle(
    QCPGraph::lsLine);

  graph->setScatterStyle(
    QCPScatterStyle(
      QCPScatterStyle::ssCircle,
      6));

  graph->setPen(
    QPen(QColor(30, 110, 200), 2));
}

void CalibrationPlot::updatePolynomial(
  const Calibration& calibration)
{
  if (calibration.coefficients.empty())
    return;

  // Пока используем фиксированный диапазон X.
  // Позже добавим настройку диапазона.
  constexpr double xMin = -10.0;
  constexpr double xMax = 10.0;
  constexpr int sampleCount = 500;

  QVector<double> x;
  QVector<double> y;

  x.reserve(sampleCount);
  y.reserve(sampleCount);

  for (int i = 0; i < sampleCount; ++i)
  {
    const double valueX =
      xMin +
      (xMax - xMin) * i /
        (sampleCount - 1);

    double valueY = 0.0;

    if (!calibration.apply(valueX, valueY))
      continue;

    if (!std::isfinite(valueY))
      continue;

    x.append(valueX);
    y.append(valueY);
  }

  if (x.isEmpty())
    return;

  auto* graph = m_plot->addGraph();

  graph->setData(x, y);

  graph->setLineStyle(
    QCPGraph::lsLine);

  graph->setScatterStyle(
    QCPScatterStyle::ssNone);

  graph->setPen(
    QPen(QColor(200, 70, 60), 2));
}

void CalibrationPlot::clear()
{
  m_plot->clearGraphs();
}

}