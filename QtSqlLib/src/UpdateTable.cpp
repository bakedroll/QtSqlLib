#include "QtSqlLib/Query/UpdateTable.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib::Query
{
UpdateTable::UpdateTable(Schema::Id tableId)
  : m_tableId(tableId)
{
}

UpdateTable::~UpdateTable() = default;

UpdateTable& UpdateTable::set(Schema::Id columnId, const QVariant& newValue)
{
  if (m_colIdNewValueMap.count(columnId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "set() can only be called once per column id");
  }

  m_colIdNewValueMap[columnId] = newValue;
  return *this;
}

UpdateTable& UpdateTable::where(Expr& expr)
{
  if (m_whereExpr)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "where() should only be called once.");
  }

  m_whereExpr = std::make_unique<Expr>(std::move(expr));
  return *this;
}

QueryDefines::SqlQuery UpdateTable::getSqlQuery(Schema& schema)
{
  schema.throwIfTableIdNotExisting(m_tableId);

  const auto& table = schema.getTables().at(m_tableId);

  if (m_colIdNewValueMap.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "There has to be at least one column value set by an update query.");
  }

  QString setString = "";
  for (const auto& colValue : m_colIdNewValueMap)
  {
    schema.throwIfColumnIdNotExisting(table, colValue.first);

    if (!setString.isEmpty())
    {
      setString.append(", ");
    }
    setString.append(QString("'%1' = ?").arg(table.columns.at(colValue.first).name));
  }

  QString queryStr;
  queryStr.append(QString("UPDATE '%1' SET %2").arg(table.name).arg(setString));

  if (m_whereExpr)
  {
    queryStr.append(QString(" WHERE %1").arg(m_whereExpr->toQString(schema, m_tableId)));
  }

  queryStr.append(";");
  QSqlQuery query;
  query.prepare(queryStr);

  for (const auto& colValue : m_colIdNewValueMap)
  {
    query.addBindValue(colValue.second);
  }

  return { query };
}

}
