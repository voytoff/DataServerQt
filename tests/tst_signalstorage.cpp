#include "tst_signalstorage.h"
#include "buffermanager.h"
#include "frame.h"
#include "rawmemory.h"
#include "systemconfiguration.h"
#include <qtestcase.h>

tst_signalstorage::tst_signalstorage() { }
tst_signalstorage::~tst_signalstorage() = default;

static qds::SystemConfiguration createTestConfig() {
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{0};
  cfg.addModule(m);

  TagInfo t1{.tag = {0}, .module = {0}, .channel = {0}};
  cfg.addTag(t1);

  TagInfo t2{.tag = {1}, .module = {0}, .channel = {0}};
  cfg.addTag(t2);

  SignalDefinition sd1 {.id = 0, .name = "U000", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 100};
  cfg.addSignalDefinition(sd1);

  SignalDefinition sd2 {.id = 1, .name = "D10", .kind = SignalKind::Calculated, .source = {0}, .archiveFrequency = 10, .calibrationId = 72};
  cfg.addSignalDefinition(sd2);

  SignalDefinition sd3 {.id = 2, .name = "P100", .kind = SignalKind::Calculated, .source = {0}, .archiveFrequency = 100, .formulaId = 15};
  cfg.addSignalDefinition(sd3);

  SignalDefinition sd4 {.id = 3, .name = "U001", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 10};
  cfg.addSignalDefinition(sd4);

  return cfg;
}

void tst_signalstorage::test_signalstorage_base()
{
  qds::Frame frame;

  frame.number.value = 1;
  frame.timestamp.value = 1000;
  frame.wallTime.unixMicroseconds = 1750000000000000;

}

void tst_signalstorage::test_buffer_manager()
{
  using namespace qds;
  BufferManager manager;
  SystemConfiguration cfg = createTestConfig();
  const auto& definitions = cfg.signalDefinitions();
  SignalMemoryLayout layout;
  layout.build(cfg); // TODO сделать bool build(...)

  QCOMPARE(layout.calculatedSignalCount(), 2);
  QCOMPARE(layout.rawSignalCount(), 2);

  QVERIFY(layout.contains(0));
  QVERIFY(layout.contains(1));
  QVERIFY(layout.contains(2));
  QVERIFY(layout.contains(3));


  manager.initialize(layout);

  Frame &frame1 = manager.beginWrite();

  //QVERIFY(frame1.number.value == 0);
  QVERIFY(frame1.storage.rawValues().size() == 2);
  QVERIFY(frame1.storage.calculatedValues().size() == 2);

  frame1.storage.setRawValue(0, 10);
  manager.publish();

  auto &read1 = manager.readFrame();
  QCOMPARE(read1.storage.rawValues()[0], 10);


  Frame &frame2 = manager.beginWrite();

  //QVERIFY(frame2.number.value == 0);
  QVERIFY(frame2.storage.rawValues().size() == 2);
  QVERIFY(frame2.storage.calculatedValues().size() == 2);

  frame2.storage.setCalculatedValue(0, 0xFF);
  frame2.storage.setRawValue(1, 11);
  manager.publish();

  frame1 = manager.beginWrite();

  auto &read2 = manager.readFrame();
  QCOMPARE(read2.storage.rawValues()[0], 0);
  QCOMPARE(read2.storage.rawValues()[1], 11);
  QCOMPARE(read2.storage.calculatedValues()[0], 0xFF);

  manager.publish();

  frame2 = manager.beginWrite();

  auto &read3 = manager.readFrame();
  QCOMPARE(read3.storage.rawValues()[0], 10);
  QCOMPARE(read3.storage.calculatedValues()[0], 0);
}

void tst_signalstorage::test_raw_memory()
{
  using namespace qds;

  SystemConfiguration cfg = createTestConfig();

  SignalMemoryLayout layout;
  layout.build(cfg);

  RawMemory memory;
  memory.initialize(layout);

  QCOMPARE(memory.values().size(), 2);

  memory.setValue(0, 10.0f);

  QCOMPARE(memory.value(0), 10.0f);


  std::array<double,2> block =
    {
      20.0f,
      30.0f
    };

  memory.setValues(
    0,
    block);

  QCOMPARE(memory.value(0), 20.0f);
  QCOMPARE(memory.value(1), 30.0f);


  std::array<double, 2> snapshot{};

  memory.snapshot(snapshot);

  QCOMPARE(snapshot[0], 20.0f);
  QCOMPARE(snapshot[1], 30.0f);
}