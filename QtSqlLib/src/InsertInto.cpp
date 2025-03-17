#include "QtSqlLib/Query/InsertInto.h"

namespace QtSqlLib::Query
{

InsertInto::InsertInto(const API::IID& tableId)
  : BaseInsert(tableId)
{
}

InsertInto::~InsertInto() = default;

InsertInto& InsertInto::value(const API::IID& columnId, const QVariant& value)
{
  addColumn(columnId);
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

API::IQuery::SqlQuery InsertInto::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, const ResultSet& /*previousQueryResults*/)
{
  return { getQSqlQuery(db, schema), QueryMode::Single };
}

}
