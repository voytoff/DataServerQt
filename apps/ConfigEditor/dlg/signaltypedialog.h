
#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;

namespace qds
{

class SignalTypeDialog final : public QDialog
{
  Q_OBJECT

public:
  explicit SignalTypeDialog(
    QWidget* parent = nullptr);

  QString typeName() const;

private:
  QLineEdit* m_nameEdit = nullptr;
};

}
