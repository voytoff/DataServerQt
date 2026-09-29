#include "tst_database.h"
#include "archivedescriptionbuilder.h"
#include "archivedescriptionwriter.h"
#include "archivemanager.h"
#include "archivereader.h"
#include "configurationrepository.h"
#include "protocol/publishheader.h"
#include "qds/db.h"
#include "archiveformat.h"
#include "fakeschedulerclock.h"
#include "qds/nullarchivewriter.h"
#include "qds/testarchiveframewriter.h"
#include "qds/testdatastreamsource.h"
#include "systembuilder.h"
#include "systemconfiguration.h"
#include "qds/testpublishersender.h"
#include "testsrv.h"
#include "testlogger.h"
#include <QSqlTableModel>
#include <qtestcase.h>
#include <cmath>

tst_database::tst_database() { }
tst_database::~tst_database() = default;

void tst_database::test_database_loadConfiguration()
{
  using namespace qds;
  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repo(db);

  SystemConfiguration cfg;

  QVERIFY(repo.load(ConfigurationId{1}, cfg));

  QCOMPARE(cfg.udpPort(), 5001);
  QCOMPARE(cfg.name(), "Иследование 1");
  QCOMPARE(cfg.description(), "Тестовая конфигурация");

  QCOMPARE(cfg.crates().size(), 1);
  QCOMPARE(cfg.modules().size(), 1);
  QCOMPARE(cfg.signalDefinitions().size(), 5);

  const CrateInfo &ci = cfg.crates()[0];
  QCOMPARE(ci.type, CrateType::LTR_EU_16_1); // в базе - 1

  QCOMPARE(
    ci.serial,
    "3T778029");

  QCOMPARE(
    ci.host,
    "127.0.0.1");

  QCOMPARE(
    ci.port,
    11111u);

  QCOMPARE(
    ci.description,
    "CRATE16");

  const ModuleInfo &mi = cfg.modules()[0];

  QCOMPARE(mi.crate, ci.id);
  QCOMPARE(mi.type, ModuleType::LTR11);

  QCOMPARE(
    mi.serial,
    "LTR11-000001");

  QCOMPARE(
    mi.description,
    "LTR11 test module");

  //QCOMPARE(
  //  mi.settings,
  //  QJsonObject{});

  auto mtags = cfg.moduleTags(mi.id);
  QCOMPARE(mtags.size(), 2);

  auto tags = cfg.tags();
  QCOMPARE(tags.size(), 2);

  QCOMPARE(tags[0].channel, {0});
  QCOMPARE(tags[1].channel, {1});

  auto ss = cfg.signalDefinitions();
  QCOMPARE(ss.size(), 5);

  auto raw0 = findSignalDefinition(ss, "Raw0");
  QVERIFY(raw0);
  QCOMPARE(raw0->source.tag, mtags[0]);
  QCOMPARE(raw0->archiveFrequency, 1000);
  QCOMPARE(raw0->kind, SignalKind::Raw);
  QCOMPARE(raw0->calibrationMode, CalibrationMode::None);
  QCOMPARE(raw0->formula, "");
  QCOMPARE(raw0->signalType, SignalTypeId{1});

  auto raw1 = findSignalDefinition(ss, "Raw1");
  QVERIFY(raw1);
  QCOMPARE(raw1->source.tag, mtags[1]);
  QCOMPARE(raw1->archiveFrequency, 100);
  QCOMPARE(raw1->kind, SignalKind::Raw);
  QCOMPARE(raw1->calibrationMode, CalibrationMode::None);
  QCOMPARE(raw1->formula, "");
  QCOMPARE(raw1->signalType, SignalTypeId{2});

  auto a = findSignalDefinition(ss, "A");
  QVERIFY(a);
  QCOMPARE(a->archiveFrequency, 100);
  QCOMPARE(a->kind, SignalKind::Calculated);
  QCOMPARE(a->calibrationMode, CalibrationMode::BySignal);
  QCOMPARE(a->formula, "Raw0");
  QCOMPARE(a->signalType, SignalTypeId{1});

  auto b = findSignalDefinition(ss, "B");
  QVERIFY(b);
  QCOMPARE(b->archiveFrequency, 10);
  QCOMPARE(b->kind, SignalKind::Calculated);
  QCOMPARE(b->calibrationMode, CalibrationMode::BySignalType);
  QCOMPARE(b->formula, "Raw1");
  QCOMPARE(b->signalType, SignalTypeId{2});

  auto c = findSignalDefinition(ss, "C");
  QVERIFY(c);
  QCOMPARE(c->archiveFrequency, 10);
  QCOMPARE(c->kind, SignalKind::Calculated);
  QCOMPARE(c->calibrationMode, CalibrationMode::None);
  QCOMPARE(c->formula, "A + B");
  QCOMPARE(c->signalType, SignalTypeId{2});
}

