#include "testmodule.h"
#include "applicationsettings.h"
#include "applicationsettingsloader.h"
#include "applicationsettingsdialog.h"
#include "archivereader.h"
#include "archivesignalsource.h"
#include "calibration.h"
#include "calibrationdialog.h"
#include "datatypes.h"
#include "settingssection.h"
#include "timechartview.h"
#include "timerangecontroller.h"
#include <QApplication>
#include <QToolBar>
#include <QMainWindow>
#include <qboxlayout.h>

#include <timeaxis.h>

#include <QLineSeries>
#include <QVector>

#include <cmath>

namespace qds
{

namespace qds
{

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

}

TestModule::TestModule(QObject *parent)
  : QObject(parent)
{
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

}
