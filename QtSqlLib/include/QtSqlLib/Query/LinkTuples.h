#pragma once

#include <QtSqlLib/Query/BatchInsertInto.h>
#include <QtSqlLib/Query/QuerySequence.h>
#include <QtSqlLib/Query/UpdateTable.h>

#include <QtSqlLib/API/IID.h>
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

  LinkTuples& fromOne(const API::TupleValues& tupleKeyValues);
  LinkTuples& fromRemainingKey();

  LinkTuples& toOne(const API::TupleValues& tupleKeyValues);
  LinkTuples& toMany(const std::vector<API::TupleValues>& tupleKeyValuesList);

  void prepare(API::ISchema& schema) override;

private:
  RelationshipPreparationData m_relationshipPreparationData;

};

}
