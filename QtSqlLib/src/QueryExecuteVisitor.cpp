#include "QtSqlLib/QueryExecuteVisitor.h"

#include "QtSqlLib/API/IQuerySequence.h"
#include "QtSqlLib/API/IQuery.h"
#include "QtSqlLib/DatabaseException.h"

#include <QSqlQuery>
#include <QSqlError>

namespace QtSqlLib
{

QueryExecuteVisitor::QueryExecuteVisitor(const QSqlDatabase& sqlDb, API::ISchema& schema) :
  m_sqlDb(sqlDb),
  m_schema(schema)
{
}

QueryExecuteVisitor::~QueryExecuteVisitor() = default;

void QueryExecuteVisitor::visit(API::IQuery& query)
{
  auto q = query.getSqlQuery(m_sqlDb, m_schema, m_lastResults);
  const auto isBatch = (q.mode == API::IQuery::QueryMode::Batch);

  if ((!isBatch && !q.qtQuery.exec()) || (isBatch && !q.qtQuery.execBatch()))
  {
    throw DatabaseException(DatabaseException::Type::QueryError,
      QString("Could not execute query: %1").arg(q.qtQuery.lastError().text()));
  }

  auto results = query.getQueryResults(m_schema, std::move(q.qtQuery));
  if (results.isValid())
  {
    m_lastResults = std::move(results);
  }
}

void QueryExecuteVisitor::visit(API::IQuerySequence& query)
{
  query.traverse(*this);
}

ResultSet QueryExecuteVisitor::takeLastQueryResults()
{
  return std::move(m_lastResults);
}

}
