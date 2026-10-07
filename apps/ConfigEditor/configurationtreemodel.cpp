#include "configurationtreemodel.h"

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

      const bool enabled =
        configModule != configModules.end();

      ModuleItemData moduleData{
        .id = module.id,
        .type = module.type,
        .serial = module.serial,
        .slot = module.slot,
        .enabled = enabled,
        .settings =
        enabled
          ? configModule->settings
          : QJsonObject{}
      };

      crateItem->addChild(
        std::move(moduleData));
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

  if (const auto* module =
      std::get_if<ModuleItemData>(&itemData))
  {
    if (role == Qt::DisplayRole)
      return module->serial;
    else if (role == Qt::CheckStateRole) {
      return module->enabled ? Qt::Checked : Qt::Unchecked;
    }
  }

  return {};
}

bool ConfigurationTreeModel::setData(
  const QModelIndex& index,
  const QVariant& value,
  int role)
{
  if (!index.isValid())
    return false;

  if (role != Qt::CheckStateRole)
    return false;

  auto* item =
    static_cast<TreeItem*>(
      index.internalPointer());

  if (!item)
    return false;

  auto& itemData =
    item->data();

  auto* module =
    std::get_if<ModuleItemData>(
      &itemData);

  if (!module)
    return false;

  const bool checked =
    value.toInt() == Qt::Checked;

  if (checked == module->enabled)
    return true;

  const bool success =
    checked
      ? m_repository.addConfigModule(
          m_configuration,
          module->id)
      : m_repository.removeConfigModule(
          m_configuration,
          module->id);

  if (!success)
    return false;

  module->enabled = checked;

  if (!checked)
    module->settings = {};

  emit dataChanged(
    index,
    index,
    {Qt::CheckStateRole});

  return true;
}

Qt::ItemFlags ConfigurationTreeModel::flags(
  const QModelIndex& index) const
{
  auto result =  QAbstractItemModel::flags(index);

  if (!index.isValid())
    return result;

  const auto* item =
    static_cast<TreeItem*>(
      index.internalPointer());

  if (!item)
    return result;

  const auto& itemData =
    item->data();

  if (std::get_if<ModuleItemData>(&itemData))
    result  = result | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;

  return result;
}

}