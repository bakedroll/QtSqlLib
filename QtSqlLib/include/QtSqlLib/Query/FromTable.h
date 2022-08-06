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
    Schema::Id tableId = 0U;
    QString tableAlias;
  
    std::vector<ColumnInfo> columnInfos;
    bool bColumnsSelected = false;
    bool bIsSelecting = false;
    std::vector<int> primaryKeyColumnIndicesInQuery;
    std::vector<int> foreignKeyColumnIndicesInQuery;
  };

  struct TableAliasColumnId
  {
    QString tableAlias;
    Schema::TableColumnId tableColumnId;
  };

  ColumnSelectionInfo m_columnSelectionInfo;
  std::map<Schema::Id, ColumnSelectionInfo> m_joins;

  std::vector<TableAliasColumnId> m_allSelectedColumns;
  // TODO: consider extra columns
  //std::set<Schema::TableColumnId> m_extraSelectedColumns;

  std::unique_ptr<Expr> m_whereExpr;
  bool m_isTableAliasesNeeded;

  void throwIfMultipleSelects() const;
  void throwIfMultipleJoins(Schema::Id relationshipId) const;

  void verifyJoinsAndCheckAliasesNeeded(Schema& schema);
  void generateTableAliases();

  void addToSelectedColumns(const Schema& schema, const Schema::Table& table,
                            ColumnSelectionInfo& columnSelectionInfo);
  void addForeignKeyColumns(const Schema::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColumnIdMap,
                            std::vector<int>& foreignKeyColumnIndicesInQuery,
                            Schema::Id childTableId, const QString& childTableAlias);

  QString processJoinsAndCreateQuerySubstring(Schema& schema, const Schema::Table& table);

  static std::vector<ColumnInfo> getAllTableColumnIds(const Schema::Table& table);

  QString createSelectString(Schema& schema, const std::vector<TableAliasColumnId>& tableColumnIds) const;
  void appendJoinQuerySubstring(QString& joinStrOut, Schema& schema, Schema::Id relationshipId,
                                Schema::Id parentTableId, const QString& parentTableAlias,
                                Schema::Id childTableId, const QString& childTableAlias,
                                const Schema::Table& joinTable, const QString& joinTableAlias,
                                const std::map<Schema::RelationshipTableId, Schema::ForeignKeyReference>& foreignKeyReferences,
                                std::vector<int>& foreignKeyColumnIndicesInQuery);

};

}
