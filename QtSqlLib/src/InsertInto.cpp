#include "QtSqlLib/InsertInto.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{
InsertInto::InsertInto(Schema::Id tableId)
  : BaseInsert(tableId)
  , m_returnIdMode(ReturnIdMode::Undefined)
{
}

InsertInto::~InsertInto() = default;

InsertInto& InsertInto::value(Schema::Id columnId, const QVariant& value)
{
  checkColumnIdExisting(columnId);

  addColumnId(columnId);
  m_values.emplace_back(value);
  return *this;
}

InsertInto& InsertInto::relatedEntity(Schema::Id relationshipId, const ColumnResultMap& entryIdsMap)
{
  if (m_relatedEntities.count(relationshipId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery,
      QString("Invalid insert query: More than one related entity of same relationship with id %1 specified.").arg(relationshipId));
  }

  m_relatedEntities[relationshipId] = entryIdsMap;
  return *this;
}

InsertInto& InsertInto::returnIds()
{
  if (m_returnIdMode != ReturnIdMode::Undefined)
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery, 
      "returnId() can only be called once per query.");
  }

  m_returnIdMode = ReturnIdMode::Yes;
  return *this;
}

QSqlQuery InsertInto::getSqlQuery(Schema& schema)
{
  checkTableExisting(schema);

  const auto& table = schema.getTables().at(getTableId());

  std::vector<QVariant> foreignKeyValues;

  const auto& relationships = schema.getRelationships();
  for (const auto& relatedEntity : m_relatedEntities)
  {
    if (relationships.count(relatedEntity.first) == 0)
    {
      throw DatabaseException(DatabaseException::Type::InvalidQuery, 
        QString("No relationship found with with id %1.").arg(relatedEntity.first));
    }

    const auto& relationship = relationships.at(relatedEntity.first);

    if ((relationship.type == Schema::RelationshipType::ManyToMany) ||
      ((relationship.type == Schema::RelationshipType::OneToMany) && (relationship.tableFromId == getTableId())) ||
      ((relationship.type == Schema::RelationshipType::ManyToOne) && (relationship.tableToId == getTableId())))
    {
      throw DatabaseException(DatabaseException::Type::InvalidQuery, 
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
        throw DatabaseException(DatabaseException::Type::InvalidQuery, 
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

  return query;
}

IQuery::QueryResults InsertInto::getQueryResults(Schema& schema, QSqlQuery& query) const
{
  if (m_returnIdMode != ReturnIdMode::Yes)
  {
    return {};
  }

  const auto& table = schema.getTables().at(getTableId());

  QString keyColumns;
  for (const auto& primaryKey : table.primaryKeys)
  {
    keyColumns += QString("'%1'.'%2', ").arg(table.name).arg(table.columns.at(primaryKey).name);
  }
  keyColumns = keyColumns.left(keyColumns.length() - 2);

  QSqlQuery lastIdQuery(QString("SELECT rowid, %1 FROM '%2' WHERE rowid = last_insert_rowid();")
    .arg(keyColumns).arg(table.name));

  lastIdQuery.exec();
  if (!lastIdQuery.next())
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery, 
      QString("Could not query last inserted id from table '%1'.").arg(table.name));
  }

  ColumnResultMap resultsMap;

  auto currentValue = 1;
  for (const auto& primaryKey : table.primaryKeys)
  {
    resultsMap[{ getTableId(), primaryKey }] = lastIdQuery.value(currentValue);
    currentValue++;
  }

  return { resultsMap };
}

}
