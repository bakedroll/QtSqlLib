#include "QtSqlLib/QuerySequence.h"

namespace QtSqlLib
{

int QuerySequence::getNumQueries() const
{
  auto num = static_cast<int>(m_singleQueriesMap.size());
  for (const auto& seq : m_querySequencesMap)
  {
    num += seq.second->getNumQueries();
  }

  return num;
}

void QuerySequence::prepare(Schema& schema)
{
  for (const auto& seq : m_querySequencesMap)
  {
    seq.second->prepare(schema);
  }

  auto index = 0;
  auto num = 0;
  for (const auto& query : m_queries)
  {
    if (query == QueryType::SingleQuery)
    {
      m_queryNumSingleQueryMap[num] = index;
      num++;
    }
    else
    {
      const auto& seq = m_querySequencesMap.at(index);
      for (auto i = 0; i < seq->getNumQueries(); i++)
      {
        m_queryNumQuerySequenceMap[num] = { index, i };
        num++;
      }
    }

    index++;
  }
}

QueryDefines::SqlQuery QuerySequence::getSqlQuery(int num, Schema& schema)
{
  if (m_queryNumSingleQueryMap.count(num) > 0)
  {
    return m_singleQueriesMap.at(m_queryNumSingleQueryMap.at(num))->getSqlQuery(schema);
  }

  const auto idNumPair = m_queryNumQuerySequenceMap.at(num);
  return m_querySequencesMap.at(idNumPair.first)->getSqlQuery(idNumPair.second, schema);
}

QueryDefines::QueryResults QuerySequence::getQueryResults(int num, Schema& schema, QSqlQuery& query) const
{
  if (m_queryNumSingleQueryMap.count(num) > 0)
  {
    return m_singleQueriesMap.at(m_queryNumSingleQueryMap.at(num))->getQueryResults(schema, query);
  }

  const auto idNumPair = m_queryNumQuerySequenceMap.at(num);
  return m_querySequencesMap.at(idNumPair.first)->getQueryResults(idNumPair.second, schema, query);
}

void QuerySequence::addQuery(std::unique_ptr<IQuery> query)
{
  m_queries.emplace_back(QueryType::SingleQuery);
  m_singleQueriesMap[static_cast<int>(m_queries.size()) - 1] = std::move(query);
}

void QuerySequence::addQuery(std::unique_ptr<IQuerySequence> sequence)
{
  m_queries.emplace_back(QueryType::Sequence);
  m_querySequencesMap[static_cast<int>(m_queries.size()) - 1] = std::move(sequence);
}

IQuery& QuerySequence::getQuery(int index)
{
  return *m_singleQueriesMap.at(index);
}

}
