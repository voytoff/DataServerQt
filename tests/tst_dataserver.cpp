#include "tst_dataserver.h"
#include "archivedescriptionbuilder.h"
#include "archivedescriptionwriter.h"
#include "archivemanager.h"
#include "archivereader.h"
#include "configurationrepository.h"
#include "datasourcefactory.h"
#include "qds/db.h"
#include "logger.h"
#include "qds/testarchiveframewriter.h"
#include "qds/testdatastreamsource.h"
#include "qds/teststreamingdatasource.h"
#include "testlogger.h"
#include "failingarchivewriter.h"
#include "failingdatasource.h"
#include "failoncearchivewriter.h"
#include "fakedatasource.h"
#include "fakeschedulerclock.h"
#include "testlogger.h"
#include "protocol/publishheader.h"
#include "runtimesystem.h"
#include "systembuilder.h"
#include "systemconfiguration.h"
#include "qds/testarchivewriter.h"
#include "testdatasource.h"
#include "qds/testpublisher.h"
#include "testsrv.h"
#include "dataserver.h"
#include "qds/udp.h"
#include <qtestsupport_core.h>

tst_dataserver::tst_dataserver() { }
tst_dataserver::~tst_dataserver() = default;


void tst_dataserver::test_systemBuilder_success()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);
  cfg.addSignalDefinition({.id = {24}, .name = "D", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formula = "A + A", .formulaId = {2}, .dependencies = {{17}, {17}}});

  DataStreamSourceFactory factory;

  QVERIFY(
    factory.registerType(
      ModuleType::Test,
      [](const ModuleRuntimeConfiguration&,
         IClock&,
         IDataBlockSink&,
         IDataStreamEventSink&)
      {
        return std::make_unique<
          TestDataStreamSource>();
      }));

  TestArchiveWriter archive;
  TestPublisher publisher;
  FakeSchedulerClock clock;
  TestLogger logger;

  CalibrationRepository cr;

  SystemBuilder builder;

  auto runtime =
    builder.build(
      cfg,
      factory,
      cr,
      clock,
      archive,
      logger);

  const auto* c =
    cfg.findSignalDefinition(SignalId{23});
  const auto* d =
    cfg.findSignalDefinition(SignalId{24});

  QCOMPARE(
    c->formulaId,
    FormulaId{23});

  QCOMPARE(
    c->dependencies.size(),
    std::size_t(2));

  QCOMPARE(
    c->dependencies[0],
    SignalId{17});

  QCOMPARE(
    c->dependencies[1],
    SignalId{4});

  QCOMPARE(
    d->formulaId,
    FormulaId{24});

  QCOMPARE(
    d->dependencies.size(),
    std::size_t(1));

  QCOMPARE(
    d->dependencies[0],
    SignalId{17});

  QCOMPARE(
    runtime->layout.rawSignalCount(),
    2u);

  QCOMPARE(
    runtime->layout.calculatedSignalCount(),
    4u);

  QCOMPARE(
    runtime->formulas.size(),
    std::size_t(4));

  QCOMPARE(
    runtime->calculationPlan.size(),
    std::size_t(4));

  QCOMPARE(
    runtime->dataSources.size(),
    std::size_t(1));

  QVERIFY(
    runtime->signalProcessor != nullptr);

  QVERIFY(
    runtime->engine != nullptr);
}

void tst_dataserver::test_systemBuilder_buildRuntime()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(
      ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(
    factory.registerType(
      ModuleType::Test,
      [](const ModuleRuntimeConfiguration&,
         IClock&,
         IDataBlockSink&,
         IDataStreamEventSink&)
      {
        return std::make_unique<
          TestDataStreamSource>();
      }));

  TestArchiveFrameWriter archive;
  TestPublisher publisher;
  FakeSchedulerClock clock;
  TestLogger logger;

  CalibrationRepository cr;

  SystemBuilder builder;

  auto runtime =
    builder.build(
      cfg,
      factory,
      cr,
      clock,
      archive,
      logger);

  QVERIFY(runtime);

  QVERIFY(runtime->signalProcessor);
  QVERIFY(runtime->streamReader);
  QVERIFY(runtime->frameAssembler);
  QVERIFY(runtime->streamProcessor);
  QVERIFY(runtime->streamWorker);
  QVERIFY(runtime->engine);

  QCOMPARE(
    runtime->dataSources.size(),
    cfg.modules().size());

  QVERIFY(
    !runtime->dataSources.isRunning());

  QVERIFY(
    !runtime->streamWorker->isRunning());

  QVERIFY(
    !runtime->engine->isRunning());

  QVERIFY(
    runtime->engine->initialize(
      runtime->buffers,
      publisher));

  QVERIFY(
    !runtime->engine->isRunning());

  QVERIFY(
    runtime->engine->start());

  QVERIFY(
    runtime->engine->isRunning());

  QVERIFY(
    runtime->engine->process());

  runtime->engine->stop();

  QVERIFY(
    !runtime->engine->isRunning());
}

void tst_dataserver::test_systemBuilder_failErrorFormula()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(
      ModuleType::Test);

  cfg.addSignalDefinition({
    .id = {30},
    .name = "D",
    .kind = SignalKind::Calculated,
    .formula = "unknown + C",
    .formulaId = {2},
    .dependencies = {{4}, {23}}
  });

  DataStreamSourceFactory factory;

  QVERIFY(
    factory.registerType(
      ModuleType::Test,
      [](const ModuleRuntimeConfiguration&,
         IClock&,
         IDataBlockSink&,
         IDataStreamEventSink&)
      {
        return std::make_unique<
          TestDataStreamSource>();
      }));

  TestArchiveFrameWriter archive;
  FakeSchedulerClock clock;
  TestLogger logger;

  CalibrationRepository cr;

  SystemBuilder builder;

  auto runtime =
    builder.build(
      cfg,
      factory,
      cr,
      clock,
      archive,
      logger);

  QVERIFY(!runtime);
}

void tst_dataserver::test_systemBuilder_failDataSourceManager()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(
      ModuleType::LTR11);

  DataStreamSourceFactory factory;

  QVERIFY(
    factory.registerType(
      ModuleType::Test,
      [](const ModuleRuntimeConfiguration&,
         IClock&,
         IDataBlockSink&,
         IDataStreamEventSink&)
      {
        return std::make_unique<
          TestDataStreamSource>();
      }));

  TestArchiveFrameWriter archive;
  FakeSchedulerClock clock;
  TestLogger logger;

  CalibrationRepository cr;

  SystemBuilder builder;

  auto runtime =
    builder.build(
      cfg,
      factory,
      cr,
      clock,
      archive,
      logger);

  QVERIFY(!runtime);
}

