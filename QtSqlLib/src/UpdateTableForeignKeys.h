#pragma once

#include "QtSqlLib/Query/UpdateTable.h"

#include "QtSqlLib/RelationshipPreparationData.h"

namespace QtSqlLib::Query
{

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

}