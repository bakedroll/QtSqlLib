#include "QtSqlLib/Query/FromTable.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib::Query
{

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

    auto i = 0;
    auto cmp = 0;
    for (; i<m_values.size(); ++i)
    {
      cmp = m_values[i].compare(rhs.m_values[i]);
      if (cmp == 0)
      {
        continue;
      }
      else
      {
        break;
      }
    }

    return cmp < 0;
  }

private:
  std::vector<QVariant> m_values;

};

static NTuple getKeyTuple(const QSqlQuery& query, const std::vector<int>& keyIndices)
{
  std::vector<QVariant> keys(keyIndices.size());
  for (auto i=0; i<keyIndices.size(); ++i)
  {
    keys[i] = query.value(keyIndices[i]);
  }
  return NTuple(keys);
}

static QString createSelectString(Schema& schema, const std::vector<Schema::TableColumnId>& tableColumnIds)
{
  const auto& tables = schema.getTables();

  QString selectColsStr = "";
  for (const auto& id : tableColumnIds)
  {
    if (!selectColsStr.isEmpty())
    {
      selectColsStr.append(", ");
    }

    const auto& table = tables.at(id.tableId);
    selectColsStr.append(QString("'%1'.'%2'").arg(table.name).arg(table.columns.at(id.columnId).name));
  }

  return selectColsStr;
}

static void appendJoinQuerySubstring(QString& joinStrOut, Schema& schema, Schema::Id relationshipId, Schema::Id parentTableId,
                                     const Schema::Table& joinTable, Schema::Id childTableId,
                                     const std::map<Schema::RelationshipTableId, Schema::ForeignKeyReference>& foreignKeyReferences)
{
  if (foreignKeyReferences.count({ relationshipId, parentTableId }) == 0)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Foreign keys configuration seems to be corrupted.");
  }

  Expr joinOnExpr;

  const auto& foreignKeyReference = foreignKeyReferences.at({ relationshipId, parentTableId });
  for (const auto& idMapping : foreignKeyReference.primaryForeignKeyColIdMap)
  {
    if (idMapping.first != foreignKeyReference.primaryForeignKeyColIdMap.cbegin()->first)
    {
      joinOnExpr.and();
    }
    joinOnExpr.equal({{ parentTableId, idMapping.first.columnId }}, Expr::ColumnId({ childTableId, idMapping.second }));
  }

  joinStrOut.append(QString(" INNER JOIN '%1' ON %2")
    .arg(joinTable.name)
    .arg(joinOnExpr.toQString(schema)));
}

FromTable::FromTable(Schema::Id tableId)
{
  m_columnSelectionInfo.tableId = tableId;
}

FromTable::~FromTable() = default;

FromTable& FromTable::selectAll()
{
  throwIfMultipleSelects();

  // empty m_columnInfo implies all column ids
  m_columnSelectionInfo.bColumnsSelected = true;
  return *this;
}

FromTable& FromTable::select(Schema::Id columnId)
{
  throwIfMultipleSelects();

  m_columnSelectionInfo.columnInfos.emplace_back(ColumnInfo { columnId, -1 });

  if (!m_columnSelectionInfo.bIsSelecting)
  {
    m_columnSelectionInfo.bColumnsSelected = true;
  }
  return *this;
}

FromTable& FromTable::joinAll(Schema::Id relationshipId)
{
  throwIfMultipleJoins(relationshipId);

  // empty JoinData::m_columnInfo implies all column ids
  m_joins[relationshipId].bColumnsSelected = true;

  return *this;
}

FromTable& FromTable::joinColumns(Schema::Id relationshipId, Schema::Id columnId)
{
  throwIfMultipleJoins(relationshipId);

  auto& joinData = m_joins[relationshipId];
  joinData.columnInfos.emplace_back(ColumnInfo{ columnId, -1 });

  if (!joinData.bIsSelecting)
  {
    joinData.bColumnsSelected = true;
  }
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

API::IQuery::SqlQuery FromTable::getSqlQuery(Schema& schema, QueryResults& previousQueryResults)
{
  schema.throwIfTableIdNotExisting(m_columnSelectionInfo.tableId);
  const auto& table = schema.getTables().at(m_columnSelectionInfo.tableId);

  if (m_columnSelectionInfo.columnInfos.empty())
  {
    m_columnSelectionInfo.columnInfos = getAllTableColumnIds(table);
  }

  addToSelectedColumns(schema, table, m_columnSelectionInfo);
  const auto joinStr = processJoinsAndCreateQuerySubstring(schema, table);
  const auto selectColsStr = createSelectString(schema, m_allSelectedColumns);

  QString queryStr;
  queryStr.append(QString("SELECT %1 FROM '%2'").arg(selectColsStr).arg(table.name));
  queryStr.append(joinStr);

  if (m_whereExpr)
  {
    queryStr.append(QString(" WHERE %1").arg(m_whereExpr->toQString(schema, m_columnSelectionInfo.tableId)));
  }

  queryStr.append(";");

  if (!m_joins.empty())
  {
    printf("");
  }

  return { QSqlQuery(queryStr) };
}

API::IQuery::QueryResults FromTable::getQueryResults(Schema& schema, QSqlQuery& query) const
{
  using JoinKey = std::pair<Schema::Id, NTuple>;

  std::set<NTuple> retrievedResultKeys;
  std::map<JoinKey, std::set<NTuple>> retrievedRelationResultKeys;

  QueryResults::ResultTuples resultTuples;
  std::map<NTuple, int> resultTupleIndices;

  while (query.next())
  {
    const auto keyTuple = getKeyTuple(query, m_columnSelectionInfo.keyColumnIndicesInQuery);

    if (retrievedResultKeys.count(keyTuple) == 0)
    {
      retrievedResultKeys.emplace(keyTuple);

      ResultTuple tuple;
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

      const auto joinKeyTuple = getKeyTuple(query, join.second.keyColumnIndicesInQuery);
      auto& relationResultKeys = retrievedRelationResultKeys[{ relationshipId, keyTuple }];

      if (relationResultKeys.count(joinKeyTuple) == 0)
      {
        relationResultKeys.emplace(joinKeyTuple);

        Schema::TupleValues values;
        for (auto& info : join.second.columnInfos)
        {
          values[{ join.second.tableId, info.columnId }] = query.value(info.indexInQuery);
        }

        joinedTuples.emplace_back(values);
      }
    }
  }

  if (!m_joins.empty())
  {
    printf("");
  }

  return { QueryResults::Validity::Valid, resultTuples };
}

void FromTable::throwIfMultipleSelects() const
{
  if (m_columnSelectionInfo.bColumnsSelected)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "select() or selectAll() should only be called once.");
  }
}