void tst_dataserver::test_systemBuilder_pipeline()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(
      ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(
    factory.registerType(
      ModuleType::Test,
      [](const ModuleRuntimeConfiguration&,
         IClock&,
         IDataBlockSink&,
         IDataStreamEventSink&)
      {
        return std::make_unique<
          TestDataStreamSource>();
      }));

  TestArchiveWriter archive;
  TestPublisher publisher;
  FakeSchedulerClock clock;
  TestLogger logger;

  CalibrationRepository cr;

  SystemBuilder builder;

  auto runtime =
    builder.build(
      cfg,
      factory,
      cr,
      clock,
      archive,
      logger);

  QVERIFY(runtime);

  QVERIFY(
    runtime->engine->initialize(
      runtime->buffers,
      publisher));

  QVERIFY(
    runtime->engine->start());

  DataStreamAnchor anchor;

  anchor.module =
    ModuleId{0};

  anchor.firstFrameIndex = 0;

  anchor.startTimestamp =
    Timestamp{2};

  anchor.startWallTime =
    WallClockTime{5};

  anchor.frameRate = 1.0;

  QVERIFY(
    runtime->streamProcessor->process(
      anchor));

  for (int n = 0; n < 1000; ++n)
  {
    DataBlock block;

    block.module =
      ModuleId{0};

    block.firstFrameIndex =
      static_cast<uint64_t>(n);

    block.frameRate = 1.0;
    block.channelCount = 2;
    block.frameCount = 1;

    block.values = {
      static_cast<double>(n),
      static_cast<double>(n) * 10.0
    };

    QVERIFY(
      runtime->streamProcessor->process(
        block));

    QVERIFY(
      runtime->engine->process());

    const auto count =
      n + 1;

    const double a =
      static_cast<double>(n);

    const double b =
      static_cast<double>(n) * 10.0;

    QCOMPARE(
      archive.size(),
      static_cast<std::size_t>(count));

    QCOMPARE(
      publisher.size(),
      static_cast<std::size_t>(count));

    const auto& archived =
      archive.last();

    const auto& published =
      publisher.last();

    QCOMPARE(
      archived->raw().valueRef(0),
      a);

    QCOMPARE(
      archived->raw().valueRef(1),
      b);

    QCOMPARE(
      archived->calculated().valueRef(0),
      a);

    QCOMPARE(
      archived->calculated().valueRef(1),
      b);

    QCOMPARE(
      archived->calculated().valueRef(2),
      a + b);

    QCOMPARE(
      published->number,
      archived->number);

    QCOMPARE(
      published->timestamp,
      archived->timestamp);

    QCOMPARE(
      published->wallTime,
      archived->wallTime);

    QCOMPARE(
      published->raw().valueRef(0),
      archived->raw().valueRef(0));

    QCOMPARE(
      published->raw().valueRef(1),
      archived->raw().valueRef(1));

    QCOMPARE(
      published->calculated().valueRef(0),
      archived->calculated().valueRef(0));

    QCOMPARE(
      published->calculated().valueRef(1),
      archived->calculated().valueRef(1));

    QCOMPARE(
      published->calculated().valueRef(2),
      archived->calculated().valueRef(2));
  }

  runtime->engine->stop();

  QVERIFY(
    !runtime->engine->isRunning());
}
/*
void tst_dataserver::test_dataServer_udpSubscription()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(
    factory.registerType(
      ModuleType::Test,
      [](const ModuleRuntimeConfiguration&,
         IClock&,
         IDataBlockSink&,
         IDataStreamEventSink&)
      {
        return std::make_unique<
          TestDataStreamSource>();
      }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(ds.start());

  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  // ------------------------------------------------------------
  // Subscribe
  // ------------------------------------------------------------

  PacketWriter writer;
  writer.begin(
    PacketType::SubscribeListRequest);

  constexpr SignalId signalIds[]
    {
      {17},
      {4},
      {23}
    };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount =
    std::size(signalIds);

  writer.write(req);

  writer.writeArray(
    signalIds,
    std::size(signalIds));

  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));

  // ------------------------------------------------------------
  // SubscribeResponse
  // ------------------------------------------------------------

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  QByteArray data;
  data.resize(
    client.pendingDatagramSize());

  client.readDatagram(
    data.data(),
    data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(
      data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeResponse);

  SubscribeResponse response;

  QVERIFY(reader.read(response));

  QCOMPARE(
    reader.remaining(),
    std::size_t(0));

  QCOMPARE(
    response.result,
    SubscribeResult::Ok);

  QCOMPARE(
    response.id,
    SubscriptionId{1});

  // ------------------------------------------------------------
  // LiveData #1
  // ------------------------------------------------------------

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  data.resize(
    client.pendingDatagramSize());

  client.readDatagram(
    data.data(),
    data.size());

  reader.clear();

  reader.append(
    reinterpret_cast<const std::byte*>(
      data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::LiveData);

  PublishHeader ldh;

  QVERIFY(reader.read(ldh));

  QCOMPARE(
    ldh.subscriptionId,
    SubscriptionId{1});

  QCOMPARE(
    ldh.sequence,
    1u);

  QVERIFY(
    ldh.timestamp > 0u);

  QCOMPARE(
    ldh.valueCount,
    3u);

  std::array<Sample, 3> samples{};

  QVERIFY(
    reader.readArray(
      samples.data(),
      samples.size()));

  QCOMPARE(
    reader.remaining(),
    std::size_t(0));

  QCOMPARE(samples[0].value + samples[1].value, samples[2].value);

  // ------------------------------------------------------------
  // LiveData #2
  // ------------------------------------------------------------

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  data.resize(
    client.pendingDatagramSize());

  client.readDatagram(
    data.data(),
    data.size());

  reader.clear();

  reader.append(
    reinterpret_cast<const std::byte*>(
      data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::LiveData);

  QVERIFY(reader.read(ldh));

  QCOMPARE(
    ldh.subscriptionId,
    SubscriptionId{1});

  QCOMPARE(
    ldh.sequence,
    2u);

  QVERIFY(
    ldh.timestamp > 0u);

  QCOMPARE(
    ldh.valueCount,
    3u);

  QVERIFY(
    reader.readArray(
      samples.data(),
      samples.size()));

  QCOMPARE(
    reader.remaining(),
    std::size_t(0));

  QCOMPARE(samples[0].value + samples[1].value, samples[2].value);

  // ------------------------------------------------------------
  // Stop
  // ------------------------------------------------------------

  ds.stop();
}
*/
void tst_dataserver::test_dataServer_failStart_moduleType()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::LTR11);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>();
    }));

  TestArchiveWriter archive;
  TestPublisherSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(!ds.start());

  QCOMPARE(sender.sendCount, std::size_t(0));

  QTest::qWait(100);

  QCOMPARE(sender.sendCount, std::size_t(0));
}

