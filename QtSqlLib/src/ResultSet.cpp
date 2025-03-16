#include "QtSqlLib/ResultSet.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

ResultSet::ResultSet(
    QSqlQuery&& query,
    API::QueryMetaInfo&& queryMetaInfo,
    std::vector<API::QueryMetaInfo>&& joinMetaInfo) :
  m_sqlQuery(std::move(query)),
  m_queryMetaInfo(std::move(queryMetaInfo)),
  m_joinMetaInfo(std::move(joinMetaInfo)),
  m_isValid(true),
  m_nextTupleResult({ false, false, std::vector<bool>(m_joinMetaInfo.size(), false) })
{
}

ResultSet::ResultSet() :
  m_isValid(false)
{
}

ResultSet::ResultSet(ResultSet&& rhs) :
  ResultSet(std::move(rhs.m_sqlQuery), std::move(rhs.m_queryMetaInfo), std::move(rhs.m_joinMetaInfo))
{
  m_isValid = std::move(rhs.m_isValid);
  m_nextTupleResult = std::move(rhs.m_nextTupleResult);
  m_retrievedResultKeys = std::move(rhs.m_retrievedResultKeys);
  m_retrievedJoinResultKeys = std::move(rhs.m_retrievedJoinResultKeys);
}

ResultSet& ResultSet::operator=(ResultSet&& rhs)
{
  m_sqlQuery = std::move(rhs.m_sqlQuery);
  m_queryMetaInfo = std::move(rhs.m_queryMetaInfo);
  m_joinMetaInfo = std::move(rhs.m_joinMetaInfo);
  m_isValid = std::move(rhs.m_isValid);
  m_nextTupleResult = std::move(rhs.m_nextTupleResult);
  m_retrievedResultKeys = std::move(rhs.m_retrievedResultKeys);
  m_retrievedJoinResultKeys = std::move(rhs.m_retrievedJoinResultKeys);
  return *this;
}

ResultSet::~ResultSet() = default;

bool ResultSet::isValid() const
{
  return m_isValid;
}

void ResultSet::resetIteration()
{
  if (!m_isValid)
  {
    return;
  }

  m_sqlQuery.seek(-1);
  resetNextTupleResult();
  m_retrievedResultKeys.clear();
  m_retrievedJoinResultKeys.clear();
}

bool ResultSet::hasNextTuple()
{
  searchNextTuple(SearchMode::MAIN_TUPLE);
  return m_nextTupleResult.hasNext;
}

bool ResultSet::hasNextJoinedTuple()
{
  searchNextTuple(SearchMode::JOIN_TUPLE);
  return !m_nextTupleResult.hasNext && m_nextTupleResult.hasNextJoin;
}

TupleView ResultSet::nextTuple()
{
  searchNextTuple(SearchMode::MAIN_TUPLE);
  if (!m_nextTupleResult.hasNext)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "No next tuple found.");
  }

  m_nextTupleResult.hasNext = false;
  return TupleView(m_sqlQuery, m_queryMetaInfo);
}

TupleView ResultSet::nextJoinedTuple()
{
  if (m_nextTupleResult.hasNext)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Retreive next tuple first.");
  }

  searchNextTuple(SearchMode::JOIN_TUPLE);
  if (!m_nextTupleResult.hasNextJoin)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "No next join tuple found.");
  }

  for (size_t i=0; i<m_joinMetaInfo.size(); ++i)
  {
    if (m_nextTupleResult.nextJoinsMask.at(i))
    {
      m_nextTupleResult.nextJoinsMask[i] = false;
      m_nextTupleResult.hasNextJoin = std::any_of(m_nextTupleResult.nextJoinsMask.cbegin()+i+1, m_nextTupleResult.nextJoinsMask.cend(),
        [](bool value) { return value; });

      return TupleView(m_sqlQuery, m_joinMetaInfo.at(i));
    }
  }

  throw DatabaseException(DatabaseException::Type::UnexpectedError, "Error due to inconsistent join data.");
}

void ResultSet::searchNextTuple(SearchMode searchMode)
{
  if (!m_isValid || m_nextTupleResult.hasNext ||
    (searchMode == SearchMode::JOIN_TUPLE && m_nextTupleResult.hasNextJoin))
  {
    return;
  }

  while (m_sqlQuery.next())
  {
    if (m_joinMetaInfo.empty())
    {
      m_nextTupleResult.hasNext = true;
      return;
    }

    TupleView tuple(m_sqlQuery, m_queryMetaInfo);
    const auto tupleKey = tuple.primaryKey();

    if (m_retrievedResultKeys.count(tupleKey) == 0)
    {
      m_retrievedResultKeys.emplace(tupleKey);
      m_nextTupleResult.hasNext = true;
      clearNextJoinsMask();
    }

    if (searchMode == SearchMode::JOIN_TUPLE || m_nextTupleResult.hasNext)
    {
      findNextJoinTuple(tupleKey);

      if (searchMode == SearchMode::MAIN_TUPLE || m_nextTupleResult.hasNext || m_nextTupleResult.hasNextJoin)
      {
        return;
      }
    }
  }

  resetNextTupleResult();
}

void ResultSet::findNextJoinTuple(const PrimaryKey& tupleKey)
{
  for (size_t i=0; i<m_joinMetaInfo.size(); ++i)
  {
    const auto& join = m_joinMetaInfo.at(i);
    const auto key = std::make_pair(join.relationshipId.value(), tupleKey);
    auto& retreivedJoinResultKeys = m_retrievedJoinResultKeys[key];

    TupleView joinTuple(m_sqlQuery, join);
    const auto joinKeyTuple = joinTuple.primaryKey();

    if (joinKeyTuple.isNull())
    {
      continue;
    }

    if (retreivedJoinResultKeys.count(joinKeyTuple) == 0)
    {
      retreivedJoinResultKeys.emplace(joinKeyTuple);
      m_nextTupleResult.hasNextJoin = true;
      m_nextTupleResult.nextJoinsMask[i] = true;
    }
  }
}

void ResultSet::resetNextTupleResult()
{
  m_nextTupleResult.hasNext = false;
  clearNextJoinsMask();
}

void ResultSet::clearNextJoinsMask()
{
  m_nextTupleResult.hasNextJoin = false;
  std::fill(m_nextTupleResult.nextJoinsMask.begin(), m_nextTupleResult.nextJoinsMask.end(), false);
}

}
