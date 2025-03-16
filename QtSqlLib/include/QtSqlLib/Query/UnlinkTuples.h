#pragma once

#include <QtSqlLib/Query/QuerySequence.h>

#include <QtSqlLib/RelationshipPreparationData.h>

namespace QtSqlLib::API
{
class IID;
}

namespace QtSqlLib::Query
{

class UnlinkTuples : public QuerySequence
{
public:
  UnlinkTuples(const API::IID& relationshipId);
  ~UnlinkTuples() override;

  UnlinkTuples& fromOne(const PrimaryKey& tupleKeyValues);

  UnlinkTuples& toOne(const PrimaryKey& tupleKeyValues);
  UnlinkTuples& toMany(const std::vector<PrimaryKey>& tupleKeyValuesList);

  void prepare(API::ISchema& schema) override;

private:
  RelationshipPreparationData m_relationshipPreparationData;

};

}
