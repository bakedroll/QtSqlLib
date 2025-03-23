#pragma once

#include <QtSqlLib/API/SchemaTypes.h>

#include <vector>

namespace QtSqlLib
{

// TODO: remove
class ResultSet_bak
{
public:
  using TupleValuesList = std::vector<API::TupleValues>;

  struct Tuple
  {
    API::TupleValues values;
    std::map<API::IID::Type, TupleValuesList> joinedTuples;
  };

  using TupleList = std::vector<Tuple>;

  static ResultSet_bak create(const TupleList& tuples);
  static ResultSet_bak invalid();

  virtual ~ResultSet_bak();

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

  explicit ResultSet_bak(Validity validity, const TupleList& tuples = {});

  Validity m_validity;
  TupleList m_tuples;

  mutable long long m_currentTupleIndex;
  mutable long long m_currentJoinedTupleIndex;

};

}