#include "QueryInsertedIDs.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/API/ISchema.h"
#include "QtSqlLib/DatabaseException.h"

#include <numeric>

namespace QtSqlLib::Query
{

QueryInsertedIDs::QueryInsertedIDs(API::IID::Type tableId)
  : Query()
  , m_tableId(tableId)
{
}

QueryInsertedIDs::~QueryInsertedIDs() = default;

API::IQuery::SqlQuery QueryInsertedIDs::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, ResultSet& /*previousQueryResults*/)
{
  schema.getSanityChecker().throwIfTableIdNotExisting(m_tableId);
  const auto& table = schema.getTables().at(m_tableId);

  QString keyColumns;
  for (const auto& primaryKey : table.primaryKeys)
  {
    keyColumns += QString("'%1'.'%2', ").arg(table.name).arg(table.columns.at(primaryKey).name);
  }
  keyColumns = keyColumns.left(keyColumns.length() - 2);

  return { QSqlQuery(QString("SELECT rowid, %1 FROM '%2' WHERE rowid = last_insert_rowid();")
    .arg(keyColumns).arg(table.name),
    db) };
}

ResultSet QueryInsertedIDs::getQueryResults(API::ISchema& schema, QSqlQuery&& query)
{
  const auto& table = schema.getTables().at(m_tableId);

  if (!query.next())
  {
    throw DatabaseException(DatabaseException::Type::QueryError, 
      QString("Could not query last inserted id from table '%1'.").arg(table.name));
  }
  query.seek(-1);

  ColumnList columns(table.primaryKeys);
  std::vector<size_t> columnQueryIndices(table.primaryKeys.size());
  std::vector<size_t> primaryKeyColumnIndices(table.primaryKeys.size());

  std::iota(columnQueryIndices.begin(), columnQueryIndices.end(), 1);
  std::iota(primaryKeyColumnIndices.begin(), primaryKeyColumnIndices.end(), 0);

  API::QueryMetaInfo queryMetaInfo { m_tableId, std::nullopt, columns, columnQueryIndices, primaryKeyColumnIndices };
  return ResultSet(std::move(query), std::move(queryMetaInfo), {} );
}

}
