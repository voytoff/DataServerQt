#include "configurationrepository.h"
#include "sensorrepository.h"
#include "sensormodel.h"

#include <algorithm>
#include <QColor>

namespace qds
{

SensorModel::SensorModel(
  ConfigurationRepository& repository,
  QObject* parent)
  : QAbstractTableModel(parent)
  , m_repository(repository)
{
}

bool SensorModel::load()
{
  SensorRepository repository;

  if (!m_repository.loadSensors(repository))
    return false;

  auto sensors = repository.sensors();
  auto types = repository.sensorTypes();

  std::sort(
    sensors.begin(),
    sensors.end(),
    [](const Sensor& a, const Sensor& b)
    {
      return a.name < b.name;
    });

  beginResetModel();

  m_sensors = std::move(sensors);
  m_sensorTypes = std::move(types);

  endResetModel();

  return true;
}

int SensorModel::rowCount(
  const QModelIndex& parent) const
{
  return parent.isValid()
  ? 0
  : static_cast<int>(m_sensors.size());
}

int SensorModel::columnCount(
  const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 6;
}

QVariant SensorModel::data(
  const QModelIndex& index,
  int role) const
{
  if (!index.isValid() ||
      index.model() != this ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_sensors.size()) ||
      index.column() < 0 ||
      index.column() >= 6)
  {
    return {};
  }

  const Sensor& sensor = m_sensors[index.row()];

  // Проверяем несовпадение видов тарировки
  // только для режима "По типу".
  if (index.column() == 4 &&
      sensor.calibrationMode == SensorCalibrationMode::ByType)
  {
    const auto it = std::find_if(
      m_sensorTypes.begin(),
      m_sensorTypes.end(),
      [&sensor](const SensorType& type)
      {
        return type.id == sensor.sensorTypeId;
      });

    if (it != m_sensorTypes.end() &&
        sensor.calibrationKind != it->calibrationKind)
    {
      if (role == Qt::ForegroundRole)
        return QColor(210, 140, 40);

      else if (role == Qt::ToolTipRole)
      {
        const QString typeKind =
          it->calibrationKind == CalibrationKind::Table
            ? tr("табличная")
            : tr("полиномиальная");

        const QString sensorKind =
          sensor.calibrationKind == CalibrationKind::Table
            ? tr("табличная")
            : tr("полиномиальная");

        return tr(
                 "Используется тарировка типа датчика: %1.\n"
                 "Собственный вид тарировки датчика: %2.")
          .arg(typeKind, sensorKind);
      }
    }
  }

  if (role != Qt::DisplayRole &&
      role != Qt::EditRole)
  {
    return {};
  }

  switch (index.column())
  {
  case 0:
    return sensor.id.value;

  case 1:
    return QString::fromStdString(sensor.name);

  case 2:
  {
    if (role == Qt::EditRole)
      return sensor.sensorTypeId.value;

    const auto it = std::find_if(
      m_sensorTypes.begin(),
      m_sensorTypes.end(),
      [&sensor](const SensorType& type)
      {
        return type.id == sensor.sensorTypeId;
      });

    return it != m_sensorTypes.end()
             ? QString::fromStdString(it->name)
             : tr("<Неизвестный тип>");
  }

  case 3:
    return QString::fromStdString(sensor.serial);

  case 4:
    if (role == Qt::EditRole)
      return static_cast<int>(sensor.calibrationMode);

    return sensor.calibrationMode ==
               SensorCalibrationMode::Own
             ? tr("Собственная")
             : tr("По типу");

  case 5:
    if (role == Qt::EditRole)
      return static_cast<int>(sensor.calibrationKind);

    return sensor.calibrationKind ==
               CalibrationKind::Table
             ? tr("Табличная")
             : tr("Полиномиальная");
  }

  return {};
}

QVariant SensorModel::headerData(
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
  case 2: return tr("Тип датчика");
  case 3: return tr("Серийный номер");
  case 4: return tr("Режим тарировки");
  case 5: return tr("Вид тарировки");
  }

  return {};
}


Qt::ItemFlags SensorModel::flags(
  const QModelIndex& index) const
{
  auto result = QAbstractTableModel::flags(index);

  if (!index.isValid() ||
      index.model() != this ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_sensors.size()))
  {
    return result;
  }

  // ID не редактируется.
  if (index.column() >= 1 &&
      index.column() <= 5)
  {
    // При тарировке по типу собственный
    // вид тарировки датчика не редактируем.
    if (index.column() != 5 ||
        m_sensors[index.row()].calibrationMode ==
          SensorCalibrationMode::Own)
    {
      result |= Qt::ItemIsEditable;
    }
  }

  return result;
}

