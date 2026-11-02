#pragma once

#include "ltr11configuration.h"

#include <string>

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

  [[nodiscard]]
  static bool validate(
    const Ltr11Configuration& configuration,
    std::string& error);
};

}