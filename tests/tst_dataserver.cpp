#include "tst_dataserver.h"
#include "datasourcefactory.h"
#include "failoncearchivewriter.h"
#include "fakedatasource.h"
#include "fakeschedulerclock.h"
#include "protocol/publishheader.h"
#include "runtimesystem.h"
#include "systembuilder.h"
#include "systemconfiguration.h"
#include "testarchivewriter.h"
#include "testdatasource.h"
#include "testpublisher.h"
#include "testsrv.h"
#include "dataserver.h"
#include <qtestsupport_core.h>

tst_dataserver::tst_dataserver() { }
tst_dataserver::~tst_dataserver() = default;

void tst_dataserver::test_systemBuilder_success()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);
  cfg.addSignalDefinition({.id = {24}, .name = "D", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {2}, .formula = "A + A", .dependencies = {{17}, {17}}});

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  TestPublisher publisher;
  FakeSchedulerClock clock;

  RuntimeSystem runtime;

  SystemBuilder builder;

  QVERIFY(builder.build(
    cfg,
    factory,
    runtime));

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
    runtime.layout.rawSignalCount(),
    2u);

  QCOMPARE(
    runtime.layout.calculatedSignalCount(),
    4u);

  QCOMPARE(
    runtime.formulas.size(),
    std::size_t(4));

  QCOMPARE(
    runtime.calculationPlan.size(),
    std::size_t(4));

  QCOMPARE(
    runtime.dataSources.size(),
    std::size_t(1));

  QVERIFY(
    runtime.signalProcessor != nullptr);

  QVERIFY(
    runtime.engine != nullptr);
}

void tst_dataserver::test_systemBuilder_process()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  TestPublisher publisher;
  FakeSchedulerClock clock;

  RuntimeSystem runtime;

  SystemBuilder builder;

  QVERIFY(builder.build(
    cfg,
    factory,
    runtime));

  QVERIFY(runtime.engine->initialize(
    runtime.dataSources,
    *runtime.signalProcessor,
    runtime.buffers,
    archive,
    publisher,
    clock));

  QVERIFY(
    runtime.engine->process());

  QCOMPARE(
    archive.count,
    1);

  QCOMPARE(
    publisher.count,
    1);
}

void tst_dataserver::test_systemBuilder_failErrorFormula()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  cfg.addSignalDefinition({
    .id = {30},
    .name = "D",
    .kind = SignalKind::Calculated,
    .formulaId = {2},
    .formula = "unknown + C",
    .dependencies = {{4}, {23}}
  });

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  TestPublisher publisher;
  FakeSchedulerClock clock;

  RuntimeSystem runtime;

  SystemBuilder builder;

  QVERIFY(!builder.build(
    cfg,
    factory,
    runtime));
}

void tst_dataserver::test_systemBuilder_failDataSourceManager()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::LCard);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  TestPublisher publisher;
  FakeSchedulerClock clock;

  RuntimeSystem runtime;

  SystemBuilder builder;

  QVERIFY(!builder.build(
    cfg,
    factory,
    runtime));
}

void tst_dataserver::test_systemBuilder_cycle()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  TestPublisher publisher;
  FakeSchedulerClock clock(2, 5);

  RuntimeSystem runtime;

  SystemBuilder builder;

  QVERIFY(builder.build(
    cfg,
    factory,
    runtime));

  QVERIFY(runtime.engine->initialize(
    runtime.dataSources,
    *runtime.signalProcessor,
    runtime.buffers,
    archive,
    publisher,
    clock));

  for (int n = 0; n < 1000; ++n)
  {
    QVERIFY(runtime.engine->process());

    auto count = n + 1;
    double a = n;
    double b = n * 10;
    QCOMPARE(archive.count, count);
    QCOMPARE(publisher.count, count);

    const auto& archived = archive.last();
    const auto& published = publisher.last();

    QCOMPARE(archived.number, FrameNumber{static_cast<uint64_t>(count)});

    QCOMPARE(archived.timestamp, Timestamp{static_cast<uint64_t>(count * 2)});

    QCOMPARE(archived.wallTime, WallClockTime{static_cast<int64_t>(count * 5)});

    QCOMPARE(archived.raw().valueRef(0), a);
    QCOMPARE(archived.raw().valueRef(1), b);

    QCOMPARE(archived.calculated().valueRef(0), a);
    QCOMPARE(archived.calculated().valueRef(1), b);
    QCOMPARE(archived.calculated().valueRef(2), a + b);

    QCOMPARE(
      published.raw().valueRef(0),
      archived.raw().valueRef(0));

    QCOMPARE(
      published.raw().valueRef(1),
      archived.raw().valueRef(1));

    QCOMPARE(
      published.calculated().valueRef(0),
      archived.calculated().valueRef(0));

    QCOMPARE(
      published.calculated().valueRef(1),
      archived.calculated().valueRef(1));

    QCOMPARE(
      published.calculated().valueRef(2),
      archived.calculated().valueRef(2));
  }
}

