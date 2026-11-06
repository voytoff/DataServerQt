#pragma once

#include "datawidgets_export.h"

#include <QObject>
#include <QLineSeries>
#include <QScatterSeries>
#include <QPointF>
#include <QVector>
#include <QColor>

#include <vector>

namespace qds
{

class TimeChartView;

class DATAWIDGETS_EXPORT ArchiveChartSeries : public QObject
{
  Q_OBJECT

public:
  using Segment = QVector<QPointF>;
  using Segments = std::vector<Segment>;

  explicit ArchiveChartSeries(
    TimeChartView* view);

  void setName(
    const QString& name);

  void setSegments(
    const Segments& segments);

  void setColor(
    const QColor& color);

  void setVisible(
    bool visible);

  bool isVisible() const noexcept;

private:
  void updateLegend();

  TimeChartView* m_view = nullptr;

  QLineSeries* m_legendSeries = nullptr;

  std::vector<QLineSeries*> m_lines;
  std::vector<QScatterSeries*> m_points;

  QColor m_color = Qt::blue;
  bool m_visible = true;
};

}