#include "ltr11configurationvalidator.h"

#include <algorithm>
#include <cmath>

namespace qds
{

namespace
{

constexpr uint32_t MaxPhysicalChannels = 32;
constexpr uint32_t MaxLogicalChannels = 128;

constexpr uint8_t MaxChannelMode = 2;
constexpr uint8_t MaxChannelRange = 3;

constexpr double MaxAdcFrequency = 400000.0;

}

bool Ltr11ConfigurationValidator::validate(
  const Ltr11ChannelConfiguration& configuration) noexcept
{
  return
    configuration.channel < MaxPhysicalChannels &&
    configuration.mode <= MaxChannelMode &&
    configuration.range <= MaxChannelRange;
}

bool Ltr11ConfigurationValidator::validate(
  const Ltr11Configuration& configuration) noexcept
{
  const auto& channels = configuration.channels;

  if (channels.empty() ||
      channels.size() > MaxLogicalChannels)
  {
    return false;
  }

  if (!std::all_of(
        channels.begin(),
        channels.end(),
        [](const auto& channel)
        {
          return validate(channel);
        }))
  {
    return false;
  }

  const double channelRate =
    configuration.channelRate;

  if (!std::isfinite(channelRate) ||
      channelRate <= 0.0)
  {
    return false;
  }

  const double adcFrequency =
    channelRate *
    static_cast<double>(channels.size());

  return adcFrequency <= MaxAdcFrequency;
}

}