#include "archivetablemodel.h"
#include "archivetableblock.h"

#include <QDateTime>

#include <map>
#include <vector>
#include <ranges>
#include <algorithm>
#include <limits>
#include <cmath>

namespace qds
{

namespace
{

constexpr uint64_t MicrosecondsPerSecond = 1'000'000;

bool validFrequency(uint32_t frequency)
{
  return frequency == 1 ||
         frequency == 10 ||
         frequency == 100 ||
         frequency == 1000;
}

}

ArchiveTableModel::ArchiveTableModel(
  QObject* parent)
  : QAbstractTableModel(parent)
{
}

ArchiveTableModel::~ArchiveTableModel() = default;

int ArchiveTableModel::rowCount(
  const QModelIndex& parent) const
{
  if (parent.isValid() || !m_hasTimeRange)
    return 0;

  constexpr uint64_t MicrosecondsPerSecond = 1'000'000;

  const uint64_t step =
    MicrosecondsPerSecond / m_frequency;

  const uint64_t duration =
    m_gridEnd - m_gridStart;

  const uint64_t count =
    duration / step + 1;

  return static_cast<int>(
    std::min<uint64_t>(
      count,
      std::numeric_limits<int>::max()));
}

int ArchiveTableModel::columnCount(
  const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  return 2 + static_cast<int>(m_channels.size());
}

QVariant ArchiveTableModel::data(
  const QModelIndex& index,
  int role) const
{
  if (!index.isValid() ||
      index.row() < 0 ||
      index.row() >= rowCount() ||
      index.column() >= columnCount())
  {
    return {};
  }

  if (role == Qt::TextAlignmentRole)
  {
    return index.column() == 1
             ? QVariant(Qt::AlignCenter)
             : QVariant(Qt::AlignRight | Qt::AlignVCenter);
  }

  if (role != Qt::DisplayRole &&
      role != Qt::EditRole)
  {
    return {};
  }

  constexpr uint64_t MicrosecondsPerSecond = 1'000'000;

  const uint64_t step =
    MicrosecondsPerSecond / m_frequency;

  const uint64_t offset =
    static_cast<uint64_t>(index.row()) * step;

  //const uint64_t timestamp =
  //  m_gridStart + offset;

  if (index.column() == 0)
  {
    const double seconds =
      static_cast<double>(offset) /
      MicrosecondsPerSecond;

    if (role == Qt::EditRole)
      return seconds;

    const int decimals =
      m_frequency == 1000 ? 3 :
        m_frequency == 100  ? 2 :
        m_frequency == 10   ? 1 : 0;

    return QLocale(QLocale::Russian)
      .toString(seconds, 'f', decimals);
  }
  else if (index.column() == 1)
  {
    const qint64 milliseconds =
      m_displayWallOrigin / 1000 +
      static_cast<qint64>(offset / 1000);

    const QDateTime dateTime =
      QDateTime::fromMSecsSinceEpoch(milliseconds);

    if (role == Qt::EditRole)
      return dateTime;

    const int decimals =
      m_frequency == 1000 ? 3 :
        m_frequency == 100  ? 2 :
        m_frequency == 10   ? 1 : 0;

    QString format =
      QStringLiteral("dd.MM.yyyy HH:mm:ss");

    if (decimals > 0)
    {
      format += QStringLiteral(".zzz");
    }

    return dateTime.toString(format);
  }

  const uint64_t row =
    static_cast<uint64_t>(index.row());

  auto it = m_blocks.upper_bound(row);

  if (it == m_blocks.begin())
    return {};

  --it;

  const auto& block = it->second;

  const uint64_t blockRow =
    row - block.firstRow;

  const std::size_t column =
    static_cast<std::size_t>(index.column() - 2);

  if (column >= block.values.size())
    return {};

  const auto& values = block.values[column];

  if (blockRow >= values.size())
    return {};

  const auto& value = values[blockRow];

  if (!value)
    return {};

  if (role == Qt::EditRole)
    return *value;

  return QLocale(QLocale::Russian).toString(
    *value,
    'f',
    m_decimals);
}

QVariant ArchiveTableModel::headerData(
  int section,
  Qt::Orientation orientation,
  int role) const
{
  if (role != Qt::DisplayRole)
    return {};

  if (orientation == Qt::Vertical)
    return section + 1;

  if (section == 0)
    return QStringLiteral("Секунды");

  if (section == 1)
    return QStringLiteral("Время");

  const int channelIndex = section - 2;

  if (channelIndex >= 0 &&
      channelIndex < static_cast<int>(m_channels.size()))
  {
    const auto& channel = m_channels[channelIndex];

    if (m_showUnitsInHeaders &&
        !channel.unit.isEmpty())
    {
      return QStringLiteral("%1, %2")
      .arg(channel.name, channel.unit);
    }

    return channel.name;
  }

  return {};
}

void ArchiveTableModel::setFrequency(
  uint32_t frequency)
{
  if (!validFrequency(frequency) ||
      m_frequency == frequency)
  {
    return;
  }

  beginResetModel();

  m_frequency = frequency;
  updateGrid();
  m_blocks.clear();

  endResetModel();
}

uint32_t ArchiveTableModel::frequency() const noexcept
{
  return m_frequency;
}

void ArchiveTableModel::setChannels(
  std::vector<ArchiveTableChannel> channels)
{
  beginResetModel();

  m_channels = std::move(channels);
  m_blocks.clear();

  endResetModel();
}

const std::vector<ArchiveTableChannel>&
ArchiveTableModel::channels() const noexcept
{
  return m_channels;
}

void ArchiveTableModel::setTimeRange(
  Timestamp firstTimestamp,
  Timestamp lastTimestamp,
  Timestamp referenceTimestamp,
  WallClockTime referenceWallTime)
{
  beginResetModel();

  m_firstTimestamp = firstTimestamp;
  m_lastTimestamp = lastTimestamp;

  m_referenceTimestamp = referenceTimestamp;
  m_referenceWallTime = referenceWallTime;

  m_hasTimeRange =
    firstTimestamp <= lastTimestamp;

  updateGrid();

  m_blocks.clear();

  endResetModel();
}

void ArchiveTableModel::setBlock(
  ArchiveTableBlock block)
{
  const uint64_t firstRow = block.firstRow;

  if (hasBlock(firstRow))
    return;

  if (block.frequency != m_frequency ||
      block.channels.size() != m_channels.size())
    return;

  for (std::size_t i = 0; i < m_channels.size(); ++i)
  {
    if (block.channels[i] != m_channels[i].id)
      return;
  }

  if (block.values.size() != m_channels.size() ||
      block.values.empty())
    return;

  const std::size_t count =
    block.values.front().size();

  if (count == 0)
    return;

  for (const auto& values : block.values)
  {
    if (values.size() != count)
      return;
  }

  const uint64_t totalRows =
    static_cast<uint64_t>(rowCount());

  if (firstRow >= totalRows)
    return;

  const uint64_t lastRow =
    firstRow + std::min<uint64_t>(
      count,
      totalRows - firstRow);

  m_blocks.insert_or_assign(
    firstRow,
    std::move(block));

  constexpr std::size_t MaxCachedBlocks = 4;

  if (m_blocks.size() > MaxCachedBlocks)
  {
    uint64_t maxDistance = 0;
    auto farthest = m_blocks.end();

    for (auto it = m_blocks.begin();
         it != m_blocks.end();
         ++it)
    {
      const uint64_t key = it->first;

      const uint64_t distance =
        key > firstRow
          ? key - firstRow
          : firstRow - key;

      if (farthest == m_blocks.end() ||
          distance > maxDistance)
      {
        maxDistance = distance;
        farthest = it;
      }
    }

    if (farthest != m_blocks.end())
      m_blocks.erase(farthest);
  }

  emit dataChanged(
    index(static_cast<int>(firstRow), 2),
    index(static_cast<int>(lastRow - 1),
          columnCount() - 1),
    {Qt::DisplayRole, Qt::EditRole});
}

void ArchiveTableModel::clearBlocks()
{
  m_blocks.clear();

  if (rowCount() == 0 || m_channels.empty())
    return;

  emit dataChanged(
    index(0, 2),
    index(rowCount() - 1, columnCount() - 1),
    {Qt::DisplayRole, Qt::EditRole});
}

bool ArchiveTableModel::hasBlock(
  uint64_t firstRow) const noexcept
{
  return m_blocks.contains(firstRow);
}

void ArchiveTableModel::setDecimals(int decimals)
{
  if (decimals < 0 || decimals > 12)
    return;

  if (m_decimals == decimals)
    return;

  m_decimals = decimals;

  if (rowCount() > 0 && columnCount() > 2)
  {
    emit dataChanged(
      index(0, 2),
      index(rowCount() - 1, columnCount() - 1),
      {Qt::DisplayRole});
  }
}

int ArchiveTableModel::decimals() const noexcept
{
  return m_decimals;
}


void ArchiveTableModel::setShowUnitsInHeaders(
  bool enabled)
{
  if (m_showUnitsInHeaders == enabled)
    return;

  m_showUnitsInHeaders = enabled;

  if (m_channels.empty())
    return;

  emit headerDataChanged(
    Qt::Horizontal,
    2,
    columnCount() - 1);
}

bool ArchiveTableModel::showUnitsInHeaders() const noexcept
{
  return m_showUnitsInHeaders;
}

void ArchiveTableModel::updateGrid() noexcept
{
  m_gridStart = 0;
  m_gridEnd = 0;
  m_displayWallOrigin = 0;

  if (!m_hasTimeRange)
    return;

  constexpr int64_t MicrosecondsPerSecond = 1'000'000;

  m_gridStart = m_firstTimestamp.value;
  m_gridEnd = m_lastTimestamp.value;

  const int64_t offset =
    static_cast<int64_t>(
      m_firstTimestamp.value -
      m_referenceTimestamp.value);

  const int64_t firstWallTime =
    m_referenceWallTime.unixMicroseconds + offset;

  // Округление к ближайшей целой секунде.
  const int64_t seconds =
    firstWallTime / MicrosecondsPerSecond;

  const int64_t remainder =
    firstWallTime % MicrosecondsPerSecond;

  m_displayWallOrigin =
    (seconds + (remainder >= 500'000 ? 1 : 0)) *
    MicrosecondsPerSecond;
}

void ArchiveTableModel::clear()
{
  beginResetModel();

  m_channels.clear();
  m_firstTimestamp = {};
  m_lastTimestamp = {};
  m_referenceTimestamp = {};
  m_referenceWallTime = {};
  m_hasTimeRange = false;

  m_blocks.clear();

  endResetModel();
}

}