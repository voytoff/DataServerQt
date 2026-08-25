#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "datatypes.h"
#include "signaldefinition.h"

namespace qds
{

struct ArchiveSignal
{
  uint32_t index = 0;

  SignalId id;

  std::string name;

  SignalKind kind =
    SignalKind::Raw;

  // Для Raw-сигнала.
  std::optional<ModuleId> module;
  std::optional<ChannelId> channel;
};


struct ArchiveFileDescription
{
  std::string name;

  uint32_t frequency = 0;

  // Пока единственный тип данных.
  std::string dataType = "float";

  std::vector<ArchiveSignal> signalIds;
};


struct ArchiveDescription
{
  uint32_t version = 1;

  std::vector<ArchiveFileDescription> files;
};

}