void tst_dataserver::test_dataServer_udpSubscription()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  cfg.setUdpPort(35000);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  DataServer ds(
    cfg,
    factory,
    archive,
    clock,
    sender);

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

  QCOMPARE(samples[0].value, 100.0);
  QCOMPARE(samples[1].value, 1000.0);
  QCOMPARE(samples[2].value, 1100.0);

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

  QCOMPARE(samples[0].value, 200.0);
  QCOMPARE(samples[1].value, 2000.0);
  QCOMPARE(samples[2].value, 2200.0);

  // ------------------------------------------------------------
  // Stop
  // ------------------------------------------------------------

  ds.stop();
}

void tst_dataserver::test_dataServer_failStart_moduleType()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::LCard);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  TestPublisherSender sender;
  FakeSchedulerClock clock;

  DataServer ds(
    cfg,
    factory,
    archive,
    clock,
    sender);

  QVERIFY(!ds.start());

  QCOMPARE(sender.sendCount, std::size_t(0));

  QTest::qWait(100);

  QCOMPARE(sender.sendCount, std::size_t(0));
}

void tst_dataserver::test_dataServer_failSubscribe_invalidSignalId()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  cfg.setUdpPort(35000);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  DataServer ds(
    cfg,
    factory,
    archive,
    clock,
    sender);

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

  cfg.setUdpPort(35000);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  DataServer ds(
    cfg,
    factory,
    archive,
    clock,
    sender);

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
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  cfg.setUdpPort(35000);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  DataServer ds(
    cfg,
    factory,
    archive,
    clock,
    sender);

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

  cfg.setUdpPort(35000);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  DataServer ds(
    cfg,
    factory,
    archive,
    clock,
    sender);

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

  cfg.setUdpPort(35000);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  DataServer ds(
    cfg,
    factory,
    archive,
    clock,
    sender);

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
  req.signalCount = req.signalCount = MaxSubscriptionSignals + 1; // <---

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

  cfg.setUdpPort(35000);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  DataServer ds(
    cfg,
    factory,
    archive,
    clock,
    sender);

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

  cfg.setUdpPort(35000);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  DataServer ds(
    cfg,
    factory,
    archive,
    clock,
    sender);

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

  while (client.hasPendingDatagrams())
  {
    data.resize(client.pendingDatagramSize());
    client.readDatagram(data.data(), data.size());
  }

  // а пакеты продолжают идти

  QTest::qWait(200);

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

  ds.stop();
}

void tst_dataserver::test_dataServer_start_stop()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Test);

  cfg.setUdpPort(35000);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  TestArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  DataServer ds(
    cfg,
    factory,
    archive,
    clock,
    sender);

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

  // подписка создана, теперь останавливаем сервер ===============

  ds.stop();

  // Отбрасываем всё, что уже находилось в UDP-очереди
  while (client.hasPendingDatagrams())
  {
    data.resize(client.pendingDatagramSize());
    client.readDatagram(data.data(), data.size());
  }

  // Теперь в течение некоторого времени новых пакетов
  // появиться не должно.
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
}

void tst_dataserver::test_dataServer_start_after_failed_start()
{
  SystemConfiguration cfg =
    createTestConfig_calculate(ModuleType::Fake);

  cfg.setUdpPort(35000);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Test,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<TestDataSource>(
        cfg.settings);
    }));

  FailOnceArchiveWriter archive;
  UdpSender sender;
  FakeSchedulerClock clock;

  DataServer ds(
    cfg,
    factory,
    archive,
    clock,
    sender);

  QVERIFY(!ds.start());

  QVERIFY(factory.registerType(
    ModuleType::Fake,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FakeDataSource>(
        cfg.settings);
    }));

  QVERIFY(ds.start());

  ds.stop();
}