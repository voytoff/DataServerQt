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

  SystemConfiguration cfg =
    createTestConfig_1module_2raw_2calc();

  SignalMemoryLayout layout;
  layout.build(cfg);

  QCOMPARE(
    layout.calculatedSignalCount(),
    uint32_t{2});

  QCOMPARE(
    layout.rawSignalCount(),
    uint32_t{2});

  BufferManager buffers;
  buffers.initialize(layout);

  QVERIFY(
    !buffers.ready());

  Frame empty;

  QVERIFY(
    !buffers.readFrame(empty));

  // ==========================
  // Первый frame

  Frame frame1;
  frame1.initialize(layout);

  frame1.number =
    FrameNumber{10};

  frame1.raw().setValue(
    0,
    10.0);

  buffers.publish(frame1);

  QVERIFY(
    buffers.ready());

  Frame read1;

  QVERIFY(
    buffers.readFrame(read1));

  QCOMPARE(
    read1.number,
    FrameNumber{10});

  QCOMPARE(
    read1.raw().value(0),
    10.0);

  QVERIFY(
    std::isnan(
      read1.raw().value(1)));

  // ==========================
  // Второй frame

  Frame frame2;
  frame2.initialize(layout);

  frame2.number =
    FrameNumber{20};

  frame2.raw().setValue(
    1,
    11.0);

  frame2.calculated().setValue(
    0,
    255.0);

  buffers.publish(frame2);

  Frame read2;

  QVERIFY(
    buffers.readFrame(read2));

  QCOMPARE(
    read2.number,
    FrameNumber{20});

  QVERIFY(
    std::isnan(
      read2.raw().value(0)));

  QCOMPARE(
    read2.raw().value(1),
    11.0);

  QCOMPARE(
    read2.calculated().value(0),
    255.0);

  // read1 — независимый snapshot.
  // Следующая публикация его не изменила.

  QCOMPARE(
    read1.number,
    FrameNumber{10});

  QCOMPARE(
    read1.raw().value(0),
    10.0);

  QVERIFY(
    std::isnan(
      read1.raw().value(1)));
}

void tst_signalstorage::test_raw_memory()
{
  using namespace qds;

  SystemConfiguration cfg = createTestConfig_1module_2raw_2calc();

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

void tst_signalstorage::test_signalMemoryLayout_rawOffset_rawCount()
{
  SystemConfiguration cfg =
    createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  QCOMPARE(
    layout.rawOffset(ModuleId{0}),
    std::optional<uint32_t>{0});

  QCOMPARE(
    layout.rawCount(ModuleId{0}),
    std::optional<uint32_t>{2});


  QCOMPARE(
    layout.rawOffset(ModuleId{1}),
    std::optional<uint32_t>{2});

  QCOMPARE(
    layout.rawCount(ModuleId{1}),
    std::optional<uint32_t>{3});

  QCOMPARE(
    layout.rawOffset(ModuleId{2}),
    std::optional<uint32_t>{5});

  QCOMPARE(
    layout.rawCount(ModuleId{2}),
    std::optional<uint32_t>{2});

  QVERIFY(
    !layout.rawOffset(ModuleId{999}).has_value());

  QVERIFY(
    !layout.rawCount(ModuleId{999}).has_value());
}

void tst_signalstorage::test_bufferManager_thread_safe()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_1module_2raw_2calc();

  SignalMemoryLayout layout;
  layout.build(cfg);

  QCOMPARE(
    layout.calculatedSignalCount(),
    uint32_t{2});

  QCOMPARE(
    layout.rawSignalCount(),
    uint32_t{2});

  BufferManager buffers;
  buffers.initialize(layout);

  Frame frame;
  Frame read;

  for (uint32_t n = 0; n < 10; ++n)
  {
    frame.initialize(layout);

    frame.number =
      FrameNumber{n};

    frame.raw().setValue(
      0,
      n * 10.0);

    buffers.publish(frame);

    QVERIFY(
      buffers.ready());

    bool result = false;

    std::thread thread([&]() {
      result = buffers.readFrame(read);
    });

    thread.join();

    QVERIFY(result);

    QCOMPARE(
      read.number,
      FrameNumber{n});

    QCOMPARE(
      read.raw().value(0),
      n * 10.0);

    QVERIFY(
      std::isnan(
        read.raw().value(1)));
  }
}

void tst_signalstorage::test_bufferManager_thread_safe2()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_1module_2raw_2calc();

  SignalMemoryLayout layout;
  layout.build(cfg);

  BufferManager buffers;
  buffers.initialize(layout);

  constexpr uint64_t frameCount =
    10'000;

  std::atomic_bool writerFinished{false};
  std::atomic_bool failed{false};

  std::thread writer(
    [&]()
    {
      for (uint64_t n = 1;
           n <= frameCount;
           ++n)
      {
        Frame frame;
        frame.initialize(layout);

        frame.number =
          FrameNumber{n};

        frame.raw().setValue(
          0,
          static_cast<double>(n));

        frame.raw().setValue(
          1,
          static_cast<double>(n * 10));

        frame.calculated().setValue(
          0,
          static_cast<double>(n * 100));

        buffers.publish(frame);
      }

      writerFinished.store(true);
    });

  std::thread reader(
    [&]()
    {
      Frame frame;

      while (!writerFinished.load())
      {
        if (!buffers.readFrame(frame))
          continue;

        const uint64_t n =
          frame.number.value;

        if (frame.raw().value(0) !=
            static_cast<double>(n))
        {
          failed.store(true);
          return;
        }

        if (frame.raw().value(1) !=
            static_cast<double>(n * 10))
        {
          failed.store(true);
          return;
        }

        if (frame.calculated().value(0) !=
            static_cast<double>(n * 100))
        {
          failed.store(true);
          return;
        }
      }
    });

  writer.join();
  reader.join();

  QVERIFY(
    !failed.load());

  Frame last;

  QVERIFY(
    buffers.readFrame(last));

  QCOMPARE(
    last.number,
    FrameNumber{frameCount});

  QCOMPARE(
    last.raw().value(0),
    static_cast<double>(frameCount));

  QCOMPARE(
    last.raw().value(1),
    static_cast<double>(
      frameCount * 10));

  QCOMPARE(
    last.calculated().value(0),
    static_cast<double>(
      frameCount * 100));
}