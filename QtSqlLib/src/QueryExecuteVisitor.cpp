#include "QtSqlLib/QueryExecuteVisitor.h"

#include "QtSqlLib/API/IQuerySequence.h"
#include "QtSqlLib/API/IQuery.h"
#include "QtSqlLib/DatabaseException.h"

#include <QSqlQuery>
#include <QSqlError>

namespace QtSqlLib
{

QueryExecuteVisitor::QueryExecuteVisitor(Schema& schema)
  : m_schema(schema)
{
}

QueryExecuteVisitor::~QueryExecuteVisitor() = default;

void QueryExecuteVisitor::visit(API::IQuery& query)
{
  auto q = query.getSqlQuery(m_schema, m_lastResults);
  const auto isBatch = (q.mode == API::IQuery::QueryMode::Batch);

  if ((!isBatch && !q.qtQuery.exec()) || (isBatch && !q.qtQuery.execBatch()))
  {
    throw DatabaseException(DatabaseException::Type::QueryError,
      QString("Could not execute query: %1").arg(q.qtQuery.lastError().text()));
  }

  const auto results = query.getQueryResults(m_schema, q.qtQuery);
  if (results.validity == API::IQuery::QueryResults::Validity::Valid)
  {
    m_lastResults = results;
  }
}

void QueryExecuteVisitor::visit(API::IQuerySequence& query)
{
  query.traverse(*this);
}

API::IQuery::QueryResults QueryExecuteVisitor::getLastQueryResults() const
{
  return m_lastResults;
}

}
