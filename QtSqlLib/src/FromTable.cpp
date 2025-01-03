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

static bool qVariantsLess(const QVariant& lhs, const QVariant& rhs)
{
  if (lhs.userType() != rhs.userType())
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Cannot compare tuple values of different types.");
  }

  switch (lhs.userType())
  {
  case QMetaType::Int:
    return lhs.toInt() < rhs.toInt();
  case QMetaType::UInt:
    return lhs.toUInt() < rhs.toUInt();
  case QMetaType::LongLong:
    return lhs.toLongLong() < rhs.toLongLong();
  case QMetaType::ULongLong:
    return lhs.toULongLong() < rhs.toULongLong();
  case QMetaType::QString:
    return lhs.toString() < rhs.toString();
  case QMetaType::QByteArray:
    return lhs.toByteArray() < rhs.toByteArray();
  default:
    break;
  }

  throw DatabaseException(DatabaseException::Type::UnexpectedError, "Tuple types not comparable.");
}

class NTuple
{
public:
  NTuple(const std::vector<QVariant>& values)
    : m_values(values)
  {
  }

  virtual ~NTuple() = default;

  bool operator<(const NTuple& rhs) const
  {
    if (m_values.size() != rhs.m_values.size())
    {
      throw DatabaseException(DatabaseException::Type::UnexpectedError, "Trying to compare incompatible types.");
    }

    bool isLess = false;
    for (size_t i = 0; i<m_values.size(); ++i)
    {
      isLess = qVariantsLess(m_values.at(i), rhs.m_values.at(i));
      if (!isLess)
      {
        continue;
      }
      else
      {
        break;
      }
    }

    return isLess;
  }

  bool isNull() const
  {
    for (const auto& value : m_values)
    {
      if (!value.isNull())
      {
        return false;
      }
    }
    return true;
  }

private:
  std::vector<QVariant> m_values;

};

static NTuple getKeyTuple(const QSqlQuery& query, const std::vector<int>& keyIndices)
{
  std::vector<QVariant> keys(keyIndices.size());
  for (size_t i=0; i<keyIndices.size(); ++i)
  {
    keys[i] = query.value(keyIndices[i]);
  }
  return NTuple(keys);
}

FromTable::FromTable(const API::IID& tableId)
  : m_isTableAliasesNeeded(false)
{
  m_columnSelectionInfo.tableId = tableId.get();
}

FromTable::~FromTable() = default;

FromTable& FromTable::selectAll()
{
  throwIfMultipleSelects();

  // empty m_columnInfo implies all column ids
  m_columnSelectionInfo.bColumnsSelected = true;
  return *this;
}

FromTable& FromTable::select(const std::vector<API::IID::Type>& columnIds)
{
  throwIfMultipleSelects();

  if (columnIds.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "At least one column must be selected");
  }

  for (const auto& id : columnIds)
  {
    m_columnSelectionInfo.columnInfos.emplace_back(ColumnInfo{ id, -1 });
  }

  m_columnSelectionInfo.bColumnsSelected = true;
  return *this;
}

FromTable& FromTable::joinAll(const API::IID& relationshipId)
{
  throwIfMultipleJoins(relationshipId.get());

  // empty JoinData::m_columnInfo implies all column ids
  m_joins[relationshipId.get()].bColumnsSelected = true;

  return *this;
}

FromTable& FromTable::join(const API::IID& relationshipId, const std::vector<API::IID::Type>& columnIds)
{
  throwIfMultipleJoins(relationshipId.get());

  if (columnIds.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "At least one column must be selected");
  }

  auto& joinData = m_joins[relationshipId.get()];
  for (const auto& id : columnIds)
  {
    joinData.columnInfos.emplace_back(ColumnInfo{ id, -1 });
  }

  joinData.bColumnsSelected = true;
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

