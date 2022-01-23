#include "QtSqlLib/BatchInsertInto.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{
BatchInsertInto::BatchInsertInto(Schema::Id tableId)
  : BaseInsert(tableId)
{
}

BatchInsertInto::~BatchInsertInto() = default;

BatchInsertInto& BatchInsertInto::values(Schema::Id columnId, const QVariantList& value)
{
  checkColumnIdExisting(columnId);

  addColumnId(columnId);
  m_values.emplace_back(value);
  return *this;
}

QSqlQuery BatchInsertInto::getSqlQuery(Schema& schema) const
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

bool BatchInsertInto::isBatchExecution() const
{
  return true;
}
}
