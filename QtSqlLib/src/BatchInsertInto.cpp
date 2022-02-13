#include "QtSqlLib/Query/BatchInsertInto.h"

namespace QtSqlLib::Query
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

API::IQuery::SqlQuery BatchInsertInto::getSqlQuery(Schema& schema)
{
  return { getQSqlQuery(schema), QueryMode::Batch };
}

void BatchInsertInto::bindQueryValues(QSqlQuery& query) const
{
  for (const auto& value : m_values)
  {
    query.addBindValue(value);
  }
}

}
