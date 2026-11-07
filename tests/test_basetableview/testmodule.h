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

  /*1*/ void createDynamicArchiveChartView();
  /*2*/ void createDynamicArchiveChartsView();

  /*3*/ void createCombinedArchiveChartView();
  /*4*/ void createCombinedArchiveChartView2();
  /*5*/ void createFinalArchiveChartView();

private:
  std::unique_ptr<QMainWindow> m_mainWindow;
};

}