API::IQuery::SqlQuery FromTable::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, const ResultSet& /*previousQueryResults*/)
{
  schema.getSanityChecker().throwIfTableIdNotExisting(m_columnSelectionInfo.tableId);
  const auto& table = schema.getTables().at(m_columnSelectionInfo.tableId);

  if (m_columnSelectionInfo.columnInfos.empty())
  {
    m_columnSelectionInfo.columnInfos = getAllTableColumnIds(table);
  }

  verifyJoinsAndCheckAliasesNeeded(schema);
  if (m_isTableAliasesNeeded)
  {
    generateTableAliases();
  }

  addToSelectedColumns(schema, table, m_columnSelectionInfo);
  const auto joinStr = processJoinsAndCreateQuerySubstring(schema, table);
  const auto selectColsStr = createSelectString(schema, m_allSelectedColumns);

  QString queryStr;
  queryStr.append(QString("SELECT %1 FROM '%2'").arg(selectColsStr).arg(table.name));
  if (m_isTableAliasesNeeded)
  {
    queryStr.append(QString(" as '%1'").arg(m_columnSelectionInfo.tableAlias));
  }

  queryStr.append(joinStr);

  if (m_whereExpr)
  {
    ID tid(m_columnSelectionInfo.tableId);
    queryStr.append(QString(" WHERE %1").arg(m_whereExpr->toQString(schema, tid)));
  }

  queryStr.append(";");

  return { QSqlQuery(queryStr, db) };
}

ResultSet FromTable::getQueryResults(API::ISchema& /*schema*/, QSqlQuery& query) const
{
  using JoinKey = std::pair<API::IID::Type, NTuple>;

  std::set<NTuple> retrievedResultKeys;
  std::map<JoinKey, std::set<NTuple>> retrievedRelationResultKeys;

  ResultSet::TupleList resultTuples;
  std::map<NTuple, int> resultTupleIndices;

  while (query.next())
  {
    const auto keyTuple = getKeyTuple(query, m_columnSelectionInfo.primaryKeyColumnIndicesInQuery);

    if (retrievedResultKeys.count(keyTuple) == 0)
    {
      retrievedResultKeys.emplace(keyTuple);

      ResultSet::Tuple tuple;
      for (auto& info : m_columnSelectionInfo.columnInfos)
      {
        tuple.values[{ m_columnSelectionInfo.tableId, info.columnId }] = query.value(info.indexInQuery);
      }

      resultTupleIndices[keyTuple] = static_cast<int>(resultTuples.size());
      resultTuples.emplace_back(tuple);
    }

    auto& currentTuple = resultTuples[resultTupleIndices.at(keyTuple)];
    for (const auto& join : m_joins)
    {
      const auto relationshipId = join.first;
      auto& joinedTuples = currentTuple.joinedTuples[relationshipId];

      const auto foreignKeyTuple = getKeyTuple(query, join.second.foreignKeyColumnIndicesInQuery);
      if (foreignKeyTuple.isNull())
      {
        continue;
      }

      const auto joinKeyTuple = getKeyTuple(query, join.second.primaryKeyColumnIndicesInQuery);
      auto& relationResultKeys = retrievedRelationResultKeys[{ relationshipId, keyTuple }];

      if (relationResultKeys.count(joinKeyTuple) == 0)
      {
        relationResultKeys.emplace(joinKeyTuple);

        API::TupleValues values;
        for (auto& info : join.second.columnInfos)
        {
          values[{ join.second.tableId, info.columnId }] = query.value(info.indexInQuery);
        }

        joinedTuples.emplace_back(values);
      }
    }
  }

  return ResultSet::create(resultTuples);
}

void FromTable::throwIfMultipleSelects() const
{
  if (m_columnSelectionInfo.bColumnsSelected)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "select() or selectAll() should only be called once.");
  }
}

void FromTable::throwIfMultipleJoins(API::IID::Type relationshipId) const
{
  if (m_joins.count(relationshipId) > 0 && m_joins.at(relationshipId).bColumnsSelected)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "join() or joinAll() should only be called once.");
  }
}

