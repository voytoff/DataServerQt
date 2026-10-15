#include "configurationeditor.h"
#include "delegate/basecomboboxdelegate.h"
#include "delegate/configurationpropertiesdelegate.h"
#include "delegate/configurationsignalpropertiesdelegate.h"
#include "dlg/configurationsignaldialog.h"
#include "sensordialog.h"

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
#include <QInputDialog>
#include <QLineEdit>

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

  /// типы датчиков ========================================
  m_sensorTypesView =
    new BaseTableView(150, this);

  m_sensorTypeModel =
    new SensorTypeModel(
      m_repository, this);

  if (!m_sensorTypeModel->load())
  {
    const auto& error = m_repository.lastError();

    QMessageBox::warning(
      this,
      tr("Ошибка загрузки типов датчиков"),
      error.isValid()
        ? error.text()
        : tr("Не удалось загрузить типы датчиков"));
  }


  m_sensorTypesView->setModel(
    m_sensorTypeModel);

  m_sensorTypesView->setColumnHidden(
    0,
    true);

  m_sensorTypesView->setItemDelegateForColumn(
    3,
    new BaseComboBoxDelegate(
      {
        {
          tr("Табличная"),
          static_cast<int>(CalibrationKind::Table)
        },
        {
          tr("Полиномиальная"),
          static_cast<int>(CalibrationKind::Polynomial)
        }
      },
      m_sensorTypesView));

  m_sensorTypesView->setEditTriggers(
    QAbstractItemView::DoubleClicked |
    QAbstractItemView::EditKeyPressed);


  m_tabs->addTab(
    m_sensorTypesView,
    tr("Типы датчиков"));


  /// датчики ======================================
  m_sensorsView =
    new BaseTableView(150, this);

  m_sensorModel =
    new SensorModel(
      m_repository, this);

  if (!m_sensorModel->load())
  {
    const auto& error = m_repository.lastError();

    QMessageBox::warning(
      this,
      tr("Ошибка загрузки датчиков"),
      error.isValid()
        ? error.text()
        : tr("Не удалось загрузить датчики"));
  }

  m_sensorsView->setModel(
    m_sensorModel);

  m_sensorsView->setColumnHidden(
    0,
    true);

  // Тип датчика.
  m_sensorTypeDelegate =
    new BaseComboBoxDelegate(
      {},
      m_sensorsView);

  m_sensorsView->setItemDelegateForColumn(
    2,
    m_sensorTypeDelegate);

  // Режим тарировки.
  m_sensorsView->setItemDelegateForColumn(
    4,
    new BaseComboBoxDelegate(
      {
        {
          tr("Собственная"),
          static_cast<int>(SensorCalibrationMode::Own)
        },
        {
          tr("По типу"),
          static_cast<int>(SensorCalibrationMode::ByType)
        }
      },
      m_sensorsView));

  // Вид тарировки.
  m_sensorsView->setItemDelegateForColumn(
    5,
    new BaseComboBoxDelegate(
      {
        {
          tr("Табличная"),
          static_cast<int>(CalibrationKind::Table)
        },
        {
          tr("Полиномиальная"),
          static_cast<int>(CalibrationKind::Polynomial)
        }
      },
      m_sensorsView));

  m_sensorsView->setEditTriggers(
    QAbstractItemView::DoubleClicked |
    QAbstractItemView::EditKeyPressed);


  /// свойства датчиков ===========================================
  m_sensorPropertiesModel =
    new SensorPropertiesModel(
      *m_sensorModel,
      m_repository,
      this);

  m_sensorPropertiesView =
    new BaseTableView(150, this);

  m_sensorPropertiesView->setModel(
    m_sensorPropertiesModel);

  m_sensorPropertiesView->setEditTriggers(
    QAbstractItemView::DoubleClicked |
    QAbstractItemView::EditKeyPressed);


  m_sensorsSplitter =
    new QSplitter(Qt::Vertical, this);

  m_sensorsSplitter->addWidget(
    m_sensorsView);

  m_sensorsSplitter->addWidget(
    m_sensorPropertiesView);

  m_sensorsSplitter->setStretchFactor(0, 3);
  m_sensorsSplitter->setStretchFactor(1, 1);

  m_tabs->addTab(
    m_sensorsSplitter,
    tr("Датчики"));


  /// splitter
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


  // изменение выделения в дереве объектов
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

  // подключаем обработку ошибок после создания модели типа датчика:
  connect(
    m_sensorTypeModel,
    &SensorTypeModel::operationFailed,
    this,
    [this](const QString& message)
    {
      QMessageBox::warning(
        this,
        tr("Ошибка изменения типа датчика"),
        message);
    });

  // подключаем изменение выделения в таблице типов датчика:
  connect(
    m_sensorTypesView->selectionModel(),
    &QItemSelectionModel::currentRowChanged,
    this,
    [this]()
    {
      updateActions();
    });

  // подключаем обработку ошибок после создания модели датчика:
  connect(
    m_sensorModel,
    &SensorModel::operationFailed,
    this,
    [this](const QString& message)
    {
      QMessageBox::warning(
        this,
        tr("Ошибка изменения датчика"),
        message);
    });

  // свяжем модель SensorTypeModel с делегатом списка типов датчиков
  connect(
    m_sensorTypeModel,
    &SensorTypeModel::sensorTypesChanged,
    this,
    &ConfigurationEditor::updateSensorTypes);

  // добавим обработку обновления при выборе строки датчика
  connect(
    m_sensorsView->selectionModel(),
    &QItemSelectionModel::currentRowChanged,
    this,
    [this]()
    {
      updateActions();
    });

  // связываем выделение датчика со свойствами
  connect(
    m_sensorsView->selectionModel(),
    &QItemSelectionModel::currentRowChanged,
    m_sensorPropertiesModel,
    [this](const QModelIndex& current)
    {
      m_sensorPropertiesModel->setSensor(current);
    });

  // обработка ошибок работы с датчиками
  connect(
    m_sensorPropertiesModel,
    &SensorPropertiesModel::operationFailed,
    this,
    [this](const QString& message)
    {
      QMessageBox::warning(
        this,
        tr("Ошибка изменения датчика"),
        message);
    });

  // после удаления выбранного датчика нужно гарантировать,
  // что панель свойств не продолжит отображать устаревшее состояние.
  connect(
    m_sensorModel,
    &QAbstractItemModel::rowsRemoved,
    this,
    [this]()
    {
      m_sensorPropertiesModel->setSensor(
        m_sensorsView->currentIndex());
    });

  // если выбранный датчик изменён через основную таблицу,
  // нижняя панель должна обновляться без повторного выбора строки.
  connect(
    m_sensorModel,
    &QAbstractItemModel::dataChanged,
    this,
    [this](const QModelIndex& topLeft,
           const QModelIndex& bottomRight)
    {
      const QModelIndex current =
        m_sensorsView->currentIndex();

      if (!current.isValid() ||
          current.row() < topLeft.row() ||
          current.row() > bottomRight.row())
        return;

      m_sensorPropertiesModel->setSensor(current);
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
      if (m_tabs->currentWidget() == m_sensorTypesView)
      {
        bool ok = false;

        const QString name =
          QInputDialog::getText(
            this,
            tr("Новый тип датчика"),
            tr("Название:"),
            QLineEdit::Normal,
            {},
            &ok).trimmed();

        if (!ok || name.isEmpty())
          return;

        if (m_sensorTypeModel->createSensorType(name))
        {
          for (int row = 0;
               row < m_sensorTypeModel->rowCount();
               ++row)
          {
            const QModelIndex index =
              m_sensorTypeModel->index(row, 1);

            if (index.data().toString() == name)
            {
              m_sensorTypesView->selectRow(row);
              m_sensorTypesView->scrollTo(index);
              break;
            }
          }
        }

        return;
      }
      else if (m_tabs->currentWidget() == m_sensorsSplitter)
      {
        SensorDialog dialog(
          m_sensorTypeModel->sensorTypes(),
          this);

        if (dialog.exec() != QDialog::Accepted)
          return;

        SensorId createdId;

        if (m_sensorModel->createSensor(
              dialog.sensorName(),
              dialog.sensorTypeId(),
              createdId))
        {
          const QModelIndex index =
            m_sensorModel->indexOf(createdId);

          if (index.isValid())
          {
            m_sensorsView->setCurrentIndex(index);
            m_sensorsView->selectRow(index.row());
            m_sensorsView->scrollTo(index);
          }
        }

        return;
      }
      else if (m_tabs->currentIndex() != 1 ||
          !m_configurationsView->currentIndex().isValid())
      {
        return;
      }
      else if (m_tabs->currentIndex() == 1)
      {
        ConfigurationSignalDialog dialog(this);

        if (dialog.exec() != QDialog::Accepted)
          return;

        const QString name = dialog.signalName();
        const SignalKind kind = dialog.signalKind();

        if (m_signalModel->createSignal(name, kind))
          m_signalsView->selectRow(m_signalModel->rowCount() - 1);
      }
    });

  connect(
    m_removeAction,
    &QAction::triggered,
    this,
    [this]()
    {
      if (m_tabs->currentWidget() == m_sensorTypesView)
      {
        const QModelIndex current =
          m_sensorTypesView->currentIndex();

        const auto* type =
          m_sensorTypeModel->sensorType(current);

        if (!type)
          return;

        const QString name =
          QString::fromStdString(type->name);

        const auto answer = QMessageBox::question(
          this,
          tr("Удаление типа датчика"),
          tr("Удалить тип датчика «%1»?\n"
             "Это действие нельзя отменить.")
            .arg(name),
          QMessageBox::Yes | QMessageBox::No,
          QMessageBox::No);

        if (answer == QMessageBox::Yes)
          m_sensorTypeModel->removeSensorType(current);

        return;
      }
      else if (m_tabs->currentWidget() == m_sensorsSplitter)
      {
        const QModelIndex current =
          m_sensorsView->currentIndex();

        const auto* sensor =
          m_sensorModel->sensor(current);

        if (!sensor)
          return;

        const QString name =
          QString::fromStdString(sensor->name);

        const auto answer = QMessageBox::question(
          this,
          tr("Удаление датчика"),
          tr("Удалить датчик «%1»?\n"
             "Это действие нельзя отменить.")
            .arg(name),
          QMessageBox::Yes | QMessageBox::No,
          QMessageBox::No);

        if (answer == QMessageBox::Yes)
          m_sensorModel->removeSensor(current);

        return;
      }
      else if (m_tabs->currentIndex() == 1)
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
      }
    });
}

