#include "tst_signalstorage.h"
#include "buffermanager.h"
#include "frame.h"
#include "signalmemory.h"
#include "systemconfiguration.h"
#include "testsrv.h"
#include <qtestcase.h>

tst_signalstorage::tst_signalstorage() { }
tst_signalstorage::~tst_signalstorage() = default;

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
  SystemConfiguration cfg = createTestConfig00();
  const auto& definitions = cfg.signalDefinitions();
  SignalMemoryLayout layout;
  layout.build(cfg); // TODO сделать bool build(...)

  QCOMPARE(layout.calculatedSignalCount(), 2);
  QCOMPARE(layout.rawSignalCount(), 2);

  QVERIFY(layout.contains({0}));
  QVERIFY(layout.contains({1}));
  QVERIFY(layout.contains({2}));
  QVERIFY(layout.contains({3}));


  manager.initialize(layout);

  Frame &frame1 = manager.beginWrite();

  //QVERIFY(frame1.number.value == 0);
  QVERIFY(frame1.raw().values().size() == 2);
  QVERIFY(frame1.calculated().values().size() == 2);

  frame1.raw().setValue(0, 10);
  manager.publish();

  auto read1 = manager.readFrame();
  QCOMPARE(read1.raw().values()[0], 10);


  Frame &frame2 = manager.beginWrite();

  //QVERIFY(frame2.number.value == 0);
  QVERIFY(frame2.raw().values().size() == 2);
  QVERIFY(frame2.calculated().values().size() == 2);

  frame2.calculated().setValue(0, 0xFF);
  frame2.raw().setValue(1, 11);
  manager.publish();

  Frame &frame3 = manager.beginWrite();

  auto read2 = manager.readFrame();
  QCOMPARE(read2.raw().values()[0], 0);
  QCOMPARE(read2.raw().values()[1], 11);
  QCOMPARE(read2.calculated().values()[0], 0xFF);

  manager.publish();

  Frame &frame4 = manager.beginWrite();

  auto read3 = manager.readFrame();
  QCOMPARE(read3.raw().values()[0], 10);
  QCOMPARE(read3.calculated().values()[0], 0);
}

void tst_signalstorage::test_raw_memory()
{
  using namespace qds;

  SystemConfiguration cfg = createTestConfig00();

  SignalMemoryLayout layout;
  layout.build(cfg);

  RawMemory memory;
  memory.initialize(layout.rawSignalCount());

  QCOMPARE(memory.values().size(), 2);

  memory.setValue(0, 10.0);

  QCOMPARE(memory.value(0), 10.0);


  std::array<double,2> block =
    {
      20.0,
      30.0
    };

  memory.setValues(
    0,
    block);

  QCOMPARE(memory.value(0), 20.0);
  QCOMPARE(memory.value(1), 30.0);


  std::array<double, 2> snapshot{};

  memory.snapshot(snapshot);

  QCOMPARE(snapshot[0], 20.0);
  QCOMPARE(snapshot[1], 30.0);
}

void tst_signalstorage::test_datasource_layout_contains()
{
  using namespace qds;

  SystemConfiguration cfg = createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  QVERIFY(layout.contains(SignalId{0}));
  QVERIFY(layout.contains(SignalId{1}));
  QVERIFY(layout.contains(SignalId{2}));
  QVERIFY(layout.contains(SignalId{3}));
  QVERIFY(layout.contains(SignalId{4}));

  QVERIFY(!layout.contains({100}));
}

void tst_signalstorage::test_datasource_layout_reference()
{
  using namespace qds;

  SystemConfiguration cfg = createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  const auto raw = layout.reference({0});
  QVERIFY(raw.isValid());
  QCOMPARE(raw.area, SignalMemoryArea::Raw);
  QCOMPARE(raw.index, 0u);

  const auto calculated = layout.reference({2});
  QVERIFY(calculated.isValid());
  QCOMPARE(calculated.area, SignalMemoryArea::Calculated);
  QCOMPARE(calculated.index, 0u);

  const auto unknown = layout.reference({100});
  QVERIFY(!unknown.isValid());
}

void tst_signalstorage::test_datasource_layout_unknownModule()
{
  using namespace qds;

  SystemConfiguration cfg = createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  QCOMPARE(layout.rawOffset(ModuleId{999}), std::nullopt);
}

void tst_signalstorage::test_datasource_layout_rebuild()
{
  using namespace qds;

  SystemConfiguration cfg = createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  QCOMPARE(layout.rawSignalCount(), 2u);
  QCOMPARE(layout.calculatedSignalCount(), 3u);

  // Добавляем новый raw signal.
  QVERIFY(cfg.addTag({
    .tag = {10},
    .module = {0},
    .channel = {2}
  }));

  QVERIFY(cfg.addSignalDefinition({
    .id = {5},
    .name = "RAW5",
    .kind = SignalKind::Raw,
    .source = {10},
    .archiveFrequency = 100
  }));

  layout.build(cfg);

  QCOMPARE(layout.rawSignalCount(), 3u);
  QCOMPARE(layout.calculatedSignalCount(), 3u);

  const auto ref = layout.reference({5});
  QVERIFY(ref.isValid());
  QCOMPARE(ref.area, SignalMemoryArea::Raw);
  QCOMPARE(ref.index, 2u);
}

void tst_signalstorage::test_bufferManager_cancelWrite()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  BufferManager buffers;
  buffers.initialize(layout);

  QVERIFY(!buffers.ready());

  Frame& frame = buffers.beginWrite();

  frame.number = {123};

  buffers.cancelWrite();

  QVERIFY(!buffers.ready());

  // После cancel можно снова начать запись.
  Frame& frame2 = buffers.beginWrite();

  QCOMPARE(&frame2, &frame);
  QCOMPARE(frame2.number, FrameNumber{123});

  buffers.cancelWrite();
}