void FromTable::verifyJoinsAndCheckAliasesNeeded(API::ISchema& schema)
{
  std::set<API::IID::Type> joinTableIds;
  joinTableIds.insert(m_columnSelectionInfo.tableId);

  for (auto& join : m_joins)
  {
    const auto relationshipId = join.first;

    schema.getSanityChecker().throwIfRelationshipIsNotExisting(relationshipId);
    const auto& relationship = schema.getRelationships().at(relationshipId);

    if (m_columnSelectionInfo.tableId == relationship.tableFromId)
    {
      join.second.tableId = relationship.tableToId;
    }
    else if (m_columnSelectionInfo.tableId == relationship.tableToId)
    {
      join.second.tableId = relationship.tableFromId;
    }
    else
    {
      throw DatabaseException(DatabaseException::Type::InvalidId,
        QString("Invalid relationship id %1 for join with table with id %2.")
        .arg(relationshipId)
        .arg(m_columnSelectionInfo.tableId));
    }

    schema.getSanityChecker().throwIfTableIdNotExisting(join.second.tableId);
    if (joinTableIds.count(join.second.tableId) == 0)
    {
      joinTableIds.insert(join.second.tableId);
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

  m_columnSelectionInfo.tableAlias = getNextTableAlias();
  for (auto& join : m_joins)
  {
    join.second.tableAlias = getNextTableAlias();
  }
}

void FromTable::addToSelectedColumns(const API::ISchema& schema, const API::Table& table,
                                     ColumnSelectionInfo& columnSelectionInfo)
{
  for (auto& info : columnSelectionInfo.columnInfos)
  {
    schema.getSanityChecker().throwIfColumnIdNotExisting(table, info.columnId);

    const auto indexInQuery = static_cast<int>(m_allSelectedColumns.size());
    info.indexInQuery = indexInQuery;
    if (table.primaryKeys.count(info.columnId) > 0)
    {
      columnSelectionInfo.primaryKeyColumnIndicesInQuery.emplace_back(indexInQuery);
    }

    m_allSelectedColumns.emplace_back(TableAliasColumnId{ columnSelectionInfo.tableAlias,
      API::TableColumnId{ columnSelectionInfo.tableId, info.columnId } });
  }

  for (const auto& keyColumnId : table.primaryKeys)
  {
    if (std::count_if(columnSelectionInfo.columnInfos.begin(), columnSelectionInfo.columnInfos.end(),
      [&keyColumnId](const ColumnInfo& info) { return info.columnId == keyColumnId; }) == 0)
    {
      const API::TableColumnId keyId{ columnSelectionInfo.tableId, keyColumnId };

      columnSelectionInfo.primaryKeyColumnIndicesInQuery.emplace_back(static_cast<int>(m_allSelectedColumns.size()));

      m_allSelectedColumns.emplace_back(TableAliasColumnId{ columnSelectionInfo.tableAlias, keyId });
    }
  }
}

void FromTable::addForeignKeyColumns(const API::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColumnIdMap,
                                     std::vector<int>& foreignKeyColumnIndicesInQuery,
                                     API::IID::Type childTableId, const QString& childTableAlias)
{
  for (const auto& foreignKey : primaryForeignKeyColumnIdMap)
  {
    foreignKeyColumnIndicesInQuery.emplace_back(static_cast<int>(m_allSelectedColumns.size()));
    m_allSelectedColumns.emplace_back(TableAliasColumnId{ childTableAlias, { childTableId, foreignKey.second } });
  }
}

QString FromTable::processJoinsAndCreateQuerySubstring(API::ISchema& schema, const API::Table& table)
{
  QString joinStr = "";
  for (auto& join : m_joins)
  {
    const auto relationshipId = join.first;
    const auto& relationship = schema.getRelationships().at(relationshipId);
    const auto& joinTable = schema.getTables().at(join.second.tableId);
    const auto& joinTableAlias = join.second.tableAlias;

    if (join.second.columnInfos.empty())
    {
      join.second.columnInfos = getAllTableColumnIds(joinTable);
    }

    addToSelectedColumns(schema, joinTable, join.second);

    if (relationship.type == API::RelationshipType::ManyToMany)
    {
      const auto parentFromTableId = m_columnSelectionInfo.tableId;
      const auto& parentFromTableAlias = m_columnSelectionInfo.tableAlias;

      const auto linkTableId = schema.getManyToManyLinkTableId(relationshipId);
      schema.getSanityChecker().throwIfTableIdNotExisting(linkTableId);

      const auto& linkTable = schema.getTables().at(linkTableId);

      const auto parentToTableId = join.second.tableId;
      const auto& parentToTableAlias = join.second.tableAlias;

      const auto& foreignKeyReferences = linkTable.relationshipToForeignKeyReferencesMap;

      const auto secondForeignKeyRefIndex = (parentFromTableId == parentToTableId ? 1 : 0);

      appendJoinQuerySubstring(joinStr, schema, relationshipId, parentFromTableId, parentFromTableAlias,
                               linkTableId, "", linkTable, "", foreignKeyReferences, 0,
                               join.second.foreignKeyColumnIndicesInQuery);
      appendJoinQuerySubstring(joinStr, schema, relationshipId, parentToTableId, parentToTableAlias,
                               linkTableId, "", joinTable, joinTableAlias, foreignKeyReferences, secondForeignKeyRefIndex,
                               join.second.foreignKeyColumnIndicesInQuery);
    }
    else
    {
      const auto needToSwapParentChild = (
        (relationship.type == API::RelationshipType::OneToMany && relationship.tableFromId == m_columnSelectionInfo.tableId) ||
        (relationship.type == API::RelationshipType::ManyToOne && relationship.tableToId == m_columnSelectionInfo.tableId));

      const auto& foreignKeyReferences = (needToSwapParentChild
        ? joinTable.relationshipToForeignKeyReferencesMap
        : table.relationshipToForeignKeyReferencesMap);

      auto* parentTableColSelInfo = &join.second;
      auto* childTableColSelInfo = &m_columnSelectionInfo;
      if (needToSwapParentChild)
      {
        std::swap(parentTableColSelInfo, childTableColSelInfo);
      }

      appendJoinQuerySubstring(joinStr, schema, relationshipId, parentTableColSelInfo->tableId, parentTableColSelInfo->tableAlias,
                               childTableColSelInfo->tableId, childTableColSelInfo->tableAlias, joinTable, joinTableAlias, foreignKeyReferences, 0,
                               join.second.foreignKeyColumnIndicesInQuery);
    }
  }

  return joinStr;
}

std::vector<FromTable::ColumnInfo> FromTable::getAllTableColumnIds(const API::Table& table)
{
  std::vector<ColumnInfo> columnInfos;
  for (const auto& column : table.columns)
  {
    columnInfos.emplace_back(ColumnInfo{ column.first, -1 });
  }
  return columnInfos;
}

QString FromTable::createSelectString(API::ISchema& schema, const std::vector<TableAliasColumnId>& tableColumnIds) const
{
  const auto& tables = schema.getTables();

  QString selectColsStr = "";
  for (const auto& id : tableColumnIds)
  {
    if (!selectColsStr.isEmpty())
    {
      selectColsStr.append(", ");
    }

    const auto& table = tables.at(id.tableColumnId.tableId);
    const auto tableName = (m_isTableAliasesNeeded && !id.tableAlias.isEmpty() ? id.tableAlias : table.name);

    selectColsStr.append(QString("'%1'.'%2'").arg(tableName).arg(table.columns.at(id.tableColumnId.columnId).name));
  }

  return selectColsStr;
}

void FromTable::appendJoinQuerySubstring(QString& joinStrOut, API::ISchema& schema, API::IID::Type relationshipId,
                                         API::IID::Type parentTableId, const QString& parentTableAlias,
                                         API::IID::Type childTableId, const QString& childTableAlias,
                                         const API::Table& joinTable, const QString& joinTableAlias,
                                         const API::RelationshipToForeignKeyReferencesMap& foreignKeyReferences,
                                         int foreignKeyReferencesIndex,
                                         std::vector<int>& foreignKeyColumnIndicesInQuery)
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

  addForeignKeyColumns(foreignKeyRef.primaryForeignKeyColIdMap, foreignKeyColumnIndicesInQuery,
    childTableId, childTableAlias);

  for (const auto& idMapping : foreignKeyRef.primaryForeignKeyColIdMap)
  {
    if (idMapping.first != foreignKeyRef.primaryForeignKeyColIdMap.cbegin()->first)
    {
      joinOnExpr.opAnd();
    }

    const auto parentColumnId = (m_isTableAliasesNeeded && !parentTableAlias.isEmpty()
      ? ColumnID(parentTableAlias, { parentTableId, idMapping.first.columnId})
      : ColumnID({ parentTableId, idMapping.first.columnId }));

    const auto childColumnId = (m_isTableAliasesNeeded && !childTableAlias.isEmpty()
      ? ColumnID(childTableAlias, { childTableId, idMapping.second})
      : ColumnID({childTableId, idMapping.second}));

    joinOnExpr.equal(parentColumnId, QVariant::fromValue(childColumnId));
  }

  joinStrOut.append(QString(" LEFT JOIN '%1'").arg(joinTable.name));

  if (m_isTableAliasesNeeded && !joinTableAlias.isEmpty())
  {
    joinStrOut.append(QString(" AS '%1'").arg(joinTableAlias));
  }

  joinStrOut.append(QString(" ON %1").arg(joinOnExpr.toQString(schema)));
}

}
