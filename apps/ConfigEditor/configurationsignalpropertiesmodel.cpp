#include "configurationsignalpropertiesmodel.h"

#include <QString>

namespace qds
{

ConfigurationSignalPropertiesModel::
  ConfigurationSignalPropertiesModel(
    ConfigurationSignalModel* signalModel,
    QObject* parent)
  : QAbstractTableModel(parent)
  , m_signalModel(signalModel)
{
  connect(
    m_signalModel,
    &QAbstractItemModel::modelAboutToBeReset,
    this,
    [this]()
    {
      setItem({});
    });
}

void ConfigurationSignalPropertiesModel::setItem(
  const QModelIndex& index)
{
  std::vector<Property> properties;

  if (const auto* signal =
      m_signalModel->signal(index))
  {
    properties.push_back({
      tr("Название"),
      QString::fromStdString(signal->name)
    });

    properties.push_back({
      tr("Вид"),
      signal->kind == SignalKind::Raw
        ? QStringLiteral("Raw")
        : QStringLiteral("Calculated")
    });

    if (signal->kind == SignalKind::Raw)
    {
      properties.push_back({
        tr("Источник (TagId)"),
        signal->source.tag.value
      });
    }
    else if (signal->kind == SignalKind::Calculated)
    {
      properties.push_back({
        tr("Формула"),
        QString::fromStdString(signal->formula)
      });
    }

    properties.push_back({
      tr("Тип сигнала (ID)"),
      signal->signalType.value
    });

    properties.push_back({
      tr("Частота архивирования"),
      static_cast<uint16_t>(signal->archiveRate)
    });

    properties.push_back({
      tr("Режим калибровки"),
      static_cast<int>(signal->calibrationMode)
    });
  }

  beginResetModel();

  m_currentItem = index;
  m_properties = std::move(properties);

  endResetModel();
}

int ConfigurationSignalPropertiesModel::rowCount(
  const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  return static_cast<int>(m_properties.size());
}

int ConfigurationSignalPropertiesModel::columnCount(
  const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 2;
}

QVariant ConfigurationSignalPropertiesModel::data(
  const QModelIndex& index,
  int role) const
{
  if (!index.isValid() ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_properties.size()) ||
      index.column() < 0 ||
      index.column() >= 2)
  {
    return {};
  }

  if (role != Qt::DisplayRole &&
      role != Qt::EditRole)
  {
    return {};
  }

  const auto& property = m_properties[index.row()];

  if (index.column() == 0)
    return role == Qt::DisplayRole
             ? property.name
             : QVariant{};

  if (role == Qt::EditRole)
    return property.value;

  if (property.name == tr("Частота архивирования"))
  {
    return QString("%1 Гц")
      .arg(property.value.toUInt());
  }
  else if (property.name == tr("Режим калибровки"))
  {
    switch (property.value.toInt())
    {
    case 0: return tr("Нет");
    case 1: return tr("По сигналу");
    case 2: return tr("По типу сигнала");
    default: return property.value;
    }
  }

  return property.value;
}

QVariant ConfigurationSignalPropertiesModel::headerData(
  int section,
  Qt::Orientation orientation,
  int role) const
{
  if (orientation != Qt::Horizontal ||
      role != Qt::DisplayRole)
  {
    return {};
  }

  switch (section)
  {
  case 0:
    return tr("Параметр");

  case 1:
    return tr("Значение");
  }

  return {};
}

bool ConfigurationSignalPropertiesModel::setData(
  const QModelIndex& index,
  const QVariant& value,
  int role)
{
  if (index.model() != this)
    return false;

  if (role != Qt::EditRole ||
      !index.isValid() ||
      index.column() != 1 ||
      !m_currentItem.isValid() ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_properties.size()))
  {
    return false;
  }

  auto& property = m_properties.at(index.row());

  if (property.name == tr("Название"))
  {
    const QString name = value.toString().trimmed();

    if (name.isEmpty() || name.size() > 128)
    {
      emit operationFailed(
        tr("Недопустимое имя сигнала: %1.\n"
           "Имя должно быть непустым и иметь длину не более 128 символов.")
          .arg(value.toString()));

      return false;
    }

    if (!m_signalModel->updateSignalName(
          m_currentItem, name))
    {
      return false;
    }

    property.value = name;
  }
  else if (property.name == tr("Частота архивирования"))
  {
    bool ok = false;
    const uint32_t frequency = value.toUInt(&ok);

    if (!ok ||
        !isValidArchiveRate(static_cast<ArchiveRate>(frequency)))
    {
      emit operationFailed(
        tr("Недопустимая частота архивирования: %1 Гц.\n"
           "Допустимые значения: 1, 10, 100, 1000 Гц.")
          .arg(value.toString()));

      return false;
    }

    const auto rate = static_cast<ArchiveRate>(frequency);

    if (!m_signalModel->updateSignalArchiveRate(
          m_currentItem, rate))
    {
      return false;
    }

    property.value = frequency;
  }
  else
  {
    return false;
  }

  emit dataChanged(
    index,
    index,
    {Qt::DisplayRole, Qt::EditRole});

  return true;
}

Qt::ItemFlags ConfigurationSignalPropertiesModel::flags(
  const QModelIndex& index) const
{
  auto result = QAbstractTableModel::flags(index);

  if (!index.isValid() ||
      index.column() != 1 ||
      !m_currentItem.isValid() ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_properties.size()))
  {
    return result;
  }

  const auto& propertyName = m_properties[index.row()].name;

  if (propertyName == tr("Название") ||
      propertyName == tr("Частота архивирования"))
  {
    result |= Qt::ItemIsEditable;
  }

  return result;
}

}