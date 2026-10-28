#pragma once

#include "datawidgets_export.h"

#include <QIconEngine>
#include <QString>

namespace qds
{

class DATAWIDGETS_EXPORT ThemedSvgIconEngine final : public QIconEngine
{
public:
  explicit ThemedSvgIconEngine(
    QString path);

  QIconEngine* clone() const override;

  void paint(
    QPainter* painter,
    const QRect& rect,
    QIcon::Mode mode,
    QIcon::State state) override;

  QPixmap pixmap(
    const QSize& size,
    QIcon::Mode mode,
    QIcon::State state) override;

private:
  QString m_path;
};

}