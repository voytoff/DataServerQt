#pragma once

#include <functional>
#include "strongid.h"

namespace std
{

template<typename Tag>
struct hash<qds::StrongId<Tag>>
{
  size_t operator()(
    const qds::StrongId<Tag>& id) const noexcept
  {
    return std::hash<uint32_t>{}(id.value);
  }
};

}