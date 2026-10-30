#include "ltr11configurationbuilder.h"
#include "networkutils.h"

#include <utility>

namespace qds
{

bool Ltr11ConfigurationBuilder::build(
  const ModuleRuntimeConfiguration& moduleConfiguration,
  Ltr11Configuration& configuration)
{
  m_lastError.clear();

  Ltr11Configuration cfg;

  uint32_t address = 0;

  if (!parseIpv4Address(
        moduleConfiguration.crate.host.toStdString(),
        address))
  {
    m_lastError =
      "LTR11: некорректный IPv4-адрес крейта: " +
      moduleConfiguration.crate.host.toStdString();

    return false;
  }

  cfg.address = address;
  cfg.crateSerial =
    moduleConfiguration.crate.serial.toStdString();

  cfg.port = moduleConfiguration.crate.port;
  cfg.slot = moduleConfiguration.module.slot + 1;

  cfg.channelRate = 1000.0;

  const auto& moduleSettings =
    moduleConfiguration.configuration.settings;

  if (!moduleSettings.contains("mode") ||
      !moduleSettings.value("mode").isDouble())
  {
    m_lastError =
      "LTR11: не задан или некорректен режим работы модуля";

    return false;
  }

  const int mode =
    moduleSettings.value("mode").toInt(-1);

  if (mode < 0 || mode > 2)
  {
    m_lastError =
      "LTR11: недопустимый режим работы модуля: " +
      std::to_string(mode);

    return false;
  }

  cfg.mode = static_cast<uint8_t>(mode);

  cfg.channels.reserve(
    moduleConfiguration.tags.size());

  for (const auto& tag :
       moduleConfiguration.tags)
  {
    const auto& settings = tag.settings;

    if (!settings.contains("range") ||
        !settings.value("range").isDouble())
    {
      m_lastError =
        "LTR11: не задан или некорректен диапазон "
        "измерения канала " +
        std::to_string(tag.channel.value);

      return false;
    }

    const int range =
      settings.value("range").toInt(-1);

    if (range < 0 || range > 3)
    {
      m_lastError =
        "LTR11: недопустимый диапазон "
        "измерения канала " +
        std::to_string(tag.channel.value) +
        ": " + std::to_string(range);

      return false;
    }

    cfg.channels.push_back({
      .channel =
      static_cast<uint8_t>(tag.channel.value),
      .range =
      static_cast<uint8_t>(range)
    });
  }

  if (cfg.channels.empty())
  {
    m_lastError =
      "LTR11: не настроен ни один канал";

    return false;
  }

  configuration = std::move(cfg);

  return true;
}

const std::string&
Ltr11ConfigurationBuilder::lastError() const noexcept
{
  return m_lastError;
}

}