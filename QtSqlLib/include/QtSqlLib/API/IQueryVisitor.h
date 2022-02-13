#pragma once

namespace QtSqlLib::API
{

class IQuery;
class IQuerySequence;

class IQueryVisitor
{
public:
  IQueryVisitor() = default;
  virtual ~IQueryVisitor() = default;

  virtual void visit(IQuery& query) = 0;
  virtual void visit(IQuerySequence& query) = 0;

};

}
