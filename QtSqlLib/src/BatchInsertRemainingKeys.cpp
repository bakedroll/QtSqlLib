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
  const ResultSet& previousQueryResults)
{
  if (!previousQueryResults.isValid() || !previousQueryResults.hasNext())
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Expected previous query results.");
  }

  const auto& prevValues = previousQueryResults.next();
  for (const auto& value : prevValues)
  {
    if (m_primaryForeignKeyColIdMap.count(value.first) == 0)
    {
      throw DatabaseException(DatabaseException::Type::UnexpectedError, "Missing foreign key ref.");
    }
    QVariantList list;
    for (auto i=0; i<m_numRelations; ++i)
    {
      list << value.second;
    }

    values(ID(m_primaryForeignKeyColIdMap.at(value.first)), list);
  }

  previousQueryResults.resetIteration();
  return BatchInsertInto::getSqlQuery(db, schema, previousQueryResults);
}

}
