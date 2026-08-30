#include "archivemanager.h"

namespace qds
{

bool ArchiveManager::initialize(
  const std::filesystem::path& directory,
  const ArchiveDescription& description,
  const SignalMemoryLayout& layout)
{
  m_targets.clear();

  for (const auto& file : description.files)
  {
    if (!isValidArchiveFrequency(file.frequency))
      return false;

    ArchiveTarget target;

    target.frequency =
      file.frequency;

    target.periodFrames =
      BaseFrameFrequency /
      file.frequency;

    target.values.resize(
      file.signalIds.size());

    target.signalBindings.reserve(
      file.signalIds.size());

    for (const auto& signal :
         file.signalIds)
    {
      const auto reference =
        layout.reference(signal.id);

      if (!reference.isValid())
        return false;

      ArchiveSignalBinding binding;

      binding.area =
        reference.area;

      // signal.index — индекс в архиве, layout.reference(signal.id).index — индекс в Frame.
      binding.memoryIndex =
        reference.index;

      target.signalBindings.push_back(
        binding);
    }

    DataFileHeader header;

    header.sampleFrequency =
      file.frequency;

    header.channelCount =
      static_cast<uint32_t>(
        file.signalIds.size());

    header.recordSize =
      sizeof(SampleRecordHeader) +
      header.channelCount *
        sizeof(float);

    const auto fileName =
      directory / file.name;

    if (!target.writer.open(
          fileName,
          header))
    {
      return false;
    }

    m_targets.push_back(
      std::move(target));
  }

  return true;
}

bool ArchiveManager::write(const Frame& frame)
{
  for (auto& target : m_targets)
  {
    if (frame.number.value % target.periodFrames != 0)
      continue;

    for (std::size_t i = 0;
         i < target.signalBindings.size();
         ++i)
    {
      const auto& binding = target.signalBindings[i];

      double value;

      if (binding.area == SignalMemoryArea::Raw)
        value = frame.raw().value(
          binding.memoryIndex);
      else
        value = frame.calculated().value(
          binding.memoryIndex);

      target.values[i] =
        static_cast<float>(value);
    }

    if (!target.writer.write(
          frame.timestamp,
          frame.number,
          frame.wallTime,
          target.values))
    {
      return false;
    }
  }

  return true;
}

void ArchiveManager::close()
{
  for (auto& target : m_targets)
  {
    target.writer.close();
  }
}

}
