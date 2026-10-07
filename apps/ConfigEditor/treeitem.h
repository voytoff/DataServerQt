#pragma once

#include "treeitemdata.h"
#include <memory>
#include <QVariant>
#include <vector>

namespace qds
{

class TreeItem
{
public:
  explicit TreeItem(
    TreeItemData data,
    TreeItem* parent = nullptr);

  TreeItem* parent() const noexcept;

  const TreeItemData& data() const noexcept;
  TreeItemData& data() noexcept;

  const std::vector<
    std::unique_ptr<TreeItem>>& children() const noexcept;

  TreeItem* addChild(
    TreeItemData data);

  int row() const noexcept;

private:
  TreeItemData m_data;

  TreeItem* m_parent = nullptr;

  std::vector<
    std::unique_ptr<TreeItem>> m_children;
};

}
