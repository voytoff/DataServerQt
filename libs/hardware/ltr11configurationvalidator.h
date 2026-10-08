#pragma once

#include "ltr11configuration.h"

namespace qds
{

class Ltr11ConfigurationValidator
{
public:
  [[nodiscard]]
  static bool validate(
    const Ltr11ChannelConfiguration& configuration) noexcept;

  [[nodiscard]]
  static bool validate(
    const Ltr11Configuration& configuration) noexcept;
};

}