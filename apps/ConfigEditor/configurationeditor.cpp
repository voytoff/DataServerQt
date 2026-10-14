#include "configurationeditor.h"
#include "configurationpropertiesdelegate.h"
#include "configurationsignalpropertiesdelegate.h"
#include "dlg/configurationsignaldialog.h"

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
#include <QString>
#include <QToolBar>
#include <QAction>
#include <QStyle>

namespace qds
{

ConfigurationEditor::ConfigurationEditor(
  const QSqlDatabase& database,
  QWidget* parent)
  : QWidget(parent)
  , m_repository(database)
{
  m_configurationsView =
    new BaseTableView(0, this);

  m_configurationsView->horizontalHeader()->setSectionResizeMode(
    QHeaderView::ResizeToContents);

  m_configurationsView->horizontalHeader()->setStretchLastSection(
    true);

  m_configurationTree =
    new QTreeView(this);

  m_configurationTree->header()->hide();

  m_treeModel =
    new ConfigurationTreeModel(
      m_repository,
      this);

  m_tagsView =
    new BaseTableView(150, this);

  m_propertiesModel =
    new ConfigurationPropertiesModel(
      m_treeModel, this);

  m_tagsView->setModel(
    m_propertiesModel);

  m_tagsView->setItemDelegateForColumn(
    1,
    new ConfigurationPropertiesDelegate(
      m_tagsView));


  m_signalsView =
    new BaseTableView(150, this);

  m_signalModel =
    new ConfigurationSignalModel(
      m_repository, this);

  m_signalsView->setModel(
    m_signalModel);

  m_signalsView->setEditTriggers(
    QAbstractItemView::NoEditTriggers);

  m_signalsView->verticalHeader()->hide();


  m_signalPropertiesView =
    new BaseTableView(150, this);


  m_signalPropertiesModel =
    new ConfigurationSignalPropertiesModel(
      m_signalModel,
      this);

  std::vector<SignalTypeInfo> signalTypes;

  if (!m_repository.signalTypes(signalTypes))
  {
    QMessageBox::warning(
      this,
      tr("Ошибка загрузки сигналов"),
      m_repository.lastError().text());

    return;
  }

  SensorRepository sensors;

  if (!m_repository.loadSensors(sensors))
    return;

  auto loaded = sensors.sensors();

  std::sort(
    loaded.begin(),
    loaded.end(),
    [](const Sensor& a, const Sensor& b)
    {
      return a.name < b.name;
    });


  m_signalPropertiesView->setModel(
    m_signalPropertiesModel);

  m_signalPropertiesView->setEditTriggers(
    QAbstractItemView::DoubleClicked |
    QAbstractItemView::EditKeyPressed);

  m_signalPropertiesDelegate =
    new ConfigurationSignalPropertiesDelegate(
      m_signalPropertiesView);

  m_signalPropertiesModel->setSignalTypes(signalTypes);
  m_signalPropertiesModel->setSensors(loaded);
  m_signalPropertiesDelegate->setSignalTypes(std::move(signalTypes));
  m_signalPropertiesDelegate->setSensors(std::move(loaded));

  m_signalPropertiesView->setItemDelegateForColumn(1, m_signalPropertiesDelegate);

  createToolbar();

  auto* right =
    new QSplitter(Qt::Vertical, this);

  right->addWidget(m_configurationTree);
  right->addWidget(m_tagsView);

  right->setStretchFactor(0, 1);
  right->setStretchFactor(1, 1);


  m_tabs = new QTabWidget(this);

  m_tabs->addTab(
    right,
    tr("Модули"));

  auto* signalsSplitter =
    new QSplitter(Qt::Vertical, this);

  signalsSplitter->addWidget(m_signalsView);
  signalsSplitter->addWidget(m_signalPropertiesView);

  signalsSplitter->setStretchFactor(0, 1);
  signalsSplitter->setStretchFactor(1, 1);

  m_tabs->addTab(
    signalsSplitter,
    tr("Сигналы"));

  auto* splitter =
    new QSplitter(Qt::Horizontal, this);

  splitter->addWidget(m_configurationsView);
  splitter->addWidget(m_tabs);

  splitter->setStretchFactor(0, 0);
  splitter->setStretchFactor(1, 1);

  auto* layout = new QVBoxLayout(this);

  layout->addWidget(m_toolBar);
  layout->addWidget(splitter);

  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

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
    QString message = QString("Возникла ошибка при выборе конфигурации.\n%1")
      .arg(m_configurations->lastError().text());
    emit statusMessage(message);
  }

  m_configurationsView->setModel(
    m_configurations);

  m_configurationsView->setColumnHidden(
    0,
    true);

  m_configurationTree->setModel(
    m_treeModel);

  // изменение ыделения в дереве объектов
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

