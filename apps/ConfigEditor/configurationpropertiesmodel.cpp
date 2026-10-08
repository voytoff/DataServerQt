#include "configurationpropertiesmodel.h"

namespace qds
{

ConfigurationPropertiesModel::ConfigurationPropertiesModel(
  ConfigurationTreeModel* treeModel,
  QObject* parent)
  : QAbstractTableModel(parent)
  , m_treeModel(treeModel)
{
  connect(
    m_treeModel,
    &QAbstractItemModel::modelAboutToBeReset,
    this,
    [this]()
    {
      setItem({});
    });
}

void ConfigurationPropertiesModel::setItem(
  const QModelIndex& index)
{
  std::vector<Property> properties;

  if (const auto* data =
      m_treeModel->treeItemData(index))
  {
    if (const auto* module =
        std::get_if<ModuleItemData>(data))
    {
      if (module->type == ModuleType::LTR11 &&
          module->enabled)
      {
        properties.push_back({
          tr("Частота опроса, Гц"),
          module->settings.value("channelRate").toDouble(1000.0)
        });
      }
    }
    else if (const auto* channel =
             std::get_if<ChannelItemData>(data))
    {
      // Пока выводим только настройки включённых каналов.
      if (channel->tag.has_value())
      {
        properties.push_back({
          tr("Режим"),
          channel->settings.value("mode").toInt(1)
        });

        properties.push_back({
          tr("Диапазон"),
          channel->settings.value("range").toInt(0)
        });
      }
    }
  }

  beginResetModel();

  m_currentItem = index;
  m_properties = std::move(properties);

  endResetModel();
}

int ConfigurationPropertiesModel::rowCount(
  const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  return static_cast<int>(
    m_properties.size());
}

int ConfigurationPropertiesModel::columnCount(
  const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 2;
}

QVariant ConfigurationPropertiesModel::data(
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

  const auto& property =
    m_properties[index.row()];

  if (index.column() == 0)
    return role == Qt::DisplayRole
             ? QVariant(property.name)
             : QVariant{};

  if (role == Qt::EditRole)
    return property.value;

  if (property.name == tr("Режим"))
  {
    switch (property.value.toInt())
    {
    case 0: return tr("Дифференциальный");
    case 1: return tr("Общая земля");
    case 2: return tr("Измерение нуля");
    default: return property.value;
    }
  }

  if (property.name == tr("Диапазон"))
  {
    switch (property.value.toInt())
    {
    case 0: return tr("±10 В");
    case 1: return tr("±2,5 В");
    case 2: return tr("±0,625 В");
    case 3: return tr("±0,156 В");
    default: return property.value;
    }
  }

  return property.value;
}

QVariant ConfigurationPropertiesModel::headerData(
  int section,
  Qt::Orientation orientation,
  int role) const
{
  if (orientation != Qt::Horizontal ||
      role != Qt::DisplayRole)
    return {};

  return section == 0
           ? tr("Параметр")
           : tr("Значение");
}

bool ConfigurationPropertiesModel::setData(
  const QModelIndex& index,
  const QVariant& value,
  int role)
{
  if (role != Qt::EditRole ||
      !index.isValid() ||
      index.column() != 1 ||
      !m_currentItem.isValid())
  {
    return false;
  }

  if (index.row() < 0 ||
      index.row() >= static_cast<int>(m_properties.size()))
  {
    return false;
  }

  const auto* itemData =
    m_treeModel->treeItemData(m_currentItem);

  if (!itemData)
    return false;

  const auto& property =
    m_properties[index.row()];

  QJsonObject settings;

  if (const auto* module =
      std::get_if<ModuleItemData>(itemData))
  {
    settings = module->settings;

    if (property.name != tr("Частота опроса, Гц"))
      return false;

    bool ok = false;
    const double frequency = value.toDouble(&ok);

    if (!ok || frequency <= 0.0)
      return false;

    settings["channelRate"] = frequency;

    if (!m_treeModel->updateModuleSettings(
          m_currentItem,
          settings))
    {
      return false;
    }
  }
  else if (const auto* channel =
           std::get_if<ChannelItemData>(itemData))
  {
    settings = channel->settings;

    bool ok = false;
    const int number = value.toInt(&ok);

    if (!ok)
      return false;

    if (property.name == tr("Режим"))
    {
      if (number < 0 || number > 2)
        return false;

      settings["mode"] = number;
    }
    else if (property.name == tr("Диапазон"))
    {
      if (number < 0 || number > 3)
        return false;

      settings["range"] = number;
    }
    else
    {
      return false;
    }

    if (!m_treeModel->updateChannelSettings(
          m_currentItem,
          settings))
    {
      return false;
    }
  }
  else
  {
    return false;
  }

  m_properties[index.row()].value = value;

  emit dataChanged(
    index,
    index,
    {Qt::DisplayRole, Qt::EditRole});

  return true;
}

Qt::ItemFlags ConfigurationPropertiesModel::flags(
  const QModelIndex& index) const
{
  auto result =
    QAbstractTableModel::flags(index);

  if (!index.isValid() ||
      index.column() != 1 ||
      !m_currentItem.isValid())
  {
    return result;
  }

  return result | Qt::ItemIsEditable;
}

}