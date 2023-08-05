#include "QtSqlLib/ResultSet.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

ResultSet ResultSet::create(const TupleList& tuples)
{
  return ResultSet(Validity::Valid, tuples);
}

ResultSet ResultSet::invalid()
{
  return ResultSet(Validity::Invalid);
}

ResultSet::~ResultSet() = default;

bool ResultSet::isValid() const
{
  return m_validity == Validity::Valid;
}

void ResultSet::resetIteration() const
{
  m_currentTupleIndex = -1;
  m_currentJoinedTupleIndex = -1;
}

size_t ResultSet::getNumResults() const
{
  return m_tuples.size();
}

bool ResultSet::hasNext() const
{
  return m_currentTupleIndex < (static_cast<long long>(m_tuples.size()) - 1);
}

const API::ISchema::TupleValues& ResultSet::next() const
{
  if (!hasNext())
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "ResultSet has no further tuples.");
  }

  m_currentTupleIndex++;
  m_currentJoinedTupleIndex = -1;

  return m_tuples.at(m_currentTupleIndex).values;
}

size_t ResultSet::getCurrentNumJoinedResults(API::IID::Type relationshipId) const
{
  const auto& currentTuple = m_tuples.at(m_currentTupleIndex);
  if (currentTuple.joinedTuples.count(relationshipId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Invalid relationship id for result set.");
  }

  return currentTuple.joinedTuples.at(relationshipId).size();
}

bool ResultSet::hasNextJoinedTuple(API::IID::Type relationshipId) const
{
  const auto& currentTuple = m_tuples.at(m_currentTupleIndex);
  if (currentTuple.joinedTuples.count(relationshipId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Invalid relationship id for result set.");
  }

  return m_currentJoinedTupleIndex < (static_cast<long long>(currentTuple.joinedTuples.at(relationshipId).size()) - 1);
}

const API::ISchema::TupleValues& ResultSet::nextJoinedTuple(API::IID::Type relationshipId) const
{
  if (!hasNextJoinedTuple(relationshipId))
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Joined data has no further tuples.");
  }

  m_currentJoinedTupleIndex++;
  return m_tuples.at(m_currentTupleIndex).joinedTuples.at(relationshipId).at(m_currentJoinedTupleIndex);
}

ResultSet::ResultSet(Validity validity, const TupleList& tuples) :
  m_validity(validity),
  m_tuples(tuples),
  m_currentTupleIndex(-1),
  m_currentJoinedTupleIndex(-1)
{
}

}
