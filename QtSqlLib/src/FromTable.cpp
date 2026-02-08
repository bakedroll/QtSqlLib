#include "QtSqlLib/Query/FromTable.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/API/ISchema.h"
#include "QtSqlLib/ColumnStatistics.h"
#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/Expr.h"
#include "QtSqlLib/ID.h"

#include <QVariant>

namespace QtSqlLib::Query
{

static ColumnHelper::ColumnData makeColumnData(const std::optional<API::IID::Type>& relationshipId, API::IID::Type columnId)
{
  ColumnHelper::ColumnData data;
  data.relationshipId = relationshipId;
  data.columnId = columnId;
  return data;
}

static API::IID::Type relationshipIdForLink(API::IID::Type relationshipId)
{
  constexpr auto idBits = sizeof(API::IID::Type) * 8;
  return relationshipId | (0x1 << (idBits - 1));
}

static bool contains(const ColumnHelper::SelectColumnList& columns, API::IID::Type value)
{
  return std::find_if(columns.cbegin(), columns.cend(),
    [&value](const ColumnHelper::SelectColumn& col) { return col.columnId == value; }) != columns.cend();
}

static ColumnHelper::SelectColumnList getAllTableColumnIds(const API::Table& table)
{
  ColumnHelper::SelectColumnList columns(table.columns.size());
  size_t i=0;
  for (const auto& column : table.columns)
  {
    columns[i++] = column.first;
  }
  return columns;
}

static void prepareQueryMetaInfoColumns(API::QueryMetaInfo& queryMetaInfo, const API::Table& table)
{
  if (queryMetaInfo.columns.empty())
  {
    queryMetaInfo.columns = getAllTableColumnIds(table);
  }
  queryMetaInfo.columnQueryIndices.resize(queryMetaInfo.columns.size(), 0);
}

FromTable::FromTable(const API::IID& tableId) :
  m_hasColumnsSelected(false),
  m_isTableAliasesNeeded(false),
  m_isGroupByCaseInsensitive(false),
  m_isOrderByCaseInsensitive(false)
{
  m_queryMetaInfo.tableId = tableId.get();
}

FromTable::~FromTable() = default;

FromTable& FromTable::selectAll()
{
  throwIfMultipleSelects();
  m_hasColumnsSelected = true;

  // empty m_columnInfo implies all column ids
  return *this;
}

FromTable& FromTable::select(const ColumnHelper::SelectColumnList& columns)
{
  throwIfMultipleSelects();

  if (columns.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "At least one column must be selected");
  }

  m_queryMetaInfo.columns = columns;
  m_hasColumnsSelected = true;

  return *this;
}

FromTable& FromTable::joinAll(const API::IID& relationshipId)
{
  throwIfMultipleJoins(relationshipId.get());

  // empty JoinData::m_columnInfo implies all column ids
  m_joins.emplace_back(API::QueryMetaInfo { -1, relationshipId.get(), {}, {}, {} });

  return *this;
}

FromTable& FromTable::join(const API::IID& relationshipId, const ColumnHelper::SelectColumnList& columns)
{
  throwIfMultipleJoins(relationshipId.get());

  if (columns.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "At least one column must be selected");
  }

  m_joins.emplace_back(API::QueryMetaInfo { -1, relationshipId.get(), columns, {}, {} });

  return *this;
}

FromTable& FromTable::where(Expr& expr)
{
  if (m_whereExpr)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "where() should only be called once.");
  }

  m_whereExpr = std::make_unique<Expr>(std::move(expr));
  return *this;
}

FromTable& FromTable::having(Expr& expr)
{
  if (m_havingExpr)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "having() should only be called once.");
  }

  m_havingExpr = std::make_unique<Expr>(std::move(expr));
  return *this;
}

FromTable& FromTable::groupBy(const ColumnHelper::GroupColumnList& columns, bool caseInsensitive)
{
  if (!m_groupColumns.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "groupBy() should only be called once.");
  }

  m_groupColumns = columns;
  m_isGroupByCaseInsensitive = caseInsensitive;
  return *this;
}

FromTable& FromTable::orderBy(const ColumnHelper::OrderColumnList& columns, bool caseInsensitive)
{
  if (!m_orderColumns.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "orderBy() should only be called once.");
  }

  m_orderColumns = columns;
  m_isOrderByCaseInsensitive = caseInsensitive;
  return *this;
}

