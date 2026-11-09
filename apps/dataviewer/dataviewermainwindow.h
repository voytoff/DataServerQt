#pragma once

#include "archivereader.h"
#include <QMainWindow>

#include <filesystem>

class QTreeWidget;
class QSplitter;
class QAction;

namespace qds
{

class ArchiveChartView;

class DataViewerMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit DataViewerMainWindow(
    QWidget* parent = nullptr);

private:
  void createActions();
  void createUi();

  void openArchive(
    const std::filesystem::path& path);

  void populateSignalTree();

private:
  ArchiveChartView* m_chartView = nullptr;
  QTreeWidget* m_signalTree = nullptr;
  QSplitter* m_splitter = nullptr;

  QToolBar* m_toolBar = nullptr;

  QAction* m_openArchiveAction = nullptr;
  QAction* m_themeAction = nullptr;

  std::unique_ptr<ArchiveReader> m_archiveReader;
};

}