void tst_dataserver::test_dataServer_failSubscribe_invalidSignalId()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>();
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(ds.start());

  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  // ------------------------------------------------------------
  // Subscribe
  // ------------------------------------------------------------

  PacketWriter writer;
  writer.begin(
    PacketType::SubscribeListRequest);

  constexpr SignalId signalIds[]
    {
      {17},
      {4},
      {24} // <---
    };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount =
    std::size(signalIds);

  writer.write(req);

  writer.writeArray(
    signalIds,
    std::size(signalIds));

  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));

  // ------------------------------------------------------------
  // SubscribeResponse
  // ------------------------------------------------------------

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  QByteArray data;
  data.resize(
    client.pendingDatagramSize());

  client.readDatagram(
    data.data(),
    data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(
      data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeResponse);

  SubscribeResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::InvalidSignal);
  QCOMPARE(response.id, SubscriptionId{});

  ds.stop();
}

void tst_dataserver::test_dataServer_failSubscribe_duplicateSignalId()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>();
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(ds.start());

  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  // ------------------------------------------------------------
  // Subscribe
  // ------------------------------------------------------------

  PacketWriter writer;
  writer.begin(
    PacketType::SubscribeListRequest);

  constexpr SignalId signalIds[]
    {
      {17},
      {4},
      {4} // <---
    };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount =
    std::size(signalIds);

  writer.write(req);

  writer.writeArray(
    signalIds,
    std::size(signalIds));

  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));

  // ------------------------------------------------------------
  // SubscribeResponse
  // ------------------------------------------------------------

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  QByteArray data;
  data.resize(
    client.pendingDatagramSize());

  client.readDatagram(
    data.data(),
    data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(
      data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeResponse);

  SubscribeResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::DuplicateSignal);
  QCOMPARE(response.id, SubscriptionId{});

  ds.stop();
}

void tst_dataserver::test_dataServer_failSubscribe_invalidRate()
{
  using namespace qds;
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>();
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(ds.start());

  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  // ------------------------------------------------------------
  // Subscribe
  // ------------------------------------------------------------

  PacketWriter writer;
  writer.begin(
    PacketType::SubscribeListRequest);

  constexpr SignalId signalIds[]
    {
      {17},
      {4},
      {23}
    };

  SubscribeListRequest req;
  req.rate = static_cast<PublishRate>(0xFF); // <---
  req.signalCount =
    std::size(signalIds);

  writer.write(req);

  writer.writeArray(
    signalIds,
    std::size(signalIds));

  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));

  // ------------------------------------------------------------
  // SubscribeResponse
  // ------------------------------------------------------------

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  QByteArray data;
  data.resize(
    client.pendingDatagramSize());

  client.readDatagram(
    data.data(),
    data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(
      data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeResponse);

  SubscribeResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::InvalidRate);
  QCOMPARE(response.id, SubscriptionId{});

  ds.stop();
}

void tst_dataserver::test_dataServer_failSubscribe_emptyList()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>();
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(ds.start());

  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  // ------------------------------------------------------------
  // Subscribe
  // ------------------------------------------------------------

  PacketWriter writer;
  writer.begin(
    PacketType::SubscribeListRequest);

  constexpr SignalId signalIds[] { };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz100;
  req.signalCount = 0; // <---

  writer.write(req);

  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));

  // ------------------------------------------------------------
  // SubscribeResponse
  // ------------------------------------------------------------

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  QByteArray data;
  data.resize(
    client.pendingDatagramSize());

  client.readDatagram(
    data.data(),
    data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(
      data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeResponse);

  SubscribeResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::EmptyList);
  QCOMPARE(response.id, SubscriptionId{});

  ds.stop();
}

void tst_dataserver::test_dataServer_failSubscribe_tooManySignals()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>();
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(ds.start());

  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  // ------------------------------------------------------------
  // Subscribe
  // ------------------------------------------------------------

  PacketWriter writer;
  writer.begin(
    PacketType::SubscribeListRequest);

  constexpr SignalId signalIds[] { };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz100;
  req.signalCount = MaxSubscriptionSignals + 1; // <---

  writer.write(req);

  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));

  // ------------------------------------------------------------
  // SubscribeResponse
  // ------------------------------------------------------------

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  QByteArray data;
  data.resize(
    client.pendingDatagramSize());

  client.readDatagram(
    data.data(),
    data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(
      data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeResponse);

  SubscribeResponse response;
  QVERIFY(reader.read(response));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response.result, SubscribeResult::TooManySignals);
  QCOMPARE(response.id, SubscriptionId{});

  ds.stop();
}

void tst_dataserver::test_dataServer_unsubscribe_ok()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration& configuration,
       IClock&,
       IDataBlockSink& blockSink,
       IDataStreamEventSink& eventSink)
    {
      return std::make_unique<TestStreamingDataSource>(
        configuration,
        blockSink,
        eventSink);
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(ds.start());

  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  // ------------------------------------------------------------
  // Subscribe
  // ------------------------------------------------------------

  PacketWriter writer;
  writer.begin(
    PacketType::SubscribeListRequest);

  constexpr SignalId signalIds[]
    {
      {17},
      {4},
      {23}
    };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount =
    std::size(signalIds);

  writer.write(req);

  writer.writeArray(
    signalIds,
    std::size(signalIds));

  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));

  // ------------------------------------------------------------
  // SubscribeResponse
  // ------------------------------------------------------------

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  QByteArray data;
  data.resize(
    client.pendingDatagramSize());

  client.readDatagram(
    data.data(),
    data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(
      data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeResponse);

  SubscribeResponse response;

  QVERIFY(reader.read(response));

  QCOMPARE(
    reader.remaining(),
    std::size_t(0));

  QCOMPARE(
    response.result,
    SubscribeResult::Ok);

  QCOMPARE(
    response.id,
    SubscriptionId{1});

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  data.resize(client.pendingDatagramSize());
  client.readDatagram(data.data(), data.size());

  reader.clear();
  reader.append(
    reinterpret_cast<const std::byte*>(data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::LiveData);

  // подписка создана, теперь попробуем ее удалить ===============

  UnsubscribeRequest req2;
  req2.id = response.id;

  writer.begin(PacketType::UnsubscribeRequest);
  writer.write(req2);

  // Отправляем
  const auto bytes2 =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(bytes2, qint64(writer.size()));

  // Читаем ответ на удаление подписки
  do {
    QTRY_VERIFY_WITH_TIMEOUT(
      client.hasPendingDatagrams(),
      2000);

    data.resize(client.pendingDatagramSize());
    client.readDatagram(data.data(), data.size());

    reader.clear();
    reader.append(
      reinterpret_cast<const std::byte*>(data.constData()),
      data.size());

    QVERIFY(reader.nextPacket());
  } while (reader.packetType() != PacketType::UnsubscribeResponse);

  QCOMPARE(reader.packetType(), PacketType::UnsubscribeResponse);

  UnsubscribeResponse response2;
  QVERIFY(reader.read(response2));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response2.result, UnsubscribeResult::Ok);


  // Отбрасываем всё, что уже находилось в UDP-очереди
  while (client.hasPendingDatagrams())
  {
    data.resize(client.pendingDatagramSize());
    client.readDatagram(data.data(), data.size());
  }

  // Теперь в течение некоторого времени новых пакетов
  // от этой подписки появиться не должно.
  QTest::qWait(200);

  while (client.hasPendingDatagrams())
  {
    data.resize(client.pendingDatagramSize());
    client.readDatagram(data.data(), data.size());

    reader.clear();
    reader.append(
      reinterpret_cast<const std::byte*>(data.constData()),
      data.size());

    QVERIFY(reader.nextPacket());

    QVERIFY(
      reader.packetType() != PacketType::LiveData);
  }

  ds.stop();
}

