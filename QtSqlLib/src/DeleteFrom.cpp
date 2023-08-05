#include "QtSqlLib/Query/DeleteFrom.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/API/ISchema.h"
#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/Expr.h"
#include "QtSqlLib/ID.h"

namespace QtSqlLib::Query
{

DeleteFrom::DeleteFrom(const API::IID& tableId) :
  Query(),
  m_tableId(tableId.get())
{
}

DeleteFrom::~DeleteFrom() = default;

DeleteFrom& DeleteFrom::where(Expr& expr)
{
  if (m_whereExpr)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "where() should only be called once.");
  }

  m_whereExpr = std::make_unique<Expr>(std::move(expr));
  return *this;
}

API::IQuery::SqlQuery DeleteFrom::getSqlQuery(
  const QSqlDatabase& db,
  API::ISchema& schema,
  const ResultSet& previousQueryResults)
{
  schema.getSanityChecker().throwIfTableIdNotExisting(m_tableId);
  const auto& table = schema.getTables().at(m_tableId);

  QString queryStr;
  queryStr.append(QString("DELETE FROM '%1'").arg(table.name));

  if (m_whereExpr)
  {
    ID tid(m_tableId);
    queryStr.append(QString(" WHERE %1").arg(m_whereExpr->toQString(schema, tid)));
  }
  queryStr.append(";");

  return { QSqlQuery(queryStr, db) };
}

}
