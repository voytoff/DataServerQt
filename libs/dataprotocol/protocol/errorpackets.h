#ifndef ERRORPACKETS_H
#define ERRORPACKETS_H

#include <cstdint>

enum class ErrorCode : uint8_t
{
  InvalidPacket     = 1, // поврежден PacketHeader
  InvalidRequest    = 2, // невозможно разобрать payload
  ExtraData         = 3, // после запроса остались байты
  UnsupportedPacket = 4, // PacketType не поддерживается
  InvalidState      = 5, // очень часто потом оказывается полезным
  InternalError     = 6, // внутренняя ошибка сервера
};

struct ErrorResponse
{
  ErrorCode code;
  uint32_t info = 0;
};

#endif // ERRORPACKETS_H
