#include "dataviewermainwindow.h"

#include "archivechartview.h"
#include "archivetablemodel.h"
#include "colorscheme.h"
#include "themedsvgicon.h"
#include "chartpalette.h"
#include "basetableview.h"

#include <QAction>
#include <QFileDialog>
#include <QHeaderView>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QTreeWidget>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QTableView>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabBar>
#include <QTreeWidgetItemIterator>

#include <algorithm>

namespace qds
{

DataViewerMainWindow::DataViewerMainWindow(
  QWidget* parent)
  : QMainWindow(parent)
{
  createActions();
  createUi();

  setWindowTitle(tr("DataViewer"));
  setWindowIcon(
    QIcon(":/images/main.png"));

  connect(
    m_signalTree,
    &QTreeWidget::itemChanged,
    this,
    [this](QTreeWidgetItem* item, int column)
    {
      if (column != 0)
        return;

      if (!(item->flags() &
            Qt::ItemIsUserCheckable))
        return;

      onSignalChecked(
        item,
        item->checkState(0) == Qt::Checked);
    });
}

void DataViewerMainWindow::createActions()
{
  m_toolBar = addToolBar(tr("Архив"));
  m_toolBar->setIconSize(QSize(24, 24));
  m_toolBar->setMovable(false);
  m_toolBar->setFloatable(false);
  m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

  m_openArchiveAction =
    m_toolBar->addAction(
    themedSvgIcon(":/images/open.svg"),
    tr("Открыть архив"));

  m_openArchiveAction->setToolTip(
    tr("Выбор каталога с описанием и файлами архивных данных"));

  m_toolBar->addSeparator();

  m_addTableAction = m_toolBar->addAction(
    themedSvgIcon(":/images/newtable.svg"),
    tr("Новая таблица"));

  m_addTableAction->setToolTip(
    tr("Создать новую таблицу архивных данных"));

  m_toolBar->addSeparator();

  m_themeAction = m_toolBar->addAction(
    themedSvgIcon(":/images/theme.svg"),
    tr("Сменить тему"));

  m_themeAction->setToolTip(
    tr("Переключить светлую/тёмную тему"));


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


  connect(
    m_addTableAction,
    &QAction::triggered,
    this,
    &DataViewerMainWindow::addTable);

  connect(
    m_themeAction,
    &QAction::triggered,
    this,
    &toggleColorScheme);
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


  m_tabs = new QTabWidget(m_splitter);

  m_tabs->setTabsClosable(true);
  m_tabs->setMovable(false);

  m_chartView = new ArchiveChartView(m_tabs);

  m_tabs->addTab(
    m_chartView,
    tr("График"));

  m_splitter->addWidget(m_signalTree);
  m_splitter->addWidget(m_tabs);

  connect(
    m_tabs,
    &QTabWidget::currentChanged,
    this,
    [this](int)
    {
      updateSignalTreeChecks();
    });

  connect(
    m_tabs,
    &QTabWidget::tabCloseRequested,
    this,
    &DataViewerMainWindow::closeTable);

  m_tabs->tabBar()->setTabButton(
    0,
    QTabBar::RightSide,
    nullptr);

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

  while (m_tabs->count() > 1)
    closeTable(1);

  m_tabs->setCurrentIndex(0);

  m_chartView->clearSignals();
  m_chartSignals.clear();

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

  QSignalBlocker blocker(m_signalTree);

  auto* rawRoot =
    new QTreeWidgetItem(m_signalTree, {tr("Raw")});

  rawRoot->setFlags(
    rawRoot->flags() & ~Qt::ItemIsUserCheckable);

  auto* calculatedRoot =
    new QTreeWidgetItem(m_signalTree, {tr("Calculated")});

  calculatedRoot->setFlags(
    calculatedRoot->flags() & ~Qt::ItemIsUserCheckable);

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

        group->setFlags(
          group->flags() & ~Qt::ItemIsUserCheckable);

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

      QPixmap pixmap(12, 12);
      pixmap.fill(Qt::transparent);

      QPainter painter(&pixmap);
      painter.setRenderHint(QPainter::Antialiasing);
      painter.setPen(Qt::NoPen);
      painter.setBrush(
        ChartPalette::signalColor(signal.id.value));
      painter.drawEllipse(1, 1, 10, 10);
      painter.end();

      item->setIcon(0, QIcon(pixmap));
    }

    updateSignalTreeChecks();
  }

  m_signalTree->sortItems(
    0,
    Qt::AscendingOrder);

  rawRoot->setExpanded(true);
  calculatedRoot->setExpanded(true);
}

