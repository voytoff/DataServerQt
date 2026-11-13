#include "measurementunitmodel.h"

#include "db/configurationrepository.h"

#include <algorithm>

namespace qds
{

MeasurementUnitModel::MeasurementUnitModel(
  ConfigurationRepository& repository,
  QObject* parent)
  : QAbstractTableModel(parent)
  , m_repository(repository)
{
}

bool MeasurementUnitModel::load()
{
  std::vector<MeasurementUnit> units;

  if (!m_repository.measurementUnits(units))
    return false;

  std::sort(
    units.begin(),
    units.end(),
    [](const MeasurementUnit& a,
       const MeasurementUnit& b)
    {
      return a.name < b.name;
    });

  beginResetModel();

  m_units = std::move(units);

  endResetModel();

  return true;
}

int MeasurementUnitModel::rowCount(
  const QModelIndex& parent) const
{
  return parent.isValid()
  ? 0
  : static_cast<int>(m_units.size());
}

int MeasurementUnitModel::columnCount(
  const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 3;
}

QVariant MeasurementUnitModel::data(
  const QModelIndex& index,
  int role) const
{
  if (!index.isValid() ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_units.size()) ||
      index.column() < 0 ||
      index.column() >= 3)
  {
    return {};
  }

  if (role != Qt::DisplayRole &&
      role != Qt::EditRole)
  {
    return {};
  }

  const auto& unit = m_units[index.row()];

  switch (index.column())
  {
  case 0:
    return unit.id.value;

  case 1:
    return QString::fromStdString(unit.name);

  case 2:
    return QString::fromStdString(unit.symbol);
  }

  return {};
}

QVariant MeasurementUnitModel::headerData(
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
  case 1: return tr("Наименование");
  case 2: return tr("Обозначение");
  }

  return {};
}

Qt::ItemFlags MeasurementUnitModel::flags(
  const QModelIndex& index) const
{
  auto result = QAbstractTableModel::flags(index);

  if (index.isValid() &&
      index.column() >= 1 &&
      index.column() <= 2)
  {
    result |= Qt::ItemIsEditable;
  }

  return result;
}

bool MeasurementUnitModel::setData(
  const QModelIndex& index,
  const QVariant& value,
  int role)
{
  if (role != Qt::EditRole ||
      !index.isValid() ||
      index.model() != this ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_units.size()))
  {
    return false;
  }

  MeasurementUnit unit = m_units[index.row()];

  switch (index.column())
  {
  case 1:
    unit.name = value.toString()
                  .trimmed().toStdString();
    break;

  case 2:
    unit.symbol = value.toString()
                    .trimmed().toStdString();
    break;

  default:
    return false;
  }

  if (unit.name.empty() ||
      unit.symbol.empty())
  {
    return false;
  }

  const auto& previous = m_units[index.row()];

  if (unit.name == previous.name &&
      unit.symbol == previous.symbol)
  {
    return true;
  }

  if (!m_repository.updateMeasurementUnit(unit))
  {
    const auto& error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось изменить единицу измерения"));

    return false;
  }

  m_units[index.row()] = std::move(unit);

  emit dataChanged(
    index,
    index,
    {Qt::DisplayRole, Qt::EditRole});

  emit measurementUnitsChanged();

  return true;
}

const MeasurementUnit* MeasurementUnitModel::measurementUnit(
  const QModelIndex& index) const noexcept
{
  if (!index.isValid() ||
      index.model() != this ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_units.size()))
  {
    return nullptr;
  }

  return &m_units[index.row()];
}

bool MeasurementUnitModel::createMeasurementUnit(
  const QString& name,
  const QString& symbol)
{
  const QString trimmedName = name.trimmed();
  const QString trimmedSymbol = symbol.trimmed();

  if (trimmedName.isEmpty() ||
      trimmedSymbol.isEmpty())
  {
    return false;
  }

  MeasurementUnitId id;

  if (!m_repository.createMeasurementUnit(
        trimmedName,
        trimmedSymbol,
        id))
  {
    const auto& error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось создать единицу измерения"));

    return false;
  }

  MeasurementUnit unit;

  unit.id = id;
  unit.name = trimmedName.toStdString();
  unit.symbol = trimmedSymbol.toStdString();

  const auto it = std::lower_bound(
    m_units.begin(),
    m_units.end(),
    unit.name,
    [](const MeasurementUnit& item,
       const std::string& name)
    {
      return item.name < name;
    });

  const int row =
    static_cast<int>(it - m_units.begin());

  beginInsertRows({}, row, row);

  m_units.insert(it, std::move(unit));

  endInsertRows();

  emit measurementUnitsChanged();

  return true;
}

bool MeasurementUnitModel::removeMeasurementUnit(
  const QModelIndex& index)
{
  const auto* unit = measurementUnit(index);

  if (!unit)
    return false;

  if (!m_repository.removeMeasurementUnit(unit->id))
  {
    const auto& error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось удалить единицу измерения"));

    return false;
  }

  const int row = index.row();

  beginRemoveRows({}, row, row);

  m_units.erase(m_units.begin() + row);

  endRemoveRows();

  emit measurementUnitsChanged();

  return true;
}

const std::vector<MeasurementUnit>&
MeasurementUnitModel::measurementUnits() const noexcept
{
  return m_units;
}

}