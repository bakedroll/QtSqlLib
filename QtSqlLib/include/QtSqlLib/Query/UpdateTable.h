#pragma once

#include <QtSqlLib/Query/Query.h>

#include <QtSqlLib/API/IID.h>

namespace QtSqlLib
{
class Expr;
}

namespace QtSqlLib::Query
{

class UpdateTable : public Query
{
public:
  UpdateTable(const API::IID& tableId);
  ~UpdateTable() override;

  UpdateTable& set(const API::IID& columnId, const QVariant& newValue);
  UpdateTable& where(Expr& expr);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, ResultSet& previousQueryResults) override;

private:
  API::IID::Type m_tableId;

  std::map<API::IID::Type, QVariant> m_colIdNewValueMap;
  std::unique_ptr<Expr> m_whereExpr;
};

}
