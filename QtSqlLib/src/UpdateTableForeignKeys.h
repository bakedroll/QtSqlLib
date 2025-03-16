#pragma once

#include "QtSqlLib/Query/UpdateTable.h"

#include "QtSqlLib/PrimaryKey.h"
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
  void setForeignKeyValues(const PrimaryKey& parentKeyValues);
  void makeAndAddWhereExpr(const PrimaryKey& affectedChildKeyValues);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, ResultSet& previousQueryResults) override;

private:
  RelationshipPreparationData::RemainingKeysMode m_remainingKeysMode;
  const API::PrimaryForeignKeyColumnIdMap& m_primaryForeignKeyColIdMap;

};

}