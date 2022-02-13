#include "QtSqlLib/Query/FromTable.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib::Query
{

FromTable::FromTable(Schema::Id tableId)
  : m_tableId(tableId)
  , m_bColumnsSelected(false)
  , m_bIsSelecting(false)
{
}

FromTable::~FromTable() = default;

FromTable& FromTable::select(Schema::Id columnId)
{
  if (m_bColumnsSelected)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "select() should only be called once.");
  }

  m_columnIds.emplace_back(columnId);

  if (!m_bIsSelecting)
  {
    m_bColumnsSelected = true;
  }
  return *this;
}

FromTable& FromTable::where(Expr& expr)
{
  if (m_whereExpr)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "where() should only be called once.");
  }

  m_whereExpr = std::make_unique<Expr>(std::move(expr));
  return *this;
}

QueryDefines::SqlQuery FromTable::getSqlQuery(Schema& schema)
{
  schema.throwIfTableIdNotExisting(m_tableId);

  const auto& table = schema.getTables().at(m_tableId);

  QString selectColsStr = "";
  if (m_columnIds.empty())
  {
    selectColsStr = "*";
  }
  else
  {
    for (const auto& col : m_columnIds)
    {
      schema.throwIfColumnIdNotExisting(table, col);

      if (!selectColsStr.isEmpty())
      {
        selectColsStr.append(", ");
      }
      selectColsStr.append(QString("'%1'.'%2'").arg(table.name).arg(table.columns.at(col).name));
    }
  }

  QString queryStr;
  queryStr.append(QString("SELECT %1 FROM '%2'").arg(selectColsStr).arg(table.name));

  if (m_whereExpr)
  {
    queryStr.append(QString(" WHERE %1").arg(m_whereExpr->toQString(schema, m_tableId)));
  }

  queryStr.append(";");

  return { QSqlQuery(queryStr) };
}

QueryDefines::QueryResults FromTable::getQueryResults(Schema& schema, QSqlQuery& query) const
{
  QueryDefines::QueryResults results;

  while (query.next())
  {
    Schema::TableColumnValuesMap resultMap;

    auto i = 0;
    for (const auto& col : m_columnIds)
    {
      resultMap[{ m_tableId, col }] = query.value(i);
      i++;
    }

    results.emplace_back(resultMap);
  }

  return results;
}

}
