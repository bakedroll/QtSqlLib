#pragma once

#include <QtSqlLib/Query/Query.h>

#include <QtSqlLib/Expr.h>

namespace QtSqlLib::Query
{

class UpdateTable : public Query
{
public:
  UpdateTable(Schema::Id tableId);
  ~UpdateTable() override;

  UpdateTable& set(Schema::Id columnId, const QVariant& newValue);
  UpdateTable& where(Expr& expr);

  SqlQuery getSqlQuery(Schema& schema, QueryResults& previousQueryResults) override;

private:
  Schema::Id m_tableId;

  std::map<Schema::Id, QVariant> m_colIdNewValueMap;
  std::unique_ptr<Expr> m_whereExpr;
};

}