void tst_dataserver::test_dataServer_unsubscribe_invalidId()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>();
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(ds.start());

  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  // ------------------------------------------------------------
  // Subscribe
  // ------------------------------------------------------------

  PacketWriter writer;
  writer.begin(
    PacketType::SubscribeListRequest);

  constexpr SignalId signalIds[]
    {
      {17},
      {23}
    };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount =
    std::size(signalIds);

  writer.write(req);

  writer.writeArray(
    signalIds,
    std::size(signalIds));

  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));

  // ------------------------------------------------------------
  // SubscribeResponse
  // ------------------------------------------------------------

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  QByteArray data;
  data.resize(
    client.pendingDatagramSize());

  client.readDatagram(
    data.data(),
    data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(
      data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeResponse);

  SubscribeResponse response;

  QVERIFY(reader.read(response));

  QCOMPARE(
    reader.remaining(),
    std::size_t(0));

  QCOMPARE(
    response.result,
    SubscribeResult::Ok);

  QCOMPARE(
    response.id,
    SubscriptionId{1});
  // подписка создана, теперь попробуем удалить с неправильным идентификатором ===============

  UnsubscribeRequest req2;
  req2.id = SubscriptionId{999};

  writer.begin(PacketType::UnsubscribeRequest);
  writer.write(req2);

  // Отправляем
  const auto bytes2 =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(bytes2, qint64(writer.size()));

  // Читаем ответ на удаление несуществующей подписки
  do {
    QTRY_VERIFY_WITH_TIMEOUT(
      client.hasPendingDatagrams(),
      2000);

    data.resize(client.pendingDatagramSize());
    client.readDatagram(data.data(), data.size());

    reader.clear();
    reader.append(
      reinterpret_cast<const std::byte*>(data.constData()),
      data.size());

    QVERIFY(reader.nextPacket());
  } while (reader.packetType() != PacketType::UnsubscribeResponse);

  QCOMPARE(reader.packetType(), PacketType::UnsubscribeResponse);

  UnsubscribeResponse response2;
  QVERIFY(reader.read(response2));

  QVERIFY(reader.remaining() == 0u);

  QCOMPARE(response2.result, UnsubscribeResult::InvalidId);

  UnsubscribeRequest req3;
  req3.id = SubscriptionId{1};

  writer.begin(
    PacketType::UnsubscribeRequest);

  writer.write(req3);

  const auto bytes3 =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes3,
    qint64(writer.size()));

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  data.resize(
    client.pendingDatagramSize());

  client.readDatagram(
    data.data(),
    data.size());

  reader.clear();

  reader.append(
    reinterpret_cast<const std::byte*>(
      data.constData()),
    data.size());

  QVERIFY(
    reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::UnsubscribeResponse);

  UnsubscribeResponse response3;

  QVERIFY(
    reader.read(response3));

  QCOMPARE(
    reader.remaining(),
    std::size_t{0});


  QCOMPARE(
    response3.result,
    UnsubscribeResult::Ok);

  ds.stop();
}

void tst_dataserver::test_dataServer_start_stop()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>();
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(ds.start());

  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  // ------------------------------------------------------------
  // Subscribe
  // ------------------------------------------------------------

  PacketWriter writer;
  writer.begin(
    PacketType::SubscribeListRequest);

  constexpr SignalId signalIds[]
    {
      {17},
      {4},
      {23}
    };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount =
    std::size(signalIds);

  writer.write(req);

  writer.writeArray(
    signalIds,
    std::size(signalIds));

  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));

  // ------------------------------------------------------------
  // SubscribeResponse
  // ------------------------------------------------------------

  QTRY_VERIFY_WITH_TIMEOUT(
    client.hasPendingDatagrams(),
    2000);

  QByteArray data;
  data.resize(
    client.pendingDatagramSize());

  client.readDatagram(
    data.data(),
    data.size());

  PacketReader reader;

  reader.append(
    reinterpret_cast<const std::byte*>(
      data.constData()),
    data.size());

  QVERIFY(reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::SubscribeResponse);

  SubscribeResponse response;

  QVERIFY(reader.read(response));

  QCOMPARE(
    reader.remaining(),
    std::size_t(0));

  QCOMPARE(
    response.result,
    SubscribeResult::Ok);

  QCOMPARE(
    response.id,
    SubscriptionId{1});

  // Подписка создана, останавливаем сервер.

  ds.stop();

  // После полного stop сервер должен
  // корректно запуститься снова.

  QVERIFY(
    ds.start());

  ds.stop();
}

void tst_dataserver::test_dataServer_start_after_failed_start()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Fake);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>();
    }));

  FailOnceArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(!ds.start());
  QVERIFY(!ds.isRunning());

  QVERIFY(factory.registerType(
    ModuleType::Fake,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>();
    }));

  QVERIFY(ds.start());
  QVERIFY(ds.isRunning());

  ds.stop();
  QVERIFY(!ds.isRunning());

  QVERIFY(ds.start());
  QVERIFY(ds.isRunning());

  ds.stop();
  QVERIFY(!ds.isRunning());
}

