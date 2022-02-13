#include "QtSqlLib/Query/BaseInsert.h"

#include "QtSqlLib/DatabaseException.h"

#include <utilsLib/Utils.h>

namespace QtSqlLib::Query
{

BaseInsert::BaseInsert(Schema::Id tableId)
  : IQuery()
  , m_tableId(tableId)
{
}

BaseInsert::~BaseInsert() = default;

void BaseInsert::addColumnId(Schema::Id id)
{
  throwIfColumnIdAlreadyExisting(id);
  m_columnIds.emplace_back(id);
}

void BaseInsert::throwIfColumnIdAlreadyExisting(Schema::Id id) const
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

Schema::Id BaseInsert::getTableId() const
{
  return m_tableId;
}

QSqlQuery BaseInsert::getQSqlQuery(Schema& schema) const
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

  const auto queryString = QString("INSERT INTO '%1' (%2) VALUES (%3);").arg(table.name).arg(columnsString).arg(valuesString);
  UTILS_LOG_DEBUG(queryString.toStdString().c_str());

  QSqlQuery query;
  query.prepare(queryString);

  bindQueryValues(query);

  return query;
}

}
