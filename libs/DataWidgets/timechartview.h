#pragma once

#include <QChartView>
#include <QCategoryAxis>
#include <QLineSeries>
#include <QXYSeries>

#include "datawidgets_export.h"
#include "timeaxis.h"

namespace qds
{

class DATAWIDGETS_EXPORT TimeChartView : public QChartView
{
  Q_OBJECT

public:
  explicit TimeChartView(QWidget* parent = nullptr);

  void setTimeAxis(TimeAxis* timeAxis);
  [[nodiscard]] TimeAxis* timeAxis() const noexcept;

  void setWallClockReference(
    Timestamp timestamp,
    WallClockTime wallTime);

  void addSeries(QXYSeries* series);
  void clearSeries();

  void setTimeRange(double minimum, double maximum);
  void setValueRange(double minimum, double maximum);

  [[nodiscard]] QCategoryAxis* timeChartAxis() const noexcept;
  [[nodiscard]] QValueAxis* valueChartAxis() const noexcept;

  void autoScaleY();

  void resetTimeRange();

  void setVisibleTimeRange(double minimum, double maximum);

  [[nodiscard]] std::pair<double, double>
  visibleTimeRange() const noexcept;

protected:
  void wheelEvent(
    QWheelEvent* event) override;
  void mousePressEvent(
    QMouseEvent* event) override;
  void mouseMoveEvent(
    QMouseEvent* event) override;
  void mouseReleaseEvent(
    QMouseEvent* event) override;
  void mouseDoubleClickEvent(
    QMouseEvent* event) override;

private:
  [[nodiscard]] QString formatTime(double chartSeconds) const;
  void applyTimeRange(double minimum, double maximum);

  std::pair<double, double> clampTimeRange(
    double minimum,
    double maximum) const noexcept;

private:
  TimeAxis* m_timeAxis = nullptr;

  QCategoryAxis* m_axisX = nullptr;
  QValueAxis* m_axisY = nullptr;

  Timestamp m_referenceTimestamp{};
  WallClockTime m_referenceWallTime{};
  bool m_hasWallClockReference = false;

  double m_initialTimeMinimum = 0.0;
  double m_initialTimeMaximum = 10.0;

  bool m_panning = false;
  QPointF m_lastMousePosition;

private slots:
  void updateTimeLabels();

signals:
  void visibleTimeRangeChanged(double minimum, double maximum);
};

}