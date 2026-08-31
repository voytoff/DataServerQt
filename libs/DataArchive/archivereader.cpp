#include "archivereader.h"

#include <string>
#include <format>
#include <fstream>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QByteArray>

namespace qds
{

bool ArchiveReader::open(const std::filesystem::path &directory)
{
  if (m_open)
    return false;

  ArchiveDescription tmp;
  std::vector<File> tmpFiles;

  const auto path = std::format("{0}/{1}", directory.string(), DescriptionFileName);
  std::fstream jsonFile(
    path,
    std::ios::in);

  if (!jsonFile)
    return false;

  std::string json = std::string(
    (std::istreambuf_iterator<char>(jsonFile)),
    std::istreambuf_iterator<char>()
    );

  if (json.empty())
    return false;

  QByteArray jsonData = QByteArray::fromStdString(json);

  QJsonParseError error;

  const QJsonDocument document =
    QJsonDocument::fromJson(
      jsonData,
      &error);

  if (error.error != QJsonParseError::NoError)
    return false;

  if (!document.isObject())
    return false;

  const QJsonObject root =
    document.object();

  if(root.empty())
    return false;

  tmp.version =
    root["version"].toInt();

  if (tmp.version != ArchiveDescriptionVersion)
    return false;

  if (!root["files"].isArray())
    return false;

  const QJsonArray &files =
    root["files"].toArray();

  if (files.isEmpty())
    return false;

  for (const auto &file : files)
  {
    if (!file.isObject())
      return false;

    ArchiveFileDescription description;
    const auto &fileObj = file.toObject();

    description.frequency =
      fileObj["frequency"].toInt();

    if (!isValidArchiveFrequency(
          description.frequency))
    {
      return false;
    }

    description.dataType =
      fileObj["dataType"].toString().toStdString();

    if (description.dataType != "float")
      return false;

    description.name =
      fileObj["name"].toString().toStdString();

    if (!fileObj["signals"].isArray())
      return false;

    const QJsonArray &signalIds = fileObj["signals"].toArray();

    for (const auto &item : signalIds)
    {
      if (!item.isObject())
        return false;

      const auto &signalObj = item.toObject();

      ArchiveSignal signal;

      signal.index =
        static_cast<uint32_t>(signalObj["index"].toInt());

      signal.id =
        SignalId{static_cast<uint32_t>(signalObj["id"].toInt())};

      signal.name =
        signalObj["name"].toString().toStdString();

      signal.kind =
        signalKindFromString(
          signalObj["kind"].toString().toStdString());

      if (signal.kind == SignalKind::Raw)
      {
        if (!signalObj.contains("channel") ||
            !signalObj.contains("module"))
        {
          return false;
        }

        signal.channel =
          ChannelId{static_cast<uint32_t>(signalObj["channel"].toInt())};

        signal.module =
          ModuleId{static_cast<uint32_t>(signalObj["module"].toInt())};
      }
      else if (signal.kind == SignalKind::Calculated)
      {
        if (signalObj.contains("channel") ||
            signalObj.contains("module"))
        {
          return false;
        }
      }
      else
        return false;

      description.signalIds.push_back(
        std::move(signal));
    }

    if (description.signalIds.empty())
      return false;

    File reader;
    reader.description = description;
    reader.path = directory / description.name;

    tmp.files.push_back(
      std::move(description));

    tmpFiles.push_back(
      std::move(reader));
  }

  m_directory = directory;
  m_description = std::move(tmp);
  m_files = std::move(tmpFiles);

  m_open = true;
  return true;
}

void ArchiveReader::close() noexcept
{
  for (auto &file : m_files)
    file.archive.close();

  m_open = false;
}

bool ArchiveReader::isOpen() const noexcept
{
  return m_open;
}

const ArchiveDescription &ArchiveReader::description() const noexcept
{
  return m_description;
}

std::size_t ArchiveReader::fileCount() const noexcept
{
  return m_files.size();
}

const ArchiveFileDescription &ArchiveReader::fileDescription(std::size_t index) const
{
  assert(index < m_files.size()); // ??? m_open == false;

  return m_files.at(index).description;
}

bool ArchiveReader::read(std::size_t fileIndex, ArchiveSample& sample)
{
  assert(fileIndex < m_files.size());

  if (!m_open)
    return false;

  if (!ensureOpen(fileIndex))
    return false;

  auto &file =
    m_files[fileIndex].archive;

  const auto channelCount =
    file.header().channelCount;

  SampleRecordHeader header;
  if (!file.readObject(header))
    return false;

  sample.frameNumber = FrameNumber{header.frameNumber};
  sample.timestamp = Timestamp{header.timestamp};
  sample.wallTime = WallClockTime{header.wallTime};

  sample.values.resize(channelCount);

  if (!file.readArray(
        sample.values.data(),
        sample.values.size()))
    return false;

  return true;
}

bool ArchiveReader::readFrame(
  std::size_t fileIndex,
  FrameNumber frameNumber,
  ArchiveSample& sample)
{
  assert(fileIndex < m_files.size());

  if (!m_open)
    return false;

  if (!ensureOpen(fileIndex))
    return false;

  auto& file =
    m_files[fileIndex].archive;

  const auto& header =
    file.header();

  const uint64_t periodFrames =
    BaseFrameFrequency /
    header.sampleFrequency;

  if (frameNumber.value == 0 ||
      frameNumber.value % periodFrames != 0)
  {
    return false;
  }

  const uint64_t recordIndex =
    frameNumber.value / periodFrames - 1;

  const uint64_t position =
    header.headerSize +
    recordIndex * header.recordSize;

  if (position + header.recordSize >
      file.fileSize())
  {
    return false;
  }

  if (!file.seek(position))
    return false;

  if (!read(fileIndex, sample))
    return false;

  return sample.frameNumber == frameNumber;
}

bool ArchiveReader::ensureOpen(
  std::size_t fileIndex)
{
  auto& file = m_files[fileIndex];

  if (file.archive.isOpen())
    return true;

  if (!file.archive.open(
    file.path,
    OpenMode::Read))
  {
    return false;
  }

  return file.archive.header().isValid();
}

}
