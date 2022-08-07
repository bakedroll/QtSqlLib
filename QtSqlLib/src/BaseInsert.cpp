#include "QtSqlLib/Query/BaseInsert.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib::Query
{

BaseInsert::BaseInsert(API::ISchema::Id tableId)
  : Query()
  , m_tableId(tableId)
{
}

BaseInsert::~BaseInsert() = default;

void BaseInsert::addColumnId(API::ISchema::Id id)
{
  throwIfColumnIdAlreadyExisting(id);
  m_columnIds.emplace_back(id);
}

void BaseInsert::throwIfColumnIdAlreadyExisting(API::ISchema::Id id) const
{
  for (const auto& cid : m_columnIds)
  {
    if (cid == id)
    {
      throw DatabaseException(DatabaseException::Type::InvalidId,
        QString("More than one column with id %1 specified.").arg(id));
    }
  }
}

QSqlQuery BaseInsert::getQSqlQuery(const QSqlDatabase& db, API::ISchema& schema) const
{
  schema.throwIfTableIdNotExisting(m_tableId);

  const auto& table = schema.getTables().at(m_tableId);

  QString columnsString;
  QString valuesString;

  for (const auto& id : m_columnIds)
  {
    schema.throwIfColumnIdNotExisting(table, id);

    columnsString += QString("'%1', ").arg(table.columns.at(id).name);
    valuesString += "?, ";
  }

  columnsString = columnsString.left(columnsString.length() - 2);
  valuesString = valuesString.left(valuesString.length() - 2);

  QSqlQuery query(db);
  query.prepare(QString("INSERT INTO '%1' (%2) VALUES (%3);").arg(table.name).arg(columnsString).arg(valuesString));

  bindQueryValues(query);

  return query;
}

}
