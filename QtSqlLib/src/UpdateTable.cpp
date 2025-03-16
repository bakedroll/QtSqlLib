#include "QtSqlLib/Query/UpdateTable.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/API/ISchema.h"
#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/Expr.h"
#include "QtSqlLib/ID.h"

namespace QtSqlLib::Query
{
UpdateTable::UpdateTable(const API::IID& tableId)
  : m_tableId(tableId.get())
{
}

UpdateTable::~UpdateTable() = default;

UpdateTable& UpdateTable::set(const API::IID& columnId, const QVariant& newValue)
{
  if (m_colIdNewValueMap.count(columnId.get()) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "set() can only be called once per column id");
  }

  m_colIdNewValueMap[columnId.get()] = newValue;
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

API::IQuery::SqlQuery UpdateTable::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, ResultSet& /*previousQueryResults*/)
{
  schema.getSanityChecker().throwIfTableIdNotExisting(m_tableId);

  const auto& table = schema.getTables().at(m_tableId);

  if (m_colIdNewValueMap.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "There has to be at least one column value set by an update query.");
  }

  QString setString = "";
  for (const auto& colValue : m_colIdNewValueMap)
  {
    schema.getSanityChecker().throwIfColumnIdNotExisting(table, colValue.first);

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
    ID tid(m_tableId);
    queryStr.append(QString(" WHERE %1").arg(m_whereExpr->toQString(schema, tid)));
  }

  queryStr.append(";");
  QSqlQuery query(db);
  query.prepare(queryStr);

  for (const auto& colValue : m_colIdNewValueMap)
  {
    query.addBindValue(colValue.second);
  }

  return { std::move(query) };
}

}
