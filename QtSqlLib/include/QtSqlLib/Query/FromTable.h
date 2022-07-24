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

  FromTable& selectAll();
  FromTable& select(Schema::Id columnId);

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

  FromTable& joinAll(Schema::Id relationshipId);
  FromTable& joinColumns(Schema::Id relationshipId, Schema::Id columnId);

  template <typename... T>
  FromTable& joinColumns(Schema::Id relationshipId, Schema::Id columnId, T... args)
  {
    m_joins[relationshipId].bIsJoining = true;

    joinColumns(relationshipId, columnId);
    joinColumns(relationshipId, args...);

    m_joins[relationshipId].bIsJoining = false;
    m_joins[relationshipId].bJoined = true;
    return *this;
  }

  FromTable& where(Expr& expr);

  SqlQuery getSqlQuery(Schema& schema, QueryResults& previousQueryResults) override;
  QueryResults getQueryResults(Schema& schema, QSqlQuery& query) const override;

private:
  Schema::Id m_tableId;
  std::vector<Schema::Id> m_columnIds;

  bool m_bColumnsSelected;
  bool m_bIsSelecting;

  struct JoinData
  {
    bool bJoined = false;
    bool bIsJoining = false;

    std::vector<Schema::Id> columnIds;
  };

  std::map<Schema::Id, JoinData> m_joins;

  std::unique_ptr<Expr> m_whereExpr;

  void throwIfMultipleSelects() const;
  void throwIfMultipleJoins(Schema::Id relationshipId) const;

};

}