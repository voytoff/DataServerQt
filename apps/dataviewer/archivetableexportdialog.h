
#pragma once

#include "archivetableexportsettings.h"

#include <QDialog>

#include <vector>

namespace Ui
{
class ArchiveTableExportDialog;
}

namespace qds
{

class ArchiveTableExportDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ArchiveTableExportDialog(
    const ArchiveTableExportSettings& settings,
    QWidget* parent = nullptr);

  ~ArchiveTableExportDialog() override;

  [[nodiscard]]
  ArchiveTableExportSettings settings() const;

private:
  void moveChannel(int offset);

private:
  Ui::ArchiveTableExportDialog* ui;
  ArchiveTableExportSettings m_settings;
};

}
