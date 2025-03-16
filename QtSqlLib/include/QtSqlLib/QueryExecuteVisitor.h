#pragma once

#include <QtSqlLib/API/IQueryVisitor.h>

#include <QtSqlLib/ResultSet.h>

#include <QSqlDatabase>

namespace QtSqlLib::API
{
class IQuery;
class ISchema;
}

namespace QtSqlLib
{

class QueryExecuteVisitor : public API::IQueryVisitor
{
public:
  QueryExecuteVisitor(const QSqlDatabase& sqlDb, API::ISchema& schema);
  ~QueryExecuteVisitor() override;

  void visit(API::IQuery& query) override;
  void visit(API::IQuerySequence& query) override;

  ResultSet takeLastQueryResults();

private:
  const QSqlDatabase& m_sqlDb;
  API::ISchema& m_schema;

  ResultSet m_lastResults;

};

}
