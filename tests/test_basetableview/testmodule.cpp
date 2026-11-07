#include "testmodule.h"
#include "applicationsettings.h"
#include "applicationsettingsloader.h"
#include "applicationsettingsdialog.h"
#include "archivechartcontroller.h"
#include "archivechartseries.h"
#include "archivechartview.h"
#include "archivechartworker.h"
#include "archivereader.h"
#include "archivesignalsource.h"
#include "calibration.h"
#include "calibrationdialog.h"
#include "datatypes.h"
#include "settingssection.h"
#include "timechartview.h"
#include "timerangecontroller.h"

#include <timeaxis.h>
#include <cmath>
#include <array>
#include <memory>
#include <vector>
#include <algorithm>

#include <QThread>
#include <QTimer>
#include <QBoxLayout>
#include <QApplication>
#include <QToolBar>
#include <QMainWindow>
#include <QLineSeries>
#include <QVector>
#include <QChart>
#include <QLegend>
#include <QLegendMarker>
#include <QScatterSeries>

namespace qds
{

namespace
{

ArchiveChartSeries::Segments toChartSegments(
  const ArchiveSignalSegments& segments,
  const TimeAxis& timeAxis)
{
  ArchiveChartSeries::Segments result;
  result.reserve(segments.size());

  for (const auto& segment : segments)
  {
    QVector<QPointF> points;
    points.reserve(segment.points.size());

    for (const auto& sample : segment.points)
    {
      if (!std::isfinite(sample.value))
        continue;

      points.append(QPointF(
        timeAxis.chartSeconds(sample.timestamp),
        sample.value));
    }

    if (!points.empty())
      result.push_back(std::move(points));
  }

  return result;
}

QVector<QPointF> toChartPoints(
  const ArchiveSignalPoints& samples,
  const TimeAxis& timeAxis)
{
  QVector<QPointF> points;
  points.reserve(samples.size());

  for (const auto& sample : samples)
  {
    if (!std::isfinite(sample.value))
      continue;

    points.append(QPointF(
      timeAxis.chartSeconds(sample.timestamp),
      sample.value));
  }

  return points;
}

Timestamp toTimestamp(
  double seconds,
  Timestamp origin)
{
  const auto microseconds =
    static_cast<int64_t>(
      std::llround(seconds * 1'000'000.0));

  if (microseconds < 0)
  {
    const auto delta =
      static_cast<uint64_t>(-(microseconds + 1)) + 1;

    return Timestamp{
      delta > origin.value
        ? 0
        : origin.value - delta
    };
  }

  const auto delta =
    static_cast<uint64_t>(microseconds);

  return Timestamp{
    delta > UINT64_MAX - origin.value
      ? UINT64_MAX
      : origin.value + delta
  };
}

}

TestModule::TestModule(QObject *parent)
  : QObject(parent)
{
}

void TestModule::applicationSettingsDialog(QApplication &app)
{
  using namespace qds;

  const QString fileName =
    "D:/DATASERVER/ini/configeditor_test.ini";

  const SettingsSections sections =
    SettingsSection::Database |
    SettingsSection::Archive |
    SettingsSection::Log;

  ApplicationSettings settings;
  QString error;

  if (!ApplicationSettingsLoader::load(
        fileName,
        settings,
        error,
        sections))
  {
    qWarning() << error;
    return;
  }

  ApplicationSettingsDialog dialog(
    settings,
    sections);

  QObject::connect(
    &dialog,
    &ApplicationSettingsDialog::settingsApplied,
    &dialog,
    [](SettingsSections changed)
    {
      qDebug() << "Изменены разделы:" << changed;
    });

  dialog.exec();
}

void TestModule::calibrationDialog(QApplication &app)
{
  using namespace qds;

  Calibration calibration;

  calibration.kind = CalibrationKind::Polynomial;
  calibration.description = "Проверка редактора";

  if (calibration.kind == CalibrationKind::Table)
  {
    calibration.points = {
                          {0, 0.0, 0.0},
                          };
  }
  else
  {
    calibration.coefficients = {
      0
    };
  }
  CalibrationDialog dialog(calibration, nullptr);

  if (dialog.exec() != QDialog::Accepted)
    return;

  const Calibration result =
    dialog.calibration();

  if (calibration.kind == CalibrationKind::Table)
  {
    qDebug() << "Calibration:"
             << result.description
             << "Points:" << result.points.size();

    for (const auto& point : result.points)
      qDebug() << point.index << point.x << point.y;
  }
  else
  {
    qDebug() << "Calibration:"
             << result.description
             << "Coefficients:" << result.coefficients.size();

    for (const auto& point : result.coefficients)
      qDebug() << point;
  }
}

void TestModule::createTimeChartView()
{
  using namespace qds;

  m_mainWindow = std::make_unique<QMainWindow>();
  auto* window = m_mainWindow.get();
  m_mainWindow->resize(1100, 900);

  auto* timeAxis = new TimeAxis(window);

  const Timestamp origin{10'000'000};

  timeAxis->setOriginTimestamp(origin);
  timeAxis->setZeroTimestamp(origin);

  auto* view = new TimeChartView(window);
  view->setTimeAxis(timeAxis);

  view->setWallClockReference(
    origin,
    WallClockTime{1'700'000'000'000'000LL});

  auto* series = new QLineSeries();
  series->setName("Тестовый сигнал");

  for (int i = 0; i <= 1000; ++i)
  {
    const double seconds = i * 0.01;

    const Timestamp timestamp{
      origin.value + static_cast<uint64_t>(
        std::llround(seconds * 1'000'000.0))
    };

    const double value =
      std::sin(seconds * 2.0);

    series->append(
      timeAxis->chartSeconds(timestamp),
      value);
  }

  view->addSeries(series);
  view->setTimeRange(0.0, 10.0);
  view->autoScaleY();

  auto* constantSeries = new QLineSeries();
  constantSeries->setName("Постоянный сигнал");

  for (int i = 0; i <= 100; ++i)
  {
    const double seconds = i * 0.1;

    constantSeries->append(
      seconds,
      2.5);
  }

  view->addSeries(constantSeries);
  view->autoScaleY();


  auto* view1 = new TimeChartView(window);
  view1->setTimeAxis(timeAxis);

  view1->setWallClockReference(
    origin,
    WallClockTime{1'700'000'000'000'000LL});

  auto* series1 = new QLineSeries();
  series1->setName("Тестовый сигнал");

  for (int i = 0; i <= 1000; ++i)
  {
    const double seconds = i * 0.01;

    const Timestamp timestamp{
      origin.value + static_cast<uint64_t>(
        std::llround(seconds * 1'000'000.0))
    };

    const double value =
      std::sin(seconds * 2.0);

    series1->append(
      timeAxis->chartSeconds(timestamp),
      value);
  }

  view1->addSeries(series1);
  view1->setTimeRange(0.0, 10.0);
  view1->autoScaleY();

  auto* w = new QWidget(window);
  auto* layout = new QVBoxLayout(w);

  layout->addWidget(view);
  layout->addWidget(view1);

  w->setLayout(layout);

  m_mainWindow->setCentralWidget(w);


  auto* toolbar = m_mainWindow->addToolBar("Время");

  auto* localAction =
    toolbar->addAction("Локальное время");

  localAction->setCheckable(true);

  QObject::connect(
    localAction,
    &QAction::toggled,
    window,
    [timeAxis](bool checked)
    {
      timeAxis->setMode(
        checked
          ? TimeAxisMode::LocalTime
          : TimeAxisMode::RelativeSeconds);
    });

  toolbar->addAction(
    "Ноль = 0",
    window,
    [timeAxis, origin]()
    {
      timeAxis->setZeroTimestamp(origin);
    });

  toolbar->addAction(
    "Ноль = 5 с",
    window,
    [timeAxis, origin]()
    {
      timeAxis->setZeroTimestamp(
        Timestamp{origin.value + 5'000'000});
    });

  toolbar->addAction(
    "Ноль = 8 с",
    window,
    [timeAxis, origin]()
    {
      timeAxis->setZeroTimestamp(
        Timestamp{origin.value + 8'000'000});
    });

  toolbar->addAction(
    "Автомасштаб Y",
    window,
    [view]()
    {
      view->autoScaleY();
    });

  auto* constantAction =
    toolbar->addAction("Постоянный сигнал");

  constantAction->setCheckable(true);
  constantAction->setChecked(true);

  QObject::connect(
    constantAction,
    &QAction::toggled,
    window,
    [view, constantSeries](bool visible)
    {
      constantSeries->setVisible(visible);
      view->autoScaleY();
    });


  auto* rangeController = new TimeRangeController(window);

  rangeController->addView(view);
  rangeController->addView(view1);

  m_mainWindow->show();
}

void TestModule::createArchiveChartView()
{
  using namespace qds;

  const std::filesystem::path archivePath =
    "D:/DATASERVER/archive/_test";

  auto reader = std::make_shared<ArchiveReader>();

  if (!reader->open(archivePath))
  {
    qWarning() << "Не удалось открыть архив";
    return;
  }

  ArchiveSignalSource source(*reader);

  // Пока используем известный SignalId.
  constexpr SignalId signalId{10};

  ArchiveSignalPoints samples;

  // Для первой проверки читаем ограниченное число точек.
  if (!source.readRange(
        signalId,
        Timestamp{0},
        Timestamp{60'000'000},
        100'000,
        samples))
  {
    qWarning() << "Ошибка чтения архивного сигнала";
    return;
  }

  if (samples.empty())
  {
    qWarning() << "В указанном диапазоне нет данных";
    return;
  }

  m_mainWindow = std::make_unique<QMainWindow>();

  auto* window = m_mainWindow.get();

  auto* timeAxis = new TimeAxis(window);

  // Начало координат графика совпадает с первым отсчётом.
  timeAxis->setOriginTimestamp(samples.front().timestamp);
  timeAxis->setZeroTimestamp(samples.front().timestamp);

  auto* view = new TimeChartView(window);

  view->setTimeAxis(timeAxis);

  auto* series = new QLineSeries();
  series->setName("Архивный сигнал");

  series->replace(toChartPoints(samples, *timeAxis));

  view->addSeries(series);

  const double firstX =
    timeAxis->chartSeconds(samples.front().timestamp);

  const double lastX =
    timeAxis->chartSeconds(samples.back().timestamp);

  if (lastX > firstX)
    view->setTimeRange(firstX, lastX);
  else
    view->setTimeRange(firstX - 0.5, firstX + 0.5);

  view->autoScaleY();

  window->setCentralWidget(view);
  window->resize(1200, 700);
  window->show();
}

void TestModule::testWorker()
{
  using namespace qds;

  qRegisterMetaType<ArchiveSignalPoints>();
  qRegisterMetaType<SignalId>();
  qRegisterMetaType<Timestamp>();

  m_mainWindow = std::make_unique<QMainWindow>();

  auto* window = m_mainWindow.get();

  auto* thread = new QThread(window);

  auto* worker = new ArchiveChartWorker(
    "D:/DATASERVER/archive/_test");

  worker->moveToThread(thread);

  QObject::connect(
    thread,
    &QThread::started,
    worker,
    &ArchiveChartWorker::initialize);

  QObject::connect(
    worker,
    &ArchiveChartWorker::initialized,
    window,
    [worker](bool success)
    {
      if (!success)
      {
        qWarning() << "Не удалось открыть архив";
        return;
      }

      QMetaObject::invokeMethod(
        worker,
        [worker]()
        {
          auto cancel =
            std::make_shared<std::atomic_bool>(false);

          worker->readPlot(
            1,
            SignalId{10},
            Timestamp{100'000},
            Timestamp{600'000},
            4,
            cancel);
        },
        Qt::QueuedConnection);
    });

  QObject::connect(
    worker,
    &ArchiveChartWorker::plotReady,
    window,
    [](uint64_t requestId,
       bool success,
       ArchiveSignalSegments segments)
    {
      qDebug()
      << "Request:" << requestId
      << "Success:" << success
      << "Segments:" << segments.size();
    });

  window->resize(800, 500);
  window->show();

  thread->start();
}

void TestModule::testController()
{
  using namespace qds;

  m_mainWindow = std::make_unique<QMainWindow>();

  auto* window = m_mainWindow.get();

  auto* controller = new ArchiveChartController(
    "D:/DATASERVER/archive/_test",
    window);

  QObject::connect(
    controller,
    &ArchiveChartController::plotReady,
    window,
    [](bool success, ArchiveSignalSegments segments)
    {
      qDebug()
      << "Success:" << success
      << "Segments:" << segments.size();
    });

  // Пока запрос отправляем после инициализации.
  // Запрос сразу после создания контроллера.
  /*/ ======
  controller->requestPlot(
    SignalId{10},
    Timestamp{100'000},
    Timestamp{600'000},
    4);
*/
  controller->requestPlot(
    SignalId{10},
    Timestamp{100'000},
    Timestamp{200'000},
    4);

  controller->requestPlot(
    SignalId{10},
    Timestamp{200'000},
    Timestamp{400'000},
    4);

  controller->requestPlot(
    SignalId{10},
    Timestamp{100'000},
    Timestamp{600'000},
    4);

  window->resize(800, 500);
  window->show();
}

void TestModule::createDynamicArchiveChartView()
{
  using namespace qds;

  m_mainWindow = std::make_unique<QMainWindow>();

  auto* window = m_mainWindow.get();

  auto* controller = new ArchiveChartController(
    "D:/DATASERVER/archive/__raw0",
    window);

  auto* timeAxis = new TimeAxis(window);

  auto* view = new TimeChartView(window);
  view->setTimeAxis(timeAxis);
  auto* chartSeries = new ArchiveChartSeries(view);

  constexpr SignalId signalId{25};

  QObject::connect(
    controller,
    &ArchiveChartController::timeRangeReady,
    window,
    [timeAxis, view](
      bool success,
      Timestamp firstTimestamp,
      Timestamp lastTimestamp)
    {
      if (!success)
      {
        qWarning() << "Не удалось определить диапазон архива";
        return;
      }

      timeAxis->setOriginTimestamp(firstTimestamp);
      timeAxis->setZeroTimestamp(firstTimestamp);

      const double minimum =
        timeAxis->chartSeconds(firstTimestamp);

      const double maximum =
        timeAxis->chartSeconds(lastTimestamp);

      if (maximum <= minimum)
      {
        qWarning() << "Некорректный временной диапазон";
        return;
      }

      view->setTimeRange(minimum, maximum);

      qDebug()
        << "Archive time range:"
        << firstTimestamp.value
        << lastTimestamp.value
        << "Duration:"
        << maximum;
    });

  window->setCentralWidget(view);

  QObject::connect(
    controller,
    &ArchiveChartController::plotReady,
    window,
    [timeAxis, chartSeries](
      bool success,
      ArchiveSignalSegments segments)
    {
      if (!success)
      {
        qWarning() << "Ошибка чтения архивных данных";
        return;
      }

      chartSeries->setSegments(
        toChartSegments(segments, *timeAxis));
    });

  QObject::connect(
    view,
    &TimeChartView::visibleTimeRangeChanged,
    window,
    [controller, timeAxis, view, signalId](
      double minimum,
      double maximum)
    {
      const auto origin =
        timeAxis->originTimestamp();

      const auto first =
        toTimestamp(minimum, origin);

      const auto last =
        toTimestamp(maximum, origin);

      const auto width =
        view->chart()->plotArea().width();

      const auto maxPoints =
        static_cast<std::size_t>(
          std::max(2.0, width * 2.0));

      controller->requestPlot(
        signalId,
        first,
        last,
        maxPoints);
    });

  controller->requestTimeRange(
    signalId);

  controller->requestPlot(
    signalId,
    Timestamp{100'000},
    Timestamp{600'000},
    2400);

  window->resize(800, 500);
  window->show();
}


void TestModule::createDynamicArchiveChartsView()
{
  using namespace qds;

  m_mainWindow = std::make_unique<QMainWindow>();

  auto* window = m_mainWindow.get();

  auto* widget = new QWidget(window);
  auto* layout = new QVBoxLayout(widget);

  auto* tmc = new TimeRangeController(window);

  window->setCentralWidget(widget);

  constexpr std::array<SignalId, 2> signalIds = {
                                                 SignalId{26},
                                                 SignalId{27},
                                                 };

  struct ChartContext
  {
    TimeAxis* timeAxis = nullptr;
    TimeChartView* view = nullptr;
    Timestamp firstTimestamp{};
    Timestamp lastTimestamp{};
    bool ready = false;
  };

  struct SharedState
  {
    std::array<ChartContext, 2> charts;
    bool initialized = false;
  };

  auto state = std::make_shared<SharedState>();

  for (std::size_t i = 0; i < signalIds.size(); ++i)
  {
    const SignalId id = signalIds[i];

    auto* controller = new ArchiveChartController(
      "D:/DATASERVER/archive/__raw01_100_10",
      window);

    auto* timeAxis = new TimeAxis(window);
    auto* view = new TimeChartView(window);

    view->setTimeAxis(timeAxis);

    layout->addWidget(view);
    tmc->addView(view);

    auto* chartSeries = new ArchiveChartSeries(view);

    chartSeries->setName(
      id == SignalId{26}
        ? QStringLiteral("Канал 1 (100 Гц)")
        : QStringLiteral("Канал 2 (10 Гц)"));

    state->charts[i].timeAxis = timeAxis;
    state->charts[i].view = view;

    QObject::connect(
      controller,
      &ArchiveChartController::timeRangeReady,
      window,
      [state, i](
        bool success,
        Timestamp firstTimestamp,
        Timestamp lastTimestamp)
      {
        if (!success)
        {
          qWarning() << "Не удалось определить диапазон архива";
          return;
        }

        auto& current = state->charts[i];

        current.firstTimestamp = firstTimestamp;
        current.lastTimestamp = lastTimestamp;
        current.ready = true;

        if (state->initialized)
          return;

        for (const auto& chart : state->charts)
        {
          if (!chart.ready)
            return;
        }

        Timestamp commonFirst =
          state->charts.front().firstTimestamp;

        Timestamp commonLast =
          state->charts.front().lastTimestamp;

        for (const auto& chart : state->charts)
        {
          commonFirst = std::min(
            commonFirst,
            chart.firstTimestamp);

          commonLast = std::max(
            commonLast,
            chart.lastTimestamp);
        }

        if (commonLast <= commonFirst)
        {
          qWarning() << "Некорректный временной диапазон";
          return;
        }

        state->initialized = true;

        for (auto& chart : state->charts)
        {
          chart.timeAxis->setOriginTimestamp(commonFirst);
          chart.timeAxis->setZeroTimestamp(commonFirst);
        }

        const double duration =
          state->charts.front().timeAxis->chartSeconds(
            commonLast);

        for (auto& chart : state->charts)
        {
          chart.view->setTimeRange(0.0, duration);
        }
      });

    QObject::connect(
      controller,
      &ArchiveChartController::plotReady,
      window,
      [timeAxis, chartSeries](
        bool success,
        ArchiveSignalSegments segments)
      {
        if (!success)
        {
          qWarning() << "Ошибка чтения архивных данных";
          return;
        }

        chartSeries->setSegments(
          toChartSegments(segments, *timeAxis));
      });

    QObject::connect(
      view,
      &TimeChartView::visibleTimeRangeChanged,
      window,
      [controller, timeAxis, view, id, state](
        double minimum,
        double maximum)
      {
        if (!state->initialized)
          return;

        const auto origin =
          timeAxis->originTimestamp();

        const auto first =
          toTimestamp(minimum, origin);

        const auto last =
          toTimestamp(maximum, origin);

        const auto width =
          view->chart()->plotArea().width();

        const auto maxPoints =
          static_cast<std::size_t>(
            std::max(2.0, width * 2.0));

        controller->requestPlot(
          id,
          first,
          last,
          maxPoints);
      });

    controller->requestTimeRange(id);
  }

  window->resize(800, 500);
  window->show();
}


void TestModule::createCombinedArchiveChartView()
{
  using namespace qds;

  m_mainWindow = std::make_unique<QMainWindow>();

  auto* window = m_mainWindow.get();

  auto* view = new TimeChartView(window);
  auto* timeAxis = new TimeAxis(view);

  view->setTimeAxis(timeAxis);
  window->setCentralWidget(view);

  constexpr std::array<SignalId, 2> signalIds = {
                                                 SignalId{26},
                                                 SignalId{27},
                                                 };

  struct SignalContext
  {
    ArchiveChartController* controller = nullptr;
    ArchiveChartSeries* series = nullptr;
    Timestamp firstTimestamp{};
    Timestamp lastTimestamp{};
    bool ready = false;
  };

  struct SharedState
  {
    std::array<SignalContext, 2> signalCnts;
    bool initialized = false;
  };

  auto state = std::make_shared<SharedState>();

  for (std::size_t i = 0; i < signalIds.size(); ++i)
  {
    const SignalId id = signalIds[i];

    auto* controller = new ArchiveChartController(
      "D:/DATASERVER/archive/__raw01_100_10",
      window);

    auto* series = new ArchiveChartSeries(view);

    series->setName(
      i == 0
        ? QStringLiteral("Канал 1 (100 Гц)")
        : QStringLiteral("Канал 2 (10 Гц)"));

    series->setColor(
      i == 0
        ? QColor(Qt::blue)
        : QColor(Qt::red));

    state->signalCnts[i].controller = controller;
    state->signalCnts[i].series = series;

    QObject::connect(
      controller,
      &ArchiveChartController::timeRangeReady,
      window,
      [state, i, timeAxis, view](
        bool success,
        Timestamp firstTimestamp,
        Timestamp lastTimestamp)
      {
        if (!success)
        {
          qWarning() << "Не удалось определить диапазон архива";
          return;
        }

        auto& signal = state->signalCnts[i];

        signal.firstTimestamp = firstTimestamp;
        signal.lastTimestamp = lastTimestamp;
        signal.ready = true;

        if (state->initialized)
          return;

        for (const auto& item : state->signalCnts)
        {
          if (!item.ready)
            return;
        }

        Timestamp commonFirst =
          state->signalCnts.front().firstTimestamp;

        Timestamp commonLast =
          state->signalCnts.front().lastTimestamp;

        for (const auto& item : state->signalCnts)
        {
          commonFirst = std::min(
            commonFirst,
            item.firstTimestamp);

          commonLast = std::max(
            commonLast,
            item.lastTimestamp);
        }

        if (commonLast <= commonFirst)
        {
          qWarning() << "Некорректный временной диапазон";
          return;
        }

        state->initialized = true;

        timeAxis->setOriginTimestamp(commonFirst);
        timeAxis->setZeroTimestamp(commonFirst);

        const double duration =
          timeAxis->chartSeconds(commonLast);

        view->setTimeRange(0.0, duration);
      });

    QObject::connect(
      controller,
      &ArchiveChartController::plotReady,
      window,
      [timeAxis, series](
        bool success,
        ArchiveSignalSegments segments)
      {
        if (!success)
        {
          qWarning() << "Ошибка чтения архивных данных";
          return;
        }

        series->setSegments(
          toChartSegments(segments, *timeAxis));
      });

    controller->requestTimeRange(id);
  }

  QObject::connect(
    view,
    &TimeChartView::visibleTimeRangeChanged,
    window,
    [state, timeAxis, view, signalIds](
      double minimum,
      double maximum)
    {
      if (!state->initialized)
        return;

      const auto origin =
        timeAxis->originTimestamp();

      const auto first =
        toTimestamp(minimum, origin);

      const auto last =
        toTimestamp(maximum, origin);

      const auto width =
        view->chart()->plotArea().width();

      const auto maxPoints =
        static_cast<std::size_t>(
          std::max(2.0, width * 2.0));

      for (std::size_t i = 0; i < signalIds.size(); ++i)
      {
        state->signalCnts[i].controller->requestPlot(
          signalIds[i],
          first,
          last,
          maxPoints);
      }
    });

  window->resize(1000, 500);
  window->show();
}

void TestModule::createCombinedArchiveChartView2()
{
  using namespace qds;

  m_mainWindow = std::make_unique<QMainWindow>();

  auto* window = m_mainWindow.get();

  auto* view = new TimeChartView(window);
  auto* timeAxis = new TimeAxis(view);

  view->setTimeAxis(timeAxis);
  view->setYAxisMode(YAxisMode::Multiple);

  auto* axis2 = view->addValueAxis(Qt::AlignRight);

  window->setCentralWidget(view);

  constexpr std::array<SignalId, 2> signalIds =
  {
    SignalId{26},
    SignalId{27},
  };

  struct SignalContext
  {
    ArchiveChartController* controller = nullptr;
    ArchiveChartSeries* series = nullptr;
    Timestamp firstTimestamp{};
    Timestamp lastTimestamp{};
    bool ready = false;
  };

  struct SharedState
  {
    std::array<SignalContext, 2> signalCnts;
    bool initialized = false;
  };

  auto state = std::make_shared<SharedState>();

  for (std::size_t i = 0; i < signalIds.size(); ++i)
  {
    const SignalId id = signalIds[i];

    auto* controller = new ArchiveChartController(
      "D:/DATASERVER/archive/__raw01_100_10",
      window);

    auto* series = new ArchiveChartSeries(view);

    series->setName(
      i == 0
        ? QStringLiteral("Канал 1 (100 Гц)")
        : QStringLiteral("Канал 2 (10 Гц)"));

    series->setColor(
      i == 0
        ? QColor(Qt::blue)
        : QColor(Qt::red));

    if (i == 1)
      series->setValueAxis(axis2);

    state->signalCnts[i].controller = controller;
    state->signalCnts[i].series = series;

    QObject::connect(
      controller,
      &ArchiveChartController::timeRangeReady,
      window,
      [state, i, timeAxis, view](
        bool success,
        Timestamp firstTimestamp,
        Timestamp lastTimestamp)
      {
        if (!success)
        {
          qWarning() << "Не удалось определить диапазон архива";
          return;
        }

        auto& signal = state->signalCnts[i];

        signal.firstTimestamp = firstTimestamp;
        signal.lastTimestamp = lastTimestamp;
        signal.ready = true;

        if (state->initialized)
          return;

        for (const auto& item : state->signalCnts)
        {
          if (!item.ready)
            return;
        }

        Timestamp commonFirst =
          state->signalCnts.front().firstTimestamp;

        Timestamp commonLast =
          state->signalCnts.front().lastTimestamp;

        for (const auto& item : state->signalCnts)
        {
          commonFirst = std::min(
            commonFirst,
            item.firstTimestamp);

          commonLast = std::max(
            commonLast,
            item.lastTimestamp);
        }

        if (commonLast <= commonFirst)
        {
          qWarning() << "Некорректный временной диапазон";
          return;
        }

        state->initialized = true;

        timeAxis->setOriginTimestamp(commonFirst);
        timeAxis->setZeroTimestamp(commonFirst);

        const double duration =
          timeAxis->chartSeconds(commonLast);

        view->setTimeRange(0.0, duration);
      });

    QObject::connect(
      controller,
      &ArchiveChartController::plotReady,
      window,
      [timeAxis, series](
        bool success,
        ArchiveSignalSegments segments)
      {
        if (!success)
        {
          qWarning() << "Ошибка чтения архивных данных";
          return;
        }

        series->setSegments(
          toChartSegments(segments, *timeAxis));
      });

    controller->requestTimeRange(id);
  }

  QObject::connect(
    view,
    &TimeChartView::visibleTimeRangeChanged,
    window,
    [state, timeAxis, view, signalIds](
      double minimum,
      double maximum)
    {
      if (!state->initialized)
        return;

      const auto origin =
        timeAxis->originTimestamp();

      const auto first =
        toTimestamp(minimum, origin);

      const auto last =
        toTimestamp(maximum, origin);

      const auto width =
        view->chart()->plotArea().width();

      const auto maxPoints =
        static_cast<std::size_t>(
          std::max(2.0, width * 2.0));

      for (std::size_t i = 0; i < signalIds.size(); ++i)
      {
        state->signalCnts[i].controller->requestPlot(
          signalIds[i],
          first,
          last,
          maxPoints);
      }
    });

  window->resize(1000, 500);
  window->show();
}

void TestModule::createFinalArchiveChartView()
{
  using namespace qds;

  m_mainWindow = std::make_unique<QMainWindow>();

  auto* window = m_mainWindow.get();

  auto* view = new ArchiveChartView(window);

  view->setArchivePath(
    "D:/DATASERVER/archive/__raw01_100_10");

  view->setYAxisMode(YAxisMode::Multiple);

  view->addSignal(
    SignalId{26},
    "Канал 1 (100 Гц)",
    Qt::blue);

  view->addSignal(
    SignalId{27},
    "Канал 2 (10 Гц)",
    Qt::red);

  window->setCentralWidget(view);

  window->resize(1000, 500);
  window->show();
}

}
