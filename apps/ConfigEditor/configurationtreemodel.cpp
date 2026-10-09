#include "configurationtreemodel.h"
#include "moduletypeinfo.h"

#include <vector>
#include <ranges>
#include <algorithm>

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
            : QJsonObject{},
          .archiveRate = configured
            ? configChannel->archiveRate
            : ArchiveRate::Hz10
        };

        moduleItem->addChild(
          std::move(channelData));
      }
    }
  }

  beginResetModel();

  m_configuration = configuration;
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

int ConfigurationTreeModel::columnCount(
  const QModelIndex&) const
{
  return 1;
}

QVariant ConfigurationTreeModel::data(
  const QModelIndex& index,
  int role) const
{
  if (!index.isValid())
    return {};

  if (role != Qt::DisplayRole && role != Qt::CheckStateRole)
    return {};

  const auto* item =
    static_cast<TreeItem*>(
      index.internalPointer());

  if (!item)
    return {};

  const auto& itemData =
    item->data();

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
      return channel->channel.value;
    else if (role == Qt::CheckStateRole)
      return channel->active ? Qt::Checked : Qt::Unchecked;
  }

  return {};
}

bool ConfigurationTreeModel::setData(
  const QModelIndex& index,
  const QVariant& value,
  int role)
{
  if (!index.isValid() ||
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
          channel->settings,
          channel->archiveRate);

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
      {Qt::CheckStateRole});

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
    index,
    {Qt::CheckStateRole});

  const int count =
    static_cast<int>(item->children().size());

  if (count > 0)
  {
    emit dataChanged(
      this->index(0, 0, index),
      this->index(count - 1, 0, index));
  }

  return true;
}

Qt::ItemFlags ConfigurationTreeModel::flags(
  const QModelIndex& index) const
{
  auto result = QAbstractItemModel::flags(index);

  if (!index.isValid())
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

bool ConfigurationTreeModel::updateChannelArchiveRate(
  const QModelIndex& index,
  ArchiveRate rate)
{
  auto* data = const_cast<TreeItemData*>(
    treeItemData(index));

  if (!data)
    return false;

  auto* channel = std::get_if<ChannelItemData>(data);

  if (!channel || !channel->tag ||
      !isValidArchiveRate(rate))
    return false;

  if (!m_repository.updateConfigTagArchiveRate(
        m_configuration,
        channel->module,
        channel->channel,
        rate))
  {
    const auto error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось сохранить частоту архивирования."));

    return false;
  }

  channel->archiveRate = rate;

  emit dataChanged(index, index);

  return true;
}

}