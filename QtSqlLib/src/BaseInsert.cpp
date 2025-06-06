#include "QtSqlLib/Query/BaseInsert.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/API/ISchema.h"
#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib::Query
{

BaseInsert::BaseInsert(const API::IID& tableId)
  : Query()
  , m_tableId(tableId.get())
{
}

BaseInsert::~BaseInsert() = default;

void BaseInsert::addColumn(const API::IID& id)
{
  throwIfColumnIdAlreadyExisting(id.get());
  m_columns.data().emplace_back(id.get());
}

void BaseInsert::throwIfColumnIdAlreadyExisting(API::IID::Type id) const
{
  for (const auto& cid : m_columns.cdata())
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
  schema.getSanityChecker().throwIfTableIdNotExisting(m_tableId);

  const auto& table = schema.getTables().at(m_tableId);

  QString columnsString;
  QString valuesString;

  for (const auto& id : m_columns.cdata())
  {
    schema.getSanityChecker().throwIfColumnIdNotExisting(table, id);

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
