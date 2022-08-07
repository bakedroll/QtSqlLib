#include "QtSqlLib/Query/InsertInto.h"

namespace QtSqlLib::Query
{

InsertInto::InsertInto(API::ISchema::Id tableId)
  : BaseInsert(tableId)
{
}

InsertInto::~InsertInto() = default;

InsertInto& InsertInto::value(API::ISchema::Id columnId, const QVariant& value)
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

API::IQuery::SqlQuery InsertInto::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, QueryResults& previousQueryResults)
{
  return { getQSqlQuery(db, schema), QueryMode::Single };
}

}
