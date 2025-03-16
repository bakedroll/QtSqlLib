#include "UpdateTableForeignKeys.h"

#include "QtSqlLib/ColumnID.h"
#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/Expr.h"
#include "QtSqlLib/ID.h"

namespace QtSqlLib::Query
{

UpdateTableForeignKeys::UpdateTableForeignKeys(
  API::IID::Type tableId,
  const API::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap)
  : UpdateTable(ID(tableId))
  , m_remainingKeysMode(RelationshipPreparationData::RemainingKeysMode::NoRemainingKeys)
  , m_primaryForeignKeyColIdMap(primaryForeignKeyColIdMap)
{
}

UpdateTableForeignKeys::~UpdateTableForeignKeys() = default;

void UpdateTableForeignKeys::setRemainingKeysMode(RelationshipPreparationData::RemainingKeysMode mode)
{
  m_remainingKeysMode = mode;
}

void UpdateTableForeignKeys::setForeignKeyValues(const PrimaryKey& parentKeyValues)
{
  for (const auto& parentKeyValue : parentKeyValues.values())
  {
    const auto childColId = m_primaryForeignKeyColIdMap.at(parentKeyValue.columnId);
    set(ID(childColId), parentKeyValue.value);
  }
}

void UpdateTableForeignKeys::makeAndAddWhereExpr(const PrimaryKey& affectedChildKeyValues)
{
  Expr whereExpr;
  for (const auto& childKeyValue : affectedChildKeyValues.values())
  {
    if (childKeyValue.columnId != affectedChildKeyValues.values().begin()->columnId)
    {
      whereExpr.opAnd();
    }
    whereExpr.equal(ID(childKeyValue.columnId), childKeyValue.value);
  }

  where(whereExpr);
}

API::IQuery::SqlQuery UpdateTableForeignKeys::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema,
  ResultSet& previousQueryResults)
{
  const auto throwIfInvalidPreviousQueryResults = [&previousQueryResults]()
  {
    if (!previousQueryResults.isValid() || !previousQueryResults.hasNextTuple())
    {
      throw DatabaseException(DatabaseException::Type::InvalidSyntax,
        "Expected previous query results.");
    }
  };

  if (m_remainingKeysMode == RelationshipPreparationData::RemainingKeysMode::RemainingForeignKeys)
  {
    throwIfInvalidPreviousQueryResults();
    setForeignKeyValues(previousQueryResults.nextTuple().primaryKey());
  }
  else if (m_remainingKeysMode == RelationshipPreparationData::RemainingKeysMode::RemainingPrimaryKeys)
  {
    throwIfInvalidPreviousQueryResults();
    makeAndAddWhereExpr(previousQueryResults.nextTuple().primaryKey());
  }

  previousQueryResults.resetIteration();
  return UpdateTable::getSqlQuery(db, schema, previousQueryResults);
}

}
