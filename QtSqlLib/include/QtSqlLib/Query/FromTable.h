#pragma once

#include <QtSqlLib/Query/Query.h>

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/SchemaTypes.h>
#include <QtSqlLib/ColumnHelper.h>
#include <QtSqlLib/QueryIdentifiers.h>

#include <QString>

#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace QtSqlLib
{
class Expr;
}

namespace QtSqlLib::Query
{

class FromTable : public Query
{
public:
  FromTable(const API::IID& tableId);
  ~FromTable() override;

  FromTable& selectAll();
  FromTable& select(const ColumnHelper::SelectColumnList& columns);

  FromTable& joinAll(const API::IID& relationshipId);
  FromTable& join(const API::IID& relationshipId, const ColumnHelper::SelectColumnList& columns);

  FromTable& where(Expr& expr);
  FromTable& having(Expr& expr);

  FromTable& groupBy(const ColumnHelper::GroupColumnList& columnIds);
  FromTable& orderBy(const ColumnHelper::OrderColumnList& columnIds);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, ResultSet& previousQueryResults) override;
  ResultSet getQueryResults(API::ISchema& schema, QSqlQuery&& query) override;

private:
  bool m_hasColumnsSelected;
  bool m_isTableAliasesNeeded;

  API::QueryMetaInfo m_queryMetaInfo;
  std::vector<API::QueryMetaInfo> m_joins;

  QueryIdentifiers m_queryIdentifiers;
  std::vector<ColumnHelper::ColumnData> m_compiledColumnSelection;

  std::unique_ptr<Expr> m_whereExpr;
  std::unique_ptr<Expr> m_havingExpr;

  ColumnHelper::GroupColumnList m_groupColumns;
  ColumnHelper::OrderColumnList m_orderColumns;

  void throwIfMultipleSelects() const;
  void throwIfMultipleJoins(API::IID::Type relationshipId) const;

  void verifyJoinsAndCheckAliasesNeeded(API::ISchema& schema);
  void generateQueryIdentifiers(API::ISchema& schema);

  void addToSelectedColumns(API::QueryMetaInfo& queryMetaInfo, const API::Table& table);
  void addForeignKeyColumns(
    const std::optional<API::IID::Type>& foreignKeyRelationshipId,
    const API::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColumnIdMap);

  QString processJoinsAndCreateQuerySubstring(
    API::ISchema& schema,
    std::vector<QVariant>& boundValues,
    const API::Table& table);

  QString createSelectString(API::ISchema& schema) const;
  QString createGroupByString(API::ISchema& schema) const;
  QString createOrderByString(API::ISchema& schema) const;

  void appendJoinQuerySubstring(
    QString& joinStrOut, API::ISchema& schema, const API::Table& joinTable,
    API::IID::Type relationshipId, const std::optional<API::IID::Type>& foreignKeyRelationshipId,
    const std::optional<API::IID::Type>& relationshipIdFromTable, const std::optional<API::IID::Type>& relationshipIdToTable,
    const API::RelationshipToForeignKeyReferencesMap& foreignKeyReferences,
    int foreignKeyReferencesIndex,
    bool noJoinAlias,
    std::vector<QVariant>& boundValues);

};

}