void tst_dataserver::test_dataServer_failStart_invalidUdpPort()
{
  QUdpSocket blocker;

  QVERIFY(
    blocker.bind(
      QHostAddress::AnyIPv4,
      35000));

  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>();
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(!ds.start());

  QVERIFY(!ds.isRunning());

  blocker.close();

  QVERIFY(ds.start());
  QVERIFY(ds.isRunning());

  ds.stop();
  QVERIFY(!ds.isRunning());

  QVERIFY(ds.start());

  QVERIFY(ds.isRunning());

  QTest::qWait(100);

  // Проверим что сервер работает после повторного запуска
  // ------------------------------------------------------------
  // Subscribe
  // ------------------------------------------------------------

  PacketWriter writer;
  writer.begin(
    PacketType::SubscribeListRequest);

  constexpr SignalId signalIds[]
    {
      {17},
      {4},
      {23}
    };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount =
    std::size(signalIds);

  writer.write(req);

  writer.writeArray(
    signalIds,
    std::size(signalIds));

  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  const auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));


  QTest::qWait(100);

  // ------------------------------------------------------------
  // SubscribeResponse
  // ------------------------------------------------------------

  QByteArray data;

  PacketReader reader;

  do {
    QTRY_VERIFY_WITH_TIMEOUT(
      client.hasPendingDatagrams(),
      2000);

    data.resize(client.pendingDatagramSize());
    client.readDatagram(data.data(), data.size());

    reader.clear();
    reader.append(
      reinterpret_cast<const std::byte*>(data.constData()),
      data.size());

    QVERIFY(reader.nextPacket());
  } while (reader.packetType() != PacketType::SubscribeResponse);

  SubscribeResponse response;

  QVERIFY(reader.read(response));

  QCOMPARE(
    reader.remaining(),
    std::size_t(0));

  QCOMPARE(
    response.result,
    SubscribeResult::Ok);

  QCOMPARE(
    response.id,
    SubscriptionId{1});

  ds.stop();
}

void tst_dataserver::test_dataServer_subscriptionId_after_restart()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>();
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(ds.start());

  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  // ------------------------------------------------------------
  // Subscribe
  // ------------------------------------------------------------

  PacketWriter writer;

  writer.begin(
    PacketType::SubscribeListRequest);

  constexpr SignalId signalIds[]
    {
      {23}
    };

  SubscribeListRequest req;
  req.rate = PublishRate::Hz10;
  req.signalCount =
    std::size(signalIds);

  writer.write(req);

  writer.writeArray(
    signalIds,
    std::size(signalIds));

  auto bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));

  // ------------------------------------------------------------
  // SubscribeResponse
  // ------------------------------------------------------------

  QByteArray data;

  PacketReader reader;

  do {
    QTRY_VERIFY_WITH_TIMEOUT(
      client.hasPendingDatagrams(),
      2000);

    data.resize(client.pendingDatagramSize());
    client.readDatagram(data.data(), data.size());

    reader.clear();
    reader.append(
      reinterpret_cast<const std::byte*>(data.constData()),
      data.size());

    QVERIFY(reader.nextPacket());
  } while (reader.packetType() != PacketType::SubscribeResponse);

  SubscribeResponse response;

  QVERIFY(reader.read(response));

  QCOMPARE(
    reader.remaining(),
    std::size_t(0));

  QCOMPARE(
    response.result,
    SubscribeResult::Ok);

  QCOMPARE(
    response.id,
    SubscriptionId{1}); // <--- id == 1

  // останавливаем сервер
  ds.stop();

  // Отбрасываем всё, что уже находилось в UDP-очереди
  while (client.hasPendingDatagrams())
  {
    data.resize(client.pendingDatagramSize());
    client.readDatagram(data.data(), data.size());
  }

  QVERIFY(ds.start());

  writer.begin(
    PacketType::SubscribeListRequest);

  constexpr SignalId signalIds2[]
    {
      {4},
      {17}
    };

  req.rate = PublishRate::Hz100;
  req.signalCount =
    std::size(signalIds2);

  writer.write(req);

  writer.writeArray(
    signalIds2,
    std::size(signalIds2));

  bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));

  // ------------------------------------------------------------
  // SubscribeResponse новая подписка после перезапуска
  // ------------------------------------------------------------

  do {
    QTRY_VERIFY_WITH_TIMEOUT(
      client.hasPendingDatagrams(),
      2000);

    data.resize(client.pendingDatagramSize());
    client.readDatagram(data.data(), data.size());

    reader.clear();
    reader.append(
      reinterpret_cast<const std::byte*>(data.constData()),
      data.size());

    QVERIFY(reader.nextPacket());
  } while (reader.packetType() != PacketType::SubscribeResponse);

  QVERIFY(reader.read(response));

  QCOMPARE(
    reader.remaining(),
    std::size_t(0));

  QCOMPARE(
    response.result,
    SubscribeResult::Ok);

  QCOMPARE(
    response.id,
    SubscriptionId{2}); // <--- id == 2 (1 + 1)

  ds.stop();
}

void tst_dataserver::test_SystemBuilder_buildAfterFailure()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(
      ModuleType::Fake);

  DataStreamSourceFactory factory;

  TestArchiveFrameWriter archive;
  FakeSchedulerClock clock;
  TestLogger logger;

  CalibrationRepository cr;

  SystemBuilder builder;

  // Fake ещё не зарегистрирован.
  // Первая сборка должна завершиться неудачно.

  auto runtime =
    builder.build(
      cfg,
      factory,
      cr,
      clock,
      archive,
      logger);

  QVERIFY(!runtime);

  // Исправляем причину ошибки.

  QVERIFY(
    factory.registerType(
      ModuleType::Fake,
      [](const ModuleRuntimeConfiguration&,
         IClock&,
         IDataBlockSink&,
         IDataStreamEventSink&)
      {
        return std::make_unique<
          TestDataStreamSource>();
      }));

  // Повторная сборка той же configuration
  // должна пройти успешно.

  runtime =
    builder.build(
      cfg,
      factory,
      cr,
      clock,
      archive,
      logger);

  QVERIFY(runtime);

  QCOMPARE(
    runtime->dataSources.size(),
    std::size_t{1});

  QVERIFY(
    runtime->signalProcessor);

  QVERIFY(
    runtime->streamReader);

  QVERIFY(
    runtime->frameAssembler);

  QVERIFY(
    runtime->streamProcessor);

  QVERIFY(
    runtime->streamWorker);

  QVERIFY(
    runtime->engine);

  QVERIFY(
    !runtime->dataSources.isRunning());

  QVERIFY(
    !runtime->streamWorker->isRunning());

  QVERIFY(
    !runtime->engine->isRunning());
}


void tst_dataserver::test_dataEngine_process_without_initialize()
{
  using namespace qds;

  DataEngine engine;

  QVERIFY(!engine.process());
  QVERIFY(!engine.isRunning());
}

void tst_dataserver::test_dataEngine_process_success()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(
      ModuleType::Test);

  SignalMemoryLayout layout;
  layout.build(cfg);

  BufferManager buffers;
  buffers.initialize(layout);

  TestPublisher publisher;

  DataEngine engine;

  QVERIFY(
    engine.initialize(
      buffers,
      publisher));

  QVERIFY(
    engine.start());

  QVERIFY(
    engine.isRunning());

  QVERIFY(
    !buffers.ready());

  Frame frame;
  frame.initialize(layout);

  frame.number =
    FrameNumber{10};

  frame.timestamp =
    Timestamp{1000};

  frame.wallTime =
    WallClockTime{10000};

  buffers.publish(frame);

  QVERIFY(
    buffers.ready());

  QVERIFY(
    engine.process());

  QVERIFY(
    engine.isRunning());

  QCOMPARE(
    publisher.size(),
    std::size_t{1});

  engine.stop();

  QVERIFY(
    !engine.isRunning());
}

