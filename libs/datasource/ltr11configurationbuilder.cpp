#include "ltr11configurationbuilder.h"
#include "networkutils.h"

namespace qds
{

bool Ltr11ConfigurationBuilder::build(
  const ModuleRuntimeConfiguration& moduleConfiguration,
  Ltr11Configuration& configuration,
  ILogger& logger)
{
  Ltr11Configuration cfg;

  uint32_t address = 0;

  if (!parseIpv4Address(
        moduleConfiguration.crate.host.toStdString(),
        address))
  {
    logger.error(
      "LTR11: invalid crate IPv4 address: " +
      moduleConfiguration.crate.host.toStdString());

    return false;
  }

  cfg.address = address;
  cfg.crateSerial = moduleConfiguration.crate.serial.toStdString();
  cfg.port = moduleConfiguration.crate.port;
  cfg.slot = moduleConfiguration.module.slot + 1;

  cfg.channelRate = 1000.0;

  const auto& moduleSettings =
    moduleConfiguration.configuration.settings;

  if (!moduleSettings.contains("mode"))
  {
    logger.error("LTR11: не задан режим работы модуля");
    return false;
  }

  const int mode = moduleSettings["mode"].toInt();

  if (mode < 0 || mode > 2)
  {
    logger.error(
      "LTR11: недопустимый режим работы модуля: " +
      std::to_string(mode));

    return false;
  }

  cfg.mode = static_cast<uint8_t>(mode);

  cfg.channels.reserve(moduleConfiguration.tags.size());

  for (const auto& tag : moduleConfiguration.tags)
  {
    const auto& settings = tag.settings;

    if (!settings.contains("range"))
    {
      logger.error(
        "LTR11: не задан диапазон измерения канала " +
        std::to_string(tag.channel.value));

      return false;
    }

    const int range = settings["range"].toInt();

    if (range < 0 || range > 3)
    {
      logger.error(
        "LTR11: недопустимый диапазон измерения канала " +
        std::to_string(tag.channel.value) +
        ": " + std::to_string(range));

      return false;
    }

    cfg.channels.push_back(
      Ltr11ChannelConfiguration{
        .channel = static_cast<uint8_t>(tag.channel.value),
        .range = static_cast<uint8_t>(range)
      });
  }

  if (cfg.channels.empty())
  {
    logger.error("LTR11: no configured channels");
    return false;
  }

  configuration = std::move(cfg);
  return true;
}

}