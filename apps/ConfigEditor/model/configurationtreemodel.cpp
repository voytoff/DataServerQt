#include "configurationtreemodel.h"
#include "modulesettingsformatter.h"
#include "moduletypeinfo.h"

#include <vector>
#include <ranges>
#include <algorithm>
#include <QColor>

namespace qds
{

ConfigurationTreeModel::ConfigurationTreeModel(
  ConfigurationRepository& repository,
  QObject* parent)
  : QAbstractItemModel(parent)
  , m_repository(repository)
{
}

bool ConfigurationTreeModel::load(
  ConfigurationId configuration)
{  
  std::vector<CrateInfo> crates;
  std::vector<ModuleInfo> modules;
  std::vector<ConfigModule> configModules;

  if (!m_repository.crates(crates))
    return false;

  if (!m_repository.modules(modules))
    return false;

  if (!m_repository.configModules(
        configuration,
        configModules))
  {
    return false;
  }

  std::vector<SignalDefinition> definitions;

  if (!m_repository.signalDefinitions(
        configuration,
        definitions))
  {
    // нет определения сигналов
    return false;
  }

  std::unordered_set<uint32_t> rawSignalTags;

  for (const auto& signal : definitions)
  {
    if (signal.kind == SignalKind::Raw)
      rawSignalTags.insert(signal.source.tag.value);
  }


  auto root =
    std::make_unique<TreeItem>(
      RootItemData{});

  for (const auto& crate : crates)
  {
    CrateItemData crateData{
      .id = crate.id,
      .serial = crate.serial
    };

    auto* crateItem =
      root->addChild(
        std::move(crateData));

    auto crateModules =
      modules |
      std::views::filter(
        [&crate](const ModuleInfo& module) {
          return module.crate == crate.id;
        });

    for (const auto& module : crateModules)
    {
      const auto configModule =
        std::find_if(
          configModules.begin(),
          configModules.end(),
          [&module](const ConfigModule& item) {
            return item.module == module.id;
          });

      const bool configured =
        configModule != configModules.end();

      ModuleItemData moduleData{
        .id = module.id,
        .type = module.type,
        .serial = module.serial,
        .slot = module.slot,
        .configured = configured,
        .active = configured ? configModule->active : false,
        .settings = configured
          ? configModule->settings
          : QJsonObject{}
      };

      auto* moduleItem =
        crateItem->addChild(
          std::move(moduleData));

      const auto* moduleInfo =
        moduleTypeInfo(module.type);

      if (!moduleInfo)
        continue;

      std::vector<ConfigurationTag> channels;

      if (configured &&
          !m_repository.moduleConfigTags(
            configuration,
            module.id,
            channels))
      {
        return false;
      }

      for (uint32_t channel = 0; channel < moduleInfo->channelCount; ++channel) {
        const auto configChannel =
          std::find_if(
            channels.begin(),
            channels.end(),
            [channel](const ConfigurationTag& item) {
              return item.channel == ChannelId{channel};
            });

        const bool configured =
          configChannel != channels.end();

        ChannelItemData channelData{
          .module = module.id,
          .channel = ChannelId{channel},
          .tag = configured
            ? std::optional<TagId>{configChannel->tag}
            : std::nullopt,
          .active = configured ? configChannel->active : false,
          .settings = configured
            ? configChannel->settings
            : QJsonObject{}
        };

        moduleItem->addChild(
          std::move(channelData));
      }
    }
  }

  beginResetModel();

  m_configuration = configuration;
  m_rawSignalTags = std::move(rawSignalTags);
  m_root = std::move(root);

  endResetModel();

  return true;
}

QModelIndex ConfigurationTreeModel::index(
  int row,
  int column,
  const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return {};

  TreeItem* parentItem =
    parent.isValid()
      ? static_cast<TreeItem*>(
          parent.internalPointer())
      : m_root.get();

  if (!parentItem)
    return {};

  auto& children =
    parentItem->children();

  if (row < 0 ||
      static_cast<std::size_t>(row) >= children.size())
    return {};

  return createIndex(
    row,
    column,
    children[row].get());
}

QModelIndex ConfigurationTreeModel::parent(
  const QModelIndex& index) const
{
  if (!index.isValid())
    return {};

  auto* item =
    static_cast<TreeItem*>(
      index.internalPointer());

  if (!item)
    return {};

  auto* parentItem =
    item->parent();

  if (!parentItem ||
      parentItem == m_root.get())
  {
    return {};
  }

  return createIndex(
    parentItem->row(),
    0,
    parentItem);
}

int ConfigurationTreeModel::rowCount(
  const QModelIndex& parent) const
{
  if (parent.isValid() && parent.column() != 0)
    return 0;

  const TreeItem* item =
    parent.isValid()
      ? static_cast<TreeItem*>(
          parent.internalPointer())
      : m_root.get();

  if (!item)
    return 0;

  return static_cast<int>(
    item->children().size());
}

QVariant ConfigurationTreeModel::headerData(
  int section,
  Qt::Orientation orientation,
  int role) const
{
  if (orientation != Qt::Horizontal ||
      role != Qt::DisplayRole)
    return {};

  switch (section)
  {
  case 0:
    return tr("Объект");

  case 1:
    return tr("Параметры");

  default:
    return {};
  }
}

int ConfigurationTreeModel::columnCount(
  const QModelIndex&) const
{
  return 2;
}

QVariant ConfigurationTreeModel::data(
  const QModelIndex& index,
  int role) const
{
  if (!index.isValid())
    return {};

  if (role != Qt::DisplayRole &&
      role != Qt::CheckStateRole &&
      role != Qt::ForegroundRole)
  {
    return {};
  }

  const auto* item =
    static_cast<TreeItem*>(
      index.internalPointer());

  if (!item)
    return {};

  const auto& itemData =
    item->data();

  if (index.column() == 1)
  {
    if (role != Qt::DisplayRole)
      return {};

    if (const auto* module =
        std::get_if<ModuleItemData>(&itemData))
    {
      return ModuleSettingsFormatter::moduleText(*module);
    }

    if (const auto* channel =
        std::get_if<ChannelItemData>(&itemData))
    {
      const auto* parentItem = item->parent();

      if (!parentItem)
        return {};

      const auto* module =
        std::get_if<ModuleItemData>(
          &parentItem->data());

      if (!module)
        return {};

      return ModuleSettingsFormatter::channelText(
        *module, *channel);
    }

    return {};
  }

  if (const auto* crate =
    std::get_if<CrateItemData>(&itemData))
  {
    if (role == Qt::DisplayRole)
      return crate->serial;
  }

  else if (const auto* module =
    std::get_if<ModuleItemData>(&itemData))
  {
    if (role == Qt::DisplayRole)
      return module->serial;
    else if (role == Qt::CheckStateRole)
      return module->active ? Qt::Checked : Qt::Unchecked;
  }

  else if (const auto* channel =
           std::get_if<ChannelItemData>(&itemData))
  {
    if (role == Qt::DisplayRole)
      return channel->channel.value + 1;

    if (role == Qt::CheckStateRole)
      return channel->active
               ? Qt::Checked
               : Qt::Unchecked;

    if (role == Qt::ForegroundRole)
    {
      if (channel->active &&
          (!channel->tag ||
           !hasRawSignal(*channel->tag)))
      {
        return QColor(Qt::red);
      }
    }
  }

  return {};
}

bool ConfigurationTreeModel::setData(
  const QModelIndex& index,
  const QVariant& value,
  int role)
{
  if (!index.isValid() ||
      index.column() != 0 ||
      role != Qt::CheckStateRole)
  {
    return false;
  }

  auto* item =
    static_cast<TreeItem*>(
      index.internalPointer());

  if (!item)
    return false;

  auto& itemData = item->data();

  const bool active =
    value.toInt() == Qt::Checked;

  if (auto* channel =
      std::get_if<ChannelItemData>(&itemData))
  {
    const auto* parentItem = item->parent();

    if (!parentItem)
      return false;

    const auto* module =
      std::get_if<ModuleItemData>(
        &parentItem->data());

    if (!module || !module->active)
      return false;

    if (active == channel->active)
      return true;

    if (!channel->tag)
    {
      if (!active)
        return true;

      const auto tag =
        m_repository.addConfigTag(
          m_configuration,
          channel->module,
          channel->channel,
          channel->settings);

      if (!tag)
      {
        const auto error =
          m_repository.lastError();

        emit operationFailed(
          error.isValid()
            ? error.text()
            : tr("Не удалось добавить канал."));

        return false;
      }

      channel->tag = *tag;
    }
    else
    {
      const bool success =
        m_repository.setConfigTagActive(
          m_configuration,
          channel->module,
          channel->channel,
          active);

      if (!success)
      {
        const auto error =
          m_repository.lastError();

        emit operationFailed(
          error.isValid()
            ? error.text()
            : tr("Не удалось изменить состояние канала."));

        return false;
      }
    }

    channel->active = active;
    
    emit dataChanged(
      index,
      index,
      {
        Qt::CheckStateRole,
        Qt::ForegroundRole
      });

    emit tagSourcesChanged();

    return true;
  }

  auto* module =
    std::get_if<ModuleItemData>(
      &itemData);

  if (!module)
    return false;

  if (active == module->active)
    return true;

  bool success = false;

  if (!module->configured)
  {
    if (!active)
      return true;

    success =
      m_repository.addConfigModule(
        m_configuration,
        module->id);
  }
  else
  {
    success =
      m_repository.setConfigModuleActive(
        m_configuration,
        module->id,
        active);
  }

  if (!success)
  {
    const auto error =
      m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось изменить состояние модуля."));

    return false;
  }

  module->configured = true;
  module->active = active;

  emit dataChanged(
    index,
    this->index(index.row(), 1, index.parent()),
    {
      Qt::CheckStateRole,
      Qt::DisplayRole
    });

  const int count =
    static_cast<int>(item->children().size());

  if (count > 0)
  {
    emit dataChanged(
      this->index(0, 0, index),
      this->index(count - 1, 0, index));
  }

  emit tagSourcesChanged();

  return true;
}

Qt::ItemFlags ConfigurationTreeModel::flags(
  const QModelIndex& index) const
{
  auto result = QAbstractItemModel::flags(index);

  if (!index.isValid())
    return result;

  if (index.column() != 0)
    return result;

  const auto* item =
    static_cast<TreeItem*>(
      index.internalPointer());

  if (!item)
    return result;

  const auto& itemData = item->data();

  if (std::get_if<ModuleItemData>(&itemData))
  {
    result |= Qt::ItemIsUserCheckable;
  }
  else if (std::get_if<ChannelItemData>(&itemData))
  {
    const auto* parentItem = item->parent();

    if (!parentItem)
      return result;

    const auto* module =
      std::get_if<ModuleItemData>(
        &parentItem->data());

    if (module && module->active)
      result |= Qt::ItemIsUserCheckable;
  }

  return result;
}

const TreeItemData* ConfigurationTreeModel::treeItemData(
  const QModelIndex& index) const noexcept
{
  if (!index.isValid() ||
      index.model() != this)
  {
    return nullptr;
  }

  const auto* item =
    static_cast<const TreeItem*>(
      index.internalPointer());

  return item
   ? &item->data()
   : nullptr;
}

bool ConfigurationTreeModel::updateModuleSettings(
  const QModelIndex& index,
  const QJsonObject& settings)
{
  auto* data = const_cast<TreeItemData*>(
    treeItemData(index));

  if (!data)
    return false;

  auto* module = std::get_if<ModuleItemData>(data);

  if (!module || !module->configured)
    return false;

  if (!m_repository.updateConfigModuleSettings(
        m_configuration,
        module->id,
        settings))
  {
    const auto error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось сохранить настройки модуля."));

    return false;
  }

  module->settings = settings;

  emit dataChanged(index, index);

  return true;
}

bool ConfigurationTreeModel::updateChannelSettings(
  const QModelIndex& index,
  const QJsonObject& settings)
{
  auto* data = const_cast<TreeItemData*>(
    treeItemData(index));

  if (!data)
    return false;

  auto* channel = std::get_if<ChannelItemData>(data);

  if (!channel || !channel->tag)
    return false;

  if (!m_repository.updateConfigTagSettings(
        m_configuration,
        channel->module,
        channel->channel,
        settings))
  {
    const auto error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось сохранить настройки канала."));

    return false;
  }

  channel->settings = settings;

  emit dataChanged(index, index);

  return true;
}

bool ConfigurationTreeModel::refreshRawSignalTags()
{
  std::vector<SignalDefinition> definitions;

  if (!m_repository.signalDefinitions(
        m_configuration,
        definitions))
  {
    const auto error =
      m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось обновить привязки сигналов."));

    return false;
  }

  std::unordered_set<uint32_t> tags;

  for (const auto& signal : definitions)
  {
    if (signal.kind == SignalKind::Raw)
      tags.insert(signal.source.tag.value);
  }

  if (tags == m_rawSignalTags)
    return true;

  m_rawSignalTags = std::move(tags);

  const int crateCount = rowCount();

  for (int crateRow = 0; crateRow < crateCount; ++crateRow)
  {
    const QModelIndex crateIndex =
      index(crateRow, 0);

    const int moduleCount =
      rowCount(crateIndex);

    for (int moduleRow = 0; moduleRow < moduleCount; ++moduleRow)
    {
      const QModelIndex moduleIndex =
        index(moduleRow, 0, crateIndex);

      const int channelCount =
        rowCount(moduleIndex);

      if (channelCount == 0)
        continue;

      emit dataChanged(
        index(0, 0, moduleIndex),
        index(channelCount - 1, 0, moduleIndex),
        {Qt::ForegroundRole});
    }
  }

  return true;
}

bool ConfigurationTreeModel::hasRawSignal(
  TagId tag) const noexcept
{
  return m_rawSignalTags.contains(tag.value);
}

}