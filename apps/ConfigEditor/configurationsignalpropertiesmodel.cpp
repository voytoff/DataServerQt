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
      PropertyId::Name,
      tr("Название"),
      QString::fromStdString(signal->name)
    });

    properties.push_back({
      PropertyId::Kind,
      tr("Вид"),
      signal->kind == SignalKind::Raw
        ? QStringLiteral("Raw")
        : QStringLiteral("Calculated")
    });

    if (signal->kind == SignalKind::Raw)
    {
      properties.push_back({
        PropertyId::Source,
        tr("Источник (TagId)"),
        signal->source.tag.value
      });
    }
    else if (signal->kind == SignalKind::Calculated)
    {
      properties.push_back({
        PropertyId::Formula,
        tr("Формула"),
        QString::fromStdString(signal->formula)
      });
    }

    properties.push_back({
      PropertyId::SignalType,
      tr("Тип сигнала (ID)"),
      signal->signalType.value
    });

    properties.push_back({
      PropertyId::ArchiveRate,
      tr("Частота архивирования"),
      static_cast<uint16_t>(signal->archiveRate)
    });

    properties.push_back({
      PropertyId::CalibrationMode,
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

  if (role == PropertyIdRole)
  {
    if (index.column() != 1)
      return {};

    return static_cast<int>(
      m_properties[index.row()].id);
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

  if (property.id == PropertyId::ArchiveRate)
  {
    return QString("%1 Гц")
      .arg(property.value.toUInt());
  }
  else if (property.id == PropertyId::CalibrationMode)
  {
    switch (property.value.toInt())
    {
    case 0: return tr("Нет");
    case 1: return tr("По сигналу");
    case 2: return tr("По типу сигнала");
    default: return property.value;
    }
  }
  else if (property.id == PropertyId::SignalType)
  {
    const uint32_t id = property.value.toUInt();

    if (id == 0)
      return tr("Нет");

    const auto it = std::find_if(
      m_signalTypes.begin(),
      m_signalTypes.end(),
      [id](const SignalTypeInfo& info)
      {
        return info.id.value == id;
      });

    if (it != m_signalTypes.end())
      return QString::fromStdString(it->name);

    return QString::number(id);
  }
  else if (property.id == PropertyId::Source)
  {
    const uint32_t id = property.value.toUInt();

    if (id == 0)
      return tr("Нет");

    const auto it = std::find_if(
      m_tagSources.begin(),
      m_tagSources.end(),
      [id](const TagSourceInfo& info)
      {
        return info.id.value == id;
      });

    if (it != m_tagSources.end())
      return QString::fromStdString(it->name);

    return QString::number(id);
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
  case 0: return tr("Параметр");
  case 1: return tr("Значение");
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

  auto& property =
    m_properties[index.row()];

  QVariant newValue = value;

  if (property.id == PropertyId::Name)
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

    newValue = name;
  }
  else if (property.id == PropertyId::ArchiveRate)
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

    newValue = frequency;
  }
  else if (property.id == PropertyId::CalibrationMode)
  {
    bool ok = false;
    const uint32_t mode = value.toUInt(&ok);

    if (!ok ||
        !isValidCalibrationMode(static_cast<CalibrationMode>(mode)))
    {
      emit operationFailed(
        tr("Недопустимый режим калибровки: %1.\n")
          .arg(value.toString()));

      return false;
    }

    const auto calibrationMode = static_cast<CalibrationMode>(mode);

    if (!m_signalModel->updateSignalCalibrationMode(
          m_currentItem, calibrationMode))
    {
      return false;
    }

    newValue = mode;
  }
  else if (property.id == PropertyId::SignalType)
  {
    bool ok = false;
    const uint32_t type = value.toUInt(&ok);

    if (!ok)
    {
      emit operationFailed(
        tr("Недопустимый тип сигнала: %1.\n")
          .arg(value.toString()));

      return false;
    }

    const auto signalType = SignalTypeId{type};

    if (!m_signalModel->updateSignalSignalType(
          m_currentItem, signalType))
    {
      return false;
    }

    newValue = type;
  }
  else if (property.id == PropertyId::Source)
  {
    bool ok = false;
    const uint32_t id = value.toUInt(&ok);

    if (!ok)
    {
      emit operationFailed(
        tr("Недопустимый источник сигнала: %1.")
          .arg(value.toString()));

      return false;
    }

    const TagId tag{id};

    if (!m_signalModel->updateSignalSource(
          m_currentItem, tag))
    {
      return false;
    }

    newValue = id;
  }
  else
  {
    return false;
  }

  property.value = newValue;

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

  if (index.model() != this ||
      !index.isValid() ||
      index.column() != 1 ||
      !m_currentItem.isValid() ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_properties.size()))
  {
    return result;
  }

  const auto& propertyId = m_properties[index.row()].id;

  if (propertyId == PropertyId::Name ||
      propertyId == PropertyId::ArchiveRate ||
      propertyId == PropertyId::CalibrationMode ||
      propertyId == PropertyId::SignalType ||
      propertyId == PropertyId::Source)
  {
    result |= Qt::ItemIsEditable;
  }

  return result;
}

void ConfigurationSignalPropertiesModel::setTagSources(
  std::vector<TagSourceInfo> sources)
{
  m_tagSources = std::move(sources);

  if (!m_properties.empty())
  {
    emit dataChanged(
      index(0, 1),
      index(rowCount() - 1, 1),
      {Qt::DisplayRole});
  }
}

void ConfigurationSignalPropertiesModel::setSignalTypes(
  std::vector<SignalTypeInfo> types)
{
  m_signalTypes = std::move(types);

  if (!m_properties.empty())
  {
    emit dataChanged(
      index(0, 1),
      index(rowCount() - 1, 1),
      {Qt::DisplayRole});
  }
}

}