#pragma once

#include <QtSqlLib/Query/Query.h>

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/SchemaTypes.h>
#include <QtSqlLib/ColumnList.h>

#include <QString>

#include <map>
#include <memory>
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
  FromTable& select(const ColumnList& columns);

  FromTable& joinAll(const API::IID& relationshipId);
  FromTable& join(const API::IID& relationshipId, const ColumnList& columns);

  FromTable& where(Expr& expr);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, ResultSet& previousQueryResults) override;
  ResultSet getQueryResults(API::ISchema& schema, QSqlQuery&& query) override;

private:
  struct SelectedColumn
  {
    QString tableAlias;
    API::IID::Type tableId = 0;
    API::IID::Type columnId = 0;
  };

  struct TableAlias
  {
    std::optional<API::IID::Type> relationshipId;
    QString alias;
  };

  bool m_hasColumnsSelected;
  bool m_isTableAliasesNeeded;

  API::QueryMetaInfo m_queryMetaInfo;
  std::vector<API::QueryMetaInfo> m_joins;
  std::vector<TableAlias> m_aliases;

  std::vector<SelectedColumn> m_compiledColumnSelection;

  std::unique_ptr<Expr> m_whereExpr;

  void throwIfMultipleSelects() const;
  void throwIfMultipleJoins(API::IID::Type relationshipId) const;

  void verifyJoinsAndCheckAliasesNeeded(API::ISchema& schema);
  void generateTableAliases();

  void addToSelectedColumns(
    const API::ISchema& schema, const API::Table& table,
    API::QueryMetaInfo& queryMetaInfo);
  void addForeignKeyColumns(
    const QString& childTableAlias,
    API::IID::Type childTableId,
    const API::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColumnIdMap);

  QString processJoinsAndCreateQuerySubstring(
    API::ISchema& schema,
    std::vector<QVariant>& boundValues,
    const API::Table& table);

  QString createSelectString(API::ISchema& schema) const;
  void appendJoinQuerySubstring(
    QString& joinStrOut, API::ISchema& schema, API::IID::Type relationshipId,
    API::IID::Type parentTableId, const QString& parentTableAlias,
    API::IID::Type childTableId, const QString& childTableAlias,
    const API::Table& joinTable, const QString& joinTableAlias,
    const API::RelationshipToForeignKeyReferencesMap& foreignKeyReferences,
    int foreignKeyReferencesIndex,
    std::vector<QVariant>& boundValues);

  QString tableAlias(const std::optional<API::IID::Type> relationshipId = std::nullopt) const;

};

}
