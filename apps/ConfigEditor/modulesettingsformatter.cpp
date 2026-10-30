#include "modulesettingsformatter.h"

namespace qds
{

namespace
{

QString ltr11ModuleText(
  const ModuleItemData& module)
{
  if (!module.configured)
    return {};

  const double rate =
    module.settings.value("channelRate").toDouble(1000.0);

  const int mode =
    module.settings.value("mode").toInt(1);

  const QString modeText =
    mode == 0
      ? QObject::tr("Дифференциальный")
      : QObject::tr("Общая земля");

  return QString("%1 Гц · %2")
    .arg(rate)
    .arg(modeText);
}

QString ltr11ChannelText(
  const ChannelItemData& channel)
{
  if (!channel.tag)
    return {};

  const int range =
    channel.settings.value("range").toInt(0);

  switch (range)
  {
  case 0:
    return QStringLiteral("±10 В");

  case 1:
    return QStringLiteral("±2,5 В");

  case 2:
    return QStringLiteral("±0,625 В");

  case 3:
    return QStringLiteral("±0,156 В");

  default:
    return {};
  }
}

}

QString ModuleSettingsFormatter::moduleText(
  const ModuleItemData& module)
{
  switch (module.type)
  {
  case ModuleType::LTR11:
    return ltr11ModuleText(module);

  default:
    return {};
  }
}

QString ModuleSettingsFormatter::channelText(
  const ModuleItemData& module,
  const ChannelItemData& channel)
{
  switch (module.type)
  {
  case ModuleType::LTR11:
    return ltr11ChannelText(channel);

  default:
    return {};
  }
}

}