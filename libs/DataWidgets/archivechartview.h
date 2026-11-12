
#pragma once

#include "datasettings.h"
#include "datawidgets_export.h"
#include "timechartview.h"

#include <datatypes.h>

#include <QColor>
#include <QString>

#include <filesystem>
#include <memory>
#include <optional>

namespace qds
{

class ArchiveChartController;
class ArchiveChartSeries;

class DATAWIDGETS_EXPORT ArchiveChartView
  : public TimeChartView
{
  Q_OBJECT

public:
  explicit ArchiveChartView(
    QWidget* parent = nullptr);

  ~ArchiveChartView() override;

  void setArchivePath(
    const std::filesystem::path& path);

  bool addSignal(
    SignalId id,
    const QString& name,
    const QColor& color);

  void clearSignals();

  [[nodiscard]] std::size_t signalCount() const noexcept;

  bool removeSignal(SignalId id);

  [[nodiscard]]
  std::optional<double> valueAt(
    SignalId id,
    double x) const;

  void updateChartTheme() override;

  void setTimeOrigin(
    Timestamp timestamp);

  void setChartSettings(
    const ChartSettings& settings);

protected:
  void drawForeground(
    QPainter* painter,
    const QRectF& rect) override;

  void mousePressEvent(
    QMouseEvent* event) override;

private:
  struct SignalContext;
  struct Impl;

  void initializeTimeRange();
  void requestVisibleData();

  void requestSignalData(
    SignalContext* context);

  void updateAxisLayout();

  void updateTimeRange();

  void drawMeasurementPanel(
    QPainter* painter);

  void captureMeasurement();

  [[nodiscard]]
  std::optional<QPointF> measurementPoint(
    const ArchiveChartSeries* series,
    double x) const;

private:
  std::unique_ptr<Impl> m_impl;

  ChartSettings chartSettings;

};

}