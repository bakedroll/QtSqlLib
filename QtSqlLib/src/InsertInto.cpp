#include "QtSqlLib/InsertInto.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{
InsertInto::InsertInto(Schema::Id tableId)
  : BaseInsert(tableId)
  , m_returnIdMode(ReturnIdMode::Undefined)
{
}

InsertInto::~InsertInto() = default;

InsertInto& InsertInto::value(Schema::Id columnId, const QVariant& value)
{
  checkColumnIdExisting(columnId);

  addColumnId(columnId);
  m_values.emplace_back(value);
  return *this;
}

InsertInto& InsertInto::returnIds()
{
  if (m_returnIdMode != ReturnIdMode::Undefined)
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery, 
      "returnId() can only be called once per query.");
  }

  m_returnIdMode = ReturnIdMode::Yes;
  return *this;
}

QSqlQuery InsertInto::getSqlQuery(Schema& schema) const
{
  QSqlQuery query;
  const auto queryString = getSqlQueryString(schema);

  query.prepare(queryString);

  for (const auto& value : m_values)
  {
    query.addBindValue(value);
  }

  return query;
}

IQuery::QueryResults InsertInto::getQueryResults(Schema& schema, QSqlQuery& query) const
{
  if (m_returnIdMode != ReturnIdMode::Yes)
  {
    return {};
  }

  const auto& table = schema.getTables().at(getTableId());

  QString keyColumns;
  for (const auto& primaryKey : table.primaryKeys)
  {
    keyColumns += QString("'%1'.'%2', ").arg(table.name).arg(table.columns.at(primaryKey).name);
  }
  keyColumns = keyColumns.left(keyColumns.length() - 2);

  QSqlQuery lastIdQuery(QString("SELECT rowid, %1 FROM '%2' WHERE rowid = last_insert_rowid();")
    .arg(keyColumns).arg(table.name));

  lastIdQuery.exec();
  if (!lastIdQuery.next())
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery, 
      QString("Could not query last inserted id from table '%1'.").arg(table.name));
  }

  ColumnResultMap resultsMap;

  auto currentValue = 1;
  for (const auto& primaryKey : table.primaryKeys)
  {
    resultsMap[{ getTableId(), primaryKey }] = lastIdQuery.value(currentValue);
    currentValue++;
  }

  return { resultsMap };
}

}
