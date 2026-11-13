#pragma once

#include "basetableview.h"
#include "datawidgets_export.h"

#include <datatypes.h>

#include <QString>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

namespace qds
{

class ArchiveTableModel;
class ArchiveTableController;

struct ArchiveTableChannel;

class DATAWIDGETS_EXPORT ArchiveTableView
  : public BaseTableView
{
  Q_OBJECT

public:
  explicit ArchiveTableView(
    QWidget* parent = nullptr);

  ~ArchiveTableView() override;

  void setArchivePath(
    const std::filesystem::path& path);

  void setFrequency(uint32_t frequency);

  [[nodiscard]]
  uint32_t frequency() const noexcept;

  void setChannels(
    std::vector<ArchiveTableChannel> channels);

  void setTimeRange(
    Timestamp firstTimestamp,
    Timestamp lastTimestamp,
    Timestamp referenceTimestamp,
    WallClockTime referenceWallTime);

  void clear();

  void setDecimals(int decimals);

  [[nodiscard]]
  int decimals() const noexcept;

  void setShowUnitsInHeaders(bool enabled);

  [[nodiscard]]
  bool showUnitsInHeaders() const noexcept;

protected:
  void resizeEvent(
    QResizeEvent* event) override;

private:
  struct Impl;

  void requestVisibleData();
  void requestBlock(uint64_t firstRow);

  std::unique_ptr<Impl> m_impl;
};

}