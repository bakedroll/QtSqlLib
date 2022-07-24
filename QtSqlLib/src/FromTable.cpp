#include "QtSqlLib/Query/FromTable.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib::Query
{

std::vector<Schema::Id> getAllTableColumnIds(const Schema::Table& table)
{
  std::vector<Schema::Id> columnIds;
  for (const auto& column : table.columns)
  {
    columnIds.emplace_back(column.first);
  }
  return columnIds;
}

QString createSelectString(const Schema& schema, const Schema::Table& table, const std::vector<Schema::Id>& columnIds)
{
  QString selectColsStr = "";
  for (const auto& col : columnIds)
  {
    schema.throwIfColumnIdNotExisting(table, col);

    if (!selectColsStr.isEmpty())
    {
      selectColsStr.append(", ");
    }
    selectColsStr.append(QString("'%1'.'%2'").arg(table.name).arg(table.columns.at(col).name));
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

  // empty m_columnIds implies all column ids
  m_bColumnsSelected = true;
  return *this;
}

FromTable& FromTable::select(Schema::Id columnId)
{
  throwIfMultipleSelects();

  m_columnIds.emplace_back(columnId);

  if (!m_bIsSelecting)
  {
    m_bColumnsSelected = true;
  }
  return *this;
}

FromTable& FromTable::joinAll(Schema::Id relationshipId)
{
  throwIfMultipleJoins(relationshipId);

  // empty JoinData::m_columnIds implies all column ids
  m_joins[relationshipId].bJoined = true;

  return *this;
}

FromTable& FromTable::joinColumns(Schema::Id relationshipId, Schema::Id columnId)
{
  throwIfMultipleJoins(relationshipId);

  auto& joinData = m_joins[relationshipId];
  joinData.columnIds.emplace_back(columnId);

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

  auto selectAll = m_columnIds.empty();
  for (const auto& join : m_joins)
  {
    selectAll &= join.second.columnIds.empty();
  }

  QString selectColsStr = "";
  if (selectAll)
  {
    selectColsStr = "*";
  }
  else
  {
    selectColsStr.append(createSelectString(schema, table,
      m_columnIds.empty() ? getAllTableColumnIds(table) : m_columnIds));

    for (const auto& join : m_joins)
    {
      schema.throwIfRelationshipIsNotExisting(join.first);
      const auto& relationship = schema.getRelationships().at(join.first);

      auto joinTableId = 0U;
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
          QString("Invalid relationship id %1 for join with table with id %2.").arg(join.first).arg(m_tableId));
      }

      schema.throwIfTableIdNotExisting(joinTableId);
      const auto& joinTable = schema.getTables().at(joinTableId);

      selectColsStr.append(", ");
      selectColsStr.append(createSelectString(schema, joinTable,
        join.second.columnIds.empty() ? getAllTableColumnIds(joinTable) : join.second.columnIds));
    }
  }

  QString queryStr;
  queryStr.append(QString("SELECT %1 FROM '%2'").arg(selectColsStr).arg(table.name));

  if (!m_joins.empty())
  {
    printf("bla");
  }

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
    for (const auto& col : m_columnIds)
    {
      resultValues[{ m_tableId, col }] = query.value(i);
      i++;
    }

    values.emplace_back(resultValues);
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

}
