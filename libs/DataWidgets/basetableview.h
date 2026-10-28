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
public:
  explicit BaseTableView(
    int columnWidth = 0,
    QWidget* parent = nullptr);

  explicit BaseTableView(
    QWidget* parent);

  BaseTableView(
    int columnWidth,
    int dx1,
    int dy1,
    int dx2,
    int dy2,
    QWidget* parent = nullptr);

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