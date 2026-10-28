#include "themedsvgiconengine.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QSvgRenderer>

#include <utility>

namespace qds
{

ThemedSvgIconEngine::ThemedSvgIconEngine(
  QString path)
  : m_path(std::move(path))
{
}

QIconEngine* ThemedSvgIconEngine::clone() const
{
  return new ThemedSvgIconEngine(m_path);
}

void ThemedSvgIconEngine::paint(
  QPainter* painter,
  const QRect& rect,
  QIcon::Mode mode,
  QIcon::State state)
{
  Q_UNUSED(state);

  QSvgRenderer renderer(m_path);

  if (!renderer.isValid())
    return;

  const QPalette palette =
    QApplication::palette();

  const QColor color =
    palette.color(
      mode == QIcon::Disabled
        ? QPalette::Disabled
        : QPalette::Active,
      QPalette::ButtonText);

  painter->save();

  renderer.render(
    painter,
    QRectF(rect));

  painter->setCompositionMode(
    QPainter::CompositionMode_SourceIn);

  painter->fillRect(
    rect,
    color);

  painter->restore();
}

QPixmap ThemedSvgIconEngine::pixmap(
  const QSize& size,
  QIcon::Mode mode,
  QIcon::State state)
{
  QPixmap result(size);
  result.fill(Qt::transparent);

  QPainter painter(&result);

  paint(
    &painter,
    QRect(QPoint(0, 0), size),
    mode,
    state);

  return result;
}

}