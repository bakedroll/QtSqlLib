#pragma once

#include <QtSqlLib/Query/Query.h>

#include <QtSqlLib/API/IID.h>

#include <memory>

namespace QtSqlLib
{
class Expr;
}

namespace QtSqlLib::Query
{

class DeleteFrom : public Query
{
public:
  DeleteFrom(const API::IID& tableId);
  ~DeleteFrom() override;

  DeleteFrom& where(Expr& expr);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, const ResultSet_bak& previousQueryResults) override;

private:
  API::IID::Type m_tableId;
  std::unique_ptr<Expr> m_whereExpr;

};

}
