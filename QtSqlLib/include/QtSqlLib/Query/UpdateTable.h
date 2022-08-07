#pragma once

#include <QtSqlLib/Query/Query.h>

#include <QtSqlLib/Expr.h>

namespace QtSqlLib::Query
{

class UpdateTable : public Query
{
public:
  UpdateTable(API::ISchema::Id tableId);
  ~UpdateTable() override;

  UpdateTable& set(API::ISchema::Id columnId, const QVariant& newValue);
  UpdateTable& where(Expr& expr);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, QueryResults& previousQueryResults) override;

private:
  API::ISchema::Id m_tableId;

  std::map<API::ISchema::Id, QVariant> m_colIdNewValueMap;
  std::unique_ptr<Expr> m_whereExpr;
};

}
