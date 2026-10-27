#pragma once

#include "datatypes.h"

#include <QWidget>
#include <QString>

#include <cstdint>
#include <span>
#include <vector>
#include <unordered_set>
#include <strongidhash.h>

class QChart;
class QChartView;
class QLineSeries;
class QValueAxis;
class QComboBox;

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

private:
  void updateAxes(double currentTime);
    void setupLegend();

private:
  QChart* m_chart = nullptr;
  QChartView* m_chartView = nullptr;

  QValueAxis* m_axisX = nullptr;
  QValueAxis* m_axisY = nullptr;

  QComboBox* m_intervalCombo = nullptr;

  std::vector<QLineSeries*> m_series;

  uint64_t m_firstTimestamp = 0;
  uint64_t m_lastTimestamp = 0;
  bool m_hasFirstTimestamp = false;

  int m_windowSeconds = 10;

  std::vector<SignalId> m_signalIds;

  std::unordered_set<SignalId> m_hiddenSignals;

  void updateSeriesVisibility();

signals:
  void windowSecondsChanged(int seconds);
  void hiddenSignalsChanged();

};

}