  // Изменение текущей конфигурации. TODO = Я бы сделал этот элемент небольшим
  // глобальным QComboBox-ом прям в QToolbar-е...
  connect(
    m_configurationsView->selectionModel(),
    &QItemSelectionModel::currentRowChanged,
    this,
    [this, idColumn](
      const QModelIndex& current,
      const QModelIndex&)
    {
      if (!current.isValid())
      {
        m_propertiesModel->setItem({});

        m_signalModel->clear();

        m_signalPropertiesModel->setTagSources({});
        m_signalPropertiesDelegate->setTagSources({});

        return;
      }

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

      std::vector<TagSourceInfo> tagSources;

      if (!m_repository.tagSources(configuration, tagSources))
      {
        QMessageBox::warning(
          this,
          tr("Ошибка загрузки источников"),
          m_repository.lastError().text());

        return;
      }

      m_signalPropertiesModel->setTagSources(tagSources);
      m_signalPropertiesDelegate->setTagSources(std::move(tagSources));

      emit statusMessage("Конфигурация загружена");
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

  // обрабатываем событике изменения сигнала
  connect(
    m_signalsView->selectionModel(),
    &QItemSelectionModel::currentChanged,
    this,
    [this](const QModelIndex& current,
           const QModelIndex&)
    {
      m_signalPropertiesModel->setItem(current);

      m_removeAction->setEnabled(
        m_tabs->currentIndex() == 1 &&
        current.isValid());
    });

  // регистрируем обработку и отображение ошибок
  // ошибки в дереве объектов крейта
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

  // ошибки изменения сигналов
  connect(
    m_signalModel,
    &ConfigurationSignalModel::operationFailed,
    this,
    [this](const QString& message)
    {
      QMessageBox::warning(
        this,
        tr("Ошибка изменения конфигурации"),
        message);
    });

  // ошибки изменения сигнала
  connect(
    m_signalPropertiesModel,
    &ConfigurationSignalPropertiesModel::operationFailed,
    this,
    [this](const QString& message)
    {
      QMessageBox::warning(
        this,
        tr("Ошибка изменения конфигурации"),
        message);
    });

  // обновляем список tagSources сразу после изменения состава каналов.
  connect(
    m_treeModel,
    &ConfigurationTreeModel::tagSourcesChanged,
    this,
    [this]()
    {
      const QModelIndex current =
        m_configurationsView->currentIndex();

      if (!current.isValid())
        return;

      const int idColumn =
        m_configurations->record().indexOf("id");

      const ConfigurationId configuration{
        m_configurations->data(
                          m_configurations->index(
                            current.row(),
                            idColumn)).toUInt()
      };

      std::vector<TagSourceInfo> tagSources;

      if (!m_repository.tagSources(configuration, tagSources))
      {
        QMessageBox::warning(
          this,
          tr("Ошибка загрузки источников"),
          m_repository.lastError().text());

        return;
      }

      m_signalPropertiesModel->setTagSources(tagSources);
      m_signalPropertiesDelegate->setTagSources(std::move(tagSources));
    });

  // подключаем обновление действий при выборе вкладки.
  connect(
    m_tabs,
    &QTabWidget::currentChanged,
    this,
    [this]()
    {
      updateActions();
    });
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

void ConfigurationEditor::createToolbar()
{
  m_toolBar = new QToolBar(this);

  m_toolBar->setIconSize(QSize(16, 16));
  m_toolBar->setMovable(false);
  m_toolBar->setFloatable(false);
  m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  m_addAction = m_toolBar->addAction(
    style()->standardIcon(QStyle::SP_FileIcon),
    tr("Добавить"));

  m_removeAction = m_toolBar->addAction(
    style()->standardIcon(QStyle::SP_TrashIcon),
    tr("Удалить"));

  m_toolBar->addSeparator();

  m_testAction = m_toolBar->addAction(
    tr("Тест"));

  m_addAction->setEnabled(false);
  m_removeAction->setEnabled(false);
  m_testAction->setEnabled(false);

  connect(
    m_addAction,
    &QAction::triggered,
    this,
    [this]()
    {
      if (m_tabs->currentIndex() != 1 ||
          !m_configurationsView->currentIndex().isValid())
      {
        return;
      }

      ConfigurationSignalDialog dialog(this);

      if (dialog.exec() != QDialog::Accepted)
        return;

      const QString name = dialog.signalName();
      const SignalKind kind = dialog.signalKind();

      if (m_signalModel->createSignal(name, kind))
        m_signalsView->selectRow(m_signalModel->rowCount() - 1);
    });

  connect(
    m_removeAction,
    &QAction::triggered,
    this,
    [this]()
    {
      const QModelIndex current =
        m_signalsView->currentIndex();

      const auto* signal =
        m_signalModel->signal(current);

      if (!signal)
        return;

      const QString name =
        QString::fromStdString(signal->name);

      const auto answer = QMessageBox::question(
        this,
        tr("Удаление сигнала"),
        tr("Удалить сигнал «%1»?\n"
           "Это действие нельзя отменить.")
          .arg(name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

      if (answer != QMessageBox::Yes)
        return;

      (void)m_signalModel->removeSignal(
        current);
    });
}

void ConfigurationEditor::updateActions()
{
  const bool hasConfiguration =
    m_configurationsView->currentIndex().isValid();

  const bool signalsTab =
    m_tabs->currentIndex() == 1;

  const bool hasSignal =
    m_signalsView->currentIndex().isValid();

  m_addAction->setEnabled(
    hasConfiguration && signalsTab);

  m_removeAction->setEnabled(
    hasConfiguration && signalsTab && hasSignal);

  // Включим после реализации проверки конфигурации.
  m_testAction->setEnabled(false);
}

}