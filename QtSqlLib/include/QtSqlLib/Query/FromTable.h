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
    m_columnSelectionInfo.bIsSelecting = true;

    select(columnId);
    select(args...);

    m_columnSelectionInfo.bIsSelecting = false;
    m_columnSelectionInfo.bColumnsSelected = true;
    return *this;
  }

  FromTable& joinAll(Schema::Id relationshipId);
  FromTable& joinColumns(Schema::Id relationshipId, Schema::Id columnId);

  template <typename... T>
  FromTable& joinColumns(Schema::Id relationshipId, Schema::Id columnId, T... args)
  {
    m_joins[relationshipId].bIsSelecting = true;

    joinColumns(relationshipId, columnId);
    joinColumns(relationshipId, args...);

    m_joins[relationshipId].bIsSelecting = false;
    m_joins[relationshipId].bColumnsSelected = true;
    return *this;
  }

  FromTable& where(Expr& expr);

  SqlQuery getSqlQuery(Schema& schema, QueryResults& previousQueryResults) override;
  QueryResults getQueryResults(Schema& schema, QSqlQuery& query) const override;

private:
  struct ColumnInfo
  {
    Schema::Id columnId = 0U;
    int indexInQuery = -1;
  };

  struct ColumnSelectionInfo
  {
    std::vector<ColumnInfo> columnInfos;
    bool bColumnsSelected = false;
    bool bIsSelecting = false;
    std::vector<int> keyColumnIndicesInQuery;
  };

  Schema::Id m_tableId;

  ColumnSelectionInfo m_columnSelectionInfo;
  std::map<Schema::Id, ColumnSelectionInfo> m_joins;

  std::vector<Schema::TableColumnId> m_allSelectedColumns;
  std::set<Schema::TableColumnId> m_extraSelectedColumns;

  std::unique_ptr<Expr> m_whereExpr;

  void throwIfMultipleSelects() const;
  void throwIfMultipleJoins(Schema::Id relationshipId) const;

  void addToSelectedColumns(const Schema& schema, const Schema::Table& table, Schema::Id tableId,
                            ColumnSelectionInfo& columnSelectionInfo);

  QString processJoinsAndCreateQuerySubstring(Schema& schema, const Schema::Table& table);

  static std::vector<ColumnInfo> getAllTableColumnIds(const Schema::Table& table);

};

}