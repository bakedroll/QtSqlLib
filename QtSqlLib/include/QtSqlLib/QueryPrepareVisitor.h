#pragma once

#include <QtSqlLib/API/IQueryVisitor.h>

#include <QtSqlLib/Schema.h>

namespace QtSqlLib
{

class QueryPrepareVisitor : public API::IQueryVisitor
{
public:
  QueryPrepareVisitor(Schema& schema);
  ~QueryPrepareVisitor() override;

  void visit(API::IQuery& query) override;
  void visit(API::IQuerySequence& query) override;

private:
  Schema& m_schema;

};

}