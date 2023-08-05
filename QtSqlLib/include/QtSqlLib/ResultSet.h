#pragma once

#include <QtSqlLib/API/SchemaTypes.h>

#include <vector>

namespace QtSqlLib
{

class ResultSet
{
public:
  using TupleValuesList = std::vector<API::TupleValues>;

  struct Tuple
  {
    API::TupleValues values;
    std::map<API::IID::Type, TupleValuesList> joinedTuples;
  };

  using TupleList = std::vector<Tuple>;

  static ResultSet create(const TupleList& tuples);
  static ResultSet invalid();

  virtual ~ResultSet();

  bool isValid() const;
  void resetIteration() const;

  size_t getNumResults() const;
  bool hasNext() const;
  const API::TupleValues& next() const;

  size_t getCurrentNumJoinedResults(API::IID::Type relationshipId) const;
  bool hasNextJoinedTuple(API::IID::Type relationshipId) const;
  const API::TupleValues& nextJoinedTuple(API::IID::Type relationshipId) const;

private:
  enum class Validity
  {
    Valid,
    Invalid
  };

  explicit ResultSet(Validity validity, const TupleList& tuples = {});

  Validity m_validity;
  TupleList m_tuples;

  mutable long long m_currentTupleIndex;
  mutable long long m_currentJoinedTupleIndex;

};

}