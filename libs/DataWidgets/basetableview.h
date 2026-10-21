#pragma once

#include "datawidgets_export.h"

#include <QTableView>
#include <QKeyEvent>

namespace qds
{

class DATAWIDGETS_EXPORT BaseTableView
  : public QTableView
{
  Q_OBJECT

public:
  explicit BaseTableView(
    int columnWidth = 0,
    QWidget* parent = nullptr);

  explicit BaseTableView(
    QWidget* parent);

  ~BaseTableView() override = default;

  void setPasteEnabled(
    bool enabled) noexcept;

  bool pasteEnabled() const noexcept;

  bool pasteFromClipboard();

  void setPasteInsertRowsEnabled(
    bool enabled) noexcept;

protected:
  void keyPressEvent(QKeyEvent* event) override;

private:
  bool m_pasteEnabled = false;
  bool m_pasteInsertRowsEnabled = false;
};

}