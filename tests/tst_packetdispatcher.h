#ifndef TST_PACKETDISPATCHER_H
#define TST_PACKETDISPATCHER_H

#include <QObject>

class tst_packetdispatcher : public QObject
{
  Q_OBJECT
public:
  tst_packetdispatcher();
  ~tst_packetdispatcher() override;

private slots:
  void tst_packetdispatcher_InvalidTag();

};

#endif // TST_PACKETDISPATCHER_H
