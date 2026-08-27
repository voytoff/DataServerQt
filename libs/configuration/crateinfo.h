#pragma once

#include "cratetype.h"
#include "datatypes.h"
#include <QString>

namespace qds
{

struct CrateInfo
{
  CrateId id;
  QString serial;
  CrateType type = CrateType::Unknown;

  QString host;
  uint32_t port;

  QString description;
};

}
