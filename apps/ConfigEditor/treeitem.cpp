#include "treeitem.h"
#include <algorithm>
#include <iterator>
#include <utility>

namespace qds
{
TreeItem::TreeItem(
  TreeItemData data,
  TreeItem* parent)
  : m_data(std::move(data))
  , m_parent(parent)
{
}

TreeItem* TreeItem::parent() const noexcept
{
  return m_parent;
}

const TreeItemData& TreeItem::data() const noexcept
{
  return m_data;
}

TreeItemData& TreeItem::data() noexcept
{
  return m_data;
}

const std::vector<std::unique_ptr<TreeItem>>&
TreeItem::children() const noexcept
{
  return m_children;
}

TreeItem* TreeItem::addChild(
  TreeItemData data)
{
  auto child =
    std::make_unique<TreeItem>(
      std::move(data),
      this);

  auto* result = child.get();

  m_children.push_back(
    std::move(child));

  return result;
}

int TreeItem::row() const noexcept
{
  if (!m_parent)
    return 0;

  const auto& children =
    m_parent->children();

  const auto it =
    std::find_if(
      children.begin(),
      children.end(),
      [this](const auto& child) {
        return child.get() == this;
      });

  return static_cast<int>(
    std::distance(
      children.begin(),
      it));
}

}
