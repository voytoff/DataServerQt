
#pragma once

#include "datawidgets_export.h"
#include "timechartview.h"

#include <datatypes.h>

#include <QColor>
#include <QString>

#include <filesystem>
#include <memory>
#include <vector>

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

private:
  struct SignalContext;
  struct Impl;

  void initializeTimeRange();
  void requestVisibleData();

  void requestSignalData(
    SignalContext* context);

  void updateAxisLayout();

  void updateTimeRange();


private:
  std::unique_ptr<Impl> m_impl;

};

}
