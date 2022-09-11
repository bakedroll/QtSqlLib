#pragma once

namespace QtSqlLib::API
{

class IID
{
public:
  using Type = int;

  IID() = default;
  virtual ~IID() = default;

  virtual Type get() const = 0;
  virtual bool operator<(const IID& rhs) const = 0;

};

}