void ConfigurationEditor::updateActions()
{
  const bool hasConfiguration =
    m_configurationsView->currentIndex().isValid();

  const bool signalsTab =
    m_tabs->currentIndex() == 1;
  //const bool signalsTab =
  //  m_tabs->currentWidget() == m_signalsView->parentWidget();

  const bool sensorTypesTab =
    m_tabs->currentWidget() == m_sensorTypesView;

  const bool sensorsTab =
    m_tabs->currentWidget() == m_sensorsSplitter;

  const bool hasSignal =
    m_signalsView->currentIndex().isValid();

  const bool hasSensorType =
    m_sensorTypesView->currentIndex().isValid();

  const bool hasSensor =
    m_sensorsView->currentIndex().isValid();

  m_addAction->setEnabled(
    (hasConfiguration && signalsTab) ||
    sensorTypesTab ||
    sensorsTab);

  m_removeAction->setEnabled(
    (hasConfiguration && signalsTab && hasSignal) ||
    (sensorTypesTab && hasSensorType) ||
    (sensorsTab && hasSensor));


  m_testAction->setEnabled(false);
}

void ConfigurationEditor::updateSensorTypes()
{
  const auto& types =
    m_sensorTypeModel->sensorTypes();

  m_sensorModel->setSensorTypes(types);

  std::vector<BaseComboBoxDelegate::Item> items;
  items.reserve(types.size());

  for (const auto& type : types)
  {
    items.push_back({
      QString::fromStdString(type.name),
      type.id.value
    });
  }

  m_sensorTypeDelegate->setItems(
    std::move(items));
}

}