#pragma once

#include <cstdint>

namespace QtSqlLib::API
{

class IID
{
public:
  using Type = int32_t;
  static constexpr Type sc_columnIdReservedBits = 0xFFE00000;
  static constexpr Type sc_relationshipIdReservedBits = 0x80000000;

  IID() = default;
  virtual ~IID() = default;

  virtual Type get() const = 0;
  virtual bool operator<(const IID& rhs) const = 0;

};

}
