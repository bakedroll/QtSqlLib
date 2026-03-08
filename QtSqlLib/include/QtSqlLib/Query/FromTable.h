#pragma once

#include <QtSqlLib/Query/Query.h>

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/SchemaTypes.h>
#include <QtSqlLib/ColumnHelper.h>
#include <QtSqlLib/ConcatenatedColumn.h>
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

  FromTable& bidirectional();

  FromTable& selectAttributes(const ColumnHelper::ColumnList& attributes);

  FromTable& where(Expr& expr);
  FromTable& having(Expr& expr);

  FromTable& groupBy(const ColumnHelper::GroupColumnList& columnIds, bool caseInsensitive = false);
  FromTable& orderBy(const ColumnHelper::OrderColumnList& columnIds, bool caseInsensitive = false);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, ResultSet& previousQueryResults) override;
  ResultSet getQueryResults(API::ISchema& schema, QSqlQuery&& query) override;

private:
  struct SelectColumnData
  {
    QVariant column;
    QString alias;
  };

  bool m_hasColumnsSelected;
  bool m_isTableAliasesNeeded;

  API::QueryMetaInfo m_queryMetaInfo;
  std::vector<API::QueryMetaInfo> m_joins;

  QueryIdentifiers m_queryIdentifiers;
  std::vector<SelectColumnData> m_compiledColumnSelection;
  std::vector<API::IID::Type> m_bidirectionalRelationshipIds;

  std::unique_ptr<Expr> m_whereExpr;
  std::unique_ptr<Expr> m_havingExpr;

  ColumnHelper::GroupColumnList m_groupColumns;
  ColumnHelper::OrderColumnList m_orderColumns;

  bool m_isGroupByCaseInsensitive;
  bool m_isOrderByCaseInsensitive;

  void throwIfMultipleSelects() const;
  void throwIfMultipleJoins(API::IID::Type relationshipId) const;
  void throwIfMultipleSelectAttributes() const;

  void verifyJoinsAndCheckAliasesNeeded(API::ISchema& schema);
  void generateQueryIdentifiers(API::ISchema& schema);

  void addToSelectedColumns(API::QueryMetaInfo& queryMetaInfo, const API::Table& table);

  QString processJoinsAndCreateQuerySubstring(
    API::ISchema& schema,
    std::vector<QVariant>& boundValues,
    const API::Table& table);

  QString createSelectString(API::ISchema& schema) const;
  QString createGroupByString(API::ISchema& schema) const;
  QString createOrderByString(API::ISchema& schema) const;

  Expr createJoinExpression(
    const std::optional<API::IID::Type>& relationshipIdFromTable,
    const std::optional<API::IID::Type>& relationshipIdToTable,
    const API::ForeignKeyReference& foreignKeyReference) const;

  Expr createUnequalIdsExpression(
    const std::optional<API::IID::Type>& relationshipIdFromTable,
    const std::optional<API::IID::Type>& relationshipIdToTable,
    const API::ForeignKeyReference& foreignKeyReference) const;

  Expr createEqualForeignKeysExpression(
    const std::optional<API::IID::Type>& relationshipIdLinkTable,
    const API::ForeignKeyReference& foreignKeyReferenceLeft,
    const API::ForeignKeyReference& foreignKeyReferenceRight) const;

  void appendJoinQuerySubstring(
    QString& joinStrOut, API::ISchema& schema, const API::Table& joinTable, API::IID::Type relationshipId,
    const std::optional<API::IID::Type>& relationshipIdFromTable, const std::optional<API::IID::Type>& relationshipIdToTable,
    const API::RelationshipToForeignKeyReferencesMap& foreignKeyReferences,
    int foreignKeyReferencesIndex,
    bool isLinkTableJoin,
    std::vector<QVariant>& boundValues);

};

}