void tst_dataserver::test_dataServer_stop_on_dataSourceFailure()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Failing);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>(0);
    }));

  TestArchiveWriter archive;
  TestPublisherSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(!ds.start());

  QTRY_VERIFY_WITH_TIMEOUT(
    !ds.isRunning(),
    2000);

  QCOMPARE(archive.size(), 0);
  QCOMPARE(sender.sendCount, 0);
}

void tst_dataserver::test_SystemBuilder_failedThenSuccess()
{
  using namespace qds;

  SystemConfiguration badCfg =
    createTestConfig_calculate(
      ModuleType::Test);

  badCfg.addSignalDefinition({
    .id = {30},
    .name = "Bad",
    .kind = SignalKind::Calculated,
    .formula = "unknown + C"
  });

  SystemConfiguration goodCfg =
    createTestConfig_calculate(
      ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(
    factory.registerType(
      ModuleType::Test,
      [](const ModuleRuntimeConfiguration&,
         IClock&,
         IDataBlockSink&,
         IDataStreamEventSink&)
      {
        return std::make_unique<
          TestDataStreamSource>();
      }));

  TestArchiveFrameWriter archive;
  FakeSchedulerClock clock;
  TestLogger logger;

  CalibrationRepository cr;

  SystemBuilder builder;

  // ------------------------------------------------------------
  // Failed build
  // ------------------------------------------------------------

  auto runtime =
    builder.build(
      badCfg,
      factory,
      cr,
      clock,
      archive,
      logger);

  QVERIFY(!runtime);

  // ------------------------------------------------------------
  // Successful build using the same SystemBuilder
  // ------------------------------------------------------------

  runtime =
    builder.build(
      goodCfg,
      factory,
      cr,
      clock,
      archive,
      logger);

  QVERIFY(runtime);

  QVERIFY(
    runtime->signalProcessor);

  QVERIFY(
    runtime->streamReader);

  QVERIFY(
    runtime->frameAssembler);

  QVERIFY(
    runtime->streamProcessor);

  QVERIFY(
    runtime->streamWorker);

  QVERIFY(
    runtime->engine);

  QCOMPARE(
    runtime->dataSources.size(),
    std::size_t{1});

  QVERIFY(
    !runtime->dataSources.isRunning());

  QVERIFY(
    !runtime->streamWorker->isRunning());

  QVERIFY(
    !runtime->engine->isRunning());
}

void tst_dataserver::test_dataServer_start_twice()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>();
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(ds.start());
  QVERIFY(ds.isRunning());

  QVERIFY(ds.start());
  QVERIFY(ds.isRunning());

  ds.stop();

  QVERIFY(!ds.isRunning());
}

void tst_dataserver::test_dataServer_stop_before_start()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration&,
       IClock&,
       IDataBlockSink&,
       IDataStreamEventSink&)
    {
      return std::make_unique<TestDataStreamSource>();
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(!ds.isRunning());

  ds.stop();

  QVERIFY(!ds.isRunning());

  QVERIFY(ds.start());
  QVERIFY(ds.isRunning());

  ds.stop();

  QVERIFY(!ds.isRunning());
}

