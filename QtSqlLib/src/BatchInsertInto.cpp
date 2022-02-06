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
  addColumnId(columnId);
  m_values.emplace_back(value);
  return *this;
}

QueryDefines::SqlQuery BatchInsertInto::getSqlQuery(Schema& schema)
{
  QSqlQuery query;
  const auto queryString = getSqlQueryString(schema);

  query.prepare(queryString);

  for (const auto& value : m_values)
  {
    query.addBindValue(value);
  }

  return { query, QueryDefines::QueryMode::Batch };
}

}
