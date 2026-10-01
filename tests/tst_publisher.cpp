#include "tst_publisher.h"
#include "packetreader.h"
#include "protocol/publishheader.h"
#include "publisher.h"
#include "subscription.h"
#include "systemconfiguration.h"
#include "qds/testpublishersender.h"
#include "testsrv.h"
#include <qtestcase.h>

tst_publisher::tst_publisher() { }
tst_publisher::~tst_publisher() = default;

void tst_publisher::test_publish_reuseWriter()
{
  using namespace qds;
  SystemConfiguration cfg  = createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  TestPublisherSender sender;
  SubscriptionManager subscriptions;

  Publisher publisher(layout, subscriptions, sender, 1000);

  // подписка на 3 сигнала (2 raw, 1 calc)
  Subscription sub
  {
    .endpoint = {
      .address = "127.0.0.1",
      .port = 5000
    },
    .rate = PublishRate::Hz10,
    .signalIds = {
      SignalId{0},
      SignalId{1},
      SignalId{23}
    },
  };

  QVERIFY(subscriptions.add(sub));

  Frame frame;
  frame.initialize(layout);

  frame.number = FrameNumber{0};
  frame.timestamp = Timestamp{100};
  frame.wallTime = WallClockTime{200};

  frame.raw().setValue(0, 10.0);
  frame.raw().setValue(1, 20.0);

  frame.calculated().setValue(0, 30.0);
  frame.calculated().setValue(1, 40.0);
  frame.calculated().setValue(2, 50.0);

  publisher.publish(frame);

  QCOMPARE(sender.sendCount, 1);

  PacketReader reader;
  reader.append(
    sender.m_packets[0].data(),
    sender.m_packets[0].size());

  QVERIFY(reader.nextPacket());

  PublishHeader hdr;

  QVERIFY(reader.read(hdr));

  QCOMPARE(hdr.subscriptionId, SubscriptionId{1u});
  QCOMPARE(hdr.valueCount, 3);
  QCOMPARE(hdr.timestamp, 100u);
  QCOMPARE(hdr.sequence, 1);

  std::array<double, 3> values;
  QVERIFY(reader.readArray(values.data(), values.size()));

  QCOMPARE(values[0], 10.0);
  QCOMPARE(values[1], 20.0);
  QCOMPARE(values[2], 50.0);

  QVERIFY(reader.remaining() == 0);

  frame.number =
    FrameNumber{100};

  frame.timestamp =
    Timestamp{100'100};

  frame.raw().setValue(0, 11.0);
  frame.raw().setValue(1, 21.0);

  frame.calculated().setValue(0, 31.0);
  frame.calculated().setValue(1, 41.0);
  frame.calculated().setValue(2, 51.0);

  publisher.publish(frame);

  QCOMPARE(
    sender.sendCount,
    2);

  publisher.publish(frame);

  QCOMPARE(sender.sendCount, 2);

  PacketReader reader2;

  reader2.append(
    sender.m_packets[1].data(),
    sender.m_packets[1].size());

  QVERIFY(
    reader2.nextPacket());

  PublishHeader hdr2;

  QVERIFY(
    reader2.read(hdr2));

  QCOMPARE(
    hdr2.subscriptionId,
    SubscriptionId{1u});

  QCOMPARE(
    hdr2.sequence,
    2u);

  QCOMPARE(
    hdr2.timestamp,
    100'100u);

  QCOMPARE(
    hdr2.valueCount,
    3u);

  std::array<double, 3> values2;

  QVERIFY(
    reader2.readArray(
      values2.data(),
      values2.size()));

  QCOMPARE(
    values2[0],
    11.0);

  QCOMPARE(
    values2[1],
    21.0);

  QCOMPARE(
    values2[2],
    51.0);

  QCOMPARE(
    reader2.remaining(),
    std::size_t{0});
}

void tst_publisher::test_publish_failSignal()
{
  using namespace qds;
  SystemConfiguration cfg  = createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  TestPublisherSender sender;
  SubscriptionManager subscriptions;

  // подписка на 3 сигнала (2 raw, 1 calc)
  Subscription sub
    {
    .endpoint = {
      .address = "127.0.0.1",
      .port = 5000
    },
     .rate = PublishRate::Hz10,
    .signalIds = {
      SignalId{0},
      SignalId{1},
      SignalId{23}
    },
     };

  QVERIFY(subscriptions.add(sub));

  Frame frame;
  frame.initialize(layout);

  frame.number = FrameNumber{0};
  frame.timestamp = Timestamp{100};
  frame.wallTime = WallClockTime{200};

  frame.raw().setValue(0, 10.0);
  frame.raw().setValue(1, 20.0);

  frame.calculated().setValue(0, 30.0);
  frame.calculated().setValue(1, 40.0);
  frame.calculated().setValue(2, 50.0);

  Publisher publisher(layout, subscriptions, sender, 1000);

  publisher.publish(frame);

  QCOMPARE(sender.sendCount, 1);

  PacketReader reader;
  reader.append(
    sender.m_packets[0].data(),
    sender.m_packets[0].size());

  QVERIFY(reader.nextPacket());

  PublishHeader hdr;

  QVERIFY(reader.read(hdr));

  QCOMPARE(hdr.subscriptionId, SubscriptionId{1u});
  QCOMPARE(hdr.valueCount, 3);
  QCOMPARE(hdr.timestamp, 100u);
  QCOMPARE(hdr.sequence, 1);

  std::array<double, 3> values;
  QVERIFY(reader.readArray(values.data(), values.size()));

  QCOMPARE(values[0], 10.0);
  QCOMPARE(values[1], 20.0);
  QCOMPARE(values[2], 50.0);

  QVERIFY(reader.remaining() == 0);

  auto s = subscriptions.find(SubscriptionId{1});
  s->signalIds[2] = SignalId{24};

  frame.number =
    FrameNumber{100};

  frame.timestamp =
    Timestamp{100'100};

  publisher.publish(frame);

  QCOMPARE(
    sender.sendCount,
    1);

  QCOMPARE(
    sender.m_packets.size(),
    std::size_t{1});

  publisher.publish(frame);

  QCOMPARE(sender.sendCount, 1);
  QCOMPARE(sender.m_packets.size(), 1);

  s = subscriptions.find(SubscriptionId{1});

  QVERIFY(s != nullptr);

  QCOMPARE(
    s->sequence,
    1u);

  QVERIFY(
    s->publishStarted);

  QCOMPARE(
    s->nextPublishFrame,
    FrameNumber{100});

  s->signalIds[2] = SignalId{23};

  publisher.publish(frame);

  QCOMPARE(sender.sendCount, 2u);

  s = subscriptions.find(
    SubscriptionId{1});

  QVERIFY(
    s != nullptr);

  QCOMPARE(
    s->sequence,
    2u);

  QCOMPARE(
    s->nextPublishFrame,
    FrameNumber{200});

  PacketReader reader2;

  reader2.append(
    sender.m_packets[1].data(),
    sender.m_packets[1].size());

  QVERIFY(
    reader2.nextPacket());

  PublishHeader hdr2;

  QVERIFY(
    reader2.read(hdr2));

  QCOMPARE(
    hdr2.sequence,
    2u);

  QCOMPARE(
    hdr2.timestamp,
    100'100u);
}

void tst_publisher::test_publish_publishRate()
{
  using namespace qds;
  SystemConfiguration cfg  = createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  TestPublisherSender sender;
  SubscriptionManager subscriptions;

  Subscription sub0
    {
    .endpoint = {
      .address = "127.0.0.1",
      .port = 35000
    },
     .rate = PublishRate::Hz100,
    .signalIds = {
      SignalId{17}
    },
     };
  QVERIFY(subscriptions.add(sub0));

  Subscription sub1
    {
    .endpoint = {
      .address = "127.0.0.1",
      .port = 3500
    },
     .rate = PublishRate::Hz10,
    .signalIds = {
      SignalId{4}
    },
     };
  QVERIFY(subscriptions.add(sub1));

  Subscription sub2
    {
    .endpoint = {
      .address = "127.0.0.1",
      .port = 35000
    },
     .rate = PublishRate::Hz1,
    .signalIds = {
      SignalId{23}
    },
     };
  QVERIFY(subscriptions.add(sub2));

  Frame frame;
  frame.initialize(layout);

  frame.number = FrameNumber{1};
  frame.timestamp = Timestamp{100};
  frame.wallTime = WallClockTime{200};

  frame.calculated().setValue(0, 30.0);
  frame.calculated().setValue(1, 40.0);
  frame.calculated().setValue(2, 50.0);

  Publisher publisher(layout, subscriptions, sender, 1000);

  for (int i = 1; i <= 2000; ++i) {
    frame.number = FrameNumber{static_cast<uint64_t>(i)};
    frame.timestamp = Timestamp{static_cast<uint64_t>(i*100)};
    frame.wallTime = WallClockTime{i*200};

    publisher.publish(frame);
  }

  QCOMPARE(sender.sendCount, 200 + 20 + 2);

  PacketReader reader;

  int n1 = 0;
  int n10 = 0;
  int n100 = 0;

  for (int n = 0; n < 222; ++n)
  {
    reader.clear();
    reader.append(
      sender.m_packets[n].data(),
      sender.m_packets[n].size());

    QVERIFY(reader.nextPacket());

    PublishHeader hdr;
    QVERIFY(reader.read(hdr));

    double value = 0.0;
    QVERIFY(
      reader.read(value));

    QVERIFY(reader.remaining() == 0);

    if (hdr.subscriptionId == SubscriptionId{1})
    {
      QCOMPARE(value, 30.0);
      QCOMPARE(
        hdr.sequence,
        static_cast<uint32_t>(n100 + 1));
      ++n100;
    }
    else if (hdr.subscriptionId == SubscriptionId{2})
    {
      QCOMPARE(value, 40.0);
      QCOMPARE(
        hdr.sequence,
        static_cast<uint32_t>(n10 + 1));
      ++n10;
    }
    else if (hdr.subscriptionId == SubscriptionId{3})
    {
      QCOMPARE(value, 50.0);
      QCOMPARE(
        hdr.sequence,
        static_cast<uint32_t>(n1 + 1));
      ++n1;
    }
    else
    {
      QFAIL("Unexpected subscription ID");
    }
  }
  QCOMPARE(n100, 200);
  QCOMPARE(n10, 20);
  QCOMPARE(n1, 2);
}

void tst_publisher::test_publish_phase_preserving()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  TestPublisherSender sender;
  SubscriptionManager subscriptions;

  Subscription sub
    {
     .endpoint = {
       .address = "127.0.0.1",
       .port = 5000
     },
     .rate = PublishRate::Hz100,
     .signalIds = {
       SignalId{17}
     },
  };

  QVERIFY(
    subscriptions.add(sub));

  Publisher publisher(
    layout,
    subscriptions,
    sender,
    1000);

  Frame frame;
  frame.initialize(layout);

  frame.calculated().setValue(
    0,
    123.0);

  const auto publish =
    [&](uint64_t frameNumber)
  {
    frame.number =
      FrameNumber{frameNumber};

    frame.timestamp =
      Timestamp{frameNumber};

    publisher.publish(frame);
  };

  // Первый доступный frame публикуется сразу.
  publish(10);

  QCOMPARE(
    sender.sendCount,
    1);

  auto subscription =
    subscriptions.find(
      SubscriptionId{1});

  QVERIFY(
    subscription != nullptr);

  QCOMPARE(
    subscription->sequence,
    1u);

  QCOMPARE(
    subscription->nextPublishFrame,
    FrameNumber{20});

  // До следующей точки расписания публикации нет.
  publish(15);

  QCOMPARE(
    sender.sendCount,
    1);

  QCOMPARE(
    subscription->nextPublishFrame,
    FrameNumber{20});

  // Frame 20 пропущен.
  // Первый доступный после него — 21.
  publish(21);

  QCOMPARE(
    sender.sendCount,
    2);

  QCOMPARE(
    subscription->sequence,
    2u);

  // Важно:
  // следующая точка остаётся 30,
  // а не сдвигается на 31.
  QCOMPARE(
    subscription->nextPublishFrame,
    FrameNumber{30});

  publish(25);

  QCOMPARE(
    sender.sendCount,
    2);

  publish(31);

  QCOMPARE(
    sender.sendCount,
    3);

  QCOMPARE(
    subscription->sequence,
    3u);

  QCOMPARE(
    subscription->nextPublishFrame,
    FrameNumber{40});

  publish(39);

  QCOMPARE(
    sender.sendCount,
    3);

  // Точное попадание в следующую точку.
  publish(40);

  QCOMPARE(
    sender.sendCount,
    4);

  QCOMPARE(
    subscription->sequence,
    4u);

  QCOMPARE(
    subscription->nextPublishFrame,
    FrameNumber{50});

  // Проверяем, какие именно frames реально ушли.
  const uint64_t expectedFrames[]
    {
      10,
      21,
      31,
      40
    };

  QCOMPARE(
    sender.m_packets.size(),
    std::size(expectedFrames));

  PacketReader reader;

  for (std::size_t i = 0;
       i < std::size(expectedFrames);
       ++i)
  {
    reader.clear();

    reader.append(
      sender.m_packets[i].data(),
      sender.m_packets[i].size());

    QVERIFY(
      reader.nextPacket());

    QCOMPARE(
      reader.packetType(),
      PacketType::LiveData);

    PublishHeader header;

    QVERIFY(
      reader.read(header));

    QCOMPARE(
      header.subscriptionId,
      SubscriptionId{1});

    QCOMPARE(
      header.sequence,
      static_cast<uint32_t>(i + 1));

    // В этом тесте timestamp специально равен frame.number.
    QCOMPARE(
      header.timestamp,
      expectedFrames[i]);
  }
}

void tst_publisher::test_publish_sendFailure()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  TestPublisherSender sender;
  SubscriptionManager subscriptions;

  Subscription sub
    {
     .endpoint = {
       .address = "127.0.0.1",
       .port = 5000
     },
     .rate = PublishRate::Hz100,
     .signalIds = {
       SignalId{17}
     },
  };

  QVERIFY(
    subscriptions.add(sub));

  Publisher publisher(
    layout,
    subscriptions,
    sender,
    1000);

  Frame frame;
  frame.initialize(layout);

  frame.number =
    FrameNumber{10};

  frame.timestamp =
    Timestamp{100};

  frame.calculated().setValue(
    0,
    123.0);

  // Первая отправка должна завершиться ошибкой.
  sender.failCount = 1;

  publisher.publish(frame);

  QCOMPARE(
    sender.sendCount,
    std::size_t{1});

  QCOMPARE(
    sender.m_packets.size(),
    std::size_t{0});

  auto subscription =
    subscriptions.find(
      SubscriptionId{1});

  QVERIFY(
    subscription != nullptr);

  // Ошибка send() не должна менять
  // состояние подписки.
  QCOMPARE(
    subscription->sequence,
    0u);

  QVERIFY(
    !subscription->publishStarted);

  QCOMPARE(
    subscription->nextPublishFrame,
    FrameNumber{0});

  // Повторяем тот же самый frame.
  // Теперь sender должен успешно отправить пакет.
  publisher.publish(frame);

  QCOMPARE(
    sender.sendCount,
    std::size_t{2});

  QCOMPARE(
    sender.m_packets.size(),
    std::size_t{1});

  QCOMPARE(
    subscription->sequence,
    1u);

  QVERIFY(
    subscription->publishStarted);

  QCOMPARE(
    subscription->nextPublishFrame,
    FrameNumber{20});

  // Проверяем, что sequence внутри реально
  // отправленного пакета тоже равен 1.
  PacketReader reader;

  reader.append(
    sender.m_packets[0].data(),
    sender.m_packets[0].size());

  QVERIFY(
    reader.nextPacket());

  QCOMPARE(
    reader.packetType(),
    PacketType::LiveData);

  PublishHeader header;

  QVERIFY(
    reader.read(header));

  QCOMPARE(
    header.subscriptionId,
    SubscriptionId{1});

  QCOMPARE(
    header.sequence,
    1u);

  QCOMPARE(
    header.timestamp,
    100u);
}

void tst_publisher::test_publish_sendFailure_afterStart()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  TestPublisherSender sender;
  SubscriptionManager subscriptions;

  Subscription sub
    {
     .endpoint = {
       .address = "127.0.0.1",
       .port = 5000
     },
     .rate = PublishRate::Hz100,
     .signalIds = {
       SignalId{17}
     },
  };

  QVERIFY(
    subscriptions.add(sub));

  Publisher publisher(
    layout,
    subscriptions,
    sender,
    1000);

  Frame frame;
  frame.initialize(layout);

  frame.calculated().setValue(
    0,
    123.0);

  const auto publish =
    [&](uint64_t frameNumber)
  {
    frame.number =
      FrameNumber{frameNumber};

    frame.timestamp =
      Timestamp{frameNumber};

    publisher.publish(frame);
  };

  publish(10);

  QCOMPARE(
    sender.sendCount,
    1);

  auto subscription =
    subscriptions.find(
      SubscriptionId{1});

  QVERIFY(
    subscription != nullptr);

  QCOMPARE(
    subscription->sequence,
    1u);

  QCOMPARE(
    subscription->nextPublishFrame,
    FrameNumber{20});

  QCOMPARE(
    sender.m_packets.size(),
    std::size_t{1});

  // Frame 20 пропущен.
  // На первом доступном frame 21 происходит попытка отправки,
  // но sender возвращает ошибку.
  sender.failCount = 1;

  publish(21);

  QCOMPARE(
    sender.sendCount,
    2);

  QCOMPARE(
    sender.m_packets.size(),
    std::size_t{1});

  subscription =
    subscriptions.find(
      SubscriptionId{1});

  QVERIFY(
    subscription != nullptr);

  // Ошибка send() не должна изменить
  // состояние подписки.
  QCOMPARE(
    subscription->sequence,
    1u);

  QCOMPARE(
    subscription->nextPublishFrame,
    FrameNumber{20});

  publish(21);

  QCOMPARE(
    sender.sendCount,
    3);

  QCOMPARE(
    subscription->sequence,
    2u);

  QCOMPARE(
    subscription->nextPublishFrame,
    FrameNumber{30});

  QCOMPARE(
    sender.m_packets.size(),
    std::size_t{2});

  // Проверяем sequence и timestamp
  // реально отправленных пакетов.
  const uint64_t expectedTimestamps[]
    {
      10,
      21
    };

  PacketReader reader;

  for (std::size_t i = 0;
       i < std::size(expectedTimestamps);
       ++i)
  {
    reader.clear();

    reader.append(
      sender.m_packets[i].data(),
      sender.m_packets[i].size());

    QVERIFY(
      reader.nextPacket());

    QCOMPARE(
      reader.packetType(),
      PacketType::LiveData);

    PublishHeader header;

    QVERIFY(
      reader.read(header));

    QCOMPARE(
      header.subscriptionId,
      SubscriptionId{1});

    QCOMPARE(
      header.sequence,
      static_cast<uint32_t>(i + 1));

    QCOMPARE(
      header.timestamp,
      expectedTimestamps[i]);
  }
}

void tst_publisher::test_publish_zeroFrameRate()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  TestPublisherSender sender;
  SubscriptionManager subscriptions;

  Subscription sub
    {
     .endpoint = {
       .address = "127.0.0.1",
       .port = 5000
     },
     .rate = PublishRate::Hz100,
     .signalIds = {
       SignalId{17}
     },
     };

  QVERIFY(
    subscriptions.add(sub));

  Publisher publisher(
    layout,
    subscriptions,
    sender,
    0);

  Frame frame;
  frame.initialize(layout);

  frame.number =
    FrameNumber{10};

  frame.timestamp =
    Timestamp{100};

  publisher.publish(frame);

  QCOMPARE(
    sender.sendCount,
    0);

  QCOMPARE(
    sender.m_packets.size(),
    std::size_t{0});

  auto subscription =
    subscriptions.find(
      SubscriptionId{1});

  QVERIFY(
    subscription != nullptr);

  // Нулевая базовая частота не должна запускать
  // публикацию и изменять состояние подписки.
  QCOMPARE(
    subscription->sequence,
    0u);

  QVERIFY(
    !subscription->publishStarted);

  QCOMPARE(
    subscription->nextPublishFrame,
    FrameNumber{0});
}