void tst_dataserver::test_dataServer_udp_pipeline()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleRuntimeConfiguration& configuration,
       IClock&,
       IDataBlockSink& blockSink,
       IDataStreamEventSink& eventSink)
    {
      return std::make_unique<TestStreamingDataSource>(
        configuration,
        blockSink,
        eventSink);
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock(2, 3);

  CalibrationRepository cr;
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    cr,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(ds.start());

  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  PacketWriter writer;
  PacketReader reader;
  std::vector<SignalId> signalIds;
  SubscribeListRequest request;
  SubscribeResponse response;
  PublishHeader header;
  QByteArray data;
  long bytes;
  std::array<double, 2> samples1;
  std::array<double, 1> samples2;

  // ------------------------------------------------------------
  // Subscribe 1
  // ------------------------------------------------------------

  writer.begin(
    PacketType::SubscribeListRequest);

  signalIds.assign({{4}, {23}}); // B, C

  request.rate = PublishRate::Hz10;
  request.signalCount = signalIds.size();

  writer.write(request);

  writer.writeArray(
    signalIds.data(),
    signalIds.size());

  bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));


  // ------------------------------------------------------------
  // Subscribe 2
  // ------------------------------------------------------------

  writer.begin(
    PacketType::SubscribeListRequest);

  signalIds.assign({{17}}); // A

  request.rate = PublishRate::Hz100;
  request.signalCount = signalIds.size();

  writer.write(request);

  writer.writeArray(
    signalIds.data(),
    signalIds.size());

  bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));


  // ------------------------------------------------------------
  // SubscribeResponses 1 and 2
  // ------------------------------------------------------------

  bool has_sub1 = false, has_sub2 = false;

  for (int subscribe = 1; subscribe <= 2; ++subscribe)
  {
    waitPacket(client, data, reader, PacketType::SubscribeResponse);

    QCOMPARE(
      reader.packetType(),
      PacketType::SubscribeResponse);

    QVERIFY(reader.read(response));

    QCOMPARE(
      reader.remaining(),
      std::size_t(0));

    QCOMPARE(
      response.result,
      SubscribeResult::Ok);

    if (response.id == SubscriptionId{1})
      has_sub1 = true;

    else if (response.id == SubscriptionId{2})
      has_sub2 = true;

    else
      QFAIL("Неверная подписка");
  }

  QVERIFY(has_sub1);
  QVERIFY(has_sub2);


  QTest::qWait(1500);


  uint32_t sequence1 = 0;
  uint32_t sequence2 = 0;

  while(client.waitForReadyRead(100) && client.hasPendingDatagrams())
  {
    data.resize(client.pendingDatagramSize());
    client.readDatagram(data.data(), data.size());

    reader.clear();

    reader.append(
      reinterpret_cast<const std::byte*>(
        data.constData()),
      data.size());

    QVERIFY(reader.nextPacket());

    QCOMPARE(
      reader.packetType(),
      PacketType::LiveData);

    QVERIFY(reader.read(header));

    // B C
    if (header.subscriptionId == SubscriptionId{1})
    {
      QCOMPARE(
        header.sequence,
        ++sequence1);

#ifdef __APPLE__
      const auto index = sequence1 - 1;
#else
      const auto index = sequence1;
#endif

      QVERIFY(
        header.timestamp >= 1'000'000u);

      const uint64_t frameIndex =
        (header.timestamp - 1'000'000u) /
        1'000u;

      QCOMPARE(
        header.timestamp,
        1'000'000u +
          frameIndex * 1'000u);

      QCOMPARE(
        header.valueCount,
        2u);

      QCOMPARE(
        header.sequence,
        ++sequence1);

      QVERIFY(
        reader.readArray(
          samples1.data(),
          samples1.size()));

      const double b =
        static_cast<double>(
          frameIndex * 10) - 19.0;

      const double c =
        static_cast<double>(
          frameIndex * 11) - 19.0;

      QCOMPARE(
        samples1[0],
        b);

      QCOMPARE(
        samples1[1],
        c);
    }
    // A
    else if (header.subscriptionId == SubscriptionId{2})
    {
      QCOMPARE(
        header.sequence,
        ++sequence2);

#ifdef __APPLE__
      const auto index = sequence2 - 1;
#else
      const auto index = sequence2;
#endif

      QVERIFY(
        header.timestamp >= 1'000'000u);

      const uint64_t frameIndex =
        (header.timestamp - 1'000'000u) /
        1'000u;

      QCOMPARE(
        header.timestamp,
        1'000'000u +
          frameIndex * 1'000u);

      QCOMPARE(
        header.valueCount,
        1u);

      QCOMPARE(
        header.sequence,
        ++sequence2);

      QVERIFY(
        reader.readArray(
          samples2.data(),
          samples2.size()));

      const double a =
        static_cast<double>(
          index);

      QCOMPARE(
        samples2[0],
        a);
    }
    else
      QFAIL("Неверная подписка");

    QCOMPARE(
      reader.remaining(),
      std::size_t(0));
  }

  QVERIFY(sequence1 > 0);
  QVERIFY(sequence2 > 0);

  // ------------------------------------------------------------
  // Stop
  // ------------------------------------------------------------

  ds.stop();
}

void tst_dataserver::test_dataServer_publish_archive_pipeline()
{
  using namespace qds;
  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repository(db);

  SystemConfiguration cfg;
  QVERIFY(repository.load(ConfigurationId{1}, cfg));

  CalibrationRepository calibrations;
  QVERIFY(repository.loadCalibrations(cfg, calibrations));
  
  DataStreamSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::LTR11,
    [](const ModuleRuntimeConfiguration& configuration,
       IClock&,
       IDataBlockSink& blockSink,
       IDataStreamEventSink& eventSink)
    {
      return std::make_unique<TestStreamingDataSource>(
        configuration,
        blockSink,
        eventSink);
    }));

  ArchiveDescriptionBuilder builder;
  ArchiveDescription description;
  QVERIFY(builder.build(cfg, description));

  ArchiveDescriptionWriter archiveWriter;

  const auto path =
    getFilePath(
      "description.json");

  QVERIFY(
    archiveWriter.write(
      path,
      description));

  SignalMemoryLayout layout;
  layout.build(cfg);

  auto directory = getCurrentFolder();

  ArchiveManager archive;
  QVERIFY(archive.initialize(directory, description, layout));

  UdpSender sender;
  FakeSchedulerClock clock(2, 3);
  Logger logger(getCurrentFolder(), clock);

  DataServer ds(
    cfg,
    calibrations,
    factory,
    archive,
    clock,
    sender,
    logger);

  QVERIFY(ds.start());

  QUdpSocket client;

  QVERIFY(
    client.bind(
      QHostAddress::LocalHost,
      0));

  PacketWriter writer;
  PacketReader reader;
  std::vector<SignalId> signalIds;
  SubscribeListRequest request;
  SubscribeResponse response;
  PublishHeader header;
  QByteArray data;
  long bytes;
  std::array<Sample, 2> samples1;
  std::array<Sample, 1> samples2;

  // ------------------------------------------------------------
  // Subscribe 1
  // ------------------------------------------------------------

  writer.begin(
    PacketType::SubscribeListRequest);

  const auto &def = cfg.signalDefinitions();
  signalIds.assign({findSignalDefinition(def, "B")->id, findSignalDefinition(def, "C")->id});

  request.rate = PublishRate::Hz10;
  request.signalCount = signalIds.size();

  writer.write(request);

  writer.writeArray(
    signalIds.data(),
    signalIds.size());

  bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));


  // ------------------------------------------------------------
  // Subscribe 2
  // ------------------------------------------------------------

  writer.begin(
    PacketType::SubscribeListRequest);

  signalIds.assign({findSignalDefinition(def, "A")->id});

  request.rate = PublishRate::Hz100;
  request.signalCount = signalIds.size();

  writer.write(request);

  writer.writeArray(
    signalIds.data(),
    signalIds.size());

  bytes =
    client.writeDatagram(
      reinterpret_cast<const char*>(
        writer.data()),
      writer.size(),
      QHostAddress::LocalHost,
      cfg.udpPort());

  QCOMPARE(
    bytes,
    qint64(writer.size()));

  // ------------------------------------------------------------
  // Проверим ответ сервера на регистрацию подписок
  // ------------------------------------------------------------

  bool has_sub1 = false, has_sub2 = false;

  for (int subscribe = 1; subscribe <= 2; ++subscribe)
  {
    waitPacket(client, data, reader, PacketType::SubscribeResponse);

    QCOMPARE(
      reader.packetType(),
      PacketType::SubscribeResponse);

    QVERIFY(reader.read(response));

    QCOMPARE(
      reader.remaining(),
      std::size_t(0));

    QCOMPARE(
      response.result,
      SubscribeResult::Ok);

    if (response.id == SubscriptionId{1})
      has_sub1 = true;

    else if (response.id == SubscriptionId{2})
      has_sub2 = true;

    else
      QFAIL("Неверная подписка");
  }

  QVERIFY(has_sub1);
  QVERIFY(has_sub2);


  QTest::qWait(1500);


  // ------------------------------------------------------------
  // Stop
  // ------------------------------------------------------------

  ds.stop();
  QVERIFY(!ds.isRunning());

  QTest::qWait(100);

  archive.close();

  // ------------------------------------------------------------
  // Проверим данные архивов и подписок
  // ------------------------------------------------------------

  uint32_t sequence1 = 0;
  uint32_t sequence2 = 0;

  uint64_t previousTimestamp1{};
  bool hasTimestamp1 = false;

  uint64_t previousTimestamp2{};
  bool hasTimestamp2 = false;


  while(client.waitForReadyRead(100) && client.hasPendingDatagrams())
  {
    data.resize(client.pendingDatagramSize());
    client.readDatagram(data.data(), data.size());

    reader.clear();

    reader.append(
      reinterpret_cast<const std::byte*>(
        data.constData()),
      data.size());

    QVERIFY(reader.nextPacket());

    QCOMPARE(
      reader.packetType(),
      PacketType::LiveData);

    QVERIFY(reader.read(header));

    if (
      header.subscriptionId ==
      SubscriptionId{1})
    {
      QCOMPARE(
        header.sequence,
        ++sequence1);

      const uint64_t frameIndex =
        (header.timestamp - 1'000'000) / 1000;

      if (hasTimestamp1)
      {
        QVERIFY(
          header.timestamp >
          previousTimestamp1);
      }

      previousTimestamp1 =
        header.timestamp;
      hasTimestamp1 = true;

      QCOMPARE(
        header.valueCount,
        2u);

      QVERIFY(
        reader.readArray(
          samples1.data(),
          samples1.size()));

      const double raw0 =
        static_cast<double>(
          frameIndex * 10);

      const double raw1 =
        static_cast<double>(
          frameIndex * 10 + 1);

      const double a = raw0 * 0.1;
      const double b = raw1 - 20.0;
      const double c = a + b;

      QCOMPARE(
        samples1[0],
        Sample{b});

      QCOMPARE(
        samples1[1],
        Sample{c});
    }
    else if (
      header.subscriptionId ==
      SubscriptionId{2})
    {
      QCOMPARE(
        header.sequence,
        ++sequence2);

      const uint64_t frameIndex =
        (header.timestamp - 1'000'000) / 1000;

      if (hasTimestamp2)
      {
        QVERIFY(
          header.timestamp >
          previousTimestamp2);
      }

      previousTimestamp2 =
        header.timestamp;
      hasTimestamp2 = true;

      QCOMPARE(
        header.valueCount,
        1u);

      QVERIFY(
        reader.readArray(
          samples2.data(),
          samples2.size()));

      const double raw0 =
        static_cast<double>(
          frameIndex * 10);

      const double a =
        raw0 * 0.1;

      QCOMPARE(
        samples2[0],
        Sample{a});
    }
    else
      QFAIL("Неверная подписка");

    QCOMPARE(
      reader.remaining(),
      std::size_t(0));
  }

  QVERIFY(sequence1 > 0);
  QVERIFY(sequence2 > 0);

  ArchiveReader archiveReader;

  QVERIFY(archiveReader.open(getCurrentFolder()));

  QVERIFY(archiveReader.isOpen());

  description = archiveReader.description();
  QCOMPARE(description.version, ArchiveDescriptionVersion);

  std::size_t fileIndex;
  ArchiveSample sample;
  const DataFileHeader *fileHeader;

  // Raw0
  fileIndex = findFile(description, SignalKind::Raw, 1000);
  QVERIFY(fileIndex >= 0);
  fileHeader = archiveReader.fileHeader(fileIndex);
  QVERIFY(fileHeader);
  QVERIFY(fileHeader->recordCount > 0);
  QCOMPARE(fileHeader->sampleFrequency, 1000);
  QCOMPARE(fileHeader->channelCount, 1u);

  for (uint64_t i = 0; i < fileHeader->recordCount; ++i)
  {
    QVERIFY(archiveReader.read(fileIndex, sample));
    const uint64_t frameIndex = i;

    QCOMPARE(
      sample.frameNumber,
      FrameNumber{frameIndex});

    QCOMPARE(
      sample.timestamp,
      Timestamp{
                1'000'000 +
                frameIndex * 1000});

    QCOMPARE(
      sample.wallTime,
      WallClockTime{
                    2'000'000 +
                    static_cast<int64_t>(
                      frameIndex * 1000)});

    const double raw0 =
      static_cast<double>(
        frameIndex * 10);

    QCOMPARE(
      sample.values[0],
      static_cast<float>(raw0));
  }

  // Raw1
  fileIndex = findFile(description, SignalKind::Raw, 100);
  QVERIFY(fileIndex >= 0);
  fileHeader = archiveReader.fileHeader(fileIndex);
  QVERIFY(fileHeader);
  QVERIFY(fileHeader->recordCount > 0);
  QCOMPARE(fileHeader->sampleFrequency, 100);
  QCOMPARE(fileHeader->channelCount, 1u);

  for (uint64_t i = 0; i < fileHeader->recordCount; ++i)
  {
    QVERIFY(archiveReader.read(fileIndex, sample));

    const uint64_t frameIndex = i * 10;

    QCOMPARE(
      sample.frameNumber,
      FrameNumber{frameIndex});

    QCOMPARE(
      sample.timestamp,
      Timestamp{
                1'000'000 +
                frameIndex * 1000});

    QCOMPARE(
      sample.wallTime,
      WallClockTime{
                    2'000'000 +
                    static_cast<int64_t>(
                      frameIndex * 1000)});

    const double raw1 =
      static_cast<double>(
        frameIndex * 10 + 1);

    QCOMPARE(
      sample.values[0],
      static_cast<float>(raw1));
  }

  // A
  fileIndex = findFile(description, SignalKind::Calculated, 100);
  QVERIFY(fileIndex >= 0);
  fileHeader = archiveReader.fileHeader(fileIndex);
  QVERIFY(fileHeader);
  QVERIFY(fileHeader->recordCount > 0);
  QCOMPARE(fileHeader->sampleFrequency, 100);
  QCOMPARE(fileHeader->channelCount, 1u);

  for (uint64_t i = 0; i < fileHeader->recordCount; ++i)
  {
    QVERIFY(archiveReader.read(fileIndex, sample));
    const uint64_t frameIndex = i * 10;

    const double raw0 =
      static_cast<double>(
        frameIndex * 10);

    const double a =
      raw0 * 0.1;

    QCOMPARE(
      sample.values[0],
      static_cast<float>(a));
  }

  // B C
  fileIndex = findFile(description, SignalKind::Calculated, 10);
  QVERIFY(fileIndex >= 0);
  fileHeader = archiveReader.fileHeader(fileIndex);
  QVERIFY(fileHeader);
  QVERIFY(fileHeader->recordCount > 0);
  QCOMPARE(fileHeader->sampleFrequency, 10);
  QCOMPARE(fileHeader->channelCount, 2u);

  for (uint64_t i = 0; i < fileHeader->recordCount; ++i)
  {
    QVERIFY(archiveReader.read(fileIndex, sample));
    const uint64_t frameIndex = i * 100;

    const double raw0 =
      static_cast<double>(
        frameIndex * 10);

    const double raw1 =
      static_cast<double>(
        frameIndex * 10 + 1);

    const double a = raw0 * 0.1;
    const double b = raw1 - 20.0;
    const double c = a + b;

    QCOMPARE(
      sample.values[0],
      static_cast<float>(b));

    QCOMPARE(
      sample.values[1],
      static_cast<float>(c));
  }

  archiveReader.close();
  QVERIFY(!archiveReader.isOpen());
  QVERIFY(archiveReader.fileHeader(0) == nullptr);
}