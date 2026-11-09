#pragma once

#include "datawidgets_export.h"
#include "timeaxis.h"

#include <QChartView>
#include <QCategoryAxis>
#include <QLineSeries>
#include <QXYSeries>
#include <QValueAxis>

#include <vector>

namespace qds
{

enum class YAxisMode
{
  // все сигналы используют существующую m_axisY.
  Single,
  // каждый ArchiveChartSeries получает собственную ось Y и независимое автоматическое масштабирование.
  Multiple
};

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

  [[nodiscard]]
  QCategoryAxis* timeChartAxis() const noexcept;

  [[nodiscard]]
  QValueAxis* valueChartAxis() const noexcept;

  void autoScaleY();

  void autoScaleY(
    QValueAxis* axis);

  void resetTimeRange();

  void setVisibleTimeRange(double minimum, double maximum);

  [[nodiscard]] std::pair<double, double>
  visibleTimeRange() const noexcept;

  void setYAxisMode(YAxisMode mode);
  YAxisMode yAxisMode() const noexcept;

  QValueAxis* addValueAxis(
    Qt::Alignment alignment = Qt::AlignLeft);

  void removeValueAxis(QValueAxis* axis);

  bool setValueAxisAlignment(
    QValueAxis* axis,
    Qt::Alignment alignment);

  void setAvailableTimeRange(
    double minimum,
    double maximum);

  virtual void updateChartTheme();

  void setMeasurementCursorLocked(bool locked);
  [[nodiscard]] bool isMeasurementCursorLocked() const noexcept;

  [[nodiscard]] double chartTimeAt(
    qreal positionX) const noexcept;

  void updateTimeSelection(
    const QPointF& position);

protected:
  void setMeasurementCursorPosition(
    const QPointF& position);

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

  void leaveEvent(
    QEvent* event) override;

  void drawForeground(
    QPainter* painter,
    const QRectF& rect) override;

private:
  [[nodiscard]] QString formatTime(double chartSeconds) const;
  void applyTimeRange(double minimum, double maximum);

  std::pair<double, double> clampTimeRange(
    double minimum,
    double maximum) const noexcept;

  void updateMeasurementCursor(
    const QPointF& position);
  void hideMeasurementCursor();

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

  YAxisMode m_yAxisMode = YAxisMode::Single;

  std::vector<QValueAxis*> m_additionalAxesY;

  // хранит координату времени в единицах оси X, а не координату мыши в пикселях
  // это позволит правильно перестраивать курсор при изменении диапазона времени
  double m_measurementCursorX = 0.0;
  bool m_measurementCursorVisible = false;

  bool m_measurementCursorLocked = false;

  bool m_selectingTimeRange = false;

  double m_selectionStartX = 0.0;
  double m_selectionEndX = 0.0;

  QPointF m_selectionStartPosition;

private slots:
  void updateTimeLabels();

signals:
  void visibleTimeRangeChanged(
    double minimum,
    double maximum);

  void measurementCursorMoved(double x);
  void measurementCursorHidden();
};

}