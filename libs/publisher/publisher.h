#pragma once

#include "frame.h"
#include "isender.h"
#include "packetwriter.h"
#include "signalmemorylayout.h"
#include "subscription.h"

namespace qds
{

class Publisher
{
public:

  Publisher(
    const SignalMemoryLayout& layout,
    ISender& sender);

  bool publish(
    const Frame& frame,
    Subscription& subscription);

private:

  const SignalMemoryLayout& m_layout;
  ISender& m_sender;

  PacketWriter m_writer;
};

}