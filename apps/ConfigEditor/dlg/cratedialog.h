#pragma once

#include "cratetype.h"

#include <QDialog>
#include <QString>
#include <cstdint>

namespace Ui
{
class CrateDialog;
}

namespace qds
{

class CrateDialog final : public QDialog
{
  Q_OBJECT

public:
  explicit CrateDialog(
    QWidget* parent = nullptr);

  ~CrateDialog() override;

  [[nodiscard]]
  CrateType crateType() const;

  [[nodiscard]]
  QString serial() const;

  [[nodiscard]]
  QString host() const;

  [[nodiscard]]
  uint32_t port() const;

  [[nodiscard]]
  QString description() const;

private:
  void updateActions();

private:
  Ui::CrateDialog* ui;
};

}