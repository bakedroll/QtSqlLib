#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/ISchema.h>

#include "QtSqlLib/Query/BatchInsertInto.h"
#include "QtSqlLib/Query/UpdateTable.h"
#include "QtSqlLib/RelationshipPreparationData.h"
#include <QtSqlLib/Query/QuerySequence.h>

#define LINK_TUPLES(X) QtSqlLib::Query::LinkTuples(QtSqlLib::ID(X))

#define FROM_ONE(X) fromOne(X)
#define FROM_REMAINING_KEY fromRemainingKey()
#define TO_ONE(X) toOne(X)
#define TO_MANY(...) toMany(__VA_ARGS__)

namespace QtSqlLib::Query
{

class LinkTuples : public QuerySequence
{
public:
  LinkTuples(const API::IID& relationshipId);
  ~LinkTuples() override;

  LinkTuples& fromOne(const API::ISchema::TupleValues& tupleKeyValues);
  LinkTuples& fromRemainingKey();

  LinkTuples& toOne(const API::ISchema::TupleValues& tupleKeyValues);
  LinkTuples& toMany(const std::vector<API::ISchema::TupleValues>& tupleKeyValuesList);

  void prepare(API::ISchema& schema) override;

private:
  class UpdateTableForeignKeys : public UpdateTable
  {
  public:
    UpdateTableForeignKeys(
      API::IID::Type tableId,
      const API::ISchema::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap);
    ~UpdateTableForeignKeys() override;

    void setRemainingKeysMode(RelationshipPreparationData::RemainingKeysMode mode);
    void setForeignKeyValues(const API::ISchema::TupleValues& parentKeyValues);
    void makeAndAddWhereExpr(const API::ISchema::TupleValues& affectedChildKeyValues);

    SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, QueryResults& previousQueryResults) override;

  private:
    RelationshipPreparationData::RemainingKeysMode m_remainingKeysMode;
    const API::ISchema::PrimaryForeignKeyColumnIdMap& m_primaryForeignKeyColIdMap;

  };

  class BatchInsertRemainingKeys : public BatchInsertInto
  {
  public:
    BatchInsertRemainingKeys(
      API::IID::Type tableId,
      int numRelations,
      const API::ISchema::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap);
    ~BatchInsertRemainingKeys() override;

    SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, QueryResults& previousQueryResults) override;

  private:
    int m_numRelations;
    const API::ISchema::PrimaryForeignKeyColumnIdMap& m_primaryForeignKeyColIdMap;

  };

  RelationshipPreparationData m_relationshipPreparationData;

};

}
