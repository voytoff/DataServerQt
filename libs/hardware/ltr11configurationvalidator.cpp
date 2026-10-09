#include "ltr11configurationvalidator.h"

#include <array>
#include <cmath>

namespace qds
{

namespace
{

constexpr uint8_t MaxPhysicalChannels = 32;
constexpr uint8_t DifferentialChannels = 16;

constexpr uint8_t MaxChannelMode = 1;
constexpr uint8_t MaxChannelRange = 3;

constexpr double MaxAdcFrequency = 400000.0;

}

bool Ltr11ConfigurationValidator::validate(
  const Ltr11ChannelConfiguration& configuration) noexcept
{
  return
    configuration.channel < MaxPhysicalChannels &&
    configuration.range <= MaxChannelRange;
}

bool Ltr11ConfigurationValidator::validate(
  const Ltr11Configuration& configuration) noexcept
{
  if (configuration.mode > MaxChannelMode)
    return false;

  const uint8_t maxChannels =
    configuration.mode == 0
      ? DifferentialChannels
      : MaxPhysicalChannels;

  const auto& channels = configuration.channels;

  if (channels.empty() || channels.size() > maxChannels)
    return false;

  std::array<bool, MaxPhysicalChannels> usedChannels{};

  for (const auto& channel : channels)
  {
    if (!validate(channel))
      return false;

    if (channel.channel >= maxChannels)
      return false;

    if (usedChannels[channel.channel])
      return false;

    usedChannels[channel.channel] = true;
  }

  if (!std::isfinite(configuration.channelRate) ||
      configuration.channelRate <= 0.0)
    return false;

  return configuration.channelRate *
           static_cast<double>(channels.size()) <= MaxAdcFrequency;
}

}