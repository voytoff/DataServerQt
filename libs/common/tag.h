#ifndef TAG_H
#define TAG_H

#include <QString>
#include <QVector>

using TagId = uint32_t;

struct Tag
{
  TagId id;
  QString name;
};

#endif // TAG_H
