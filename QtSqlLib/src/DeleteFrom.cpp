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
  ResultSet& /*previousQueryResults*/)
{
  schema.getSanityChecker().throwIfTableIdNotExisting(m_tableId);
  const auto& table = schema.getTables().at(m_tableId);

  QString queryStr;
  queryStr.append(QString("DELETE FROM '%1'").arg(table.name));

  std::vector<QVariant> boundValues;
  if (m_whereExpr)
  {
    ID tid(m_tableId);
    queryStr.append(QString(" WHERE %1").arg(m_whereExpr->toQueryString(schema, boundValues, tid)));
  }
  queryStr.append(";");

  QSqlQuery query(db);
  query.prepare(queryStr);
  for (const auto& value : boundValues)
  {
    query.addBindValue(value);
  }

  return { std::move(query) };
}

}
