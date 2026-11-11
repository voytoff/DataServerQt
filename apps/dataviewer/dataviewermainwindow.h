#pragma once

#include "archivereader.h"

#include <QMainWindow>

#include <filesystem>
#include <memory>
#include <vector>

class QTreeWidget;
class QTreeWidgetItem;
class QSplitter;
class QTabWidget;
class QTableView;
class QComboBox;
class QAction;
class QToolBar;

namespace qds
{

class ArchiveChartView;
class ArchiveTableView;
class ArchiveTableModel;

class DataViewerMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit DataViewerMainWindow(
    QWidget* parent = nullptr);

private:
  struct TableContext
  {
    QWidget* widget = nullptr;
    ArchiveTableView* view = nullptr;
    QComboBox* frequencyCombo = nullptr;

    uint32_t frequency = 100;
    std::vector<SignalId> channels;
  };

  void createActions();
  void createUi();

  void openArchive(
    const std::filesystem::path& path);

  void populateSignalTree();

  void addTable();
  void closeTable(int index);

  void updateSignalTreeChecks();
  void onSignalChecked(
    QTreeWidgetItem* item,
    bool checked);

  TableContext* currentTable();
  bool isSignalSelected(
    SignalId id) const;

  void updateTableChannels(
    TableContext* table);

private:
  std::filesystem::path m_path;

  ArchiveChartView* m_chartView = nullptr;
  QTreeWidget* m_signalTree = nullptr;
  QSplitter* m_splitter = nullptr;
  QTabWidget* m_tabs = nullptr;

  QToolBar* m_toolBar = nullptr;

  QAction* m_openArchiveAction = nullptr;
  QAction* m_themeAction = nullptr;
  QAction* m_addTableAction = nullptr;

  std::vector<SignalId> m_chartSignals;
  std::vector<std::unique_ptr<TableContext>> m_tables;

  std::unique_ptr<ArchiveReader> m_archiveReader;
};

}