void FromTable::throwIfMultipleJoins(Schema::Id relationshipId) const
{
  if (m_joins.count(relationshipId) > 0 && m_joins.at(relationshipId).bColumnsSelected)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "joinColumns() or joinAll() should only be called once.");
  }
}

void FromTable::addToSelectedColumns(const Schema& schema, const Schema::Table& table,
                                     ColumnSelectionInfo& columnSelectionInfo)
{
  for (auto& info : columnSelectionInfo.columnInfos)
  {
    schema.throwIfColumnIdNotExisting(table, info.columnId);

    const auto indexInQuery = static_cast<int>(m_allSelectedColumns.size());
    info.indexInQuery = indexInQuery;
    if (table.primaryKeys.count(info.columnId) > 0)
    {
      columnSelectionInfo.keyColumnIndicesInQuery.emplace_back(indexInQuery);
    }

    m_allSelectedColumns.emplace_back(Schema::TableColumnId{ columnSelectionInfo.tableId, info.columnId });
  }

  for (const auto& keyColumnId : table.primaryKeys)
  {
    if (std::count_if(columnSelectionInfo.columnInfos.begin(), columnSelectionInfo.columnInfos.end(),
      [&keyColumnId](const ColumnInfo& info) { return info.columnId == keyColumnId; }) == 0)
    {
      Schema::TableColumnId keyId{ columnSelectionInfo.tableId, keyColumnId };

      columnSelectionInfo.keyColumnIndicesInQuery.emplace_back(static_cast<int>(m_allSelectedColumns.size()));

      m_allSelectedColumns.emplace_back(keyId);
      m_extraSelectedColumns.insert(keyId);
    }
  }
}

QString FromTable::processJoinsAndCreateQuerySubstring(Schema& schema, const Schema::Table& table)
{
  QString joinStr = "";

  for (auto& join : m_joins)
  {
    const auto relationshipId = join.first;

    schema.throwIfRelationshipIsNotExisting(relationshipId);
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

    schema.throwIfTableIdNotExisting(join.second.tableId);
    const auto& joinTable = schema.getTables().at(join.second.tableId);

    if (join.second.columnInfos.empty())
    {
      join.second.columnInfos = getAllTableColumnIds(joinTable);
    }

    addToSelectedColumns(schema, joinTable, join.second);

    if (relationship.type == Schema::RelationshipType::ManyToMany)
    {
      const auto parentFromTableId = m_columnSelectionInfo.tableId;
      const auto linkTableId = schema.getManyToManyLinkTableId(relationshipId);
      const auto parentToTableId = join.second.tableId;

      schema.throwIfTableIdNotExisting(linkTableId);
      const auto& linkTable = schema.getTables().at(linkTableId);

      const auto& foreignKeyReferences = linkTable.relationshipToForeignKeyReferencesMap;

      appendJoinQuerySubstring(joinStr, schema, relationshipId, parentFromTableId, linkTable, linkTableId, foreignKeyReferences);
      appendJoinQuerySubstring(joinStr, schema, relationshipId, parentToTableId, joinTable, linkTableId, foreignKeyReferences);
      // TODO

      printf("bla");
    }
    else
    {
      const auto needToSwapParentChild = (relationship.type == Schema::RelationshipType::OneToMany
        && relationship.tableFromId == m_columnSelectionInfo.tableId);

      const auto& foreignKeyReferences = (needToSwapParentChild
        ? joinTable.relationshipToForeignKeyReferencesMap
        : table.relationshipToForeignKeyReferencesMap);

      auto parentTableId = join.second.tableId;
      auto childTableId = m_columnSelectionInfo.tableId;
      if (needToSwapParentChild)
      {
        std::swap(parentTableId, childTableId);
      }

      appendJoinQuerySubstring(joinStr, schema, relationshipId, parentTableId, joinTable, childTableId, foreignKeyReferences);
    }
  }

  return joinStr;
}

std::vector<FromTable::ColumnInfo> FromTable::getAllTableColumnIds(const Schema::Table& table)
{
  std::vector<ColumnInfo> columnInfos;
  for (const auto& column : table.columns)
  {
    columnInfos.emplace_back(ColumnInfo{ column.first, -1 });
  }
  return columnInfos;
}

}
