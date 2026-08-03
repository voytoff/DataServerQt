#pragma once

#include "archivewriter.h"

namespace qds
{

struct ArchiveTask
{
  uint32_t divider;

  //SignalGroup group;

  ArchiveWriter writer;
};

class ArchiveManager
{
public:
  ArchiveManager();
};

}