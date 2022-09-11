#pragma once

namespace QtSqlLib::API
{

class IID
{
public:
  IID() = default;
  virtual ~IID() = default;

  virtual int get() const = 0;
  virtual bool operator<(const IID& rhs) const = 0;

};

}
