#pragma once

#include <QFlags>

#include <cstdint>

namespace qds
{

enum class SettingsSection : uint32_t
{
  None     = 0,
  Database = 1 << 0,
  Archive  = 1 << 1,
  Log      = 1 << 2
};

Q_DECLARE_FLAGS(SettingsSections, SettingsSection)
Q_DECLARE_OPERATORS_FOR_FLAGS(SettingsSections)

}