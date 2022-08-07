#pragma once

#include <QtSqlLib/Query/Query.h>
#include <QtSqlLib/Expr.h>

#include <vector>

namespace QtSqlLib::Query
{

class FromTable : public Query
{
public:
  FromTable(API::ISchema::Id tableId);
  ~FromTable() override;

  FromTable& selectAll();
  FromTable& select(const std::vector<API::ISchema::Id>& columnIds);

  FromTable& joinAll(API::ISchema::Id relationshipId);
  FromTable& joinColumns(API::ISchema::Id relationshipId, const std::vector<API::ISchema::Id>& columnIds);

  FromTable& where(Expr& expr);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, QueryResults& previousQueryResults) override;
  QueryResults getQueryResults(API::ISchema& schema, QSqlQuery& query) const override;

private:
  struct ColumnInfo
  {
    API::ISchema::Id columnId = 0U;
    int indexInQuery = -1;
  };

  struct ColumnSelectionInfo
  {
    API::ISchema::Id tableId = 0U;
    QString tableAlias;
  
    std::vector<ColumnInfo> columnInfos;
    bool bColumnsSelected = false;
    std::vector<int> primaryKeyColumnIndicesInQuery;
    std::vector<int> foreignKeyColumnIndicesInQuery;
  };

  struct TableAliasColumnId
  {
    QString tableAlias;
    API::ISchema::TableColumnId tableColumnId;
  };

  ColumnSelectionInfo m_columnSelectionInfo;
  std::map<API::ISchema::Id, ColumnSelectionInfo> m_joins;

  std::vector<TableAliasColumnId> m_allSelectedColumns;

  std::unique_ptr<Expr> m_whereExpr;
  bool m_isTableAliasesNeeded;

  void throwIfMultipleSelects() const;
  void throwIfMultipleJoins(API::ISchema::Id relationshipId) const;

  void verifyJoinsAndCheckAliasesNeeded(API::ISchema& schema);
  void generateTableAliases();

  void addToSelectedColumns(const API::ISchema& schema, const API::ISchema::Table& table,
                            ColumnSelectionInfo& columnSelectionInfo);
  void addForeignKeyColumns(const API::ISchema::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColumnIdMap,
                            std::vector<int>& foreignKeyColumnIndicesInQuery,
                            API::ISchema::Id childTableId, const QString& childTableAlias);

  QString processJoinsAndCreateQuerySubstring(API::ISchema& schema, const API::ISchema::Table& table);

  static std::vector<ColumnInfo> getAllTableColumnIds(const API::ISchema::Table& table);

  QString createSelectString(API::ISchema& schema, const std::vector<TableAliasColumnId>& tableColumnIds) const;
  void appendJoinQuerySubstring(QString& joinStrOut, API::ISchema& schema, API::ISchema::Id relationshipId,
                                API::ISchema::Id parentTableId, const QString& parentTableAlias,
                                API::ISchema::Id childTableId, const QString& childTableAlias,
                                const API::ISchema::Table& joinTable, const QString& joinTableAlias,
                                const API::ISchema::RelationshipToForeignKeyReferencesMap& foreignKeyReferences,
                                int foreignKeyReferencesIndex,
                                std::vector<int>& foreignKeyColumnIndicesInQuery);

};

}
