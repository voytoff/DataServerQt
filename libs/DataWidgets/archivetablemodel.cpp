#include "archivetablemodel.h"

#include <QDateTime>

#include <algorithm>
#include <limits>

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

  const uint64_t timestamp =
    m_gridStart + offset;

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

  // Значения каналов добавим после
  // реализации блочного чтения.
  return {};
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
    return m_channels[channelIndex].name;
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

  endResetModel();
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

  endResetModel();
}

}