#include "QtSqlLib/Query/FromTable.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib::Query
{

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

FromTable::FromTable(Schema::Id tableId)
  : m_tableId(tableId)
  , m_bColumnsSelected(false)
  , m_bIsSelecting(false)
{
}

FromTable::~FromTable() = default;

FromTable& FromTable::selectAll()
{
  throwIfMultipleSelects();

  // empty m_columnInfo implies all column ids
  m_bColumnsSelected = true;
  return *this;
}

FromTable& FromTable::select(Schema::Id columnId)
{
  throwIfMultipleSelects();

  m_columnInfo.emplace_back(SelectColumnInfo { columnId, -1 });

  if (!m_bIsSelecting)
  {
    m_bColumnsSelected = true;
  }
  return *this;
}

FromTable& FromTable::joinAll(Schema::Id relationshipId)
{
  throwIfMultipleJoins(relationshipId);

  // empty JoinData::m_columnInfo implies all column ids
  m_joins[relationshipId].bJoined = true;

  return *this;
}

FromTable& FromTable::joinColumns(Schema::Id relationshipId, Schema::Id columnId)
{
  throwIfMultipleJoins(relationshipId);

  auto& joinData = m_joins[relationshipId];
  joinData.columnInfo.emplace_back(SelectColumnInfo{ columnId, -1 });

  if (!joinData.bIsJoining)
  {
    joinData.bJoined = true;
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
  schema.throwIfTableIdNotExisting(m_tableId);
  const auto& table = schema.getTables().at(m_tableId);

  if (m_columnInfo.empty())
  {
    m_columnInfo = getAllTableColumnIds(table);
  }

  addToSelectedColumns(schema, table, m_tableId, m_columnInfo);
  const auto joinStr = processJoinsAndCreateQuerySubstring(schema, table);
  const auto selectColsStr = createSelectString(schema, m_allSelectedColumns);

  QString queryStr;
  queryStr.append(QString("SELECT %1 FROM '%2'").arg(selectColsStr).arg(table.name));
  queryStr.append(joinStr);

  if (m_whereExpr)
  {
    queryStr.append(QString(" WHERE %1").arg(m_whereExpr->toQString(schema, m_tableId)));
  }

  queryStr.append(";");

  return { QSqlQuery(queryStr) };
}

API::IQuery::QueryResults FromTable::getQueryResults(Schema& schema, QSqlQuery& query) const
{
  QueryResults::Values values;

  while (query.next())
  {
    Schema::TupleValues resultValues;



    auto i = 0;
    for (const auto& info : m_columnInfo)
    {
      resultValues[{ m_tableId, info.columnId }] = query.value(info.indexInQuery);
      i++;
    }

    values.emplace_back(resultValues);
  }

  if (!m_joins.empty())
  {
    printf("");
  }

  return { QueryResults::Validity::Valid, values };
}

void FromTable::throwIfMultipleSelects() const
{
  if (m_bColumnsSelected)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "select() or selectAll() should only be called once.");
  }
}

void FromTable::throwIfMultipleJoins(Schema::Id relationshipId) const
{
  if (m_joins.count(relationshipId) > 0 && m_joins.at(relationshipId).bJoined)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "joinColumns() or joinAll() should only be called once.");
  }
}

void FromTable::addToSelectedColumns(const Schema& schema, const Schema::Table& table, Schema::Id tableId,
                                     std::vector<SelectColumnInfo>& columnInfos)
{
  for (auto& info : columnInfos)
  {
    schema.throwIfColumnIdNotExisting(table, info.columnId);

    const auto indexInQuery = static_cast<int>(m_allSelectedColumns.size());
    info.indexInQuery = indexInQuery;
    if (table.primaryKeys.count(info.columnId) > 0)
    {
      m_keyColumnIndicesInQuery[{ tableId, info.columnId }] = indexInQuery;
    }

    m_allSelectedColumns.emplace_back(Schema::TableColumnId{ tableId, info.columnId });
  }

  for (const auto& keyColumnId : table.primaryKeys)
  {
    if (std::count_if(columnInfos.begin(), columnInfos.end(),
      [&keyColumnId](const SelectColumnInfo& info) { return info.columnId == keyColumnId; }) == 0)
    {
      Schema::TableColumnId keyId{ tableId, keyColumnId };

      m_keyColumnIndicesInQuery[{ tableId, keyColumnId }] = static_cast<int>(m_allSelectedColumns.size());

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

    Schema::Id joinTableId = 0U;
    if (m_tableId == relationship.tableFromId)
    {
      joinTableId = relationship.tableToId;
    }
    else if (m_tableId == relationship.tableToId)
    {
      joinTableId = relationship.tableFromId;
    }
    else
    {
      throw DatabaseException(DatabaseException::Type::InvalidId,
        QString("Invalid relationship id %1 for join with table with id %2.").arg(relationshipId).arg(m_tableId));
    }

    schema.throwIfTableIdNotExisting(joinTableId);
    const auto& joinTable = schema.getTables().at(joinTableId);

    if (join.second.columnInfo.empty())
    {
      join.second.columnInfo = getAllTableColumnIds(joinTable);
    }

    addToSelectedColumns(schema, joinTable, joinTableId, join.second.columnInfo);

    if (relationship.type == Schema::RelationshipType::ManyToMany)
    {
      // TODO
    }
    else
    {
      const auto& foreignKeyReferences = (relationship.type == Schema::RelationshipType::OneToMany
        ? joinTable.relationshipToForeignKeyReferencesMap
        : table.relationshipToForeignKeyReferencesMap);

      auto parentTableId = joinTableId;
      auto childTableId = m_tableId;
      if (relationship.type == Schema::RelationshipType::OneToMany)
      {
        std::swap(parentTableId, childTableId);
      }

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

      joinStr.append(QString(" INNER JOIN '%1' ON %2")
        .arg(joinTable.name)
        .arg(joinOnExpr.toQString(schema)));
    }
  }

  return joinStr;
}

std::vector<FromTable::SelectColumnInfo> FromTable::getAllTableColumnIds(const Schema::Table& table)
{
  std::vector<SelectColumnInfo> columnInfos;
  for (const auto& column : table.columns)
  {
    columnInfos.emplace_back(SelectColumnInfo{ column.first, -1 });
  }
  return columnInfos;
}

}
