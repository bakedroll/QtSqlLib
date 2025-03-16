#pragma once

#include <QtSqlLib/Query/BatchInsertInto.h>
#include <QtSqlLib/Query/QuerySequence.h>
#include <QtSqlLib/Query/UpdateTable.h>

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/PrimaryKey.h>
#include <QtSqlLib/RelationshipPreparationData.h>

namespace QtSqlLib::API
{
class ISchema;
}

namespace QtSqlLib::Query
{

class LinkTuples : public QuerySequence
{
public:
  LinkTuples(const API::IID& relationshipId);
  ~LinkTuples() override;

  LinkTuples& fromOne(const PrimaryKey& tupleKeyValues);
  LinkTuples& fromRemainingKey();

  LinkTuples& toOne(const PrimaryKey& tupleKeyValues);
  LinkTuples& toMany(const std::vector<PrimaryKey>& tupleKeyValuesList);

  void prepare(API::ISchema& schema) override;

private:
  RelationshipPreparationData m_relationshipPreparationData;

};

}
