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
  const QPixmap image =
    pixmap(rect.size(), mode, state);

  if (image.isNull())
    return;

  painter->drawPixmap(
    rect.topLeft(),
    image);
}

QPixmap ThemedSvgIconEngine::pixmap(
  const QSize& size,
  QIcon::Mode mode,
  QIcon::State state)
{
  Q_UNUSED(state);

  if (size.isEmpty())
    return {};

  QSvgRenderer renderer(m_path);

  if (!renderer.isValid())
    return {};

  QPixmap result(size);
  result.fill(Qt::transparent);

  QPainter painter(&result);

  renderer.render(
    &painter,
    QRectF(QPointF(0, 0), QSizeF(size)));

  const QPalette palette =
    QApplication::palette();

  const QColor color =
    palette.color(
      mode == QIcon::Disabled
        ? QPalette::Disabled
        : QPalette::Active,
      QPalette::ButtonText);

  painter.setCompositionMode(
    QPainter::CompositionMode_SourceIn);

  painter.fillRect(
    QRect(QPoint(0, 0), size),
    color);

  return result;
}

}