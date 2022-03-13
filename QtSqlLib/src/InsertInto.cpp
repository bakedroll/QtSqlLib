#include "QtSqlLib/Query/InsertInto.h"

namespace QtSqlLib::Query
{

InsertInto::InsertInto(Schema::Id tableId)
  : BaseInsert(tableId)
{
}

InsertInto::~InsertInto() = default;

InsertInto& InsertInto::value(Schema::Id columnId, const QVariant& value)
{
  addColumnId(columnId);
  m_values.emplace_back(value);
  return *this;
}

void InsertInto::bindQueryValues(QSqlQuery& query) const
{
  for (const auto& value : m_values)
  {
    query.addBindValue(value);
  }
}

API::IQuery::SqlQuery InsertInto::getSqlQuery(Schema& schema, QueryResults& previousQueryResults)
{
  return { getQSqlQuery(schema), QueryMode::Single };
}

}
