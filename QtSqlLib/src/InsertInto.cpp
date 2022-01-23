#include "QtSqlLib/InsertInto.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{
InsertInto::InsertInto(Schema::Id tableId)
  : BaseInsert(tableId)
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

}
