#include "archivesignalsource.h"

#include <algorithm>
#include <cmath>
#include <vector>
#include <optional>
#include <array>

#include <QDebug>

namespace qds
{

ArchiveSignalSource::ArchiveSignalSource(
  ArchiveReader& reader)
  : m_reader(reader)
{
}

std::optional<ArchiveSignalLocation>
ArchiveSignalSource::findSignal(
  SignalId signalId) const
{
  if (!m_reader.isOpen())
    return std::nullopt;

  const auto& description =
    m_reader.description();

  for (std::size_t fileIndex = 0;
       fileIndex < description.files.size();
       ++fileIndex)
  {
    const auto& file =
      description.files[fileIndex];

    for (const auto& signal : file.signalIds)
    {
      if (signal.id == signalId)
      {
        return ArchiveSignalLocation{
          fileIndex,
          signal.index
        };
      }
    }
  }

  return std::nullopt;
}

bool ArchiveSignalSource::readRange(
  SignalId signalId,
  Timestamp firstTimestamp,
  Timestamp lastTimestamp,
  std::size_t maxRecords, // ограничивает общее число возвращаемых точек, а не размер блока чтения
  ArchiveSignalPoints& points)
{
  points.clear();

  if (firstTimestamp > lastTimestamp || maxRecords == 0)
    return false;

  const auto location = findSignal(signalId);

  if (!location)
    return false;

  const auto* header =
    m_reader.fileHeader(location->fileIndex);

  if (!header)
    return false;

  if (location->channelIndex >= header->channelCount)
    return false;

  uint64_t recordIndex = 0;

  if (!m_reader.findRecord(
        location->fileIndex,
        firstTimestamp,
        recordIndex))
  {
    return false;
  }

  constexpr std::size_t BlockSize = 4096;

  std::vector<ArchiveSample> samples;

  while (recordIndex < header->recordCount &&
         points.size() < maxRecords)
  {
    const auto remaining =
      maxRecords - points.size();

    const auto count =
      std::min(BlockSize, remaining);

    if (!m_reader.readRecords(
          location->fileIndex,
          recordIndex,
          count,
          samples))
    {
      points.clear();
      return false;
    }

    if (samples.empty())
      break;

    for (const auto& sample : samples)
    {
      if (sample.timestamp > lastTimestamp)
        return true;

      if (location->channelIndex >= sample.values.size())
      {
        points.clear();
        return false;
      }

      points.push_back({
        .timestamp = sample.timestamp,
        .wallTime = sample.wallTime,
        .frameNumber = sample.frameNumber,
        .value = static_cast<double>(
          sample.values[location->channelIndex])
      });

      if (points.size() >= maxRecords)
        break;
    }

    recordIndex += samples.size();
  }

  return true;
}

bool ArchiveSignalSource::readPlot(
  SignalId signalId,
  Timestamp firstTimestamp,
  Timestamp lastTimestamp,
  std::size_t maxPoints,
  ArchiveSignalPoints& points,
  const std::atomic_bool *cancel)
{
  points.clear();

  if (firstTimestamp > lastTimestamp || maxPoints < 6)
    return false;

  const auto location = findSignal(signalId);

  if (!location)
    return false;

  const auto* header =
    m_reader.fileHeader(location->fileIndex);

  if (!header ||
      location->channelIndex >= header->channelCount)
    return false;

  uint64_t recordIndex = 0;

  if (!m_reader.findRecord(
        location->fileIndex,
        firstTimestamp,
        recordIndex))
    return false;


  const uint64_t firstRecordIndex = recordIndex;

  std::optional<ArchiveSignalPoint> leftPoint;
  std::optional<ArchiveSignalPoint> rightPoint;

  if (firstRecordIndex > 0)
  {
    ArchiveSample sample;

    if (!m_reader.readRecord(
          location->fileIndex,
          firstRecordIndex - 1,
          sample))
    {
      return false;
    }

    if (location->channelIndex >= sample.values.size())
      return false;

    const double value =
      static_cast<double>(
        sample.values[location->channelIndex]);

    if (std::isfinite(value))
    {
      leftPoint = ArchiveSignalPoint{
        .timestamp = sample.timestamp,
        .wallTime = sample.wallTime,
        .frameNumber = sample.frameNumber,
        .value = value
      };
    }
  }


  const std::size_t bucketCount =
    (maxPoints - 2) / 4;

  // Расчёты выполняем в целых микросекундах.
  const uint64_t duration =
    lastTimestamp.value - firstTimestamp.value;

  struct Bucket
  {
    bool hasValue = false;

    ArchiveSignalPoint first;
    ArchiveSignalPoint minimum;
    ArchiveSignalPoint maximum;
    ArchiveSignalPoint last;
  };

  std::vector<Bucket> buckets(bucketCount);

  constexpr std::size_t BlockSize = 4096;

  std::vector<ArchiveSample> samples;

  while (recordIndex < header->recordCount)
  {
    if (cancel && cancel->load(std::memory_order_relaxed))
    {
      points.clear();
      return false;
    }

    if (!m_reader.readRecords(
          location->fileIndex,
          recordIndex,
          BlockSize,
          samples))
    {
      points.clear();
      return false;
    }

    if (samples.empty())
      break;

    bool finished = false;

    for (const auto& sample : samples)
    {
      if (cancel && cancel->load(std::memory_order_relaxed))
      {
        points.clear();
        return false;
      }

      if (sample.timestamp > lastTimestamp)
      {
        if (location->channelIndex >= sample.values.size())
        {
          points.clear();
          return false;
        }

        const double value =
          static_cast<double>(
            sample.values[location->channelIndex]);

        if (std::isfinite(value))
        {
          rightPoint = ArchiveSignalPoint{
            .timestamp = sample.timestamp,
            .wallTime = sample.wallTime,
            .frameNumber = sample.frameNumber,
            .value = value
          };
        }

        finished = true;
        break;
      }

      if (location->channelIndex >= sample.values.size())
      {
        points.clear();
        return false;
      }

      const double value =
        static_cast<double>(
          sample.values[location->channelIndex]);

      if (!std::isfinite(value))
        continue;

      const uint64_t offset =
        sample.timestamp.value - firstTimestamp.value;

      // Индекс временного интервала без переполнения
      // при умножении больших uint64_t.
      const std::size_t bucketIndex =
        duration == 0
          ? 0
          : std::min(
              bucketCount - 1,
              static_cast<std::size_t>(
                static_cast<long double>(offset) *
                bucketCount /
                (static_cast<long double>(duration) + 1.0L)));

      ArchiveSignalPoint point{
        .timestamp = sample.timestamp,
        .wallTime = sample.wallTime,
        .frameNumber = sample.frameNumber,
        .value = value
      };

      auto& bucket = buckets[bucketIndex];

      if (!bucket.hasValue)
      {
        bucket.hasValue = true;

        bucket.first = point;
        bucket.minimum = point;
        bucket.maximum = point;
        bucket.last = point;
      }
      else
      {
        if (point.value < bucket.minimum.value)
          bucket.minimum = point;

        if (point.value > bucket.maximum.value)
          bucket.maximum = point;

        bucket.last = point;
      }
    }

    if (finished)
      break;

    recordIndex += samples.size();
  }

  if (cancel && cancel->load(std::memory_order_relaxed))
    return false;

  points.reserve(maxPoints);

  if (leftPoint)
    points.push_back(*leftPoint);

  for (const auto& bucket : buckets)
  {
    if (!bucket.hasValue)
      continue;

    std::array<ArchiveSignalPoint, 4> candidates{
      bucket.first,
      bucket.minimum,
      bucket.maximum,
      bucket.last
    };

    std::sort(
      candidates.begin(),
      candidates.end(),
      [](const auto& a, const auto& b)
      {
        return a.timestamp < b.timestamp;
      });

    for (const auto& point : candidates)
    {
      if (!points.empty() &&
          points.back().timestamp == point.timestamp &&
          points.back().frameNumber == point.frameNumber)
      {
        continue;
      }

      points.push_back(point);
    }
  }

  if (rightPoint)
    points.push_back(*rightPoint);

  return true;
}

bool ArchiveSignalSource::readPlotSegments(
  SignalId signalId,
  Timestamp firstTimestamp,
  Timestamp lastTimestamp,
  std::size_t maxPoints,
  ArchiveSignalSegments& segments,
  const std::atomic_bool* cancel)
{
  segments.clear();

  if (firstTimestamp > lastTimestamp || maxPoints == 0)
    return false;

  const auto location = findSignal(signalId);
  if (!location)
    return false;

  const auto* header = m_reader.fileHeader(location->fileIndex);
  if (!header || header->sampleFrequency == 0)
    return false;

  const uint64_t expectedInterval =
    1'000'000ULL / header->sampleFrequency;

  if (expectedInterval == 0)
    return false;

  const uint64_t gapThreshold =
    expectedInterval + expectedInterval / 2;

  uint64_t recordIndex = 0;

  if (!m_reader.findRecord(
        location->fileIndex,
        firstTimestamp,
        recordIndex))
  {
    return false;
  }

  // Начинаем с предыдущей записи.
  if (recordIndex > 0)
    --recordIndex;

  struct SegmentInfo
  {
    ArchiveSignalPoint first;
    ArchiveSignalPoint last;
    ArchiveSignalPoint minimum;
    ArchiveSignalPoint maximum;

    std::size_t recordCount = 0;
    std::size_t pointBudget = 0;
  };
  // функция рассчета значимости экстремума
  const auto deviation =
    [](const SegmentInfo& segment,
       const ArchiveSignalPoint& point) -> double
  {
    const auto t0 = segment.first.timestamp.value;
    const auto t1 = segment.last.timestamp.value;
    const auto t = point.timestamp.value;

    if (t1 <= t0)
      return 0.0;

    const double ratio =
      static_cast<double>(t - t0) /
      static_cast<double>(t1 - t0);

    const double expected =
      segment.first.value +
      (segment.last.value - segment.first.value) * ratio;

    return std::abs(point.value - expected);
  };

  std::vector<SegmentInfo> segmentInfo;

  std::optional<Timestamp> previousTimestamp;

  const bool success = forEachSample(
    location->fileIndex,
    recordIndex,
    lastTimestamp,
    cancel,
    true,
    [&](const ArchiveSample& sample) -> bool
    {
      if (location->channelIndex >= sample.values.size())
        return false;

      const double value =
        static_cast<double>(
          sample.values[location->channelIndex]);

      if (!std::isfinite(value))
      {
        previousTimestamp.reset();
        return true;
      }

      const ArchiveSignalPoint point{
        .timestamp = sample.timestamp,
        .wallTime = sample.wallTime,
        .frameNumber = sample.frameNumber,
        .value = value
      };

      bool startsSegment = !previousTimestamp;

      if (previousTimestamp)
      {
        if (sample.timestamp <= *previousTimestamp ||
            sample.timestamp.value - previousTimestamp->value > gapThreshold)
        {
          startsSegment = true;
        }
      }

      if (startsSegment)
      {
        segmentInfo.push_back({
          .first = point,
          .last = point,
          .minimum = point,
          .maximum = point,
          .recordCount = 1
        });
      }
      else
      {
        auto& segment = segmentInfo.back();

        segment.last = point;

        if (point.value < segment.minimum.value)
          segment.minimum = point;

        if (point.value > segment.maximum.value)
          segment.maximum = point;

        ++segment.recordCount;
      }

      previousTimestamp = sample.timestamp;

      return true;
    });

  if (!success)
    return false;

  std::size_t minimumPoints = 0;

  for (const auto& segment : segmentInfo)
  {
    minimumPoints +=
      std::min(segment.recordCount, std::size_t{2});
  }

  if (minimumPoints > maxPoints)
    return false;

  // 2 проход =====================================
  std::size_t totalRecords = 0;

  for (const auto& info : segmentInfo)
    totalRecords += info.recordCount;

  if (totalRecords <= maxPoints)
  {
    segments.clear();
    segments.reserve(segmentInfo.size());

    for (const auto& info : segmentInfo)
    {
      ArchiveSignalSegment segment;
      segment.points.reserve(info.recordCount);
      segments.push_back(std::move(segment));
    }

    std::size_t segmentIndex = 0;
    std::size_t segmentRecordIndex = 0;

    const bool success = forEachSample(
      location->fileIndex,
      recordIndex,
      lastTimestamp,
      cancel,
      true,
      [&](const ArchiveSample& sample) -> bool
      {
        if (location->channelIndex >= sample.values.size())
          return false;

        const double value =
          static_cast<double>(
            sample.values[location->channelIndex]);

        if (!std::isfinite(value))
          return true;

        if (segmentIndex >= segmentInfo.size())
          return false;

        segments[segmentIndex].points.push_back({
          .timestamp = sample.timestamp,
          .wallTime = sample.wallTime,
          .frameNumber = sample.frameNumber,
          .value = value
        });

        ++segmentRecordIndex;

        if (segmentRecordIndex == segmentInfo[segmentIndex].recordCount)
        {
          ++segmentIndex;
          segmentRecordIndex = 0;
        }

        return true;
      });

    if (!success ||
        segmentIndex != segmentInfo.size() ||
        segmentRecordIndex != 0)
    {
      segments.clear();
      return false;
    }

    return true;
  }

  // Обработаем распределение бюджета
  // Минимальный бюджет: первая и последняя точки.
  std::size_t remainingPoints = maxPoints - minimumPoints;
  std::size_t totalInterior = 0;

  for (auto& info : segmentInfo)
  {
    info.pointBudget =
      std::min(info.recordCount, std::size_t{2});

    totalInterior += info.recordCount - info.pointBudget;
  }

  // Пропорциональное распределение дополнительных точек.
  struct BudgetRemainder
  {
    std::size_t segmentIndex = 0;
    long double remainder = 0.0L;
  };

  std::vector<BudgetRemainder> remainders;
  remainders.reserve(segmentInfo.size());

  const std::size_t extraBudget = remainingPoints;

  for (std::size_t i = 0; i < segmentInfo.size(); ++i)
  {
    auto& info = segmentInfo[i];

    const std::size_t interior =
      info.recordCount - info.pointBudget;

    // Расчёт целой части и остатка пропорции.
    // Используем long double, чтобы избежать
    // переполнения при умножении size_t.
    const long double share =
      static_cast<long double>(extraBudget) *
      static_cast<long double>(interior) /
      static_cast<long double>(totalInterior);

    const std::size_t extra =
      std::min(interior, static_cast<std::size_t>(share));

    info.pointBudget += extra;
    remainingPoints -= extra;

    remainders.push_back({
      .segmentIndex = i,
      .remainder = share - std::floor(share)
    });
  }

  // Оставшиеся точки распределяем по наибольшим
  // дробным остаткам.
  std::sort(
    remainders.begin(),
    remainders.end(),
    [](const auto& a, const auto& b)
    {
      if (a.remainder != b.remainder)
        return a.remainder > b.remainder;

      return a.segmentIndex < b.segmentIndex;
    });

  for (const auto& item : remainders)
  {
    if (remainingPoints == 0)
      break;

    auto& info = segmentInfo[item.segmentIndex];

    if (info.pointBudget >= info.recordCount)
      continue;

    ++info.pointBudget;
    --remainingPoints;
  }

  // Добавляем структуру корзины
  struct PlotBucket
  {
    ArchiveSignalPoint first;
    ArchiveSignalPoint last;
    ArchiveSignalPoint minimum;
    ArchiveSignalPoint maximum;

    bool initialized = false;

    void add(const ArchiveSignalPoint& point)
    {
      if (!initialized)
      {
        first = point;
        last = point;
        minimum = point;
        maximum = point;
        initialized = true;
        return;
      }

      last = point;

      if (point.value < minimum.value)
        minimum = point;

      if (point.value > maximum.value)
        maximum = point;
    }
  };


  segments.clear();
  segments.reserve(segmentInfo.size());

  struct SegmentBuckets
  {
    std::vector<PlotBucket> buckets;
  };

  std::vector<SegmentBuckets> segmentBuckets;
  segmentBuckets.reserve(segmentInfo.size());

  for (const auto& info : segmentInfo)
  {
    ArchiveSignalSegment segment;
    segment.points.reserve(info.pointBudget);
    segments.push_back(std::move(segment));

    SegmentBuckets data;

    if (info.pointBudget > 2)
    {
      const std::size_t extraBudget = info.pointBudget - 2;

      // Одна корзина может дать минимум и максимум.
      const std::size_t bucketCount =
        std::max(std::size_t{1}, (extraBudget + 1) / 2);

      data.buckets.resize(bucketCount);
    }

    segmentBuckets.push_back(std::move(data));
  }

  std::size_t segmentIndex = 0;
  std::size_t segmentRecordIndex = 0;

  const bool readSuccess = forEachSample(
    location->fileIndex,
    recordIndex,
    lastTimestamp,
    cancel,
    true,
    [&](const ArchiveSample& sample) -> bool
    {
      if (location->channelIndex >= sample.values.size())
        return false;

      const double value =
        static_cast<double>(
          sample.values[location->channelIndex]);

      if (!std::isfinite(value))
        return true;

      if (segmentIndex >= segmentInfo.size())
        return false;

      const auto& info = segmentInfo[segmentIndex];
      auto& segment = segments[segmentIndex];
      auto& buckets = segmentBuckets[segmentIndex].buckets;

      const ArchiveSignalPoint point{
        .timestamp = sample.timestamp,
        .wallTime = sample.wallTime,
        .frameNumber = sample.frameNumber,
        .value = value
      };

      const std::size_t index = segmentRecordIndex;

      if (index == 0)
      {
        segment.points.push_back(point);
      }
      else if (index + 1 == info.recordCount)
      {
        if (info.pointBudget > 1)
          segment.points.push_back(point);
      }
      else if (!buckets.empty())
      {
        const std::size_t interiorCount = info.recordCount - 2;

        const std::size_t position = index - 1;

        const std::size_t bucketIndex =
          position / interiorCount * buckets.size() +
          (position % interiorCount) * buckets.size() / interiorCount;

        buckets[bucketIndex].add(point);
      }

      ++segmentRecordIndex;

      if (segmentRecordIndex == info.recordCount)
      {
        ++segmentIndex;
        segmentRecordIndex = 0;
      }

      return true;
    });

  if (!readSuccess ||
      segmentIndex != segmentInfo.size() ||
      segmentRecordIndex != 0)
  {
    segments.clear();
    return false;
  }


  for (std::size_t i = 0; i < segments.size(); ++i)
  {
    auto& points = segments[i].points;
    const auto& info = segmentInfo[i];
    const auto& buckets = segmentBuckets[i].buckets;

    if (buckets.empty())
      continue;

    std::vector<ArchiveSignalPoint> candidates;
    candidates.reserve(buckets.size() * 4);

    for (const auto& bucket : buckets)
    {
      if (!bucket.initialized)
        continue;

      candidates.push_back(bucket.first);
      candidates.push_back(bucket.minimum);
      candidates.push_back(bucket.maximum);
      candidates.push_back(bucket.last);
    }

    std::sort(
      candidates.begin(),
      candidates.end(),
      [](const auto& a, const auto& b)
      {
        return a.timestamp < b.timestamp;
      });

    candidates.erase(
      std::unique(
        candidates.begin(),
        candidates.end(),
        [](const auto& a, const auto& b)
        {
          return a.timestamp == b.timestamp;
        }),
      candidates.end());

    const std::size_t extraBudget = info.pointBudget - 2;

    while (candidates.size() > extraBudget)
    {
      std::size_t removeIndex = 0;
      double minimumDeviation =
        std::numeric_limits<double>::infinity();

      for (std::size_t j = 0; j < candidates.size(); ++j)
      {
        const auto& previous =
          j == 0 ? points.front() : candidates[j - 1];

        const auto& next =
          j + 1 == candidates.size()
            ? points.back()
            : candidates[j + 1];

        const auto& current = candidates[j];

        const double duration =
          static_cast<double>(
            next.timestamp.value - previous.timestamp.value);

        if (duration <= 0.0)
          continue;

        const double ratio =
          static_cast<double>(
            current.timestamp.value - previous.timestamp.value) /
          duration;

        const double expected =
          previous.value +
          (next.value - previous.value) * ratio;

        const double deviation =
          std::abs(current.value - expected);

        if (deviation < minimumDeviation)
        {
          minimumDeviation = deviation;
          removeIndex = j;
        }
      }

      candidates.erase(candidates.begin() + removeIndex);
    }

    points.insert(
      points.end(),
      candidates.begin(),
      candidates.end());

    std::sort(
      points.begin(),
      points.end(),
      [](const auto& a, const auto& b)
      {
        return a.timestamp < b.timestamp;
      });
  }

  return true;
}

bool ArchiveSignalSource::timeRange(
  SignalId signalId,
  Timestamp& firstTimestamp,
  Timestamp& lastTimestamp) const
{
  const auto location = findSignal(signalId);

  if (!location)
    return false;

  const auto header =
    m_reader.fileHeader(location->fileIndex);

  if (!header || header->recordCount == 0)
    return false;

  std::vector<ArchiveSample> samples;

  if (!m_reader.readRecords(
        location->fileIndex,
        0,
        1,
        samples) ||
      samples.size() != 1)
  {
    return false;
  }

  const Timestamp first = samples.front().timestamp;

  if (!m_reader.readRecords(
        location->fileIndex,
        header->recordCount - 1,
        1,
        samples) ||
      samples.size() != 1)
  {
    return false;
  }

  const Timestamp last = samples.front().timestamp;

  if (last < first)
    return false;

  firstTimestamp = first;
  lastTimestamp = last;

  return true;
}

}