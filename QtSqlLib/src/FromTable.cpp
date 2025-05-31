#include "QtSqlLib/Query/FromTable.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/API/ISchema.h"
#include "QtSqlLib/ColumnID.h"
#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/Expr.h"
#include "QtSqlLib/ID.h"

#include <QVariant>

namespace QtSqlLib::Query
{

static bool contains(const std::vector<API::IID::Type>& ids, API::IID::Type value)
{
  return std::find(ids.cbegin(), ids.cend(), value) != ids.cend();
}

static ColumnList getAllTableColumnIds(const API::Table& table)
{
  ColumnList columns;
  columns.data().resize(table.columns.size());
  size_t i=0;
  for (const auto& column : table.columns)
  {
    columns.data()[i++] = column.first;
  }
  return columns;
}

static void prepareQueryMetaInfoColumns(API::QueryMetaInfo& queryMetaInfo, const API::Table& table)
{
  if (queryMetaInfo.columns.cdata().empty())
  {
    queryMetaInfo.columns = getAllTableColumnIds(table);
  }
  queryMetaInfo.columnQueryIndices.resize(queryMetaInfo.columns.cdata().size(), 0);
}

FromTable::FromTable(const API::IID& tableId) :
  m_hasColumnsSelected(false),
  m_isTableAliasesNeeded(false)
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

FromTable& FromTable::select(const ColumnList& columns)
{
  throwIfMultipleSelects();

  if (columns.cdata().empty())
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

FromTable& FromTable::join(const API::IID& relationshipId, const ColumnList& columns)
{
  throwIfMultipleJoins(relationshipId.get());

  if (columns.cdata().empty())
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

// TODO: optimize by preventing duplicate column selections
API::IQuery::SqlQuery FromTable::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, ResultSet& /*previousQueryResults*/)
{
  if (!m_hasColumnsSelected)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "Missing SELECT statement at FromTable query.");
  }

  schema.getSanityChecker().throwIfTableIdNotExisting(m_queryMetaInfo.tableId);
  const auto& table = schema.getTables().at(m_queryMetaInfo.tableId);

  verifyJoinsAndCheckAliasesNeeded(schema);
  if (m_isTableAliasesNeeded)
  {
    generateTableAliases();
  }

  prepareQueryMetaInfoColumns(m_queryMetaInfo, table);
  addToSelectedColumns(schema, table, m_queryMetaInfo);

  std::vector<QVariant> boundValues;
  const auto joinStr = processJoinsAndCreateQuerySubstring(schema, boundValues, table);
  const auto selectColsStr = createSelectString(schema);

  QString queryStr;
  queryStr.append(QString("SELECT %1 FROM '%2'").arg(selectColsStr).arg(table.name));
  if (m_isTableAliasesNeeded)
  {
    queryStr.append(QString(" as '%1'").arg(tableAlias()));
  }

  queryStr.append(joinStr);

