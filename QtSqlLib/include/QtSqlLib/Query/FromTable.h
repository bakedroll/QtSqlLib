#pragma once

#include <QtSqlLib/Query/Query.h>
#include <QtSqlLib/Expr.h>

#include <vector>

namespace QtSqlLib::Query
{

class FromTable : public Query
{
public:
  FromTable(Schema::Id tableId);
  ~FromTable() override;

  FromTable& select(Schema::Id columnId);
  FromTable& where(Expr& expr);

  template <typename... T>
  FromTable& select(Schema::Id columnId, T... args)
  {
    m_bIsSelecting = true;

    select(columnId);
    select(args...);

    m_bIsSelecting = false;
    m_bColumnsSelected = true;
    return *this;
  }

  SqlQuery getSqlQuery(Schema& schema) override;
  QueryResults getQueryResults(Schema& schema, QSqlQuery& query) const override;

private:
  Schema::Id m_tableId;
  std::vector<Schema::Id> m_columnIds;

  bool m_bColumnsSelected;
  bool m_bIsSelecting;

  std::unique_ptr<Expr> m_whereExpr;

};

}