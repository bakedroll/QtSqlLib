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
  struct SelectColumnInfo
  {
    Schema::Id columnId = 0U;
    int indexInQuery = -1;
  };

  Schema::Id m_tableId;
  std::vector<SelectColumnInfo> m_columnInfo;

  bool m_bColumnsSelected;
  bool m_bIsSelecting;

  struct JoinData
  {
    bool bJoined = false;
    bool bIsJoining = false;

    std::vector<SelectColumnInfo> columnInfo;
  };

  std::map<Schema::Id, JoinData> m_joins;
  std::vector<Schema::TableColumnId> m_allSelectedColumns;
  std::set<Schema::TableColumnId> m_extraSelectedColumns;

  std::map<Schema::TableColumnId, int> m_keyColumnIndicesInQuery;

  std::unique_ptr<Expr> m_whereExpr;

  void throwIfMultipleSelects() const;
  void throwIfMultipleJoins(Schema::Id relationshipId) const;

  void addToSelectedColumns(const Schema& schema, const Schema::Table& table, Schema::Id tableId,
                            std::vector<SelectColumnInfo>& columnInfos);

  QString processJoinsAndCreateQuerySubstring(Schema& schema, const Schema::Table& table);

  static std::vector<SelectColumnInfo> getAllTableColumnIds(const Schema::Table& table);

};

}