#include "ltr11configurationfactory.h"

#include "ltr11configurationvalidator.h"

#include "networkutils.h"

#include <cstdint>
#include <limits>

namespace qds
{

std::optional<Ltr11Configuration>
Ltr11ConfigurationFactory::create(
  const ModuleRuntimeConfiguration& configuration)
{
  const auto& crate = configuration.crate;
  const auto& module = configuration.module;
  const auto& settings = configuration.configuration.settings;

  /*
   * Проверка адреса крейта.
   */
  uint32_t address = 0;

  if (!parseIpv4Address(
        crate.host.toStdString(),
        address))
  {
    return std::nullopt;
  }

  /*
   * Проверка порта.
   */
  if (crate.port > std::numeric_limits<uint16_t>::max())
    return std::nullopt;

  /*
   * Проверка номера слота.
   */
  if (module.slot > static_cast<uint32_t>(
        std::numeric_limits<int>::max()))
    return std::nullopt;

  /*
   * Проверка частоты.
   */
  const QJsonValue rateValue = settings.value("channelRate");

  if (!rateValue.isDouble())
    return std::nullopt;

  Ltr11Configuration result{
    .address = address,
    .port = static_cast<uint16_t>(crate.port),
    .crateSerial = crate.serial.toStdString(),
    .slot = static_cast<int>(module.slot),
    .channelRate = rateValue.toDouble(),
  };

  result.channels.reserve(configuration.tags.size());

  /*
   * Настройка логических каналов.
   */
  for (const auto& tag : configuration.tags)
  {
    const auto& tagSettings = tag.settings;

    const QJsonValue modeValue = tagSettings.value("mode");
    const QJsonValue rangeValue = tagSettings.value("range");

    if (!modeValue.isDouble() || !rangeValue.isDouble())
      return std::nullopt;

    const double mode = modeValue.toDouble();
    const double range = rangeValue.toDouble();

    if (mode < 0 || mode > 255 ||
        range < 0 || range > 255)
    {
      return std::nullopt;
    }

    if (mode != static_cast<int>(mode) ||
        range != static_cast<int>(range))
    {
      return std::nullopt;
    }

    if (tag.channel.value >
        std::numeric_limits<uint8_t>::max())
    {
      return std::nullopt;
    }

    result.channels.push_back(
      {
        .channel = static_cast<uint8_t>(tag.channel.value),
        .mode = static_cast<uint8_t>(mode),
        .range = static_cast<uint8_t>(range),
      });
  }

  /*
   * Проверка аппаратных ограничений LTR11.
   */
  if (!Ltr11ConfigurationValidator::validate(result))
    return std::nullopt;

  return result;
}

}