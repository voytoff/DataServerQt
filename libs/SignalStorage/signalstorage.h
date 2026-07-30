#ifndef SIGNALSTORAGE_H
#define SIGNALSTORAGE_H

#include <cstdint>
#include <unordered_map>
#include <vector>


namespace qds
{

struct SignalId
{
  uint32_t value = 0;

  auto operator<=>(const SignalId&) const = default;
};

struct SignalValue
{
  uint64_t timestamp = 0;
  double value = 0.0;
};

struct SignalGroup
{
  uint32_t id;

  uint32_t frequency;

  std::vector<SignalId> signals;
};

/** Это оперативная память проекта, содержащая последние инженерные значения всех логических сигналов. */
class SignalStorage
{
public:

  void update(
    SignalId id,
    double value,
    uint64_t timestamp);

  bool get(
    SignalId id,
    SignalValue& result) const;

private:

  std::vector<SignalValue> m_values;
};

}

#endif // SIGNALSTORAGE_H
