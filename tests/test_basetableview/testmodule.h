#pragma once

#include <QObject>
#include <QMainWindow>
#include <memory>

namespace qds
{

class TestModule : QObject
{
  Q_OBJECT
public:
  explicit TestModule(QObject* parent = nullptr);

  void applicationSettingsDialog(QApplication& app);
  void calibrationDialog(QApplication& app);

  void createTimeChartView();
  void createArchiveChartView();

  void testWorker();
  void testController();
  bool createArchive();

  /*1*/ void createDynamicArchiveChartView();
  /*2*/ void createDynamicArchiveChartsView();

  /*3*/ void createCombinedArchiveChartView();
  /*4*/ void createCombinedArchiveChartView2();
  /*5*/ void createFinalArchiveChartView();
  /*6*/ void createFinalArchiveChartView2(
    QApplication *app);

private:
  std::unique_ptr<QMainWindow> m_mainWindow;
};

}
