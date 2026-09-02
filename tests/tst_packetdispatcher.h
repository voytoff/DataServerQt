#pragma once

#include <QObject>

class tst_packetdispatcher : public QObject
{
  Q_OBJECT
public:
  tst_packetdispatcher();
  ~tst_packetdispatcher() override;

private:

private slots:
  void tst_packetdispatcher_validSignal();
  void tst_packetdispatcher_invalidSignal();
  void tst_packetdispatcher_duplicateSignal();
  void tst_packetdispatcher_emptyList();
  void tst_packetdispatcher_tooManySignals();
  void tst_packetdispatcher_invalidRate();

  void tst_packetdispatcher_unsubscribe();
  void tst_packetdispatcher_unsubscribeinvalidId();

  void tst_packetdispatcher_ping();
  void tst_packetdispatcher_pingExtraData();

  void tst_packetdispatcher_subscribeWithoutPayload();
  void tst_packetdispatcher_subscribeExtraData();

  void tst_packetdispatcher_unknownPacket();
  void tst_packetdispatcher_invalidSignalCount();
  void tst_packetdispatcher_invalidPacket();
  void tst_packetdispatcher_subscriptionIdReuse();
  void tst_packetdispatcher_unsubscribeExtraData();

};