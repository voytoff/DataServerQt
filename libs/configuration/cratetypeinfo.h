#pragma once

#include "cratetype.h"

#include <QString>

// Заголовок с определением CrateType.

namespace qds
{

[[nodiscard]]
inline QString crateTypeName(CrateType type)
{
  switch (type)
  {
  case CrateType::LTR_U_8:   return QStringLiteral("LTR-U-8");
  case CrateType::LTR_U_16:  return QStringLiteral("LTR-U-16");
  case CrateType::LTR_U_1:   return QStringLiteral("LTR-U-1");
  case CrateType::LTR_EU_8:  return QStringLiteral("LTR-EU-8");
  case CrateType::LTR_EU_16: return QStringLiteral("LTR-EU-16");
  case CrateType::LTR_EU_2:  return QStringLiteral("LTR-EU-2");
  case CrateType::LTR_E_7:   return QStringLiteral("LTR-E-7");
  case CrateType::LTR_E_15:  return QStringLiteral("LTR-E-15");
  case CrateType::LTR_CU_1:  return QStringLiteral("LTR-CU-1");
  case CrateType::LTR_CEU_1: return QStringLiteral("LTR-CEU-1");

  default:
    return QStringLiteral("Unknown");
  }
}

}