#include "ltr11configurationvalidator.h"

#include <array>
#include <cmath>
#include <format>

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
  try
  {
    std::string error;
    return validate(configuration, error);
  }
  catch (...)
  {
    return false;
  }
}

bool Ltr11ConfigurationValidator::validate(
  const Ltr11Configuration& configuration,
  std::string& error)
{
  error.clear();

  if (configuration.mode > MaxChannelMode)
  {
    error = std::format(
      "LTR11: недопустимый режим работы: {}",
      configuration.mode);

    return false;
  }

  const uint8_t maxChannels =
    configuration.mode == 0
      ? DifferentialChannels
      : MaxPhysicalChannels;

  const auto& channels = configuration.channels;

  if (channels.empty())
  {
    error = "LTR11: не настроен ни один канал";
    return false;
  }

  if (channels.size() > maxChannels)
  {
    error = std::format(
      "LTR11: превышено допустимое количество каналов: "
      "{} (максимум {})",
      channels.size(),
      maxChannels);

    return false;
  }

  std::array<bool, MaxPhysicalChannels> usedChannels{};

  for (const auto& channel : channels)
  {
    if (channel.channel >= maxChannels)
    {
      error = std::format(
        "LTR11: недопустимый номер канала: {} "
        "(допустимо 0–{})",
        channel.channel,
        maxChannels - 1);

      return false;
    }

    if (channel.range > MaxChannelRange)
    {
      error = std::format(
        "LTR11: недопустимый диапазон канала {}: {}",
        channel.channel,
        channel.range);

      return false;
    }

    if (usedChannels[channel.channel])
    {
      error = std::format(
        "LTR11: физический канал {} назначен несколько раз",
        channel.channel);

      return false;
    }

    usedChannels[channel.channel] = true;
  }

  if (!std::isfinite(configuration.channelRate) ||
      configuration.channelRate <= 0.0)
  {
    error = "LTR11: недопустимая частота опроса";
    return false;
  }

  const double adcFrequency =
    configuration.channelRate *
    static_cast<double>(channels.size());

  if (!std::isfinite(adcFrequency) ||
      adcFrequency > MaxAdcFrequency)
  {
    error = std::format(
      "LTR11: превышена максимальная частота АЦП: "
      "{:.2f} Гц (максимум {:.2f} Гц)",
      adcFrequency,
      MaxAdcFrequency);

    return false;
  }

  return true;
}

}