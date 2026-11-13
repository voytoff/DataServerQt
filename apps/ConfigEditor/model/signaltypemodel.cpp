
#include "signaltypemodel.h"

#include "db/configurationrepository.h"

#include <algorithm>

namespace qds
{

SignalTypeModel::SignalTypeModel(
  ConfigurationRepository& repository,
  QObject* parent)
  : QAbstractTableModel(parent)
  , m_repository(repository)
{
}

bool SignalTypeModel::load()
{
  std::vector<SignalTypeInfo> types;

  if (!m_repository.signalTypes(types))
    return false;

  beginResetModel();
  m_types = std::move(types);
  endResetModel();

  return true;
}

void SignalTypeModel::setMeasurementUnits(
  const std::vector<qds::MeasurementUnit>& units)
{
  m_units = units;

  if (!m_types.empty())
  {
    emit dataChanged(
      index(0, MeasurementUnit),
      index(rowCount() - 1, MeasurementUnit),
      {Qt::DisplayRole});
  }
}

int SignalTypeModel::rowCount(
  const QModelIndex& parent) const
{
  return parent.isValid()
  ? 0
  : static_cast<int>(m_types.size());
}

int SignalTypeModel::columnCount(
  const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : ColumnCount;
}

QVariant SignalTypeModel::data(
  const QModelIndex& index,
  int role) const
{
  if (!index.isValid() ||
      index.row() < 0 ||
      index.row() >= rowCount() ||
      index.column() < 0 ||
      index.column() >= ColumnCount)
  {
    return {};
  }

  if (role != Qt::DisplayRole &&
      role != Qt::EditRole)
    return {};

  const auto& type = m_types[index.row()];

  switch (index.column())
  {
  case Id:
    return type.id.value;

  case Name:
    return QString::fromStdString(type.name);

  case Description:
    return QString::fromStdString(type.description);

  case MeasurementUnit:
  {
    if (role == Qt::EditRole)
      return type.measurementUnitId.value;

    if (!type.measurementUnitId)
      return {};

    const auto it = std::find_if(
      m_units.begin(),
      m_units.end(),
      [&type](const qds::MeasurementUnit& unit)
      {
        return unit.id == type.measurementUnitId;
      });

    if (it == m_units.end())
      return {};

    return QString::fromStdString(it->symbol);
  }
  }

  return {};
}

QVariant SignalTypeModel::headerData(
  int section,
  Qt::Orientation orientation,
  int role) const
{
  if (orientation != Qt::Horizontal ||
      role != Qt::DisplayRole)
    return {};

  switch (section)
  {
  case Id: return tr("ID");
  case Name: return tr("Название");
  case Description: return tr("Описание");
  case MeasurementUnit: return tr("Единица измерения");
  }

  return {};
}

Qt::ItemFlags SignalTypeModel::flags(
  const QModelIndex& index) const
{
  auto result = QAbstractTableModel::flags(index);

  if (index.isValid() &&
      index.column() >= Name &&
      index.column() <= MeasurementUnit)
  {
    result |= Qt::ItemIsEditable;
  }

  return result;
}

bool SignalTypeModel::setData(
  const QModelIndex& index,
  const QVariant& value,
  int role)
{
  if (role != Qt::EditRole ||
      !index.isValid() ||
      index.model() != this ||
      index.row() < 0 ||
      index.row() >= rowCount())
  {
    return false;
  }

  SignalTypeInfo type = m_types[index.row()];

  switch (index.column())
  {
  case Name:
    type.name = value.toString().trimmed().toStdString();

    if (type.name.empty())
      return false;

    break;

  case Description:
    type.description = value.toString().toStdString();
    break;

  case MeasurementUnit:
  {
    const MeasurementUnitId id{value.toUInt()};

    if (id)
    {
      const auto it = std::find_if(
        m_units.begin(),
        m_units.end(),
        [id](const qds::MeasurementUnit& unit)
        {
          return unit.id == id;
        });

      if (it == m_units.end())
        return false;
    }

    type.measurementUnitId = id;
    break;
  }

  default:
    return false;
  }

  const auto& previous = m_types[index.row()];

  if (type.name == previous.name &&
      type.description == previous.description &&
      type.measurementUnitId == previous.measurementUnitId)
  {
    return true;
  }

  if (!m_repository.updateSignalType(type))
  {
    const auto& error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось изменить тип сигнала"));

    return false;
  }

  m_types[index.row()] = std::move(type);

  emit dataChanged(
    index,
    index,
    {Qt::DisplayRole, Qt::EditRole});

  emit signalTypesChanged();

  return true;
}

const SignalTypeInfo* SignalTypeModel::signalType(
  const QModelIndex& index) const noexcept
{
  if (!index.isValid() ||
      index.model() != this ||
      index.row() < 0 ||
      index.row() >= rowCount())
  {
    return nullptr;
  }

  return &m_types[index.row()];
}

bool SignalTypeModel::createSignalType(
  const QString& name)
{
  const QString trimmed = name.trimmed();

  if (trimmed.isEmpty())
    return false;

  SignalTypeId id;

  if (!m_repository.createSignalType(trimmed, id))
  {
    const auto& error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось создать тип сигнала"));

    return false;
  }

  SignalTypeInfo type;
  type.id = id;
  type.name = trimmed.toStdString();

  const auto it = std::lower_bound(
    m_types.begin(),
    m_types.end(),
    type.name,
    [](const SignalTypeInfo& item,
       const std::string& name)
    {
      return item.name < name;
    });

  const int row =
    static_cast<int>(it - m_types.begin());

  beginInsertRows({}, row, row);
  m_types.insert(it, std::move(type));
  endInsertRows();

  emit signalTypesChanged();

  return true;
}

bool SignalTypeModel::removeSignalType(
  const QModelIndex& index)
{
  const auto* type = signalType(index);

  if (!type)
    return false;

  if (!m_repository.removeSignalType(type->id))
  {
    const auto& error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось удалить тип сигнала"));

    return false;
  }

  const int row = index.row();

  beginRemoveRows({}, row, row);
  m_types.erase(m_types.begin() + row);
  endRemoveRows();

  emit signalTypesChanged();

  return true;
}

const std::vector<SignalTypeInfo>&
SignalTypeModel::signalTypes() const noexcept
{
  return m_types;
}

}