void tst_database::test_database_loadCalibrations()
{
  using namespace qds;
  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repo(db);

  SystemConfiguration cfg;
  QVERIFY(repo.load(ConfigurationId{1}, cfg));

  CalibrationRepository cr;
  QVERIFY(repo.loadCalibrations(cfg, cr));

  QCOMPARE(cr.sizeSignals(), 1);
  QCOMPARE(cr.sizeSignalTypes(), 1);

  auto definitions = cfg.signalDefinitions();
  double result = 0;

  auto a = findSignalDefinition(definitions, "A");

  QVERIFY(cr.calibrateBySignal(a->id, -10, result));
  QCOMPARE(result, -1.0);

  QVERIFY(cr.calibrateBySignal(a->id, 0, result));
  QCOMPARE(result, 0.0);

  QVERIFY(cr.calibrateBySignal(a->id, 5, result));
  QCOMPARE(result, 0.5);

  QVERIFY(cr.calibrateBySignal(a->id, 10, result));
  QCOMPARE(result, 1.0);

  QVERIFY(cr.calibrateBySignal(a->id, 15, result));
  QCOMPARE(result, 1.5);

  QVERIFY(cr.calibrateBySignal(a->id, 20, result));
  QCOMPARE(result, 2.0);

  QVERIFY(cr.calibrateBySignal(a->id, 30, result));
  QCOMPARE(result, 3.0);

  auto b = SignalTypeId{2}; // B - имеет тип с идентификатором 2

  QVERIFY(cr.calibrateBySignalType(b, -10, result));
  QCOMPARE(result, -30.0);

  QVERIFY(cr.calibrateBySignalType(b, 0, result));
  QCOMPARE(result, -20.0);

  QVERIFY(cr.calibrateBySignalType(b, 5, result));
  QCOMPARE(result, -15.0);

  QVERIFY(cr.calibrateBySignalType(b, 10, result));
  QCOMPARE(result, -10.0);

  QVERIFY(cr.calibrateBySignalType(b, 15, result));
  QCOMPARE(result, -5.0);

  QVERIFY(cr.calibrateBySignalType(b, 20, result));
  QCOMPARE(result, 0.0);

  QVERIFY(cr.calibrateBySignalType(b, 30, result));
  QCOMPARE(result, 10.0);

  result = 123.0;

  QVERIFY(
    !cr.calibrateBySignal(
      SignalId{999},
      10.0,
      result));
  QCOMPARE(result, 123.0);


  QVERIFY(
    !cr.calibrateBySignalType(
      SignalTypeId{999},
      10.0,
      result));
  QCOMPARE(result, 123.0);

}

