#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/Query/QuerySequence.h>
#include <QtSqlLib/RelationshipPreparationData.h>

#define UNLINK_TUPLES(X) QtSqlLib::Query::UnlinkTuples(QtSqlLib::ID(X))

#define FROM_ONE(X) fromOne(X)
#define TO_ONE(X) toOne(X)
#define TO_MANY(...) toMany(__VA_ARGS__)

namespace QtSqlLib::Query
{

class UnlinkTuples : public QuerySequence
{
public:
  UnlinkTuples(const API::IID& relationshipId);
  ~UnlinkTuples() override;

  UnlinkTuples& fromOne(const API::TupleValues& tupleKeyValues);

  UnlinkTuples& toOne(const API::TupleValues& tupleKeyValues);
  UnlinkTuples& toMany(const std::vector<API::TupleValues>& tupleKeyValuesList);

  void prepare(API::ISchema& schema) override;

private:
  RelationshipPreparationData m_relationshipPreparationData;

};

}
