#include "QtSqlLib/InsertInto.h"

#include "QtSqlLib/DatabaseException.h"

#include <QSqlDriver>

namespace QtSqlLib
{
InsertInto::InsertInto(Schema::Id tableId)
  : QuerySequence()
{
  addQuery(std::make_unique<QueryInsert>(tableId));
}

InsertInto::~InsertInto() = default;

InsertInto& InsertInto::value(Schema::Id columnId, const QVariant& value)
{
  auto& queryInsert = dynamic_cast<QueryInsert&>(getQuery(0));

  queryInsert.addColumnId(columnId);
  queryInsert.values().emplace_back(value);
  return *this;
}

InsertInto& InsertInto::relatedEntity(Schema::Id relationshipId, const QueryDefines::ColumnResultMap& entryIdsMap)
{
  auto& queryInsert = dynamic_cast<QueryInsert&>(getQuery(0));

  if (queryInsert.relatedEntities().count(relationshipId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("More than one related entity of same relationship with id %1 specified.").arg(relationshipId));
  }

  queryInsert.relatedEntities()[relationshipId] = entryIdsMap;
  return *this;
}

InsertInto& InsertInto::returnIds()
{
  if (getNumQueries() > 1)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, 
      "returnId() can only be called once per query.");
  }

  auto& queryInsert = dynamic_cast<QueryInsert&>(getQuery(0));

  addQuery(std::make_unique<QueryInsertedIds>(queryInsert.getTableId()));
  return *this;
}

InsertInto::QueryInsert::QueryInsert(Schema::Id tableId)
  : BaseInsert(tableId)
{
}

InsertInto::QueryInsert::~QueryInsert() = default;

QueryDefines::SqlQuery InsertInto::QueryInsert::getSqlQuery(Schema& schema)
{
  schema.throwIfTableIdNotExisting(getTableId());

  const auto& table = schema.getTables().at(getTableId());

  std::vector<QVariant> foreignKeyValues;

  const auto& relationships = schema.getRelationships();
  for (const auto& relatedEntity : m_relatedEntities)
  {
    if (relationships.count(relatedEntity.first) == 0)
    {
      throw DatabaseException(DatabaseException::Type::QueryError, 
        QString("No relationship found with with id %1.").arg(relatedEntity.first));
    }

    const auto& relationship = relationships.at(relatedEntity.first);

    if ((relationship.type == Schema::RelationshipType::ManyToMany) ||
      ((relationship.type == Schema::RelationshipType::OneToMany) && (relationship.tableFromId == getTableId())) ||
      ((relationship.type == Schema::RelationshipType::ManyToOne) && (relationship.tableToId == getTableId())))
    {
      throw DatabaseException(DatabaseException::Type::QueryError, 
        QString("Direct related entity insertion not alowed for relation with id %1 to table with id %2.")
        .arg(relatedEntity.first).arg(getTableId()));
    }

    const auto parentTableId = (relationship.type == Schema::RelationshipType::OneToMany ? relationship.tableFromId : relationship.tableToId);
    const auto& parentTable = schema.getTables().at(parentTableId);

    const auto& foreignKeyReferences = table.mapRelationshioToForeignKeyReferences.at(relatedEntity.first);
    for (const auto& parentKeyCol : parentTable.primaryKeys)
    {
      if (relatedEntity.second.count({ parentTableId, parentKeyCol }) == 0)
      {
        throw DatabaseException(DatabaseException::Type::QueryError,
          QString("Missing related entity key %1.").arg(parentKeyCol));
      }

      addColumnId(foreignKeyReferences.mapReferenceParentColIdToChildColId.at(parentKeyCol));
      foreignKeyValues.emplace_back(relatedEntity.second.at({ parentTableId, parentKeyCol }));
    }
  }

  QSqlQuery query;
  const auto queryString = getSqlQueryString(schema);

  query.prepare(queryString);

  for (const auto& value : m_values)
  {
    query.addBindValue(value);
  }
  for (const auto& value : foreignKeyValues)
  {
    query.addBindValue(value);
  }

  return { query, QueryDefines::QueryMode::Single };
}

std::vector<QVariant>& InsertInto::QueryInsert::values()
{
  return m_values;
}

std::map<Schema::Id, QueryDefines::ColumnResultMap>& InsertInto::QueryInsert::relatedEntities()
{
  return m_relatedEntities;
}

InsertInto::QueryInsertedIds::QueryInsertedIds(Schema::Id tableId)
  : IQuery()
  , m_tableId(tableId)
{
}

InsertInto::QueryInsertedIds::~QueryInsertedIds() = default;

QueryDefines::SqlQuery InsertInto::QueryInsertedIds::getSqlQuery(Schema& schema)
{
  schema.throwIfTableIdNotExisting(m_tableId);
  const auto& table = schema.getTables().at(m_tableId);

  QString keyColumns;
  for (const auto& primaryKey : table.primaryKeys)
  {
    keyColumns += QString("'%1'.'%2', ").arg(table.name).arg(table.columns.at(primaryKey).name);
  }
  keyColumns = keyColumns.left(keyColumns.length() - 2);

  return { QSqlQuery(QString("SELECT rowid, %1 FROM '%2' WHERE rowid = last_insert_rowid();")
    .arg(keyColumns).arg(table.name)) };
}

QueryDefines::QueryResults InsertInto::QueryInsertedIds::getQueryResults(Schema& schema, QSqlQuery& query) const
{
  const auto& table = schema.getTables().at(m_tableId);

  if (!query.next())
  {
    throw DatabaseException(DatabaseException::Type::QueryError, 
      QString("Could not query last inserted id from table '%1'.").arg(table.name));
  }

  QueryDefines::ColumnResultMap resultsMap;

  auto currentValue = 1;
  for (const auto& primaryKey : table.primaryKeys)
  {
    resultsMap[{ m_tableId, primaryKey }] = query.value(currentValue);
    currentValue++;
  }

  return { resultsMap };
}

}
