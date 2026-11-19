#pragma once

#include "datawidgets_export.h"
#include "lcarddiscovery.h"

#include <QDialog>
#include <QFutureWatcher>

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
    uint32_t address,
    uint16_t port);

  [[nodiscard]]
  std::vector<LCardDiscoveredCrate>
  selectedCrates() const;

private:
  void populateTree(const LCardDiscoveryResult& result);
  void startScan();
  void finishScan();

  QFutureWatcher<LCardDiscoveryResult> scanWatcher_;

  Ui::LCardDiscoveryDialog* ui;
  LCardDiscoveryResult discoveryResult_;
  LCardDiscoveryDialogMode mode_ = LCardDiscoveryDialogMode::Browse;

  uint32_t address_ = 0;
  uint16_t port_ = 0;

};

}