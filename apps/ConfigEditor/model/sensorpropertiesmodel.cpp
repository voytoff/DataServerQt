
#include "sensorpropertiesmodel.h"
#include "db/configurationrepository.h"

namespace qds
{

SensorPropertiesModel::SensorPropertiesModel(
  SensorModel& sensors,
  ConfigurationRepository& repository,
  QObject* parent)
  : QAbstractTableModel(parent),
  m_sensors(sensors),
  m_repository(repository)
{
}

void SensorPropertiesModel::setSensor(
  const QModelIndex& index)
{
  SensorId id;

  if (const auto* sensor = m_sensors.sensor(index))
    id = sensor->id;

  if (m_sensorId == id)
    return;

  beginResetModel();

  m_sensorId = id;

  endResetModel();
}

int SensorPropertiesModel::rowCount(
  const QModelIndex& parent) const
{
  if (parent.isValid() || !m_sensorId)
    return 0;

  return 1;
}

int SensorPropertiesModel::columnCount(
  const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 2;
}

QVariant SensorPropertiesModel::headerData(
  int section,
  Qt::Orientation orientation,
  int role) const
{
  if (role != Qt::DisplayRole ||
      orientation != Qt::Horizontal)
    return {};

  switch (section)
  {
  case 0:
    return tr("Параметр");

  case 1:
    return tr("Значение");
  }

  return {};
}

QVariant SensorPropertiesModel::data(
  const QModelIndex& index,
  int role) const
{
  if (!index.isValid() ||
      index.row() != 0 ||
      index.column() < 0 ||
      index.column() > 1 ||
      (role != Qt::DisplayRole &&
       role != Qt::EditRole))
  {
    return {};
  }

  const QModelIndex sensorIndex =
    m_sensors.indexOf(m_sensorId);

  const auto* sensor =
    m_sensors.sensor(sensorIndex);

  if (!sensor)
    return {};

  if (index.column() == 0)
    return tr("Описание");

  return QString::fromStdString(
    sensor->description);
}

Qt::ItemFlags SensorPropertiesModel::flags(
  const QModelIndex& index) const
{
  auto result =
    QAbstractTableModel::flags(index);

  if (index.isValid() &&
      index.row() == 0 &&
      index.column() == 1)
  {
    result |= Qt::ItemIsEditable;
  }

  return result;
}

bool SensorPropertiesModel::setData(
  const QModelIndex& index,
  const QVariant& value,
  int role)
{
  if (role != Qt::EditRole ||
      !index.isValid() ||
      index.row() != 0 ||
      index.column() != 1)
  {
    return false;
  }

  const QModelIndex sensorIndex =
    m_sensors.indexOf(m_sensorId);

  const auto* current =
    m_sensors.sensor(sensorIndex);

  if (!current)
    return false;

  Sensor sensor = *current;

  sensor.description =
    value.toString().toStdString();

  if (sensor.description == current->description)
    return true;

  if (!m_repository.updateSensor(sensor))
  {
    const auto& error =
      m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось изменить описание датчика"));

    return false;
  }

  // Синхронизируем SensorModel.
  // Для этого добавим в неё небольшой метод.
  m_sensors.updateSensor(sensor);

  emit dataChanged(
    index,
    index,
    {Qt::DisplayRole, Qt::EditRole});

  return true;
}

}
