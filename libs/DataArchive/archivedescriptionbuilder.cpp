#include "archivedescriptionbuilder.h"

#include <map>

namespace qds
{

bool ArchiveDescriptionBuilder::build(
  const SystemConfiguration& configuration,
  ArchiveDescription& description) const
{
  ArchiveDescription tmp;

  tmp.version = 1;

  // Частота + SignalKind -> индекс файла в tmp.files.
  std::map<
    std::tuple<uint32_t, SignalKind>,
    std::size_t> fileIndexes;

  for (const auto& definition :
       configuration.signalDefinitions())
  {
    if (definition.archiveFrequency == 0)
      continue;

    const auto key =
      std::make_tuple(
        definition.archiveFrequency,
        definition.kind);

    auto it = fileIndexes.find(key);

    if (it == fileIndexes.end())
    {
      ArchiveFileDescription file;

      file.frequency =
        definition.archiveFrequency;

      file.dataType = "float";

      file.name =
        definition.kind == SignalKind::Raw
          ? "raw_" + std::to_string(definition.archiveFrequency) + "Hz.dat"
          : "calculated_" + std::to_string(definition.archiveFrequency) + "Hz.dat";

      tmp.files.push_back(
        std::move(file));

      const auto index =
        tmp.files.size() - 1;

      it = fileIndexes.emplace(
        key,
        index).first;
    }

    auto& file =
      tmp.files[it->second];

    ArchiveSignal signal;
    signal.index =
      static_cast<uint32_t>(file.signalIds.size());

    signal.id = definition.id;
    signal.name = definition.name;
    signal.kind = definition.kind;

    if (definition.kind == SignalKind::Raw)
    {
      const TagInfo* tag =
        configuration.findTag(
          definition.source.tag);

      if (tag == nullptr)
        return false;

      signal.module = tag->module;
      signal.channel = tag->channel;
    }

    file.signalIds.push_back(
      std::move(signal));
  }

  description =
    std::move(tmp);

  return true;
}

}