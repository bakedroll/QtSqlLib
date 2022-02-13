#pragma once

namespace QtSqlLib::API
{

class IQueryVisitor;

class IQueryElement
{
public:
  IQueryElement() = default;
  virtual ~IQueryElement() = default;
  virtual void accept(IQueryVisitor& visitor) = 0;

};

}