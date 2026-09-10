#include "ltr11configurationbuilder.h"
#include "networkutils.h"
#include <QString>

namespace qds
{

bool Ltr11ConfigurationBuilder::build(
  const ModuleRuntimeConfiguration &moduleConfiguration,
  Ltr11Configuration &configuration)
{
  Ltr11Configuration cfg;

  uint32_t address;

  if (!parseIpv4Address(moduleConfiguration.crate.host.toStdString(), address))
    return false;

  cfg.address = address;
  cfg.crateSerial = moduleConfiguration.crate.serial.toStdString();
  cfg.port = moduleConfiguration.crate.port;
  cfg.slot = moduleConfiguration.module.slot;
  cfg.channelRate = moduleConfiguration. channelRate; // отдельное поле в таблице configuration_module.
                                                      // это поле универсально, потому что частота задается для всех модулей. не убираем его в settings
  for (const auto &tag : moduleConfiguration.tags)
  {
    Ltr11ChannelConfiguration channel;
    // загружаем channel из settings
    cfg.channels.push_back(channel);
  }
  configuration = cfg;
  return true;
}

}
