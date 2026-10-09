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

  const QJsonValue modeValue = settings.value("mode");

  if (!modeValue.isDouble())
    return std::nullopt;

  const double mode = modeValue.toDouble();

  if (mode != 0.0 && mode != 1.0)
    return std::nullopt;

  Ltr11Configuration result{
    .address = address,
    .port = static_cast<uint16_t>(crate.port),
    .crateSerial = crate.serial.toStdString(),
    .slot = static_cast<int>(module.slot),
    .mode = static_cast<uint8_t>(mode),
    .channelRate = rateValue.toDouble(),
  };

  result.channels.reserve(configuration.tags.size());

  /*
 * Настройка физических каналов.
 */
  const uint32_t maxChannels =
    result.mode == 0 ? 16 : 32;

  for (const auto& tag : configuration.tags)
  {
    if (tag.channel.value >= maxChannels)
      continue;

    const QJsonValue rangeValue =
      tag.settings.value("range");

    if (!rangeValue.isDouble())
      return std::nullopt;

    const double range = rangeValue.toDouble();

    if (range != 0.0 &&
        range != 1.0 &&
        range != 2.0 &&
        range != 3.0)
    {
      return std::nullopt;
    }

    result.channels.push_back({
      .channel = static_cast<uint8_t>(tag.channel.value),
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