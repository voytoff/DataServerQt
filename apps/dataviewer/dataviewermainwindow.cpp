#include "dataviewermainwindow.h"
#include "applicationsettingsdialog.h"
#include "rowheightdelegate.h"
#include "viewersettingsloader.h"
#include "windowsettings.h"
#include "colorschemesettings.h"
#include "applicationsettingsloader.h"

#include "archivechartview.h"
#include "archivetablemodel.h"
#include "archivetableview.h"
#include "colorscheme.h"
#include "themedsvgicon.h"
#include "chartpalette.h"

#include <QCloseEvent>
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
#include <QSignalBlocker>
#include <QGraphicsLayout>

#include <algorithm>

namespace qds
{

DataViewerMainWindow::DataViewerMainWindow(
  QString configFile,
  QWidget* parent)
  : QMainWindow(parent)
  , m_configFile(configFile)
{
  resize(1200, 800);

  createActions();
  createUi();

  updateChartSettings();
  restoreWindowSettings();

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

  m_settingsAction = m_toolBar->addAction(
    themedSvgIcon(":/images/settings.svg"),
    tr("Настройки"));

  m_settingsAction->setToolTip(
    tr("Настройки приложения"));

  m_toolBar->addSeparator();

  m_themeAction = m_toolBar->addAction(
    themedSvgIcon(":/images/theme.svg"),
    tr("Сменить тему"));

  m_themeAction->setToolTip(
    tr("Переключить светлую/тёмную тему"));

  m_toolBar->addSeparator();

  m_frequencyCombo = new QComboBox(this);

  for (const uint32_t frequency :
       {1u, 10u, 100u, 1000u})
  {
    m_frequencyCombo->addItem(
      tr("%1 Гц").arg(frequency),
      frequency);
  }

  m_frequencyCombo->setCurrentIndex(
    m_frequencyCombo->findData(100u));

  m_frequencyCombo->setToolTip(
    tr("Частота активной таблицы"));

  m_toolBar->addWidget(m_frequencyCombo);

  connect(
    m_frequencyCombo,
    &QComboBox::currentIndexChanged,
    this,
    [this](int)
    {
      auto* table = currentTable();

      if (!table || !table->view)
        return;

      const uint32_t frequency =
        m_frequencyCombo->currentData().toUInt();

      table->frequency = frequency;
      table->view->setFrequency(frequency);
    });

  connect(
    m_openArchiveAction,
    &QAction::triggered,
    this,
    [this]()
    {
      const QString directory =
        QFileDialog::getExistingDirectory(
          this,
          tr("Выберите каталог архива"),
          nullptr,
          QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog);

      if (directory.isEmpty())
        return;

      openArchive(
        std::filesystem::path(directory.toStdWString()));
    });

  connect(
    m_addTableAction,
    &QAction::triggered,
    this,
    [this]()
    {
      addTable();
    });

  connect(
    m_settingsAction,
    &QAction::triggered,
    this,
    &DataViewerMainWindow::showSettings);

  connect(
    m_themeAction,
    &QAction::triggered,
    this,
    &toggleColorScheme);
}

void DataViewerMainWindow::updateActions()
{
  const bool archiveOpen =
    m_archiveReader &&
    m_archiveReader->isOpen();

  const auto* table = currentTable();

  m_addTableAction->setEnabled(archiveOpen);

  m_frequencyCombo->setEnabled(
    archiveOpen &&
    table &&
    table->view);

  updateFrequencyControl();
}

void DataViewerMainWindow::createUi()
{
  m_splitter = new QSplitter(Qt::Horizontal, this);

  m_signalTree = new QTreeWidget(m_splitter);
  m_signalTree->setItemDelegate(
    new RowHeightDelegate(22, m_signalTree));

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
  m_tabs->setIconSize(QSize(20, 20));

  m_tabs->setTabsClosable(true);
  m_tabs->setMovable(false);

  m_chartView = new ArchiveChartView(m_tabs);

  if (m_chartView->chart()->layout())
    m_chartView->chart()->layout()->setContentsMargins(0, 0, 0, 0);

  m_tabs->addTab(
    m_chartView,
    themedSvgIcon(":/images/chart.svg"),
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
      updateActions();
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
  updateFrequencyControl();

  statusBar()->showMessage(tr("Архив не открыт"));
}

void DataViewerMainWindow::openArchive(
  const std::filesystem::path& path)
{
  m_path = path;

  saveTableSettings();

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

  Timestamp firstTimestamp;
  Timestamp lastTimestamp;

  if (m_archiveReader->timeRange(
        firstTimestamp,
        lastTimestamp))
  {
    m_chartView->setTimeOrigin(firstTimestamp);
  }

  populateSignalTree();

  restoreTableSettings();

  const auto tables =
    ViewerSettingsLoader::loadTables(
      viewerSettingsFile());

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

      const QString frequency =
        tr("%1 Гц").arg(file.frequency);

      auto* item =
        new QTreeWidgetItem(
          it->second,
          {name, frequency});

      item->setTextAlignment(
        1,
        Qt::AlignRight | Qt::AlignVCenter);

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
  }

  updateSignalTreeChecks();

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

  auto* view = new ArchiveTableView(m_tabs);

  view->setArchivePath(m_path);

  ApplicationSettings settings;
  QString error;

  if (ApplicationSettingsLoader::load(
        m_configFile,
        settings,
        error,
        SettingsSection::Table,
        true))
  {
    view->setDecimals(
      settings.table.decimals);
  }

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
    view->setTimeRange(
      firstTimestamp,
      lastTimestamp,
      referenceTimestamp,
      referenceWallTime);
  }