API::IQuery::SqlQuery FromTable::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, ResultSet& /*previousQueryResults*/)
{
  if (!m_hasColumnsSelected)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "Missing SELECT statement at FromTable query.");
  }

  schema.getSanityChecker().throwIfTableIdNotExisting(m_queryMetaInfo.tableId);
  const auto& table = schema.getTables().at(m_queryMetaInfo.tableId);

  verifyJoinsAndCheckAliasesNeeded(schema);
  generateQueryIdentifiers(schema);

  prepareQueryMetaInfoColumns(m_queryMetaInfo, table);
  addToSelectedColumns(m_queryMetaInfo, table);

  std::vector<QVariant> boundValues;
  const auto joinStr = processJoinsAndCreateQuerySubstring(schema, boundValues, table);
  const auto selectColsStr = createSelectString(schema);

  QString queryStr;
  queryStr.append(QString("SELECT %1 FROM '%2'").arg(selectColsStr).arg(table.name));
  if (m_isTableAliasesNeeded)
  {
    queryStr.append(QString(" AS '%1'").arg(m_queryIdentifiers.resolveTableIdentifier(schema)));
  }

  queryStr.append(joinStr);

  if (m_whereExpr)
  {
    queryStr.append(QString(" WHERE %1").arg(m_whereExpr->toQueryString(schema, m_queryIdentifiers, boundValues)));
  }

  if (!m_groupColumns.empty())
  {
    queryStr.append(QString(" GROUP BY %1").arg(createGroupByString(schema)));
    if (m_isGroupByCaseInsensitive)
    {
      queryStr.append(" COLLATE NOCASE");
    }
  }

  if (m_havingExpr)
  {
    queryStr.append(QString(" HAVING %1").arg(m_havingExpr->toQueryString(schema, m_queryIdentifiers, boundValues)));
  }

  if (!m_orderColumns.empty())
  {
    queryStr.append(QString(" ORDER BY %1").arg(createOrderByString(schema)));
    if (m_isOrderByCaseInsensitive)
    {
      queryStr.append(" COLLATE NOCASE");
    }
  }

  queryStr.append(";");

  QSqlQuery query(db);
  query.prepare(queryStr);
  for (const auto& value : boundValues)
  {
    query.addBindValue(value);
  }

  return { std::move(query) };
}

ResultSet FromTable::getQueryResults(API::ISchema& /*schema*/, QSqlQuery&& query)
{
  return ResultSet(std::move(query), std::move(m_queryMetaInfo), std::move(m_joins));
}

void FromTable::throwIfMultipleSelects() const
{
  if (m_hasColumnsSelected)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "select() or selectAll() should only be called once.");
  }
}

void FromTable::throwIfMultipleJoins(API::IID::Type relationshipId) const
{
  for (const auto& join : m_joins)
  {
    if (join.relationshipId.value() == relationshipId)
    {
      throw DatabaseException(DatabaseException::Type::InvalidSyntax,
        "join() or joinAll() should only be called once per relationship id.");
    }
  }
}

void FromTable::verifyJoinsAndCheckAliasesNeeded(API::ISchema& schema)
{
  std::set<API::IID::Type> joinTableIds;
  joinTableIds.insert(m_queryMetaInfo.tableId);

  for (auto& join : m_joins)
  {
    const auto relationshipId = join.relationshipId.value();

    schema.getSanityChecker().throwIfRelationshipIsNotExisting(relationshipId);
    const auto& relationship = schema.getRelationships().at(relationshipId);

    if (m_queryMetaInfo.tableId == relationship.tableFromId)
    {
      join.tableId = relationship.tableToId;
    }
    else if (m_queryMetaInfo.tableId == relationship.tableToId)
    {
      join.tableId = relationship.tableFromId;
    }
    else
    {
      throw DatabaseException(DatabaseException::Type::InvalidId,
        QString("Invalid relationship id %1 for join with table with id %2.")
        .arg(relationshipId)
        .arg(m_queryMetaInfo.tableId));
    }

    schema.getSanityChecker().throwIfTableIdNotExisting(join.tableId);
    if (joinTableIds.count(join.tableId) == 0)
    {
      joinTableIds.insert(join.tableId);
    }
    else
    {
      m_isTableAliasesNeeded = true;
    }
  }
}

void FromTable::generateQueryIdentifiers(API::ISchema& schema)
{
  auto tableAliasIndex = 0;
  const auto getNextTableAlias = [&tableAliasIndex]() -> QString
  {
    QString tableAliasBody = "t_alias_%1";
    return tableAliasBody.arg(tableAliasIndex++);
  };

  m_queryIdentifiers.addTableIdentifier(std::nullopt, m_queryMetaInfo.tableId,
    m_isTableAliasesNeeded ? std::make_optional<QString>(getNextTableAlias()) : std::nullopt);

  for (auto& join : m_joins)
  {
    m_queryIdentifiers.addTableIdentifier(join.relationshipId, join.tableId,
      m_isTableAliasesNeeded ? std::make_optional<QString>(getNextTableAlias()) : std::nullopt);

    const auto relationshipId = join.relationshipId.value();
    auto& relationship = schema.getRelationships().at(relationshipId);
    if (relationship.type == API::RelationshipType::ManyToMany)
    {
      const auto linkTableId = schema.getManyToManyLinkTableId(relationshipId);
      schema.getSanityChecker().throwIfTableIdNotExisting(linkTableId);

      m_queryIdentifiers.addTableIdentifier(relationshipIdForLink(relationshipId), linkTableId);
    }
  }
}

