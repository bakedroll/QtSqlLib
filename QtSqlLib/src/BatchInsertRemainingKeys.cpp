#include "BatchInsertRemainingKeys.h"

#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/ID.h"

namespace QtSqlLib::Query
{

BatchInsertRemainingKeys::BatchInsertRemainingKeys(API::IID::Type tableId,
  int numRelations,
  const API::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap)
  : BatchInsertInto(ID(tableId))
  , m_numRelations(numRelations)
  , m_primaryForeignKeyColIdMap(primaryForeignKeyColIdMap)
{
}

BatchInsertRemainingKeys::~BatchInsertRemainingKeys() = default;

API::IQuery::SqlQuery BatchInsertRemainingKeys::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema,
  ResultSet& previousQueryResults)
{
  if (!previousQueryResults.isValid() || !previousQueryResults.hasNextTuple())
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Expected previous query results.");
  }

  auto prevValues = previousQueryResults.nextTuple();
  for (const auto& primaryForeignKeyPair : m_primaryForeignKeyColIdMap)
  {
    API::IID::Type primaryKeyColId = primaryForeignKeyPair.first;
    API::IID::Type foreignKeyColId = primaryForeignKeyPair.second;

    if (!prevValues.hasColumnValue(primaryKeyColId))
    {
      throw DatabaseException(DatabaseException::Type::UnexpectedError, "Missing primary key value.");
    }
    const auto primaryKeyValue = prevValues.columnValue(primaryKeyColId);

    QVariantList list;
    for (auto i=0; i<m_numRelations; ++i)
    {
      list << primaryKeyValue;
    }

    values(ID(foreignKeyColId), list);
  }

  previousQueryResults.resetIteration();
  return BatchInsertInto::getSqlQuery(db, schema, previousQueryResults);
}

}
