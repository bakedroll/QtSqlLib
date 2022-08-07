#pragma once

#include <QtSqlLib/API/IQueryVisitor.h>

#include <QtSqlLib/API/ISchema.h>

namespace QtSqlLib
{

class QueryPrepareVisitor : public API::IQueryVisitor
{
public:
  QueryPrepareVisitor(API::ISchema& schema);
  ~QueryPrepareVisitor() override;

  void visit(API::IQuery& query) override;
  void visit(API::IQuerySequence& query) override;

private:
  API::ISchema& m_schema;

};

}