#pragma once

#include <QtSqlLib/API/IQueryVisitor.h>

#include <QtSqlLib/API/IQuery.h>
#include <QtSqlLib/Schema.h>

#include <QSqlDatabase>

namespace QtSqlLib
{

class QueryExecuteVisitor : public API::IQueryVisitor
{
public:
  QueryExecuteVisitor(const QSqlDatabase& sqlDb, Schema& schema);
  ~QueryExecuteVisitor() override;

  void visit(API::IQuery& query) override;
  void visit(API::IQuerySequence& query) override;

  API::IQuery::QueryResults getLastQueryResults() const;

private:
  const QSqlDatabase& m_sqlDb;
  Schema& m_schema;

  API::IQuery::QueryResults m_lastResults;

};

}