void tst_database::test_database_failLoading()
{
  using namespace qds;
  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repo(db);

  SystemConfiguration cfg;

  QVERIFY(repo.load(ConfigurationId{1}, cfg));

  auto c = findSignalDefinition(cfg.signalDefinitions(), "C");

  QSqlQuery query(get_db());

  QVERIFY(query.prepare(R"(
UPDATE configuration_signal_definition
SET calibration_mode=1
WHERE id=:id;
)"));

  query.bindValue(":id", c->id.value);

  QVERIFY(query.exec());

  QVERIFY(repo.load(ConfigurationId{1}, cfg));

  CalibrationRepository cr;
  QVERIFY(!repo.loadCalibrations(cfg, cr));

  QCOMPARE(cr.sizeSignals(), 0);
  QCOMPARE(cr.sizeSignalTypes(), 0);

  QVERIFY(query.prepare(R"(
UPDATE configuration_signal_definition
SET calibration_mode=0
WHERE id=:id;
)"));

  query.bindValue(":id", c->id.value);

  QVERIFY(query.exec());

  QVERIFY(repo.load(ConfigurationId{1}, cfg));

  QVERIFY(repo.loadCalibrations(cfg, cr));

  QCOMPARE(cr.sizeSignals(), 1);
  QCOMPARE(cr.sizeSignalTypes(), 1);
}

void tst_database::test_database_pipeline()
{
  using namespace qds;

  auto db = get_db();

  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repo(db);

  SystemConfiguration cfg;

  QVERIFY(
    repo.load(
      ConfigurationId{1},
      cfg));

  QCOMPARE(
    cfg.modules().size(),
    std::size_t{1});

  const auto module =
    cfg.modules().front().id;

  QCOMPARE(
    cfg.moduleTags(module).size(),
    std::size_t{2});

  CalibrationRepository cr;

  QVERIFY(
    repo.loadCalibrations(
      cfg,
      cr));

  QCOMPARE(
    cr.sizeSignals(),
    std::size_t{1});

  QCOMPARE(
    cr.sizeSignalTypes(),
    std::size_t{1});

  DataStreamSourceFactory factory;

  QVERIFY(
    factory.registerType(
      ModuleType::LTR11,
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

  QCOMPARE(
    runtime->calibrations.sizeSignals(),
    std::size_t{1});

  QCOMPARE(
    runtime->calibrations.sizeSignalTypes(),
    std::size_t{1});

  // ------------------------------------------------------------
  // Stream anchor
  // ------------------------------------------------------------

  DataStreamAnchor anchor;
  anchor.module = module;
  anchor.firstFrameIndex = 0;
  anchor.startTimestamp = Timestamp{1000};
  anchor.startWallTime = WallClockTime{10000};
  anchor.frameRate = 100.0;

  QVERIFY(
    runtime->streamProcessor->process(
      anchor));

  // ------------------------------------------------------------
  // Frame 0: {0, 0}
  // ------------------------------------------------------------

  DataBlock block0;
  block0.module = module;
  block0.firstFrameIndex = 0;
  block0.frameRate = 100.0;
  block0.channelCount = 2;
  block0.frameCount = 1;
  block0.values = {
    0.0,
    0.0
  };

  QVERIFY(
    runtime->streamProcessor->process(
      block0));

  Frame frame0;

  QVERIFY(
    runtime->buffers.readFrame(
      frame0));

  QCOMPARE(
    frame0.raw().valueRef(0),
    0.0);

  QCOMPARE(
    frame0.raw().valueRef(1),
    0.0);

  QCOMPARE(
    frame0.calculated().valueRef(0),
    0.0);

  QCOMPARE(
    frame0.calculated().valueRef(1),
    -20.0);

  QCOMPARE(
    frame0.calculated().valueRef(2),
    0.0 + -20.0);

  // ------------------------------------------------------------
  // Frame 1: {1, 10}
  // ------------------------------------------------------------

  DataBlock block1;
  block1.module = module;
  block1.firstFrameIndex = 1;
  block1.frameRate = 100.0;
  block1.channelCount = 2;
  block1.frameCount = 1;
  block1.values = {
    1.0,
    10.0
  };

  QVERIFY(
    runtime->streamProcessor->process(
      block1));

  Frame frame1;

  QVERIFY(
    runtime->buffers.readFrame(
      frame1));

  QCOMPARE(
    frame1.raw().valueRef(0),
    1.0);

  QCOMPARE(
    frame1.raw().valueRef(1),
    10.0);

  QCOMPARE(
    frame1.calculated().valueRef(0),
    0.1);

  QCOMPARE(
    frame1.calculated().valueRef(1),
    -10.0);

  QCOMPARE(
    frame1.calculated().valueRef(2),
    0.1 + -10.0);

  // ------------------------------------------------------------
  // Frame 2: {2, 20}
  // ------------------------------------------------------------

  DataBlock block2;
  block2.module = module;
  block2.firstFrameIndex = 2;
  block2.frameRate = 100.0;
  block2.channelCount = 2;
  block2.frameCount = 1;
  block2.values = {
    2.0,
    20.0
  };

  QVERIFY(
    runtime->streamProcessor->process(
      block2));

  Frame frame2;

  QVERIFY(
    runtime->buffers.readFrame(
      frame2));

  QCOMPARE(
    frame2.raw().valueRef(0),
    2.0);

  QCOMPARE(
    frame2.raw().valueRef(1),
    20.0);

  QCOMPARE(
    frame2.calculated().valueRef(0),
    0.2);

  QCOMPARE(
    frame2.calculated().valueRef(1),
    0.0);

  QCOMPARE(
    frame2.calculated().valueRef(2),
    0.2 + 0.0);
}

void tst_database::test_database_archive()
{
  using namespace qds;

  auto db = get_db();

  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repo(db);

  SystemConfiguration cfg;

  QVERIFY(
    repo.load(
      ConfigurationId{1},
      cfg));

  QCOMPARE(
    cfg.modules().size(),
    std::size_t{1});

  const auto module =
    cfg.modules().front().id;

  QCOMPARE(
    cfg.moduleTags(module).size(),
    std::size_t{2});

  CalibrationRepository cr;

  QVERIFY(
    repo.loadCalibrations(
      cfg,
      cr));

  QCOMPARE(
    cr.sizeSignals(),
    std::size_t{1});

  QCOMPARE(
    cr.sizeSignalTypes(),
    std::size_t{1});

  // ------------------------------------------------------------
  // Archive description
  // ------------------------------------------------------------

  ArchiveDescriptionBuilder descriptionBuilder;

  ArchiveDescription description;

  QVERIFY(
    descriptionBuilder.build(
      cfg,
      description));

  ArchiveDescriptionWriter descriptionWriter;

  const auto path =
    getFilePath(
      "description.json");

  QVERIFY(
    descriptionWriter.write(
      path,
      description));

  // ------------------------------------------------------------
  // Layout for ArchiveManager
  // ------------------------------------------------------------

  SignalMemoryLayout layout;
  layout.build(cfg);

  auto directory =
    getCurrentFolder();

  ArchiveManager archive;

  QVERIFY(
    archive.initialize(
      directory,
      description,
      layout));

  // ------------------------------------------------------------
  // Runtime
  // ------------------------------------------------------------

  DataStreamSourceFactory factory;

  QVERIFY(
    factory.registerType(
      ModuleType::LTR11,
      [](const ModuleRuntimeConfiguration&,
         IClock&,
         IDataBlockSink&,
         IDataStreamEventSink&)
      {
        return std::make_unique<
          TestDataStreamSource>();
      }));

  FakeSchedulerClock clock;
  TestLogger logger;

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

  QCOMPARE(
    runtime->calibrations.sizeSignals(),
    std::size_t{1});

  QCOMPARE(
    runtime->calibrations.sizeSignalTypes(),
    std::size_t{1});

  // ------------------------------------------------------------
  // Stream
  // ------------------------------------------------------------

  DataStreamAnchor anchor;
  anchor.module = module;
  anchor.firstFrameIndex = 0;
  anchor.startTimestamp = Timestamp{2};
  anchor.startWallTime = WallClockTime{3};
  anchor.frameRate =
    static_cast<double>(
      BaseFrameFrequency);

  QVERIFY(
    runtime->streamProcessor->process(
      anchor));

  for (
    int i = 0;
    i < BaseFrameFrequency;
    ++i)
  {
    DataBlock block;

    block.module =
      module;

    block.firstFrameIndex =
      static_cast<uint64_t>(i);

    block.frameRate =
      static_cast<double>(
        BaseFrameFrequency);

    block.channelCount = 2;
    block.frameCount = 1;

    block.values = {
      static_cast<double>(i),
      static_cast<double>(i) * 10.0
    };

    QVERIFY(
      runtime->streamProcessor->process(
        block));
  }

  archive.close();

  // ------------------------------------------------------------
  // Signal definitions
  // ------------------------------------------------------------

  const auto* sdraw0 =
    findSignalDefinition(
      cfg.signalDefinitions(),
      "Raw0");

  const auto* sdraw1 =
    findSignalDefinition(
      cfg.signalDefinitions(),
      "Raw1");

  const auto* sda =
    findSignalDefinition(
      cfg.signalDefinitions(),
      "A");

  const auto* sdb =
    findSignalDefinition(
      cfg.signalDefinitions(),
      "B");

  const auto* sdc =
    findSignalDefinition(
      cfg.signalDefinitions(),
      "C");

  QVERIFY(sdraw0);
  QVERIFY(sdraw1);
  QVERIFY(sda);
  QVERIFY(sdb);
  QVERIFY(sdc);

  // ------------------------------------------------------------
  // Verify archive
  // ------------------------------------------------------------

  ArchiveFile file;

  for (const auto& desc : description.files)
  {
    auto fileName =
      getCurrentFolder() /
      desc.name;

    QVERIFY(
      file.open(
        fileName,
        OpenMode::Read));

    QVERIFY(
      file.header().isValid());



    const auto p =
      BaseFrameFrequency /
      desc.frequency;

    QCOMPARE(
      file.header().channelCount,
      desc.signalIds.size());

    QCOMPARE(
      file.header().recordCount,
      desc.frequency);

    QCOMPARE(
      file.header().firstTimestamp,
      uint64_t{2});

    const auto lastFrameIndex =
      static_cast<uint64_t>(
        (desc.frequency - 1) * p);

    const auto lastDelta =
      static_cast<uint64_t>(
        std::llround(
          static_cast<double>(
            lastFrameIndex) *
          1'000'000.0 /
          static_cast<double>(
            BaseFrameFrequency)));

    QCOMPARE(
      file.header().lastTimestamp,
      2 + lastDelta);

    QCOMPARE(
      file.header().sampleFrequency,
      desc.frequency);

    QCOMPARE(
      file.header().recordSize,
      sizeof(SampleRecordHeader) +
        desc.signalIds.size() *
          sizeof(float));



    const auto channelCount =
      file.header().channelCount;

    for (
      int n = 0;
      n < desc.frequency;
      ++n)
    {
      SampleRecordHeader rh;

      QVERIFY(
        file.readObject(rh));

      const auto frameIndex =
        static_cast<uint64_t>(
          n * p);

      QCOMPARE(
        rh.frameNumber,
        frameIndex);

      const auto delta =
        static_cast<uint64_t>(
          std::llround(
            static_cast<double>(
              frameIndex) *
            1'000'000.0 /
            static_cast<double>(
              BaseFrameFrequency)));

      QCOMPARE(
        rh.timestamp,
        2 + delta);

      QCOMPARE(
        rh.wallTime,
        3 + static_cast<int64_t>(
          delta));

      std::vector<float> values(
        channelCount,
        0.0f);

      QVERIFY(
        file.readArray(
          values.data(),
          channelCount));

      const double raw0 =
        static_cast<double>(
          frameIndex);

      const double raw1 =
        static_cast<double>(
          frameIndex) *
        10.0;

      for (const auto& signal : desc.signalIds)
      {
        const auto index =
          signal.index;

        if (signal.kind == SignalKind::Raw)
        {
          if (signal.id == sdraw0->id)
          {
            QCOMPARE(
              values[index],
              static_cast<float>(
                raw0));
          }
          else if (signal.id == sdraw1->id)
          {
            QCOMPARE(
              values[index],
              static_cast<float>(
                raw1));
          }
          else
          {
            QFAIL(
              "Unexpected signal in archive");
          }
        }
        else if (
          signal.kind ==
          SignalKind::Calculated)
        {
          double a;

          QVERIFY(
            runtime->calibrations
              .calibrateBySignal(
                sda->id,
                raw0,
                a));

          double b;

          QVERIFY(
            runtime->calibrations
              .calibrateBySignalType(
                sdb->signalType,
                raw1,
                b));

          const double c =
            a + b;

          if (signal.id == sda->id)
          {
            QCOMPARE(
              values[index],
              static_cast<float>(a));
          }
          else if (signal.id == sdb->id)
          {
            QCOMPARE(
              values[index],
              static_cast<float>(b));
          }
          else if (signal.id == sdc->id)
          {
            QCOMPARE(
              values[index],
              static_cast<float>(c));
          }
          else
          {
            QFAIL(
              "Unexpected signal in archive");
          }
        }
      }
    }

    QVERIFY(
      file.position() ==
      file.fileSize());

    file.close();

    QVERIFY(
      !file.isOpen());
  }
}

void tst_database::test_archiveReader_open()
{
  ArchiveReader reader;

  QVERIFY(reader.open(getCurrentFolder()));

  QVERIFY(reader.isOpen());

  const ArchiveDescription &description = reader.description();
  QCOMPARE(description.version, ArchiveDescriptionVersion);

  QCOMPARE(description.files.size(), 4);
  QCOMPARE(description.files.size(), reader.fileCount());

  for (std::size_t i = 0; i < reader.fileCount(); ++i)
  {
    const auto &file = reader.description().files[i];
    const auto &desc = reader.fileDescription(i);

    QCOMPARE(file.dataType, "float");
    QVERIFY(isValidArchiveFrequency(file.frequency));

    QVERIFY(!file.signalIds.empty());

    const auto expectedName =
      file.signalIds[0].kind == SignalKind::Raw
        ? "raw_" +
            std::to_string(file.frequency) +
            "Hz.dat"
        : "calculated_" +
            std::to_string(file.frequency) +
            "Hz.dat";

    QCOMPARE(
      file.name,
      expectedName);

    QCOMPARE(desc.name, file.name);
    QCOMPARE(desc.frequency, file.frequency);
    QCOMPARE(desc.dataType, file.dataType);
    QCOMPARE(desc.signalIds.size(), file.signalIds.size());
  }

  reader.close();

  QVERIFY(!reader.isOpen());
  QVERIFY(reader.fileHeader(0) == nullptr);
}

void tst_database::test_archiveReader_read()
{
  ArchiveReader reader;

  QVERIFY(reader.open(getCurrentFolder()));

  QVERIFY(reader.isOpen());

  const ArchiveDescription &description = reader.description();
  QCOMPARE(description.version, ArchiveDescriptionVersion);

  QCOMPARE(description.files.size(), 4);

  const auto &files = description.files;

  auto it = std::find_if(
    files.begin(),
    files.end(),
    [](const ArchiveFileDescription &desc)
    {
      return desc.frequency == 10 &&
             desc.signalIds[0].kind == SignalKind::Calculated;
    });

  QVERIFY(it != files.end());

  QCOMPARE(
    it->name,
    "calculated_10Hz.dat");

  int index = std::distance(files.begin(), it);

  QVERIFY(index >= 0 && index < files.size());

  ArchiveSample sample;

  for (
    int i = 0;
    i < files[index].frequency;
    ++i)
  {
    QVERIFY(
      reader.read(
        index,
        sample));

    const auto frameIndex =
      static_cast<uint64_t>(
        i * 100);

    QCOMPARE(
      sample.frameNumber,
      FrameNumber{frameIndex});

    QCOMPARE(
      sample.timestamp,
      Timestamp{
                2 + frameIndex * 1000});

    QCOMPARE(
      sample.wallTime,
      WallClockTime{
                    3 +
                    static_cast<int64_t>(
                      frameIndex * 1000)});

    QCOMPARE(
      sample.values.size(),
      std::size_t{2});

    QCOMPARE(
      sample.values.size(),
      files[index].signalIds.size());

    const double raw0 =
      static_cast<double>(
        frameIndex);

    const double raw1 =
      static_cast<double>(
        frameIndex) *
      10.0;

    const double a =
      raw0 * 0.1;

    const double b =
      raw1 - 20.0;

    const double c =
      a + b;

    QCOMPARE(
      sample.values[0],
      static_cast<float>(b));

    QCOMPARE(
      sample.values[1],
      static_cast<float>(c));
  }

  QVERIFY(!reader.read(index, sample));

  reader.close();

  QVERIFY(!reader.isOpen());
  QVERIFY(reader.fileHeader(0) == nullptr);
}

void tst_database::test_archiveReader_readFrame()
{
  using namespace qds;

  ArchiveReader reader;

  QVERIFY(
    reader.open(
      getCurrentFolder()));

  QVERIFY(
    reader.isOpen());

  const ArchiveDescription& description =
    reader.description();

  QCOMPARE(
    description.version,
    ArchiveDescriptionVersion);

  QCOMPARE(
    description.files.size(),
    std::size_t{4});

  const auto& files =
    description.files;

  auto it =
    std::find_if(
      files.begin(),
      files.end(),
      [](const ArchiveFileDescription& desc)
      {
        return
          desc.frequency == 10 &&
          desc.signalIds[0].kind ==
            SignalKind::Calculated;
      });

  QVERIFY(
    it != files.end());

  QCOMPARE(
    it->name,
    "calculated_10Hz.dat");

  const auto index =
    std::distance(
      files.begin(),
      it);

  QVERIFY(
    index >= 0 &&
    index <
      static_cast<decltype(index)>(
        files.size()));

  const auto frequency =
    files[index].frequency;

  const auto p =
    BaseFrameFrequency /
    frequency;

  ArchiveSample sample;

  // Проверяем в обратном порядке,
  // как и в исходном тесте.
  for (
    uint64_t i = frequency;
    i > 0;
    --i)
  {
    const auto frameIndex =
      (i - 1) * p;

    const FrameNumber frameNumber{
                                  static_cast<uint64_t>(
                                    frameIndex)};

    QVERIFY(
      reader.readFrame(
        index,
        frameNumber,
        sample));

    QCOMPARE(
      sample.frameNumber,
      frameNumber);

    const auto delta =
      static_cast<uint64_t>(
        std::llround(
          static_cast<double>(
            frameIndex) *
          1'000'000.0 /
          static_cast<double>(
            BaseFrameFrequency)));

    QCOMPARE(
      sample.timestamp,
      Timestamp{
                2 + delta});

    QCOMPARE(
      sample.wallTime,
      WallClockTime{
                    3 +
                    static_cast<int64_t>(
                      delta)});

    QCOMPARE(
      sample.values.size(),
      std::size_t{2});

    QCOMPARE(
      sample.values.size(),
      files[index].signalIds.size());

    const double raw0 =
      static_cast<double>(
        frameIndex);

    const double raw1 =
      static_cast<double>(
        frameIndex) *
      10.0;

    const double a =
      raw0 * 0.1;

    const double b =
      raw1 - 20.0;

    const double c =
      a + b;

    QCOMPARE(
      sample.values[0],
      static_cast<float>(b));

    QCOMPARE(
      sample.values[1],
      static_cast<float>(c));
  }

  // FrameNumber{0} теперь валиден.
  QVERIFY(
    reader.readFrame(
      index,
      FrameNumber{0},
      sample));

  QCOMPARE(
    sample.frameNumber,
    FrameNumber{0});

  // Не попадает в сетку 10 Hz.
  QVERIFY(
    !reader.readFrame(
      index,
      FrameNumber{99},
      sample));

  QVERIFY(
    !reader.readFrame(
      index,
      FrameNumber{999},
      sample));

  // За последним архивным кадром.
  QVERIFY(
    !reader.readFrame(
      index,
      FrameNumber{
                  static_cast<uint64_t>(
                    BaseFrameFrequency)},
      sample));

  reader.close();

  QVERIFY(
    !reader.isOpen());

  QVERIFY(
    reader.fileHeader(0) ==
    nullptr);
}

void tst_database::test_publisher()
{
  using namespace qds;

  auto db = get_db();

  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repo(db);

  SystemConfiguration cfg;

  QVERIFY(
    repo.load(
      ConfigurationId{1},
      cfg));

  SignalMemoryLayout layout;
  layout.build(cfg);

  BufferManager buffers;
  buffers.initialize(layout);

  SubscriptionManager subscriptions;
  TestPublisherSender sender;

  Publisher publisher(
    layout,
    subscriptions,
    sender,
    1000);

  const auto& definitions =
    cfg.signalDefinitions();

  QCOMPARE(
    definitions.size(),
    std::size_t{5});

  const auto* raw0 =
    findSignalDefinition(
      definitions,
      "Raw0");

  const auto* raw1 =
    findSignalDefinition(
      definitions,
      "Raw1");

  QVERIFY(raw0);
  QVERIFY(raw1);

  Subscription sub;

  sub.endpoint.address =
    "127.0.0.1";

  sub.endpoint.port =
    cfg.udpPort();

  sub.rate =
    PublishRate::Hz10;

  sub.signalIds = {
    raw0->id,
    raw1->id
  };

  const auto id =
    subscriptions.add(sub);

  QCOMPARE(
    id,
    SubscriptionId{1});

  DataEngine engine;

  QVERIFY(
    engine.initialize(
      buffers,
      publisher));

  QVERIFY(
    engine.start());

  // ------------------------------------------------------------
  // 1000 Hz input stream for one second
  // ------------------------------------------------------------

  for (uint64_t i = 0; i < 1000; ++i)
  {
    Frame frame;

    frame.initialize(layout);

    frame.number =
      FrameNumber{i};

    frame.timestamp =
      Timestamp{i * 2 + 2};

    frame.wallTime =
      WallClockTime{static_cast<int64_t>(i * 3 + 3)};

    frame.raw().valueRef(0) =
      static_cast<double>(i);

    frame.raw().valueRef(1) =
      static_cast<double>(i) * 10.0;

    buffers.publish(frame);

    QVERIFY(
      engine.process());
  }

  QCOMPARE(
    sender.sendCount,
    10);

  // ------------------------------------------------------------
  // Verify packets
  // ------------------------------------------------------------

  PacketReader reader;

  uint32_t sequence = 0;

  for (const auto& packet : sender.m_packets)
  {
    reader.clear();

    reader.append(
      packet.data(),
      packet.size());

    QVERIFY(
      reader.nextPacket());

    QCOMPARE(
      reader.packetType(),
      PacketType::LiveData);

    PublishHeader ldh;

    QVERIFY(
      reader.read(ldh));

    QCOMPARE(
      ldh.subscriptionId,
      SubscriptionId{1});

    QCOMPARE(
      ldh.sequence,
      sequence + 1);

    QCOMPARE(
      ldh.timestamp,
      sequence * 100 * 2 + 2);

    QCOMPARE(
      ldh.valueCount,
      2u);

    std::array<Sample, 2> samples{};

    QVERIFY(
      reader.readArray(
        samples.data(),
        samples.size()));

    QCOMPARE(
      reader.remaining(),
      std::size_t{0});

    QCOMPARE(
      samples[0].value,
      sequence * 100);

    QCOMPARE(
      samples[1].value,
      sequence * 100 * 10);

    ++sequence;
  }

  engine.stop();

  QVERIFY(
    !engine.isRunning());
}

void tst_database::test_publisher_raw_calculated()
{
  using namespace qds;

  auto db = get_db();

  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repo(db);

  SystemConfiguration cfg;

  QVERIFY(
    repo.load(
      ConfigurationId{1},
      cfg));

  QCOMPARE(
    cfg.modules().size(),
    std::size_t{1});

  const auto module =
    cfg.modules().front().id;

  QCOMPARE(
    cfg.moduleTags(module).size(),
    std::size_t{2});

  CalibrationRepository cr;

  QVERIFY(
    repo.loadCalibrations(
      cfg,
      cr));

  DataStreamSourceFactory factory;

  QVERIFY(
    factory.registerType(
      ModuleType::LTR11,
      [](const ModuleRuntimeConfiguration&,
         IClock&,
         IDataBlockSink&,
         IDataStreamEventSink&)
      {
        return std::make_unique<
          TestDataStreamSource>();
      }));

  NullArchiveWriter archive;
  FakeSchedulerClock clock(2, 3);
  TestLogger logger;

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

  SubscriptionManager subscriptions;
  TestPublisherSender sender;

  Publisher publisher(
    runtime->layout,
    subscriptions,
    sender,
    1000);

  const auto& definitions =
    cfg.signalDefinitions();

  QCOMPARE(
    definitions.size(),
    std::size_t{5});

  const auto* raw0 =
    findSignalDefinition(
      definitions,
      "Raw0");

  const auto* raw1 =
    findSignalDefinition(
      definitions,
      "Raw1");

  const auto* a =
    findSignalDefinition(
      definitions,
      "A");

  const auto* c =
    findSignalDefinition(
      definitions,
      "C");

  QVERIFY(raw0);
  QVERIFY(raw1);
  QVERIFY(a);
  QVERIFY(c);

  // ------------------------------------------------------------
  // Subscription #1: Raw0 + C, 10 Hz
  // ------------------------------------------------------------

  Subscription sub;

  sub.endpoint.address =
    "127.0.0.1";

  sub.endpoint.port =
    cfg.udpPort();

  sub.rate =
    PublishRate::Hz10;

  sub.signalIds = {
    raw0->id,
    c->id
  };

  const auto id1 =
    subscriptions.add(sub);

  // ------------------------------------------------------------
  // Subscription #2: Raw1 + A, 100 Hz
  // ------------------------------------------------------------

  sub.rate =
    PublishRate::Hz100;

  sub.signalIds = {
    raw1->id,
    a->id
  };

  const auto id2 =
    subscriptions.add(sub);

  QCOMPARE(
    id1,
    SubscriptionId{1});

  QCOMPARE(
    id2,
    SubscriptionId{2});

  // ------------------------------------------------------------
  // DataEngine
  // ------------------------------------------------------------

  QVERIFY(
    runtime->engine->initialize(
      runtime->buffers,
      publisher));

  QVERIFY(
    runtime->engine->start());

  // ------------------------------------------------------------
  // Stream anchor
  // ------------------------------------------------------------

  DataStreamAnchor anchor;

  anchor.module =
    module;

  anchor.firstFrameIndex = 0;

  anchor.startTimestamp =
    Timestamp{2};

  anchor.startWallTime =
    WallClockTime{3};

  anchor.frameRate = 1000.0;

  QVERIFY(
    runtime->streamProcessor->process(
      anchor));

  // ------------------------------------------------------------
  // 1000 input frames
  //
  // old TestDataSource:
  //   Raw0 = i
  //   Raw1 = i * 10
  // ------------------------------------------------------------

  for (uint64_t i = 0; i < 1000; ++i)
  {
    DataBlock block;

    block.module =
      module;

    block.firstFrameIndex = i;
    block.frameRate = 1000.0;
    block.channelCount = 2;
    block.frameCount = 1;

    block.values = {
      static_cast<double>(i),
      static_cast<double>(i) * 10.0
    };

    QVERIFY(
      runtime->streamProcessor->process(
        block));

    QVERIFY(
      runtime->engine->process());
  }

  QCOMPARE(
    sender.sendCount,
    10 + 100);

  QCOMPARE(
    sender.m_packets.size(),
    std::size_t{110});

  // ------------------------------------------------------------
  // Verify packets
  // ------------------------------------------------------------

  PacketReader reader;

  std::array<Sample, 2> samples{};

  uint32_t sequence1 = 0;
  uint32_t sequence2 = 0;

  for (const auto& packet : sender.m_packets)
  {
    reader.clear();

    reader.append(
      packet.data(),
      packet.size());

    QVERIFY(
      reader.nextPacket());

    QCOMPARE(
      reader.packetType(),
      PacketType::LiveData);

    PublishHeader ldh;

    QVERIFY(
      reader.read(ldh));

    QCOMPARE(
      ldh.valueCount,
      2u);

    QVERIFY(
      reader.readArray(
        samples.data(),
        samples.size()));

    QVERIFY(
      ldh.sequence > 0);

    const auto index =
      ldh.sequence - 1;

    if (
      ldh.subscriptionId ==
      SubscriptionId{1})
    {
      constexpr uint32_t period = 100;

      const double raw0Value =
        static_cast<double>(
          period * index);

      const double aValue =
        raw0Value * 0.1;

      const double bValue =
        -20.0 +
        static_cast<double>(
          index * 1000);

      const double cValue =
        aValue + bValue;

      QCOMPARE(
        ldh.sequence,
        ++sequence1);

      QCOMPARE(
        samples[0].value,
        raw0Value);

      QCOMPARE(
        samples[1].value,
        cValue);
    }
    else if (
      ldh.subscriptionId ==
      SubscriptionId{2})
    {
      constexpr uint32_t period = 10;

      const double raw1Value =
        static_cast<double>(
          index * period * 10);

      const double aValue =
        static_cast<double>(
          period * index) *
        0.1;

      QCOMPARE(
        ldh.sequence,
        ++sequence2);

      QCOMPARE(
        samples[0].value,
        raw1Value);

      QCOMPARE(
        samples[1].value,
        aValue);
    }
    else
    {
      QFAIL(
        "Unexpected subscription");
    }

    QCOMPARE(
      reader.remaining(),
      std::size_t{0});
  }

  QCOMPARE(
    sequence1,
    10u);

  QCOMPARE(
    sequence2,
    100u);

  runtime->engine->stop();

  QVERIFY(
    !runtime->engine->isRunning());
}