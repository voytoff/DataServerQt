#include "archivetableview.h"

#include "archivetablecontroller.h"
#include "archivetablemodel.h"

#include <QScrollBar>
#include <QFontDatabase>

#include <memory>
#include <utility>
#include <vector>
#include <ranges>

namespace qds
{

struct ArchiveTableView::Impl
{
  std::filesystem::path archivePath;

  ArchiveTableController* controller = nullptr;
  ArchiveTableModel* model = nullptr;

  bool initialized = false;

  Timestamp experimentStart{};
  bool hasTimeRange = false;

  std::optional<uint64_t> requestedBlock;
  std::optional<uint64_t> requestId;
};

ArchiveTableView::ArchiveTableView(
  QWidget* parent)
  : BaseTableView(100, parent)
  , m_impl(std::make_unique<Impl>())
{
  m_impl->model = new ArchiveTableModel(this);

  setModel(m_impl->model);

  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setAlternatingRowColors(true);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setSelectionMode(QAbstractItemView::ExtendedSelection);

  QFont font = QFontDatabase::systemFont(
    QFontDatabase::FixedFont);

  font.setPointSize(9);
  //font.setStyleHint(QFont::Monospace);
  //font.setFixedPitch(true);

  setFont(font);

  connect(
    verticalScrollBar(),
    &QScrollBar::valueChanged,
    this,
    [this](int)
    {
      requestVisibleData();
    });
}

ArchiveTableView::~ArchiveTableView()
{
  delete m_impl->controller;
}

void ArchiveTableView::setArchivePath(
  const std::filesystem::path& path)
{
  if (m_impl->archivePath == path)
    return;

  delete m_impl->controller;
  m_impl->controller = nullptr;

  m_impl->initialized = false;
  m_impl->archivePath = path;

  m_impl->model->clear();
  m_impl->hasTimeRange = false;

  if (path.empty())
    return;

  m_impl->controller =
    new ArchiveTableController(path, this);

  connect(
    m_impl->controller,
    &ArchiveTableController::blockReady,
    this,
    [this](
      uint64_t requestId,
      ArchiveTableBlock block)
    {
      if (m_impl->requestId != requestId)
        return;

      m_impl->requestId.reset();
      m_impl->requestedBlock.reset();

      m_impl->model->setBlock(
        std::move(block));

      requestVisibleData();
    });

  connect(
    m_impl->controller,
    &ArchiveTableController::blockFailed,
    this,
    [this](uint64_t requestId)
    {
      if (m_impl->requestId != requestId)
        return;

      m_impl->requestId.reset();
      m_impl->requestedBlock.reset();
    });

  m_impl->initialized = true;
}

void ArchiveTableView::setFrequency(
  uint32_t frequency)
{
  if (!m_impl->initialized)
    return;

  if (m_impl->model->frequency() == frequency)
    return;

  m_impl->controller->cancel();
  m_impl->requestedBlock.reset();
  m_impl->model->setFrequency(frequency);

  requestVisibleData();
}


uint32_t ArchiveTableView::frequency() const noexcept
{
  return m_impl->initialized
    ? m_impl->model->frequency()
    : 0;
}

void ArchiveTableView::setChannels(
  std::vector<ArchiveTableChannel> channels)
{
  if (!m_impl->initialized)
    return;

  m_impl->controller->cancel();
  m_impl->requestedBlock.reset();
  m_impl->model->setChannels(
    std::move(channels));

  requestVisibleData();
}

void ArchiveTableView::setTimeRange(
  Timestamp firstTimestamp,
  Timestamp lastTimestamp,
  Timestamp referenceTimestamp,
  WallClockTime referenceWallTime)
{
  if (!m_impl->initialized)
    return;

  m_impl->controller->cancel();
  m_impl->requestedBlock.reset();

  m_impl->experimentStart = firstTimestamp;
  m_impl->hasTimeRange = true;

  m_impl->model->setTimeRange(
    firstTimestamp,
    lastTimestamp,
    referenceTimestamp,
    referenceWallTime);

  requestVisibleData();
}

void ArchiveTableView::setDecimals(int decimals)
{
  if (!m_impl->model)
    return;

  m_impl->model->setDecimals(decimals);
}

int ArchiveTableView::decimals() const noexcept
{
  return m_impl->model->decimals();
}

void ArchiveTableView::resizeEvent(
  QResizeEvent* event)
{
  BaseTableView::resizeEvent(event);

  requestVisibleData();
}

void ArchiveTableView::requestVisibleData()
{
  if (!m_impl->initialized ||
      !m_impl->hasTimeRange ||
      !m_impl->controller ||
      m_impl->requestedBlock ||
      m_impl->model->channels().empty())
  {
    return;
  }

  const int totalRows = m_impl->model->rowCount();

  if (totalRows <= 0)
    return;

  const int firstRow = rowAt(0);

  if (firstRow < 0)
    return;

  int lastRow =
    rowAt(viewport()->height() - 1);

  if (lastRow < 0)
  {
    const int visibleRows =
      std::max(1, viewport()->height() / 26 + 1);

    lastRow = std::min(
      totalRows - 1,
      firstRow + visibleRows);
  }

  constexpr uint64_t BlockSize = 1024;

  const uint64_t firstBlock =
    static_cast<uint64_t>(firstRow) / BlockSize;

  const uint64_t lastBlock =
    static_cast<uint64_t>(lastRow) / BlockSize;

  for (uint64_t blockIndex = firstBlock;
       blockIndex <= lastBlock;
       ++blockIndex)
  {
    const uint64_t blockFirstRow =
      blockIndex * BlockSize;

    if (m_impl->model->hasBlock(blockFirstRow))
      continue;

    const uint32_t count =
      static_cast<uint32_t>(
        std::min<uint64_t>(
          BlockSize,
          static_cast<uint64_t>(totalRows) -
            blockFirstRow));

    std::vector<SignalId> signalIds;

    signalIds.reserve(
      m_impl->model->channels().size());

    for (const auto& channel :
         m_impl->model->channels())
    {
      signalIds.push_back(channel.id);
    }

    m_impl->requestedBlock = blockFirstRow;

    m_impl->requestId =
      m_impl->controller->readBlock(
        m_impl->experimentStart,
        m_impl->model->frequency(),
        blockFirstRow,
        count,
        std::move(signalIds));

    return;
  }
}

}