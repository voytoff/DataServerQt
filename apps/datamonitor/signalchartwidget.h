#pragma once

#include "datatypes.h"

#include <QWidget>
#include <QString>
#include <QTimer>
#include <QPointF>

#include <cstdint>
#include <span>
#include <vector>
#include <unordered_set>
#include <strongidhash.h>

class QChart;
class QChartView;
class QLineSeries;
class QValueAxis;

namespace qds
{

class SignalChartWidget : public QWidget
{
  Q_OBJECT

public:
  explicit SignalChartWidget(
    QWidget* parent = nullptr);

  void setSignals(
    std::span<const SignalId> ids,
    const std::vector<QString>& names);

  void appendData(
    uint64_t timestamp,
    std::span<const double> values);

  void clear();

  void setWindowSeconds(int seconds);

  int windowSeconds() const noexcept;

  void setHiddenSignals(
    std::span<const SignalId> ids);

  std::vector<SignalId> hiddenSignals() const;

  enum class YAxisMode
  {
    Single,
    Multiple
  };

  void setYAxisMode(
    YAxisMode mode);

private:
  void updateAxes(double currentTime);
  void setupLegend();
  void updateSeriesVisibility();
  void refreshChart();

  void updateAxisVisibility();
  void updateAxisAlignment();

private:
  QChart* m_chart = nullptr;
  QChartView* m_chartView = nullptr;

  QValueAxis* m_axisX = nullptr;

  std::vector<QLineSeries*> m_series;

  uint64_t m_firstTimestamp = 0;
  uint64_t m_lastTimestamp = 0;
  bool m_hasFirstTimestamp = false;

  int m_windowSeconds = 10;

  std::vector<SignalId> m_signalIds;

  std::unordered_set<SignalId> m_hiddenSignals;

  QTimer m_refreshTimer;
  std::vector<std::vector<QPointF>> m_pendingPoints;
  static constexpr int RefreshIntervalMs = 50;

  // оси Y
  YAxisMode m_yAxisMode = YAxisMode::Single;
  std::vector<QValueAxis*> m_axesY;
  std::size_t m_activeAxisIndex = 0;
  std::vector<Qt::Alignment> m_axisAlignments;

signals:
  void windowSecondsChanged(int seconds);
  void hiddenSignalsChanged();

};

}