void FromTable::addToSelectedColumns(API::QueryMetaInfo& queryMetaInfo, const API::Table& table)
{
  for (size_t i=0; i<queryMetaInfo.columns.size(); ++i)
  {
    const auto& selectColumn = queryMetaInfo.columns.at(i);
    const auto columnId = selectColumn.columnId;
    const auto& alias = selectColumn.alias;
    if (contains(table.primaryKeys, columnId))
    {
      queryMetaInfo.primaryKeyColumnIndices.emplace_back(i);
    }

    const auto indexInQuery = m_compiledColumnSelection.size();
    queryMetaInfo.columnQueryIndices[i] = indexInQuery;

    m_compiledColumnSelection.emplace_back(SelectColumnData{ makeColumnData(queryMetaInfo.relationshipId, columnId), alias });
  }

  for (const auto& keyColumn : table.primaryKeys)
  {
    if (std::count_if(queryMetaInfo.columns.cbegin(), queryMetaInfo.columns.cend(),
      [&keyColumn](const ColumnHelper::SelectColumn& col) { return col.columnId == keyColumn.columnId; }) == 0)
    {
      const auto primeryKeyColumnIndex = queryMetaInfo.columns.size();
      const auto indexInQuery = m_compiledColumnSelection.size();

      queryMetaInfo.primaryKeyColumnIndices.emplace_back(primeryKeyColumnIndex);
      queryMetaInfo.columns.emplace_back(ColumnHelper::SelectColumn{ keyColumn.columnId });
      queryMetaInfo.columnQueryIndices.emplace_back(indexInQuery);

      m_compiledColumnSelection.emplace_back(SelectColumnData{ makeColumnData(queryMetaInfo.relationshipId, keyColumn.columnId), "" });
    }
  }
}

void FromTable::addForeignKeyColumns(
  const std::optional<API::IID::Type>& foreignKeyRelationshipId,
  const API::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColumnIdMap)
{
  for (const auto& foreignKey : primaryForeignKeyColumnIdMap)
  {
    m_compiledColumnSelection.emplace_back(SelectColumnData{ makeColumnData(foreignKeyRelationshipId, foreignKey.second), "" });
  }
}

QString FromTable::processJoinsAndCreateQuerySubstring(
  API::ISchema& schema,
  std::vector<QVariant>& boundValues,
  const API::Table& table)
{
  QString joinStr = "";
  for (auto& join : m_joins)
  {
    const auto relationshipId = join.relationshipId.value();
    const auto& relationship = schema.getRelationships().at(relationshipId);
    const auto& joinTable = schema.getTables().at(join.tableId);
    const QString joinTableAlias = m_isTableAliasesNeeded ? m_queryIdentifiers.resolveTableIdentifier(schema, relationshipId) : "";

    prepareQueryMetaInfoColumns(join, joinTable);
    addToSelectedColumns(join, joinTable);

    if (relationship.type == API::RelationshipType::ManyToMany)
    {
      const auto parentFromTableId = m_queryMetaInfo.tableId;
      const auto linkTableId = schema.getManyToManyLinkTableId(relationshipId);
      const auto& linkTable = schema.getTables().at(linkTableId);

      const auto parentToTableId = join.tableId;
      const auto& foreignKeyReferences = linkTable.relationshipToForeignKeyReferencesMap;
      const auto secondForeignKeyRefIndex = (parentFromTableId == parentToTableId ? 1 : 0);

      appendJoinQuerySubstring(
        joinStr, schema, linkTable, relationshipId, relationshipIdForLink(relationshipId),
        std::nullopt, relationshipIdForLink(relationshipId),
        foreignKeyReferences, 0, true, boundValues);

      appendJoinQuerySubstring(
        joinStr, schema, joinTable, relationshipId, relationshipIdForLink(relationshipId),
        relationshipId, relationshipIdForLink(relationshipId),
        foreignKeyReferences, secondForeignKeyRefIndex, false, boundValues);
    }
    else
    {
      const auto needToSwapParentChild = (
        (relationship.type == API::RelationshipType::OneToMany && relationship.tableFromId == m_queryMetaInfo.tableId) ||
        (relationship.type == API::RelationshipType::ManyToOne && relationship.tableToId == m_queryMetaInfo.tableId));

      const auto& foreignKeyReferences = (needToSwapParentChild
        ? joinTable.relationshipToForeignKeyReferencesMap
        : table.relationshipToForeignKeyReferencesMap);

      std::optional<API::IID::Type> foreignKeyRelationshipId = std::nullopt;
      std::optional<API::IID::Type> relationshipIdParentTable = join.relationshipId;
      std::optional<API::IID::Type> relationshipIdChildTable = std::nullopt;

      if (needToSwapParentChild)
      {
        foreignKeyRelationshipId = relationshipId;
        std::swap(relationshipIdParentTable, relationshipIdChildTable);
      }

      appendJoinQuerySubstring(
        joinStr, schema, joinTable, relationshipId, foreignKeyRelationshipId,
        relationshipIdParentTable, relationshipIdChildTable,
        foreignKeyReferences, 0, false, boundValues);
    }
  }

  return joinStr;
}