void DataViewerMainWindow::addTable()
{
  if (!m_archiveReader ||
      !m_archiveReader->isOpen())
  {
    return;
  }

  auto context =
    std::make_unique<TableContext>();

  auto* page = new QWidget(m_tabs);

  auto* layout = new QVBoxLayout(page);

  auto* controls = new QHBoxLayout();

  auto* label = new QLabel(
    tr("Частота таблицы:"),
    page);

  auto* combo = new QComboBox(page);

  for (const uint32_t frequency :
       {1u, 10u, 100u, 1000u})
  {
    combo->addItem(
      tr("%1 Гц").arg(frequency),
      frequency);
  }

  combo->setCurrentIndex(
    combo->findData(100u));

  controls->addWidget(label);
  controls->addWidget(combo);
  controls->addStretch();

  auto* view = new BaseTableView(100, page);

  context->model = new ArchiveTableModel(view);

  Timestamp firstTimestamp;
  Timestamp lastTimestamp;

  Timestamp referenceTimestamp;
  WallClockTime referenceWallTime;

  if (m_archiveReader->timeRange(
        firstTimestamp,
        lastTimestamp) &&
      m_archiveReader->timeReference(
        referenceTimestamp,
        referenceWallTime))
  {
    context->model->setTimeRange(
      firstTimestamp,
      lastTimestamp,
      referenceTimestamp,
      referenceWallTime);
  }

  view->setModel(context->model);

  view->setEditTriggers(
    QAbstractItemView::NoEditTriggers);

  view->setAlternatingRowColors(true);

  view->setSelectionBehavior(
    QAbstractItemView::SelectRows);

  view->setSelectionMode(
    QAbstractItemView::ExtendedSelection);


  layout->addLayout(controls);
  layout->addWidget(view);

  context->widget = page;
  context->view = view;
  context->frequencyCombo = combo;

  auto* table = context.get();

  connect(
    combo,
    &QComboBox::currentIndexChanged,
    this,
    [table, combo](int)
    {
      const uint32_t frequency =
        combo->currentData().toUInt();

      table->frequency = frequency;
      table->model->setFrequency(frequency);
    });

  m_tables.push_back(
    std::move(context));

  const int index = m_tabs->addTab(
    page,
    tr("Таблица %1").arg(m_tables.size()));

  m_tabs->setCurrentIndex(index);
}

void DataViewerMainWindow::closeTable(
  int index)
{
  if (index <= 0)
    return;

  QWidget* widget =
    m_tabs->widget(index);

  const auto it = std::find_if(
    m_tables.begin(),
    m_tables.end(),
    [widget](const auto& table)
    {
      return table->widget == widget;
    });

  if (it == m_tables.end())
    return;

  m_tabs->removeTab(index);

  m_tables.erase(it);

  widget->deleteLater();

  updateSignalTreeChecks();
}

void DataViewerMainWindow::updateSignalTreeChecks()
{
  QSignalBlocker blocker(m_signalTree);

  QTreeWidgetItemIterator it(m_signalTree);

  while (*it)
  {
    auto* item = *it;
    ++it;

    if (!(item->flags() &
          Qt::ItemIsUserCheckable))
    {
      continue;
    }

    const SignalId id{
      item->data(
            0,
            Qt::UserRole).toUInt()
    };

    item->setCheckState(
      0,
      isSignalSelected(id)
        ? Qt::Checked
        : Qt::Unchecked);
  }
}

void DataViewerMainWindow::onSignalChecked(
  QTreeWidgetItem* item,
  bool checked)
{
  const SignalId id{
    item->data(
          0,
          Qt::UserRole).toUInt()
  };

  if (auto* table = currentTable())
  {
    auto& channelIds = table->channels;

    if (checked)
    {
      if (std::find(
            channelIds.begin(),
            channelIds.end(),
            id) == channelIds.end())
      {
        channelIds.push_back(id);
      }
    }
    else
    {
      std::erase(channelIds, id);
    }

    updateTableChannels(table);
    return;
  }

  if (checked)
  {
    if (m_chartView->addSignal(
          id,
          item->text(0),
          ChartPalette::signalColor(id.value)))
    {
      m_chartSignals.push_back(id);
    }
  }
  else
  {
    m_chartView->removeSignal(id);
    std::erase(m_chartSignals, id);
  }
}

bool DataViewerMainWindow::isSignalSelected(
  SignalId id) const
{
  const std::vector<SignalId>* signalIds =
    &m_chartSignals;

  QWidget* current =
    m_tabs->currentWidget();

  for (const auto& table : m_tables)
  {
    if (table->widget == current)
    {
      signalIds = &table->channels;
      break;
    }
  }

  return std::find(
           signalIds->begin(),
           signalIds->end(),
           id) != signalIds->end();
}

void DataViewerMainWindow::updateTableChannels(
  TableContext* table)
{
  if (!table || !table->model)
    return;

  std::vector<ArchiveTableChannel> channels;
  channels.reserve(table->channels.size());

  for (const SignalId id : table->channels)
  {
    QTreeWidgetItemIterator it(m_signalTree);

    while (*it)
    {
      auto* item = *it;
      ++it;

      if (!(item->flags() &
            Qt::ItemIsUserCheckable))
      {
        continue;
      }

      const SignalId itemId{
        item->data(0, Qt::UserRole).toUInt()
      };

      if (itemId != id)
        continue;

      channels.push_back({
        id,
        item->text(0)
      });

      break;
    }
  }

  table->model->setChannels(
    std::move(channels));
}

DataViewerMainWindow::TableContext*
DataViewerMainWindow::currentTable()
{
  QWidget* current =
    m_tabs->currentWidget();

  for (auto& table : m_tables)
  {
    if (table->widget == current)
      return table.get();
  }

  return nullptr;
}

}