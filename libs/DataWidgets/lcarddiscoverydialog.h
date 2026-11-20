#pragma once

#include "datawidgets_export.h"
#include "lcarddiscovery.h"

#include <QDialog>
#include <QFutureWatcher>
#include <QCloseEvent>

namespace Ui {
class LCardDiscoveryDialog;
}

namespace qds
{

enum class LCardDiscoveryDialogMode
{
  Browse,  // Только просмотр и диагностика
  Select   // Выбор оборудования
};

class DATAWIDGETS_EXPORT LCardDiscoveryDialog : public QDialog
{
  Q_OBJECT

public:
  explicit LCardDiscoveryDialog(QWidget *parent = nullptr);
  ~LCardDiscoveryDialog();

  void setMode(LCardDiscoveryDialogMode mode);

  void setServer(
    const QString& host,
    uint16_t port);

  void setServer(
    uint32_t address,
    uint16_t port);

  [[nodiscard]]
  QString host() const;

  [[nodiscard]]
  uint16_t port() const;

  [[nodiscard]]
  std::vector<LCardDiscoveredCrate>
  selectedCrates() const;

  [[nodiscard]]
  QString scanHost() const;

  [[nodiscard]]
  uint16_t scanPort() const;

protected:
  void reject() override;
  void closeEvent(QCloseEvent* event) override;

private:
  void populateTree(const LCardDiscoveryResult& result);
  void startScan();
  void finishScan();
  void updateSelection();
  void invalidateResult();
  void showContextMenu(const QPoint& pos);

  Ui::LCardDiscoveryDialog* ui;

  QFutureWatcher<LCardDiscoveryResult> m_scanWatcher;

  LCardDiscoveryResult m_discoveryResult;
  LCardDiscoveryDialogMode m_mode = LCardDiscoveryDialogMode::Browse;

  bool m_closePending = false;

  QString m_scanHost;
  uint16_t m_scanPort = 0;

};

}