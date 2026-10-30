#pragma once

#include "treeitemdata.h"

namespace qds
{

// названия режимов и диапазонов сейчас повторяются в делегате и форматтере.
// Когда модулей станет больше, лучше будет вынести их в общие определения параметров оборудования,
// чтобы не поддерживать одинаковые списки в двух местах.
class ModuleSettingsFormatter
{
public:
  static QString moduleText(
    const ModuleItemData& module);

  static QString channelText(
    const ModuleItemData& module,
    const ChannelItemData& channel);
};

}