  context->widget = view;
  context->view = view;
  context->frequency = view->frequency();

  m_tables.push_back(
    std::move(context));

  const int index = m_tabs->addTab(
    view,
    tr("Таблица %1").arg(m_tables.size()));

  m_tabs->setCurrentIndex(index);

  updateFrequencyControl();
}

void DataViewerMainWindow::addTable(
  const ViewerTableSettings& settings)
{
  addTable();

  auto* table = currentTable();

  if (!table)
    return;

  table->view->setFrequency(
    settings.frequency);

  table->frequency =
    table->view->frequency();

  table->channels = settings.channels;

  updateTableChannels(table);
  updateSignalTreeChecks();
  updateFrequencyControl();
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
  if (!table || !table->view)
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

  table->view->setChannels(
    std::move(channels));
}

void DataViewerMainWindow::updateFrequencyControl()
{
  const auto* table = currentTable();

  if (!table || !table->view)
    return;

  const QSignalBlocker blocker(m_frequencyCombo);

  const int index =
    m_frequencyCombo->findData(
      table->view->frequency());

  if (index >= 0)
    m_frequencyCombo->setCurrentIndex(index);
}

void DataViewerMainWindow::restoreWindowSettings()
{
  WindowSettings::restoreGeometry(
    m_configFile,
    "Window/Geometry",
    this);

  WindowSettings::restoreSplitter(
    m_configFile,
    "Window/MainSplitter",
    m_splitter);
}

void DataViewerMainWindow::restoreTableSettings()
{
  if (!m_archiveReader ||
      !m_archiveReader->isOpen())
  {
    return;
  }

  const auto tables =
    ViewerSettingsLoader::loadTables(
      viewerSettingsFile());

  for (const auto& settings : tables)
  {
    addTable(settings);
  }

  m_tabs->setCurrentWidget(m_chartView);

  updateActions();
}

void DataViewerMainWindow::saveWindowSettings() const
{
  WindowSettings::saveGeometry(
    m_configFile,
    "Window/Geometry",
    this);

  WindowSettings::saveSplitter(
    m_configFile,
    "Window/MainSplitter",
    m_splitter);
}

void DataViewerMainWindow::saveTableSettings() const
{
  if (!m_archiveReader ||
      !m_archiveReader->isOpen())
  {
    return;
  }

  std::vector<ViewerTableSettings> tables;

  tables.reserve(m_tables.size());

  for (const auto& context : m_tables)
  {
    if (!context->view)
      continue;

    ViewerTableSettings table;

    table.frequency =
      context->view->frequency();

    table.channels =
      context->channels;

    tables.push_back(std::move(table));
  }

  ViewerSettingsLoader::saveTables(
    viewerSettingsFile(),
    tables);
}

void DataViewerMainWindow::showSettings()
{
  constexpr SettingsSections sections =
    SettingsSection::Table |
    SettingsSection::Chart;

  ApplicationSettings settings;
  QString error;

  if (!ApplicationSettingsLoader::load(
        m_configFile,
        settings,
        error,
        sections,
        true))
  {
    QMessageBox::warning(
      this,
      tr("Настройки приложения"),
      error);
    return;
  }

  ApplicationSettingsDialog dialog(
    settings,
    sections,
    this);

  connect(
    &dialog,
    &ApplicationSettingsDialog::settingsApplied,
    this,
    [this](SettingsSections changed)
    {
      if (!changed.testFlag(SettingsSection::Chart))
        return;

      updateChartSettings();
    });

  dialog.exec();
}

QString DataViewerMainWindow::viewerSettingsFile() const
{
  if (m_path.empty())
    return {};

  const auto path =
    m_path / "viewer.ini";

#ifdef Q_OS_WIN
  return QString::fromStdWString(path.wstring());
#else
  const auto utf8 = path.u8string();

  return QString::fromUtf8(
    reinterpret_cast<const char*>(utf8.data()),
    static_cast<qsizetype>(utf8.size()));
#endif
}

void DataViewerMainWindow::updateChartSettings()
{
  ApplicationSettings settings;
  QString error;

  if (!ApplicationSettingsLoader::load(
        m_configFile,
        settings,
        error,
        SettingsSection::Chart,
        true))
  {
    return;
  }

  m_chartView->setChartSettings(
    settings.chart);
}

void DataViewerMainWindow::closeEvent(
  QCloseEvent* event)
{
  saveWindowSettings();
  saveTableSettings();

  ColorSchemeSettings::save(
    m_configFile);

  QMainWindow::closeEvent(event);
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