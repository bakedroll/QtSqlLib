#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/Query/Query.h>
#include <QtSqlLib/Expr.h>

#include <vector>

#define FROM_TABLE(X) QtSqlLib::Query::FromTable(QtSqlLib::ID(X))

#define SELECT_ALL selectAll()
#define SELECT(X) select(X)

#define JOIN_ALL(X) joinAll(QtSqlLib::ID(X))
#define JOIN(X, Y) join(QtSqlLib::ID(X), Y)

#define WHERE(X) where(QtSqlLib::Expr().X)

namespace QtSqlLib::Query
{

class FromTable : public Query
{
public:
  FromTable(const API::IID& tableId);
  ~FromTable() override;

  FromTable& selectAll();
  FromTable& select(const std::vector<API::IID::Type>& columnIds);

  FromTable& joinAll(const API::IID& relationshipId);
  FromTable& join(const API::IID& relationshipId, const std::vector<API::IID::Type>& columnIds);

  FromTable& where(Expr& expr);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, const ResultSet& previousQueryResults) override;
  ResultSet getQueryResults(API::ISchema& schema, QSqlQuery& query) const override;

private:
  struct ColumnInfo
  {
    API::IID::Type columnId = 0;
    int indexInQuery = -1;
  };

  struct ColumnSelectionInfo
  {
    API::IID::Type tableId = 0;
    QString tableAlias;
  
    std::vector<ColumnInfo> columnInfos;
    bool bColumnsSelected = false;
    std::vector<int> primaryKeyColumnIndicesInQuery;
    std::vector<int> foreignKeyColumnIndicesInQuery;
  };

  struct TableAliasColumnId
  {
    QString tableAlias;
    API::TableColumnId tableColumnId;
  };

  ColumnSelectionInfo m_columnSelectionInfo;
  std::map<API::IID::Type, ColumnSelectionInfo> m_joins;

  std::vector<TableAliasColumnId> m_allSelectedColumns;

  std::unique_ptr<Expr> m_whereExpr;
  bool m_isTableAliasesNeeded;

  void throwIfMultipleSelects() const;
  void throwIfMultipleJoins(API::IID::Type relationshipId) const;

  void verifyJoinsAndCheckAliasesNeeded(API::ISchema& schema);
  void generateTableAliases();

  void addToSelectedColumns(const API::ISchema& schema, const API::Table& table,
                            ColumnSelectionInfo& columnSelectionInfo);
  void addForeignKeyColumns(const API::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColumnIdMap,
                            std::vector<int>& foreignKeyColumnIndicesInQuery,
                            API::IID::Type childTableId, const QString& childTableAlias);

  QString processJoinsAndCreateQuerySubstring(API::ISchema& schema, const API::Table& table);

  static std::vector<ColumnInfo> getAllTableColumnIds(const API::Table& table);

  QString createSelectString(API::ISchema& schema, const std::vector<TableAliasColumnId>& tableColumnIds) const;
  void appendJoinQuerySubstring(QString& joinStrOut, API::ISchema& schema, API::IID::Type relationshipId,
                                API::IID::Type parentTableId, const QString& parentTableAlias,
                                API::IID::Type childTableId, const QString& childTableAlias,
                                const API::Table& joinTable, const QString& joinTableAlias,
                                const API::RelationshipToForeignKeyReferencesMap& foreignKeyReferences,
                                int foreignKeyReferencesIndex,
                                std::vector<int>& foreignKeyColumnIndicesInQuery);

};

}
