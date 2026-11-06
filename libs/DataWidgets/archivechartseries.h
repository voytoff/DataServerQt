#pragma once

#include "archivesignalpoint.h"
#include "timechartview.h"

#include <QObject>
#include <QLineSeries>
#include <QScatterSeries>

namespace qds
{

class ArchiveChartSeries : QObject
{
  Q_OBJECT
public:
  explicit ArchiveChartSeries(QObject* parent = nullptr);

void setChartSegments(
  TimeChartView* view,
  const ArchiveSignalSegments& segments,
  const TimeAxis& timeAxis);

private:
  std::vector<QLineSeries*> m_lines;
  std::vector<QScatterSeries*> m_points;

};

}
