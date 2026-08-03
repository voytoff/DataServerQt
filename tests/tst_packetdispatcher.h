#pragma once

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