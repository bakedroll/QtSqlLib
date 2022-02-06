#include "QtSqlLib/QuerySequence.h"

namespace QtSqlLib
{

int QuerySequence::getNumQueries() const
{
  return static_cast<int>(m_queries.size());
}

QueryDefines::SqlQuery QuerySequence::getSqlQuery(int num, Schema& schema)
{
  return m_queries.at(num)->getSqlQuery(schema);
}

QueryDefines::QueryResults QuerySequence::getQueryResults(int num, Schema& schema, QSqlQuery& query) const
{
  return m_queries.at(num)->getQueryResults(schema, query);
}

void QuerySequence::addQuery(std::unique_ptr<IQuery> query)
{
  m_queries.emplace_back(std::move(query));
}

IQuery& QuerySequence::getQuery(int index)
{
  return *m_queries.at(index);
}

}