bool SensorModel::setData(
  const QModelIndex& index,
  const QVariant& value,
  int role)
{
  if (role != Qt::EditRole ||
      !index.isValid() ||
      index.model() != this ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_sensors.size()) ||
      !(flags(index) & Qt::ItemIsEditable))
  {
    return false;
  }

  Sensor sensor = m_sensors[index.row()];

  switch (index.column())
  {
  case 1:
    sensor.name = value.toString().trimmed().toStdString();

    if (sensor.name.empty())
      return false;

    break;

  case 2:
  {
    const SensorTypeId id{value.toUInt()};

    const auto it = std::find_if(
      m_sensorTypes.begin(),
      m_sensorTypes.end(),
      [id](const SensorType& type)
      {
        return type.id == id;
      });

    if (it == m_sensorTypes.end())
      return false;

    sensor.sensorTypeId = id;
    break;
  }

  case 3:
    sensor.serial = value.toString().toStdString();
    break;

  case 4:
  {
    const int mode = value.toInt();

    if (mode < 0 || mode > 1)
      return false;

    sensor.calibrationMode =
      static_cast<SensorCalibrationMode>(mode);
    break;
  }

  case 5:
  {
    const int kind = value.toInt();

    if (kind < 0 || kind > 1)
      return false;

    sensor.calibrationKind =
      static_cast<CalibrationKind>(kind);
    break;
  }

  default:
    return false;
  }

  const Sensor& previous = m_sensors[index.row()];

  if (sensor.name == previous.name &&
      sensor.sensorTypeId == previous.sensorTypeId &&
      sensor.serial == previous.serial &&
      sensor.calibrationMode == previous.calibrationMode &&
      sensor.calibrationKind == previous.calibrationKind)
  {
    return true;
  }

  if (!m_repository.updateSensor(sensor))
  {
    const auto& error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось изменить датчик"));

    return false;
  }

  m_sensors[index.row()] = std::move(sensor);

  emit dataChanged(
    this->index(index.row(), 1),
    this->index(index.row(), 5));

  return true;
}


const Sensor* SensorModel::sensor(
  const QModelIndex& index) const noexcept
{
  if (!index.isValid() ||
      index.model() != this ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_sensors.size()))
  {
    return nullptr;
  }

  return &m_sensors[index.row()];
}

const std::vector<SensorType>& SensorModel::sensorTypes()
  const noexcept
{
  return m_sensorTypes;
}

void SensorModel::setSensorTypes(
  std::vector<SensorType> types)
{
  m_sensorTypes = std::move(types);

  if (m_sensors.empty())
    return;

  emit dataChanged(
    index(0, 2),
    index(rowCount() - 1, 5));
}

bool SensorModel::createSensor(
  const QString& name,
  SensorTypeId typeId,
  SensorId &createdId)
{
  const QString trimmed = name.trimmed();

  if (trimmed.isEmpty())
    return false;

  const auto typeIt = std::find_if(
    m_sensorTypes.begin(),
    m_sensorTypes.end(),
    [typeId](const SensorType& type)
    {
      return type.id == typeId;
    });

  if (typeIt == m_sensorTypes.end())
    return false;

  SensorId id;

  if (!m_repository.createSensor(trimmed, typeId, id))
  {
    const auto& error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось создать датчик"));

    return false;
  }

  Sensor sensor;

  sensor.id = id;
  sensor.sensorTypeId = typeId;
  sensor.name = trimmed.toStdString();

  const auto it = std::lower_bound(
    m_sensors.begin(),
    m_sensors.end(),
    sensor.name,
    [](const Sensor& item,
       const std::string& name)
    {
      return item.name < name;
    });

  const int row =
    static_cast<int>(it - m_sensors.begin());

  beginInsertRows({}, row, row);

  m_sensors.insert(it, std::move(sensor));

  endInsertRows();

  createdId = id;

  return true;
}

QModelIndex SensorModel::indexOf(
  SensorId id) const
{
  const auto it = std::find_if(
    m_sensors.begin(),
    m_sensors.end(),
    [id](const Sensor& sensor)
    {
      return sensor.id == id;
    });

  if (it == m_sensors.end())
    return {};

  const int row =
    static_cast<int>(it - m_sensors.begin());

  return index(row, 1);
}

bool SensorModel::removeSensor(
  const QModelIndex& index)
{
  const auto* sensor = this->sensor(index);

  if (!sensor)
    return false;

  const SensorId id = sensor->id;
  const int row = index.row();

  if (!m_repository.removeSensor(id))
  {
    const auto& error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось удалить датчик"));

    return false;
  }

  beginRemoveRows({}, row, row);

  m_sensors.erase(
    m_sensors.begin() + row);

  endRemoveRows();

  return true;
}

void SensorModel::updateSensor(
  const Sensor& sensor)
{
  const QModelIndex idx =
    indexOf(sensor.id);

  if (!idx.isValid())
    return;

  m_sensors[idx.row()] = sensor;

  emit dataChanged(
    index(idx.row(), 1),
    index(idx.row(), 5),
    {Qt::DisplayRole, Qt::EditRole});
}

}
