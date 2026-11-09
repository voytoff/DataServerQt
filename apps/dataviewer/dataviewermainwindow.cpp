#include "dataviewermainwindow.h"

#include "archivechartview.h"

#include <QAction>
#include <QFileDialog>
#include <QHeaderView>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QTreeWidget>
#include <QMessageBox>

namespace qds
{

DataViewerMainWindow::DataViewerMainWindow(
  QWidget* parent)
  : QMainWindow(parent)
{
  createActions();
  createUi();

  setWindowTitle(tr("DataViewer"));
}

void DataViewerMainWindow::createActions()
{
  auto* toolbar = addToolBar(tr("Архив"));

  m_openArchiveAction =
    toolbar->addAction(tr("Открыть архив"));

  connect(
    m_openArchiveAction,
    &QAction::triggered,
    this,
    [this]()
    {
      const QString directory =
        QFileDialog::getExistingDirectory(
          this,
          tr("Выберите каталог архива"));

      if (directory.isEmpty())
        return;

      openArchive(
        std::filesystem::path(directory.toStdWString()));
    });
}

void DataViewerMainWindow::createUi()
{
  m_splitter = new QSplitter(Qt::Horizontal, this);

  m_signalTree = new QTreeWidget(m_splitter);

  m_signalTree->setHeaderLabels({
    tr("Сигнал"),
    tr("Частота")
  });

  m_signalTree->setRootIsDecorated(false);
  m_signalTree->setAlternatingRowColors(true);

  m_signalTree->header()->setStretchLastSection(false);
  m_signalTree->header()->setSectionResizeMode(
    0,
    QHeaderView::Stretch);

  m_chartView = new ArchiveChartView(m_splitter);

  m_splitter->addWidget(m_signalTree);
  m_splitter->addWidget(m_chartView);

  m_splitter->setStretchFactor(0, 0);
  m_splitter->setStretchFactor(1, 1);

  m_splitter->setSizes({250, 950});

  setCentralWidget(m_splitter);

  statusBar()->showMessage(tr("Архив не открыт"));
}

void DataViewerMainWindow::openArchive(
  const std::filesystem::path& path)
{
  auto reader = std::make_unique<ArchiveReader>();

  if (!reader->open(path))
  {
    QMessageBox::warning(
      this,
      tr("Открытие архива"),
      tr("Не удалось открыть архив."));
    return;
  }

  m_chartView->clearSignals();

  m_archiveReader = std::move(reader);

  m_chartView->setArchivePath(path);

  populateSignalTree();

  statusBar()->showMessage(
    tr("Архив: %1")
      .arg(QString::fromStdString(path.string())));
}

void DataViewerMainWindow::populateSignalTree()
{
  m_signalTree->clear();

  if (!m_archiveReader ||
      !m_archiveReader->isOpen())
  {
    return;
  }

  auto* rawRoot =
    new QTreeWidgetItem(m_signalTree, {tr("Raw")});

  auto* calculatedRoot =
    new QTreeWidgetItem(m_signalTree, {tr("Calculated")});

  std::map<std::pair<int, QString>, QTreeWidgetItem*> groups;

  const auto& description =
    m_archiveReader->description();

  for (std::size_t fileIndex = 0;
       fileIndex < description.files.size();
       ++fileIndex)
  {
    const auto& file = description.files[fileIndex];

    const auto* header =
      m_archiveReader->fileHeader(fileIndex);

    for (const auto& signal : file.signalIds)
    {
      auto* root =
        signal.kind == SignalKind::Raw
          ? rawRoot
          : calculatedRoot;

      const QString name =
        QString::fromStdString(signal.name);

      const QString firstLetter =
        name.isEmpty()
          ? QStringLiteral("#")
          : name.left(1).toUpper();

      const int kind =
        signal.kind == SignalKind::Raw ? 0 : 1;

      const auto key =
        std::make_pair(kind, firstLetter);

      auto it = groups.find(key);

      if (it == groups.end())
      {
        auto* group =
          new QTreeWidgetItem(root, {firstLetter});

        it = groups.emplace(key, group).first;
      }

      auto* item =
        new QTreeWidgetItem(it->second, {name});

      item->setFlags(
        item->flags() | Qt::ItemIsUserCheckable);

      item->setCheckState(0, Qt::Unchecked);

      item->setData(
        0,
        Qt::UserRole,
        signal.id.value);

      QString tooltip =
        tr("Частота: %1 Гц").arg(file.frequency);

      if (header && header->recordCount > 0)
      {
        tooltip +=
          tr("\nНачало: %1 мкс")
            .arg(header->firstTimestamp);

        tooltip +=
          tr("\nОкончание: %1 мкс")
            .arg(header->lastTimestamp);
      }

      tooltip += tr("\nЕдиница измерения: —");

      item->setToolTip(0, tooltip);
    }
  }

  m_signalTree->sortItems(
    0,
    Qt::AscendingOrder);

  rawRoot->setExpanded(true);
  calculatedRoot->setExpanded(true);
}

}