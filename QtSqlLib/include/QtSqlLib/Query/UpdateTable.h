#pragma once

#include <QtSqlLib/Query/Query.h>

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/Expr.h>

#define UPDATE_TABLE(X) QtSqlLib::Query::UpdateTable(QtSqlLib::ID(X))

#define SET(X, Y) set(QtSqlLib::ID(X), Y)
#define WHERE(X) where(QtSqlLib::Expr().X)

namespace QtSqlLib::Query
{

class UpdateTable : public Query
{
public:
  UpdateTable(const API::IID& tableId);
  ~UpdateTable() override;

  UpdateTable& set(const API::IID& columnId, const QVariant& newValue);
  UpdateTable& where(Expr& expr);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, QueryResults& previousQueryResults) override;

private:
  API::IID::Type m_tableId;

  std::map<API::IID::Type, QVariant> m_colIdNewValueMap;
  std::unique_ptr<Expr> m_whereExpr;
};

}
