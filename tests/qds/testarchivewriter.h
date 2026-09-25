#pragma once

#include "iarchivewriter.h"

namespace  qds
{

class TestArchiveWriter : public IArchiveWriter
{
public:

  bool write(const Frame& frame) override
  {
    m_frames.push_back(frame);
    return true;
  }
  const Frame *front() const
  {
    if (m_frames.empty())
      return nullptr;
    return &m_frames.front();
  }
  const Frame *last() const
  {
    if (m_frames.empty())
      return nullptr;
    return &m_frames.back();
  }
  std::size_t size() const noexcept
  {
    return m_frames.size();
  }
private:

  std::vector<Frame> m_frames;
};

}

