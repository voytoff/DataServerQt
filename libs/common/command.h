#ifndef COMMAND_H
#define COMMAND_H

#include <cstdint>

namespace qds
{

enum class Command : uint16_t {
  Invalid = 0,
  Read,
  Write,
  ReadBlock,
  WriteBlock,
  Subscribe,
  Unsubscribe,
  Ping,
  Pong
};

}
#endif // COMMAND_H
