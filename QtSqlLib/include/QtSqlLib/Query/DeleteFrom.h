#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/Query/Query.h>
#include <QtSqlLib/Expr.h>

#include <memory>

#define DELETE_FROM(X) QtSqlLib::Query::DeleteFrom(QtSqlLib::ID(X))
#define WHERE(X) where(QtSqlLib::Expr().X)

namespace QtSqlLib::Query
{

class DeleteFrom : public Query
{
public:
  DeleteFrom(const API::IID& tableId);
  ~DeleteFrom() override;

  DeleteFrom& where(Expr& expr);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, const ResultSet& previousQueryResults) override;

private:
  API::IID::Type m_tableId;
  std::unique_ptr<Expr> m_whereExpr;

};

}
