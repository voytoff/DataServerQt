#pragma once

#include "cratetype.h"
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
  CrateType type = CrateType::Unknown;
  QString serial;
  QString host;
  uint32_t port = 0;
  QString description;
};

struct ModuleItemData
{
  ModuleId id;
  ModuleType type;
  QString serial;
  uint32_t slot = 0;
  bool configured = false;
  bool active = false;
  QJsonObject settings;
};

struct ChannelItemData
{
  ModuleId module;
  ChannelId channel;
  std::optional<TagId> tag;
  bool active = false;
  QJsonObject settings;
};

using TreeItemData =
  std::variant<
    RootItemData,
    CrateItemData,
    ModuleItemData,
    ChannelItemData>;
}