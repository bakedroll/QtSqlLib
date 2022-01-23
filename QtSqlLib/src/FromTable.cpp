#include "QtSqlLib/FromTable.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

FromTable::FromTable(unsigned int tableId)
  : m_tableId(tableId)
  , m_bColumnsSelected(false)
  , m_bIsSelecting(false)
{
}

FromTable::~FromTable() = default;

FromTable& FromTable::select(unsigned int columnId)
{
  if (m_bColumnsSelected)
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery,
      "Invalid query: select() should only be called once.");
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
    throw DatabaseException(DatabaseException::Type::InvalidQuery,
      "Invalid query: where() should only be called once.");
  }

  m_whereExpr = std::make_unique<Expr>(std::move(expr));
  return *this;
}

QSqlQuery FromTable::getSqlQuery(const SchemaConfigurator::Schema& schema) const
{
  if (schema.tables.count(m_tableId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery,
      QString("Invalid query: Unknown table id %1").arg(m_tableId));
  }

  const auto& table = schema.tables.at(m_tableId);

  QString selectColsStr = "";
  if (m_columnIds.empty())
  {
    selectColsStr = "*";
  }
  else
  {
    for (const auto& col : m_columnIds)
    {
      if (table.columns.count(col) == 0)
      {
        throw DatabaseException(DatabaseException::Type::InvalidQuery,
          QString("Invalid query: Unknown column id %1 for table id %2").arg(col).arg(m_tableId));
      }

      if (!selectColsStr.isEmpty())
      {
        selectColsStr.append(", ");
      }
      selectColsStr.append(QString("%1.%2").arg(table.name).arg(table.columns.at(col).name));
    }
  }

  QString queryStr;
  queryStr.append(QString("SELECT %1 FROM %2").arg(selectColsStr).arg(table.name));

  if (m_whereExpr)
  {
    queryStr.append(QString(" WHERE %1").arg(m_whereExpr->toQString(schema, m_tableId)));
  }

  queryStr.append(";");

  return QSqlQuery(queryStr);
}

IQuery::QueryResults FromTable::getQueryResults(const SchemaConfigurator::Schema& schema, QSqlQuery& query) const
{
  QueryResults results;

  while (query.next())
  {
    ColumnResultMap resultMap;

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
