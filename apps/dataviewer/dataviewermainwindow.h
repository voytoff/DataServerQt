#pragma once

#include "archivereader.h"
#include "viewersettingsloader.h"

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
    QString configFile,
    QWidget* parent = nullptr);

protected:
  void closeEvent(QCloseEvent* event) override;

private:
  struct TableContext
  {
    QWidget* widget = nullptr;
    ArchiveTableView* view = nullptr;

    uint32_t frequency = 100;
    std::vector<SignalId> channels;
  };

  void createActions();
  void updateActions();

  void createUi();

  void openArchive(
    const std::filesystem::path& path);

  void populateSignalTree();

  void addTable();

  void addTable(
    const ViewerTableSettings& settings);

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

  void updateFrequencyControl();

  void restoreWindowSettings();
  void restoreTableSettings();
  void saveWindowSettings() const;

  void saveTableSettings() const;

  void showSettings();

  QString viewerSettingsFile() const;

private:
  void updateChartSettings();

  void updateTableSettings();

private:
  std::filesystem::path m_path;

  ArchiveChartView* m_chartView = nullptr;
  QTreeWidget* m_signalTree = nullptr;
  QSplitter* m_splitter = nullptr;
  QTabWidget* m_tabs = nullptr;
  QComboBox* m_frequencyCombo = nullptr;

  QToolBar* m_toolBar = nullptr;

  QAction* m_openArchiveAction = nullptr;
  QAction* m_themeAction = nullptr;
  QAction* m_addTableAction = nullptr;
  QAction* m_settingsAction = nullptr;

  std::vector<SignalId> m_chartSignals;
  std::vector<std::unique_ptr<TableContext>> m_tables;

  std::unique_ptr<ArchiveReader> m_archiveReader;

  QString m_configFile;

};

}