QString FromTable::createSelectString(API::ISchema& schema) const
{
  QString selectColsStr = "";
  for (const auto& selectedColumn : m_compiledColumnSelection)
  {
    if (!selectColsStr.isEmpty())
    {
      selectColsStr.append(", ");
    }

    selectColsStr.append(m_queryIdentifiers.resolveColumnIdentifier(schema, selectedColumn.columnData));
    if (!selectedColumn.alias.isEmpty())
    {

      selectColsStr.append(QString(" AS [%1]").arg(selectedColumn.alias));
    }
  }

  return selectColsStr;
}

QString FromTable::createGroupByString(API::ISchema& schema) const
{
  QString groupByStr = "";
  for (const auto& groupCol : m_groupColumns)
  {
    if (!groupByStr.isEmpty())
    {
      groupByStr.append(", ");
    }
    groupByStr.append(m_queryIdentifiers.resolveColumnIdentifier(schema, groupCol.data));
  }

  return groupByStr;
}

QString FromTable::createOrderByString(API::ISchema& schema) const
{
  QString orderByStr = "";
  for (const auto& orderCol : m_orderColumns)
  {
    if (!orderByStr.isEmpty())
    {
      orderByStr.append(", ");
    }
    orderByStr.append(QString("%1 %2")
      .arg(m_queryIdentifiers.resolveColumnIdentifier(schema, orderCol.data))
      .arg(orderCol.order == ColumnHelper::EOrder::Ascending ? "ASC" : "DESC"));
  }

  return orderByStr;
}

void FromTable::appendJoinQuerySubstring(
  QString& joinStrOut, API::ISchema& schema, const API::Table& joinTable,
  API::IID::Type relationshipId, const std::optional<API::IID::Type>& foreignKeyRelationshipId,
  const std::optional<API::IID::Type>& relationshipIdFromTable, const std::optional<API::IID::Type>& relationshipIdToTable,
  const API::RelationshipToForeignKeyReferencesMap& foreignKeyReferences,
  int foreignKeyReferencesIndex,
  bool noJoinAlias,
  std::vector<QVariant>& boundValues)
{
  const auto parentTableId = m_queryIdentifiers.tableId(relationshipIdFromTable);
  if (foreignKeyReferences.count({ relationshipId, parentTableId }) == 0)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Foreign keys configuration seems to be corrupted.");
  }

  Expr joinOnExpr;

  const auto& foreignKeyReference = foreignKeyReferences.at({ relationshipId, parentTableId });
  if (static_cast<int>(foreignKeyReference.size()) <= foreignKeyReferencesIndex)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Foreign keys configuration seems to be corrupted.");
  }

  const auto& foreignKeyRef = foreignKeyReference[foreignKeyReferencesIndex];
  addForeignKeyColumns(foreignKeyRelationshipId, foreignKeyRef.primaryForeignKeyColIdMap);

  for (const auto& idMapping : foreignKeyRef.primaryForeignKeyColIdMap)
  {
    if (idMapping.first != foreignKeyRef.primaryForeignKeyColIdMap.cbegin()->first)
    {
      joinOnExpr.opAnd();
    }

    joinOnExpr.equal(
      makeColumnData(relationshipIdFromTable, idMapping.first),
      makeColumnData(relationshipIdToTable, idMapping.second));
  }

  joinStrOut.append(QString(" LEFT JOIN '%1'").arg(joinTable.name));

  if (m_isTableAliasesNeeded && !noJoinAlias)
  {
    joinStrOut.append(QString(" AS '%1'").arg(m_queryIdentifiers.resolveTableIdentifier(schema, relationshipId)));
  }

  joinStrOut.append(QString(" ON %1").arg(joinOnExpr.toQueryString(schema, m_queryIdentifiers, boundValues)));
}

}