void tst_publisher::test_publish_zeroSubscriptionRate()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  TestPublisherSender sender;
  SubscriptionManager subscriptions;

  Subscription sub
    {
     .endpoint = {
       .address = "127.0.0.1",
       .port = 5000
     },
     .rate = static_cast<PublishRate>(0),
     .signalIds = {
       SignalId{17}
     },
  };

  QVERIFY(
    subscriptions.add(sub));

  Publisher publisher(
    layout,
    subscriptions,
    sender,
    1000);

  Frame frame;
  frame.initialize(layout);

  frame.number =
    FrameNumber{10};

  frame.timestamp =
    Timestamp{100};

  publisher.publish(frame);

  QCOMPARE(
    sender.sendCount,
    0);

  QCOMPARE(
    sender.m_packets.size(),
    std::size_t{0});

  auto subscription =
    subscriptions.find(
      SubscriptionId{1});

  QVERIFY(
    subscription != nullptr);

  // Нулевая частота подписки не должна запускать
  // публикацию и изменять состояние подписки.
  QCOMPARE(
    subscription->sequence,
    0u);

  QVERIFY(
    !subscription->publishStarted);

  QCOMPARE(
    subscription->nextPublishFrame,
    FrameNumber{0});
}

void tst_publisher::test_publish_subscriptionRateAboveFrameRate()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  TestPublisherSender sender;
  SubscriptionManager subscriptions;

  Subscription sub
    {
     .endpoint = {
       .address = "127.0.0.1",
       .port = 5000
     },
     .rate = PublishRate::Hz100,
     .signalIds = {
       SignalId{17}
     },
     };

  QVERIFY(
    subscriptions.add(sub));

  Publisher publisher(
    layout,
    subscriptions,
    sender,
    10);

  Frame frame;
  frame.initialize(layout);

  frame.number =
    FrameNumber{10};

  frame.timestamp =
    Timestamp{100};

  publisher.publish(frame);

  QCOMPARE(
    sender.sendCount,
    0);

  QCOMPARE(
    sender.m_packets.size(),
    std::size_t{0});

  auto subscription =
    subscriptions.find(
      SubscriptionId{1});

  QVERIFY(
    subscription != nullptr);

  // Частота подписки выше базовой частоты
  // не должна изменять состояние подписки.
  QCOMPARE(
    subscription->sequence,
    0u);

  QVERIFY(
    !subscription->publishStarted);

  QCOMPARE(
    subscription->nextPublishFrame,
    FrameNumber{0});
}

