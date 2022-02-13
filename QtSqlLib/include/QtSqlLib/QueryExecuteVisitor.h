#pragma once

#include <QtSqlLib/API/IQueryVisitor.h>

#include <QtSqlLib/API/IQuery.h>
#include <QtSqlLib/Schema.h>

namespace QtSqlLib
{

class QueryExecuteVisitor : public API::IQueryVisitor
{
public:
  QueryExecuteVisitor(Schema& schema);
  ~QueryExecuteVisitor() override;

  void visit(API::IQuery& query) override;
  void visit(API::IQuerySequence& query) override;

  API::IQuery::QueryResults getLastQueryResults() const;

private:
  Schema& m_schema;
  API::IQuery::QueryResults m_lastResults;

};

}
