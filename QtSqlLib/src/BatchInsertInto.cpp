#include "QtSqlLib/Query/BatchInsertInto.h"

namespace QtSqlLib::Query
{
BatchInsertInto::BatchInsertInto(const API::IID& tableId)
  : BaseInsert(tableId)
{
}

BatchInsertInto::~BatchInsertInto() = default;

BatchInsertInto& BatchInsertInto::values(const API::IID& columnId, const QVariantList& value)
{
  addColumn(columnId);
  m_values.emplace_back(value);
  return *this;
}

API::IQuery::SqlQuery BatchInsertInto::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, ResultSet& /*previousQueryResults*/)
{
  return { getQSqlQuery(db, schema), QueryMode::Batch };
}

void BatchInsertInto::bindQueryValues(QSqlQuery& query) const
{
  for (const auto& value : m_values)
  {
    query.addBindValue(value);
  }
}

}
