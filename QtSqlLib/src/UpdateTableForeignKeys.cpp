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

void UpdateTableForeignKeys::setForeignKeyValues(const API::TupleValues& parentKeyValues)
{
  for (const auto& parentKeyValue : parentKeyValues)
  {
    const auto childColId = m_primaryForeignKeyColIdMap.at(parentKeyValue.first);
    set(ID(childColId), parentKeyValue.second);
  }
}

void UpdateTableForeignKeys::makeAndAddWhereExpr(const API::TupleValues& affectedChildKeyValues)
{
  Expr whereExpr;
  for (const auto& childKeyValue : affectedChildKeyValues)
  {
    if (childKeyValue.first != affectedChildKeyValues.begin()->first)
    {
      whereExpr.and();
    }
    whereExpr.equal(ID(childKeyValue.first.columnId), childKeyValue.second);
  }

  where(whereExpr);
}

API::IQuery::SqlQuery UpdateTableForeignKeys::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema,
  const ResultSet& previousQueryResults)
{
  const auto throwIfInvalidPreviousQueryResults = [&previousQueryResults]()
  {
    if (!previousQueryResults.isValid() || !previousQueryResults.hasNext())
    {
      throw DatabaseException(DatabaseException::Type::InvalidSyntax,
        "Expected previous query results.");
    }
  };

  if (m_remainingKeysMode == RelationshipPreparationData::RemainingKeysMode::RemainingForeignKeys)
  {
    throwIfInvalidPreviousQueryResults();
    setForeignKeyValues(previousQueryResults.next());
  }
  else if (m_remainingKeysMode == RelationshipPreparationData::RemainingKeysMode::RemainingPrimaryKeys)
  {
    throwIfInvalidPreviousQueryResults();
    makeAndAddWhereExpr(previousQueryResults.next());
  }

  previousQueryResults.resetIteration();
  return UpdateTable::getSqlQuery(db, schema, previousQueryResults);
}

}