void tst_publisher::test_publish_subscriptionRateNotDivisor()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  TestPublisherSender sender;
  SubscriptionManager subscriptions;

  Subscription sub
    {
     .endpoint = {
       .address = "127.0.0.1",
       .port = 5000
     },
     .rate = static_cast<PublishRate>(30),
     .signalIds = {
       SignalId{17}
     },
     };

  QVERIFY(
    subscriptions.add(sub));

  Publisher publisher(
    layout,
    subscriptions,
    sender,
    1000);

  Frame frame;
  frame.initialize(layout);

  frame.number =
    FrameNumber{10};

  frame.timestamp =
    Timestamp{100};

  publisher.publish(frame);

  QCOMPARE(
    sender.sendCount,
    0);

  QCOMPARE(
    sender.m_packets.size(),
    std::size_t{0});

  auto subscription =
    subscriptions.find(
      SubscriptionId{1});

  QVERIFY(
    subscription != nullptr);

  // Частота подписки должна целочисленно
  // делить базовую частоту.
  QCOMPARE(
    subscription->sequence,
    0u);

  QVERIFY(
    !subscription->publishStarted);

  QCOMPARE(
    subscription->nextPublishFrame,
    FrameNumber{0});
}

void tst_publisher::test_publish_failSubscription()
{
  using namespace qds;
  SystemConfiguration cfg  = createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  TestPublisherSender sender;
  SubscriptionManager subscriptions;

  Publisher publisher(layout, subscriptions, sender, 1000);

  Subscription sub1
    {
     .endpoint = {
       .address = "127.0.0.1",
       .port = 5000
     },
     .rate = PublishRate::Hz10,
     .signalIds = {
        SignalId{0},
        SignalId{23}
     },
  };

  Subscription sub2
    {
     .endpoint = {
       .address = "127.0.0.1",
       .port = 5000
     },
     .rate = PublishRate::Hz100,
     .signalIds = {
        SignalId{1},
        SignalId{24}
     },
  };

  QVERIFY(subscriptions.add(sub2));
  QVERIFY(subscriptions.add(sub1));

  Frame frame;
  frame.initialize(layout);

  frame.number = FrameNumber{0};
  frame.timestamp = Timestamp{100};
  frame.wallTime = WallClockTime{200};

  frame.raw().setValue(0, 10.0);
  frame.raw().setValue(1, 20.0);

  frame.calculated().setValue(0, 30.0);
  frame.calculated().setValue(1, 40.0);
  frame.calculated().setValue(2, 50.0);

  publisher.publish(frame);

  QCOMPARE(
    sender.sendCount,
    1);

  QCOMPARE(
    sender.m_packets.size(),
    std::size_t{1});

  auto validSubscription =
    subscriptions.find(
      SubscriptionId{2});

  QVERIFY(
    validSubscription != nullptr);

  QVERIFY(
    validSubscription->publishStarted);

  QCOMPARE(
    validSubscription->sequence,
    1u);

  QCOMPARE(
    validSubscription->nextPublishFrame,
    FrameNumber{100});

  PacketReader reader;
  reader.append(
    sender.m_packets[0].data(),
    sender.m_packets[0].size());

  QVERIFY(reader.nextPacket());

  PublishHeader hdr;

  QVERIFY(reader.read(hdr));

  QCOMPARE(hdr.subscriptionId, SubscriptionId{2u});
  QCOMPARE(hdr.valueCount, 2);
  QCOMPARE(hdr.timestamp, 100u);
  QCOMPARE(hdr.sequence, 1);

  std::array<double, 2> values;
  QVERIFY(reader.readArray(values.data(), values.size()));

  QCOMPARE(values[0], 10.0);
  QCOMPARE(values[1], 50.0);

  QVERIFY(reader.remaining() == 0);

  auto failedSubscription =
    subscriptions.find(
      SubscriptionId{1});

  QVERIFY(
    failedSubscription != nullptr);

  QCOMPARE(
    failedSubscription->sequence,
    0u);

  QVERIFY(
    !failedSubscription->publishStarted);

  QCOMPARE(
    failedSubscription->nextPublishFrame,
    FrameNumber{0});
}