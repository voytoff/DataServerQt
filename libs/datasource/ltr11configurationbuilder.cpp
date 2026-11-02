#include "ltr11configurationbuilder.h"
#include "ltr11configurationvalidator.h"
#include "networkutils.h"
#include "jsonutils.h"

#include <utility>
#include <cmath>

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

  const auto& moduleSettings =
    moduleConfiguration.configuration.settings;

  const QJsonValue rateValue =
    moduleSettings.value("channelRate");

  if (!rateValue.isDouble())
  {
    m_lastError =
      "LTR11: не задана или некорректна частота опроса";

    return false;
  }

  const double channelRate = rateValue.toDouble();

  if (!std::isfinite(channelRate) ||
      channelRate <= 0.0)
  {
    m_lastError =
      "LTR11: недопустимая частота опроса: " +
      std::to_string(channelRate);

    return false;
  }

  cfg.channelRate = channelRate;

  int mode = 0;

  if (!jsonInteger(
        moduleSettings.value("mode"),
        0,
        1,
        mode))
  {
    m_lastError =
      "LTR11: не задан или недопустим режим работы модуля";

    return false;
  }

  cfg.mode = static_cast<uint8_t>(mode);

  cfg.channels.reserve(
    moduleConfiguration.tags.size());

  for (const auto& tag :
       moduleConfiguration.tags)
  {
    const auto& settings = tag.settings;

    int range = 0;

    if (!jsonInteger(
          settings.value("range"),
          0,
          3,
          range))
    {
      m_lastError =
        "LTR11: не задан или недопустим диапазон "
        "измерения канала " +
        std::to_string(static_cast<int>(tag.channel.value));

      return false;
    }

    if (tag.channel.value < 0 ||
        tag.channel.value >= 32)
    {
      m_lastError =
        "LTR11: недопустимый номер канала: " +
        std::to_string(static_cast<int>(tag.channel.value));

      return false;
    }

    cfg.channels.push_back({
      .channel = static_cast<uint8_t>(tag.channel.value),
      .range = static_cast<uint8_t>(range)
    });
  }

  if (cfg.channels.empty())
  {
    m_lastError =
      "LTR11: не настроен ни один канал";

    return false;
  }

  if (!Ltr11ConfigurationValidator::validate(
        cfg,
        m_lastError))
  {
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