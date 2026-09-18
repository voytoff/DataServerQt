#pragma once

#include "idatastreameventsink.h"
namespace qds
{

class FakeDataStreamEventSink : public IDataStreamEventSink
{
  void startStream(
    const DataStreamAnchor& anchor) override {
    m_anchor = anchor;
  }
public:
  DataStreamAnchor m_anchor;
};

}