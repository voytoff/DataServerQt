#pragma once

#include "datatypes.h"
#include "datawidgets_export.h"

#include <QObject>
#include <QLineSeries>
#include <QScatterSeries>
#include <QPointF>
#include <QVector>
#include <QColor>

#include <qvalueaxis.h>
#include <vector>
#include <optional>

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

  ~ArchiveChartSeries() override;

  void setName(
    const QString& name);

  void setSegments(
    const Segments& segments);

  void setColor(
    const QColor& color);

  void setVisible(
    bool visible);

  bool isVisible() const noexcept;

  void setValueAxis(QValueAxis* axis);

  QValueAxis* valueAxis() const noexcept;

  [[nodiscard]]
  std::optional<double> valueAt(
    double x) const;

  [[nodiscard]]
  std::optional<QPointF> positionAt(
    double x) const;

  [[nodiscard]]
  QColor color() const noexcept;

  void refreshAppearance();

private:
  void updateLegend();

  TimeChartView* m_view = nullptr;

  QLineSeries* m_legendSeries = nullptr;

  QValueAxis* m_valueAxis = nullptr;

  std::vector<QLineSeries*> m_lines;
  std::vector<QScatterSeries*> m_points;

  QColor m_color = Qt::blue;
  bool m_visible = true;
};

}