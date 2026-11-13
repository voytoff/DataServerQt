#pragma once

#include "archivetableblock.h"
#include "datawidgets_export.h"
#include "datatypes.h"

#include <QAbstractTableModel>
#include <QString>

#include <cstdint>
#include <vector>

namespace qds
{

struct ArchiveTableChannel
{
  SignalId id;
  QString name;
  QString unit;
};

class DATAWIDGETS_EXPORT ArchiveTableModel
  : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit ArchiveTableModel(
    QObject* parent = nullptr);

  ~ArchiveTableModel() override;

  [[nodiscard]]
  int rowCount(
    const QModelIndex& parent = {}) const override;

  [[nodiscard]]
  int columnCount(
    const QModelIndex& parent = {}) const override;

  [[nodiscard]]
  QVariant data(
    const QModelIndex& index,
    int role = Qt::DisplayRole) const override;

  [[nodiscard]]
  QVariant headerData(
    int section,
    Qt::Orientation orientation,
    int role = Qt::DisplayRole) const override;

  void setFrequency(uint32_t frequency);

  [[nodiscard]]
  uint32_t frequency() const noexcept;

  void setSignals(
    std::vector<ArchiveTableChannel> channels);

  [[nodiscard]]
  const std::vector<ArchiveTableChannel>&
  tableSignals() const noexcept;

  void clear();

  void setChannels(
    std::vector<ArchiveTableChannel> channels);

  [[nodiscard]]
  const std::vector<ArchiveTableChannel>&
  channels() const noexcept;

  void setTimeRange(
    Timestamp firstTimestamp,
    Timestamp lastTimestamp,
    Timestamp referenceTimestamp,
    WallClockTime referenceWallTime);

  void setBlock(ArchiveTableBlock block);
  void clearBlocks();

  [[nodiscard]]
  bool hasBlock(
    uint64_t firstRow) const noexcept;

  void setDecimals(int decimals);

  [[nodiscard]]
  int decimals() const noexcept;

  void setShowUnitsInHeaders(bool enabled);

  [[nodiscard]]
  bool showUnitsInHeaders() const noexcept;

private:
  void updateGrid() noexcept;

private:
  uint32_t m_frequency = 100;

  std::vector<ArchiveTableChannel> m_channels;

  Timestamp m_firstTimestamp;
  Timestamp m_lastTimestamp;

  // значения из одной архивной записи.
  // Они позволят правильно вычислять календарное время каждой строки
  Timestamp m_referenceTimestamp;
  WallClockTime m_referenceWallTime;

  bool m_hasTimeRange = false;

  uint64_t m_gridStart = 0;
  uint64_t m_gridEnd = 0;

  Timestamp m_gridOrigin;

  int64_t m_displayWallOrigin = 0;

  // ключом будет firstRow блока
  std::map<uint64_t, ArchiveTableBlock> m_blocks;

  int m_decimals = 6;

  bool m_showUnitsInHeaders = false;
};

}