#include "QtSqlLib/ResultSet_bak.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

ResultSet_bak ResultSet_bak::create(const TupleList& tuples)
{
  return ResultSet_bak(Validity::Valid, tuples);
}

ResultSet_bak ResultSet_bak::invalid()
{
  return ResultSet_bak(Validity::Invalid);
}

ResultSet_bak::~ResultSet_bak() = default;

bool ResultSet_bak::isValid() const
{
  return m_validity == Validity::Valid;
}

void ResultSet_bak::resetIteration() const
{
  m_currentTupleIndex = -1;
  m_currentJoinedTupleIndex = -1;
}

size_t ResultSet_bak::getNumResults() const
{
  return m_tuples.size();
}

bool ResultSet_bak::hasNext() const
{
  return m_currentTupleIndex < (static_cast<long long>(m_tuples.size()) - 1);
}

const API::TupleValues& ResultSet_bak::next() const
{
  if (!hasNext())
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "ResultSet_bak has no further tuples.");
  }

  m_currentTupleIndex++;
  m_currentJoinedTupleIndex = -1;

  return m_tuples.at(m_currentTupleIndex).values;
}

size_t ResultSet_bak::getCurrentNumJoinedResults(API::IID::Type relationshipId) const
{
  const auto& currentTuple = m_tuples.at(m_currentTupleIndex);
  if (currentTuple.joinedTuples.count(relationshipId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Invalid relationship id for result set.");
  }

  return currentTuple.joinedTuples.at(relationshipId).size();
}

bool ResultSet_bak::hasNextJoinedTuple(API::IID::Type relationshipId) const
{
  const auto& currentTuple = m_tuples.at(m_currentTupleIndex);
  if (currentTuple.joinedTuples.count(relationshipId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Invalid relationship id for result set.");
  }

  return m_currentJoinedTupleIndex < (static_cast<long long>(currentTuple.joinedTuples.at(relationshipId).size()) - 1);
}

const API::TupleValues& ResultSet_bak::nextJoinedTuple(API::IID::Type relationshipId) const
{
  if (!hasNextJoinedTuple(relationshipId))
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Joined data has no further tuples.");
  }

  m_currentJoinedTupleIndex++;
  return m_tuples.at(m_currentTupleIndex).joinedTuples.at(relationshipId).at(m_currentJoinedTupleIndex);
}

ResultSet_bak::ResultSet_bak(Validity validity, const TupleList& tuples) :
  m_validity(validity),
  m_tuples(tuples),
  m_currentTupleIndex(-1),
  m_currentJoinedTupleIndex(-1)
{
}

}
