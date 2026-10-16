#include "sensortypemodel.h"

#include "db/configurationrepository.h"
#include "db/sensorrepository.h"

#include <algorithm>

namespace qds
{

SensorTypeModel::SensorTypeModel(
  ConfigurationRepository& repository,
  QObject* parent)
  : QAbstractTableModel(parent)
  , m_repository(repository)
{
}

bool SensorTypeModel::load()
{
  SensorRepository repository;

  if (!m_repository.loadSensors(repository))
    return false;

  auto types = repository.sensorTypes();

  std::sort(
    types.begin(),
    types.end(),
    [](const SensorType& a, const SensorType& b)
    {
      return a.name < b.name;
    });

  beginResetModel();

  m_types = std::move(types);

  endResetModel();

  return true;
}

int SensorTypeModel::rowCount(
  const QModelIndex& parent) const
{
  return parent.isValid()
  ? 0
  : static_cast<int>(m_types.size());
}

int SensorTypeModel::columnCount(
  const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 4;
}

QVariant SensorTypeModel::data(
  const QModelIndex& index,
  int role) const
{
  if (!index.isValid() ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_types.size()) ||
      index.column() < 0 ||
      index.column() >= 4)
  {
    return {};
  }

  if (role != Qt::DisplayRole &&
      role != Qt::EditRole)
  {
    return {};
  }

  const auto& type = m_types[index.row()];

  switch (index.column())
  {
  case 0:
    return type.id.value;

  case 1:
    return QString::fromStdString(type.name);

  case 2:
    return QString::fromStdString(type.description);

  case 3:
    if (role == Qt::EditRole)
      return static_cast<int>(type.calibrationKind);

    return type.calibrationKind == CalibrationKind::Table
             ? tr("Табличная")
             : tr("Полиномиальная");
  }

  return {};
}

QVariant SensorTypeModel::headerData(
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
  case 0: return tr("ID");
  case 1: return tr("Название");
  case 2: return tr("Описание");
  case 3: return tr("Вид тарировки");
  }

  return {};
}

Qt::ItemFlags SensorTypeModel::flags(
  const QModelIndex& index) const
{
  auto result = QAbstractTableModel::flags(index);

  if (index.isValid() &&
      index.column() >= 1 &&
      index.column() <= 3)
  {
    result |= Qt::ItemIsEditable;
  }

  return result;
}

bool SensorTypeModel::setData(
  const QModelIndex& index,
  const QVariant& value,
  int role)
{
  if (role != Qt::EditRole ||
      !index.isValid() ||
      index.model() != this ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_types.size()))
  {
    return false;
  }

  SensorType type = m_types[index.row()];

  switch (index.column())
  {
  case 1:
    type.name = value.toString().trimmed().toStdString();

    if (type.name.empty())
      return false;

    break;

  case 2:
    type.description = value.toString().toStdString();
    break;

  case 3:
  {
    const int kind = value.toInt();

    if (kind < 0 || kind > 1)
      return false;

    type.calibrationKind =
      static_cast<CalibrationKind>(kind);

    break;
  }

  default:
    return false;
  }

  const SensorType& previous = m_types[index.row()];

  if (type.name == previous.name &&
      type.description == previous.description &&
      type.calibrationKind == previous.calibrationKind)
  {
    return true;
  }

  if (!m_repository.updateSensorType(type))
  {
    const auto& error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось изменить тип датчика"));

    return false;
  }

  m_types[index.row()] = std::move(type);

  emit dataChanged(
    index,
    index,
    {Qt::DisplayRole, Qt::EditRole});

  emit sensorTypesChanged();

  return true;
}

const SensorType* SensorTypeModel::sensorType(
  const QModelIndex& index) const noexcept
{
  if (!index.isValid() ||
      index.model() != this ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_types.size()))
  {
    return nullptr;
  }

  return &m_types[index.row()];
}

bool SensorTypeModel::createSensorType(
  const QString& name)
{
  const QString trimmed = name.trimmed();

  if (trimmed.isEmpty())
    return false;

  SensorTypeId id;

  if (!m_repository.createSensorType(trimmed, id))
  {
    const auto& error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось создать тип датчика"));

    return false;
  }

  SensorType type;

  type.id = id;
  type.name = trimmed.toStdString();
  type.calibrationKind = CalibrationKind::Table;

  const auto it = std::lower_bound(
    m_types.begin(),
    m_types.end(),
    type.name,
    [](const SensorType& item,
       const std::string& name)
    {
      return item.name < name;
    });

  const int row =
    static_cast<int>(it - m_types.begin());

  beginInsertRows({}, row, row);

  m_types.insert(it, std::move(type));

  endInsertRows();

  emit sensorTypesChanged();

  return true;
}

bool SensorTypeModel::removeSensorType(
  const QModelIndex& index)
{
  const auto* type = sensorType(index);

  if (!type)
    return false;

  if (!m_repository.removeSensorType(type->id))
  {
    const auto& error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось удалить тип датчика"));

    return false;
  }

  const int row = index.row();

  beginRemoveRows({}, row, row);

  m_types.erase(m_types.begin() + row);

  endRemoveRows();

  emit sensorTypesChanged();

  return true;
}

const std::vector<SensorType>& SensorTypeModel::sensorTypes()
  const noexcept
{
  return m_types;
}

}
