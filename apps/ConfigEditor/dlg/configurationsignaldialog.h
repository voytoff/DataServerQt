#pragma once

#include "signaldefinition.h"

#include <QDialog>

class QLineEdit;
class QComboBox;

namespace qds
{

class ConfigurationSignalDialog final : public QDialog
{
  Q_OBJECT

public:
  explicit ConfigurationSignalDialog(
    QWidget* parent = nullptr);

  [[nodiscard]]
  QString signalName() const;

  [[nodiscard]]
  SignalKind signalKind() const;

private:
  void updateActions();

private:
  QLineEdit* m_nameEdit = nullptr;
  QComboBox* m_kindCombo = nullptr;

  QPushButton* m_okButton = nullptr;
};

}