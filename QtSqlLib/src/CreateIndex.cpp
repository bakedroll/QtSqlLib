#include "CreateIndex.h"

#include <QtSqlLib/API/ISchema.h>

namespace QtSqlLib::Query
{

CreateIndex::CreateIndex(const API::Index& index) :
  m_index(index)
{
}

CreateIndex::~CreateIndex() = default;

API::IQuery::SqlQuery CreateIndex::getSqlQuery(
  const QSqlDatabase& db, API::ISchema& schema,
  const ResultSet& previousQueryResults)
{
  const auto& table = schema.getTables().at(m_index.tableId);

  QString columns;
  auto isFirst = true;
  for (const auto& colId : m_index.columnIds)
  {
    if (!isFirst)
    {
      columns.append(", ");
    }

    columns.append(QString("'%1'").arg(table.columns.at(colId).name));
    isFirst = false;
  }

  auto test = QString("CREATE %1INDEX '%2' ON '%3'(%4);")
    .arg(m_index.isUnique ? "UNIQUE " : "")
    .arg(m_index.name)
    .arg(table.name)
    .arg(columns);

  QSqlQuery query;
  query.prepare(QString("CREATE %1INDEX '%2' ON '%3'(%4);")
    .arg(m_index.isUnique ? "UNIQUE " : "")
    .arg(m_index.name)
    .arg(table.name)
    .arg(columns));

  return { std::move(query) };
}

}
