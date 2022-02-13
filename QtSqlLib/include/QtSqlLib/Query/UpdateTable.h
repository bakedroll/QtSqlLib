#pragma once

#include <QtSqlLib/API/IQuery.h>

#include <QtSqlLib/Expr.h>

namespace QtSqlLib::Query
{

class UpdateTable : public API::IQuery
{
public:
  UpdateTable(Schema::Id tableId);
  ~UpdateTable() override;

  UpdateTable& set(Schema::Id columnId, const QVariant& newValue);
  UpdateTable& where(Expr& expr);

  QueryDefines::SqlQuery getSqlQuery(Schema& schema) override;

private:
  Schema::Id m_tableId;

  std::map<Schema::Id, QVariant> m_colIdNewValueMap;
  std::unique_ptr<Expr> m_whereExpr;
};

}
