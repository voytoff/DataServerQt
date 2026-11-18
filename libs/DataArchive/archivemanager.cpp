#include "archivemanager.h"
#include "archivesegment.h"

#include <utility>
#include <format>

namespace qds
{

ArchiveManager::ArchiveManager(
  ILogger& logger)
  : m_logger(logger)
{
}

bool ArchiveManager::initialize(
  const std::filesystem::path& directory,
  const ArchiveDescription& description,
  const SignalMemoryLayout& layout,
  uint32_t segmentDuration)
{
  close();

  m_targets.clear();
  m_directory.clear();
  m_currentSegment.reset();
  m_firstFrameNumber = 0;
  m_segmentFrameCount = 0;
  m_initialized = false;

  if (segmentDuration == 0)
    return false;

  m_directory = directory;

  m_segmentFrameCount =
    uint64_t(segmentDuration) *
    BaseFrameFrequency;

  for (const auto& file : description.files)
  {
    if (!isValidArchiveFrequency(file.frequency))
    {
      close();
      return false;
    }

    ArchiveTarget target;

    target.fileName = file.name;

    target.frequency = file.frequency;

    target.periodFrames =
      BaseFrameFrequency / file.frequency;

    target.values.resize(
      file.signalIds.size());

    target.signalBindings.reserve(
      file.signalIds.size());

    for (const auto& signal : file.signalIds)
    {
      const auto reference =
        layout.reference(signal.id);

      if (!reference.isValid())
      {
        close();
        return false;
      }

      ArchiveSignalBinding binding;

      binding.area = reference.area;
      binding.memoryIndex = reference.index;

      target.signalBindings.push_back(binding);
    }

    target.header.sampleFrequency =
      file.frequency;

    target.header.channelCount =
      static_cast<uint32_t>(
        file.signalIds.size());

    target.header.recordSize =
      sizeof(SampleRecordHeader) +
      target.header.channelCount * sizeof(float);

    m_targets.push_back(std::move(target));
  }

  if (m_targets.empty())
    return false;

  m_initialized = true;

  return true;
}

bool ArchiveManager::openSegment(uint64_t index)
{
  for (auto& target : m_targets)
    target.writer.close();

  for (auto& target : m_targets)
  {
    const auto fileName =
      m_directory /
      segmentFileName(target.fileName, index);

    if (!target.writer.open(fileName, target.header))
    {
      for (auto& opened : m_targets)
        opened.writer.close();

      m_logger.error(
        std::format(
          "Архив: ошибка открытия сегмента {:04}, файл {}",
          index,
          fileName.string()));

      return false;
    }
  }

  m_currentSegment = index;

  m_logger.info(
    std::format(
      "Архив: сегмент {:04} открыт, граница кадра {}",
      index,
      m_firstFrameNumber +
        (index - 1) * m_segmentFrameCount));

  return true;
}

bool ArchiveManager::write(const Frame& frame)
{
  if (!m_initialized)
    return false;

  const uint64_t frameNumber = frame.number.value;

  if (m_previousFrameNumber)
  {
    if (frameNumber <= *m_previousFrameNumber ||
        frameNumber - *m_previousFrameNumber != 1)
    {
      close();
      return false;
    }
  }

  if (!m_currentSegment)
    m_firstFrameNumber = frameNumber;

  const uint64_t elapsedFrames =
    frameNumber - m_firstFrameNumber;

  const uint64_t segmentIndex =
    elapsedFrames / m_segmentFrameCount + 1;

  if (!m_currentSegment ||
      *m_currentSegment != segmentIndex)
  {
    if (!openSegment(segmentIndex))
    {
      close();
      return false;
    }
  }

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
      {
        value = frame.raw().value(
          binding.memoryIndex);
      }
      else
      {
        value = frame.calculated().value(
          binding.memoryIndex);
      }

      target.values[i] =
        static_cast<float>(value);
    }

    if (!target.writer.write(
          frame.timestamp,
          frame.number,
          frame.wallTime,
          target.values))
    {
      close();
      return false;
    }
  }

  m_previousFrameNumber = frameNumber;

  return true;
}

void ArchiveManager::close()
{
  for (auto& target : m_targets)
    target.writer.close();

  m_initialized = false;
  m_currentSegment.reset();
  m_previousFrameNumber.reset();
}

}