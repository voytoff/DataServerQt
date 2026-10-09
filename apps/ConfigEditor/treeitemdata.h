#pragma once

#include "datatypes.h"
#include "moduletype.h"
#include <optional>
#include <QJsonObject>

namespace qds
{

struct RootItemData
{
};

struct CrateItemData
{
  CrateId id;
  QString serial;
};

struct ModuleItemData
{
  ModuleId id;
  ModuleType type;
  QString serial;
  uint32_t slot = 0;
  bool enabled = false;
  QJsonObject settings;
};

struct ChannelItemData
{
  ModuleId module;
  ChannelId channel;
  std::optional<TagId> tag;
  QJsonObject settings;
  ArchiveRate archiveRate = ArchiveRate::Hz10;
};

using TreeItemData =
  std::variant<
    RootItemData,
    CrateItemData,
    ModuleItemData,
    ChannelItemData>;
}