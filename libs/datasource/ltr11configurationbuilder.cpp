#include "ltr11configurationbuilder.h"

#include "networkutils.h"

namespace qds
{

bool Ltr11ConfigurationBuilder::build(
  const ModuleRuntimeConfiguration& moduleConfiguration,
  Ltr11Configuration& configuration)
{
  Ltr11Configuration cfg;

  uint32_t address = 0;

  if (!parseIpv4Address(
        moduleConfiguration.crate.host.toStdString(),
        address))
  {
    return false;
  }

  cfg.address = address;
  cfg.crateSerial = moduleConfiguration.crate.serial.toStdString();
  cfg.port = moduleConfiguration.crate.port;
  cfg.slot = moduleConfiguration.module.slot;

  // Пока максимальная частота опроса модуля.
  cfg.channelRate = 1000.0;

  cfg.channels.reserve(moduleConfiguration.tags.size());

  for (const auto& tag : moduleConfiguration.tags)
  {
    const auto& settings = tag.settings;

    if (!settings.contains("mode") ||
        !settings.contains("range"))
    {
      return false;
    }

    const int mode = settings["mode"].toInt();
    const int range = settings["range"].toInt();

    if (mode < 0 || mode > 2)
      return false;

    if (range < 0 || range > 3)
      return false;

    cfg.channels.push_back(
      Ltr11ChannelConfiguration{
        .channel = tag.channel.value,
        .mode = static_cast<uint8_t>(mode),
        .range = static_cast<uint8_t>(range)
      });
  }

  if (cfg.channels.empty())
    return false;

  configuration = std::move(cfg);
  return true;
}

}