  if (m_whereExpr)
  {
    ID tid(m_queryMetaInfo.tableId);
    queryStr.append(QString(" WHERE %1").arg(m_whereExpr->toQueryString(schema, boundValues, tid)));
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

void FromTable::generateTableAliases()
{
  auto tableAliasIndex = 0;
  QString tableAliasBody = "t_alias_%1";

  const auto getNextTableAlias = [&tableAliasIndex, &tableAliasBody]() -> QString
  {
    return tableAliasBody.arg(tableAliasIndex++);
  };

  m_aliases.emplace_back(TableAlias { std::nullopt, getNextTableAlias() });
  for (auto& join : m_joins)
  {
    m_aliases.emplace_back(TableAlias { join.relationshipId.value(), getNextTableAlias() });
  }
}

void FromTable::addToSelectedColumns(
  const API::ISchema& schema, const API::Table& table,
  API::QueryMetaInfo& queryMetaInfo)
{
  const auto alias = tableAlias(queryMetaInfo.relationshipId);
  for (size_t i=0; i<queryMetaInfo.columns.cdata().size(); ++i)
  {
    const auto columnId = queryMetaInfo.columns.cdata().at(i);
    schema.getSanityChecker().throwIfColumnIdNotExisting(table, columnId);

    const auto indexInQuery = m_compiledColumnSelection.size();
    queryMetaInfo.columnQueryIndices[i] = indexInQuery;
    if (contains(table.primaryKeys, columnId))
    {
      queryMetaInfo.primaryKeyColumnIndices.emplace_back(i);
    }

    m_compiledColumnSelection.emplace_back(
      SelectedColumn{ alias, queryMetaInfo.tableId, columnId });
  }

  for (const auto& keyColumnId : table.primaryKeys)
  {
    if (std::count_if(queryMetaInfo.columns.cdata().cbegin(), queryMetaInfo.columns.cdata().cend(),
      [&keyColumnId](API::IID::Type colId) { return colId == keyColumnId; }) == 0)
    {
      const auto primeryKeyColumnIndex = queryMetaInfo.columns.cdata().size();
      const auto indexInQuery = m_compiledColumnSelection.size();

      queryMetaInfo.primaryKeyColumnIndices.emplace_back(primeryKeyColumnIndex);
      queryMetaInfo.columns.data().emplace_back(keyColumnId);
      queryMetaInfo.columnQueryIndices.emplace_back(indexInQuery);

      m_compiledColumnSelection.emplace_back(
        SelectedColumn{ alias, queryMetaInfo.tableId, keyColumnId });
    }
  }
}

void FromTable::addForeignKeyColumns(
  const QString& childTableAlias,
  API::IID::Type childTableId,
  const API::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColumnIdMap)
{
  for (const auto& foreignKey : primaryForeignKeyColumnIdMap)
  {
    m_compiledColumnSelection.emplace_back(SelectedColumn { childTableAlias, childTableId, foreignKey.second });
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
    const auto joinTableAlias = tableAlias(relationshipId);

    prepareQueryMetaInfoColumns(join, joinTable);
    addToSelectedColumns(schema, joinTable, join);

    if (relationship.type == API::RelationshipType::ManyToMany)
    {
      const auto parentFromTableId = m_queryMetaInfo.tableId;
      const auto parentFromTableAlias = tableAlias();

      const auto linkTableId = schema.getManyToManyLinkTableId(relationshipId);
      schema.getSanityChecker().throwIfTableIdNotExisting(linkTableId);

      const auto& linkTable = schema.getTables().at(linkTableId);

      const auto parentToTableId = join.tableId;
      const auto parentToTableAlias = tableAlias(join.relationshipId);

      const auto& foreignKeyReferences = linkTable.relationshipToForeignKeyReferencesMap;

      const auto secondForeignKeyRefIndex = (parentFromTableId == parentToTableId ? 1 : 0);

      appendJoinQuerySubstring(
        joinStr, schema, relationshipId, parentFromTableId, parentFromTableAlias,
        linkTableId, "", linkTable, "", foreignKeyReferences, 0, boundValues);
      appendJoinQuerySubstring(
        joinStr, schema, relationshipId, parentToTableId, parentToTableAlias,
        linkTableId, "", joinTable, joinTableAlias, foreignKeyReferences, secondForeignKeyRefIndex, boundValues);
    }
    else
    {
      const auto needToSwapParentChild = (
        (relationship.type == API::RelationshipType::OneToMany && relationship.tableFromId == m_queryMetaInfo.tableId) ||
        (relationship.type == API::RelationshipType::ManyToOne && relationship.tableToId == m_queryMetaInfo.tableId));

      const auto& foreignKeyReferences = (needToSwapParentChild
        ? joinTable.relationshipToForeignKeyReferencesMap
        : table.relationshipToForeignKeyReferencesMap);

      auto* parentTableColSelInfo = &join;
      auto* childTableColSelInfo = &m_queryMetaInfo;
      if (needToSwapParentChild)
      {
        std::swap(parentTableColSelInfo, childTableColSelInfo);
      }

      const auto parentTableAlias = tableAlias(parentTableColSelInfo->relationshipId);
      const auto childTableAlias = tableAlias(childTableColSelInfo->relationshipId);

      appendJoinQuerySubstring(
        joinStr, schema, relationshipId, parentTableColSelInfo->tableId, parentTableAlias,
        childTableColSelInfo->tableId, childTableAlias, joinTable, joinTableAlias, foreignKeyReferences, 0, boundValues);
    }
  }

  return joinStr;
}

QString FromTable::createSelectString(API::ISchema& schema) const
{
  const auto& tables = schema.getTables();

  QString selectColsStr = "";
  for (const auto& selectedColumn : m_compiledColumnSelection)
  {
    if (!selectColsStr.isEmpty())
    {
      selectColsStr.append(", ");
    }

    const auto& table = tables.at(selectedColumn.tableId);
    const auto& alias = selectedColumn.tableAlias;

    const auto tableName = (m_isTableAliasesNeeded && !alias.isEmpty() ? alias : table.name);
    selectColsStr.append(QString("'%1'.'%2'").arg(tableName).arg(table.columns.at(selectedColumn.columnId).name));
  }

  return selectColsStr;
}

void FromTable::appendJoinQuerySubstring(
  QString& joinStrOut, API::ISchema& schema, API::IID::Type relationshipId,
  API::IID::Type parentTableId, const QString& parentTableAlias,
  API::IID::Type childTableId, const QString& childTableAlias,
  const API::Table& joinTable, const QString& joinTableAlias,
  const API::RelationshipToForeignKeyReferencesMap& foreignKeyReferences,
  int foreignKeyReferencesIndex,
  std::vector<QVariant>& boundValues)
{
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
  addForeignKeyColumns(childTableAlias, childTableId, foreignKeyRef.primaryForeignKeyColIdMap);

  for (const auto& idMapping : foreignKeyRef.primaryForeignKeyColIdMap)
  {
    if (idMapping.first != foreignKeyRef.primaryForeignKeyColIdMap.cbegin()->first)
    {
      joinOnExpr.opAnd();
    }

    const auto parentColumnId = (m_isTableAliasesNeeded && !parentTableAlias.isEmpty()
      ? ColumnID(parentTableAlias, ID(parentTableId), ID(idMapping.first))
      : ColumnID(ID(parentTableId),ID(idMapping.first)));

    const auto childColumnId = (m_isTableAliasesNeeded && !childTableAlias.isEmpty()
      ? ColumnID(childTableAlias, ID(childTableId), ID(idMapping.second))
      : ColumnID(ID(childTableId), ID(idMapping.second)));

    joinOnExpr.equal(parentColumnId, QVariant::fromValue(childColumnId));
  }

  joinStrOut.append(QString(" LEFT JOIN '%1'").arg(joinTable.name));

  if (m_isTableAliasesNeeded && !joinTableAlias.isEmpty())
  {
    joinStrOut.append(QString(" AS '%1'").arg(joinTableAlias));
  }

  joinStrOut.append(QString(" ON %1").arg(joinOnExpr.toQueryString(schema, boundValues)));
}

QString FromTable::tableAlias(const std::optional<API::IID::Type> relationshipId) const
{
  for (const auto& alias : m_aliases)
  {
    if (alias.relationshipId == relationshipId)
    {
      return alias.alias;
    }
  }
  return "";
}

}
