### QtDataServer

- Протокол — little-endian.
- Протокол не зависит от Qt.
- Все структуры должны быть trivially_copyable.
- Для массивов используется std::span.
- PacketReader закрывает соединение при повреждённом пакете.