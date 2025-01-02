#include "QueryInsertedIDs.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/API/ISchema.h"
#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib::Query
{

QueryInsertedIDs::QueryInsertedIDs(API::IID::Type tableId)
  : Query()
  , m_tableId(tableId)
{
}

QueryInsertedIDs::~QueryInsertedIDs() = default;

API::IQuery::SqlQuery QueryInsertedIDs::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, const ResultSet& /*previousQueryResults*/)
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

ResultSet QueryInsertedIDs::getQueryResults(API::ISchema& schema, QSqlQuery& query) const
{
  const auto& table = schema.getTables().at(m_tableId);

  if (!query.next())
  {
    throw DatabaseException(DatabaseException::Type::QueryError, 
      QString("Could not query last inserted id from table '%1'.").arg(table.name));
  }

  ResultSet::Tuple tuple;
  auto currentValue = 1;
  for (const auto& primaryKey : table.primaryKeys)
  {
    tuple.values[{ m_tableId, primaryKey }] = query.value(currentValue);
    currentValue++;
  }

  return ResultSet::create({ tuple });
}

}
