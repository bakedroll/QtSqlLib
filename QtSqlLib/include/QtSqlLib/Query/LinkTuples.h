#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/ISchema.h>

#include "QtSqlLib/Query/BatchInsertInto.h"
#include "QtSqlLib/Query/UpdateTable.h"
#include "QtSqlLib/RelationshipPreparationData.h"
#include <QtSqlLib/Query/QuerySequence.h>

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
  class UpdateTableForeignKeys : public UpdateTable
  {
  public:
    UpdateTableForeignKeys(
      API::IID::Type tableId,
      const API::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap);
    ~UpdateTableForeignKeys() override;

    void setRemainingKeysMode(RelationshipPreparationData::RemainingKeysMode mode);
    void setForeignKeyValues(const API::TupleValues& parentKeyValues);
    void makeAndAddWhereExpr(const API::TupleValues& affectedChildKeyValues);

    SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, const ResultSet& previousQueryResults) override;

  private:
    RelationshipPreparationData::RemainingKeysMode m_remainingKeysMode;
    const API::PrimaryForeignKeyColumnIdMap& m_primaryForeignKeyColIdMap;

  };

  class BatchInsertRemainingKeys : public BatchInsertInto
  {
  public:
    BatchInsertRemainingKeys(
      API::IID::Type tableId,
      int numRelations,
      const API::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap);
    ~BatchInsertRemainingKeys() override;

    SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, const ResultSet& previousQueryResults) override;

  private:
    int m_numRelations;
    const API::PrimaryForeignKeyColumnIdMap& m_primaryForeignKeyColIdMap;

  };

  RelationshipPreparationData m_relationshipPreparationData;

};

}
