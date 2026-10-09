#include "configurationeditor.h"
#include "configurationpropertiesdelegate.h"

#include <QSplitter>
#include <QTableView>
#include <QVBoxLayout>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QHeaderView>
#include <QDebug>
#include <QTreeView>
#include <QMessageBox>
#include <QTabWidget>

namespace qds
{

ConfigurationEditor::ConfigurationEditor(
  const QSqlDatabase& database,
  QWidget* parent)
  : QWidget(parent)
  , m_repository(database)
{
  m_configurationsView =
    new QTableView(this);

  m_configurationTree =
    new QTreeView(this);

  m_configurationTree->header()->hide();

  m_treeModel =
    new ConfigurationTreeModel(
      m_repository,
      this);

  connect(
    m_treeModel,
    &ConfigurationTreeModel::operationFailed,
    this,
    [this](const QString& message)
    {
      QMessageBox::warning(
        this,
        tr("Ошибка изменения конфигурации"),
        message);
    });

  m_tagsView =
    new QTableView(this);

  m_propertiesModel =
    new ConfigurationPropertiesModel(
      m_treeModel, this);

  m_tagsView->setModel(
    m_propertiesModel);

  //m_tagsView->horizontalHeader()
  //  ->setSectionResizeMode(
  //    QHeaderView::Stretch);

  m_tagsView->setColumnWidth(0, 150);
  m_tagsView->setColumnWidth(1, 150);

  m_tagsView->setItemDelegateForColumn(
    1,
    new ConfigurationPropertiesDelegate(
      m_tagsView));


  m_signalsView =
    new QTableView(this);

  m_signalModel =
    new ConfigurationSignalModel(
      m_repository, this);

  m_signalsView->setModel(
    m_signalModel);

  m_signalsView->setEditTriggers(
    QAbstractItemView::NoEditTriggers);

  //m_signalsView->horizontalHeader()
  //  ->setSectionResizeMode(
  //    QHeaderView::Stretch);
  m_signalsView->setColumnWidth(0, 150);
  m_signalsView->setColumnWidth(1, 150);
  m_signalsView->setColumnWidth(2, 150);

  m_signalsView->verticalHeader()->hide();

  for (QTableView* view :
       {m_configurationsView,
        m_signalsView,
        m_tagsView})
  {
    view->verticalHeader()->setSectionResizeMode(
      QHeaderView::Fixed);

    view->verticalHeader()
      ->setDefaultSectionSize(24);

    view->setSelectionBehavior(
      QAbstractItemView::SelectRows);

    view->setSelectionMode(
      QAbstractItemView::SingleSelection);

    //view->setAlternatingRowColors(true);
  }

  auto* right =
    new QSplitter(Qt::Vertical, this);

  right->addWidget(m_configurationTree);
  right->addWidget(m_tagsView);

  right->setStretchFactor(0, 1);
  right->setStretchFactor(1, 1);


  auto* tabs = new QTabWidget(this);

  tabs->addTab(
    right,
    tr("Модули"));

  tabs->addTab(
    m_signalsView,
    tr("Сигналы"));


  auto* splitter =
    new QSplitter(Qt::Horizontal, this);

  splitter->addWidget(m_configurationsView);
  splitter->addWidget(tabs);

  splitter->setStretchFactor(0, 0);
  splitter->setStretchFactor(1, 1);

  auto* layout =
    new QVBoxLayout(this);

  layout->addWidget(splitter);
  layout->setContentsMargins(0, 0, 0, 0);

  m_configurations =
    new QSqlTableModel(
      this,
      database);

  m_configurations->setTable(
    "configuration");

  m_configurations->setEditStrategy(
    QSqlTableModel::OnManualSubmit);

  m_configurations->setHeaderData(
    0,
    Qt::Horizontal,
    tr("ID"));

  m_configurations->setHeaderData(
    1,
    Qt::Horizontal,
    tr("Название"));

  m_configurations->setHeaderData(
    2,
    Qt::Horizontal,
    tr("Описание"));

  m_configurations->setHeaderData(
    3,
    Qt::Horizontal,
    tr("UDP порт"));

  if (!m_configurations->select())
  {
    qWarning()
    << "Failed to select configuration:"
    << m_configurations->lastError().text();
  }


  m_configurationsView->setModel(
    m_configurations);

  m_configurationsView->setColumnHidden(
    0,
    true);

  m_configurationsView
    ->horizontalHeader()
    ->setStretchLastSection(true);

  m_configurationTree->setModel(
    m_treeModel);

  connect(
    m_treeModel,
    &QAbstractItemModel::dataChanged,
    this,
    [this](const QModelIndex& topLeft,
           const QModelIndex&,
           const QList<int>&)
    {
      const auto* data =
        m_treeModel->treeItemData(topLeft);

      if (data && std::holds_alternative<ModuleItemData>(*data))
        updateChannelVisibility(topLeft);
    });

  const int idColumn =
    m_configurations
      ->record()
      .indexOf("id");

  connect(
    m_configurationsView->selectionModel(),
    &QItemSelectionModel::currentRowChanged,
    this,
    [this, idColumn](
      const QModelIndex& current,
      const QModelIndex&)
    {
      if (!current.isValid())
        return;

      const auto configurationId =
        m_configurations
          ->data(
            m_configurations->index(
              current.row(),
              idColumn))
          .toUInt();

      const ConfigurationId configuration{
        configurationId
      };

      m_propertiesModel->setItem({});

      if (!m_treeModel->load(configuration))
      {
        const auto& error = m_repository.lastError();
        const QString message = error.isValid() ? error.text() : "Failed to load configuration tree";

        QMessageBox::warning(
          this,
          tr("Ошибка загрузки конфигурации"),
          message);

        return;
      }

      if (!m_signalModel->load(configuration))
      {
        const auto& error =
          m_repository.lastError();

        const QString message =
          error.isValid()
            ? error.text()
            : tr("Не удалось загрузить сигналы");

        QMessageBox::warning(
          this,
          tr("Ошибка загрузки сигналов"),
          message);

        return;
      }

      const int crateCount = m_treeModel->rowCount();
      // Обновление после загрузки конфигурации
      for (int crateRow = 0; crateRow < crateCount; ++crateRow)
      {
        const QModelIndex crateIndex =
          m_treeModel->index(crateRow, 0);

        const int moduleCount =
          m_treeModel->rowCount(crateIndex);

        for (int moduleRow = 0; moduleRow < moduleCount; ++moduleRow)
        {
          const QModelIndex moduleIndex =
            m_treeModel->index(moduleRow, 0, crateIndex);

          updateChannelVisibility(moduleIndex);
        }
      }
    });

  connect(
    m_configurationTree->selectionModel(),
    &QItemSelectionModel::currentChanged,
    this,
    [this](const QModelIndex& current,
           const QModelIndex&)
    {
      m_propertiesModel->setItem(current);
    });

  if (m_configurations->rowCount() > 0)
    m_configurationsView->selectRow(0);
}

void ConfigurationEditor::updateChannelVisibility(
  const QModelIndex& moduleIndex)
{
  const auto* data =
    m_treeModel->treeItemData(moduleIndex);

  if (!data)
    return;

  const auto* module =
    std::get_if<ModuleItemData>(data);

  if (!module || module->type != ModuleType::LTR11)
    return;

  const int mode =
    module->settings.value("mode").toInt(1);

  const int maxChannels = mode == 0 ? 16 : 32;

  const int count =
    m_treeModel->rowCount(moduleIndex);

  for (int row = 0; row < count; ++row)
  {
    m_configurationTree->setRowHidden(
      row,
      moduleIndex,
      row >= maxChannels);
  }
}

}