#include "QtSqlLib/Query/InsertIntoExt.h"

#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/Query/InsertInto.h"
#include "QtSqlLib/Query/LinkTuples.h"

namespace QtSqlLib::Query
{

InsertIntoExt::InsertIntoExt(Schema::Id tableId)
  : QuerySequence()
  , m_tableId(tableId)
  , m_bIsReturningInsertedIds(false)
{
}

InsertIntoExt::~InsertIntoExt() = default;

InsertIntoExt& InsertIntoExt::value(Schema::Id columnId, const QVariant& value)
{
  getOrCreateInsertQuery()->value(columnId, value);
  return *this;
}

InsertIntoExt& InsertIntoExt::linkToOneTuple(Schema::Id relationshipId, const Schema::TableColumnValuesMap& tupleIdsMap)
{
  throwIdLinkedTupleAlreadyExisting(relationshipId);
  
  m_linkedTuplesMap[relationshipId] = { LinkType::ToOne, { tupleIdsMap } };
  return *this;
}

InsertIntoExt& InsertIntoExt::linkToManyTuples(Schema::Id relationshipId,
                                               const std::vector<Schema::TableColumnValuesMap>& tupleIdsMapList)
{
  throwIdLinkedTupleAlreadyExisting(relationshipId);

  m_linkedTuplesMap[relationshipId] = { LinkType::ToMany, tupleIdsMapList };
  return *this;
}

InsertIntoExt& InsertIntoExt::returnIds()
{
  if (m_bIsReturningInsertedIds)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, 
      "returnId() can only be called once per query.");
  }

  m_bIsReturningInsertedIds = true;
  return *this;
}

void InsertIntoExt::prepare(Schema& schema)
{
  getOrCreateInsertQuery();

  schema.throwIfTableIdNotExisting(m_tableId);

  const auto& table = schema.getTables().at(m_tableId);

  std::set<Schema::Id> specialInsertionRelationshipIds;

  const auto& relationships = schema.getRelationships();
  for (const auto& linkedTuples : m_linkedTuplesMap)
  {
    const auto relationshipId = linkedTuples.first;

    schema.throwIfRelationshipIsNotExisting(relationshipId);
    const auto& relationship = relationships.at(relationshipId);

    if ((relationship.tableFromId != m_tableId) && (relationship.tableToId != m_tableId))
    {
      throw DatabaseException(DatabaseException::Type::InvalidId,
        "Invalid relationship ID.");
    }

    if (linkedTuples.second.linkType == LinkType::ToOne)
    {
      schema.validateOneToOneRelationshipPrimaryKeysAndGetTableIds(relationshipId, {}, linkedTuples.second.linkedPrimaryKeys.at(0));
    }
    else
    {
      schema.validateOneToManyRelationshipPrimaryKeysAndGetTableIds(relationshipId, {}, linkedTuples.second.linkedPrimaryKeys);
    }

    if ((linkedTuples.second.linkType == LinkType::ToMany) ||
      isSeparateLinkTuplesQueryNeeded(relationship))
    {
      specialInsertionRelationshipIds.insert(relationshipId);
      continue;
    }

    addUpdateForeignKeyColumnsToInsertIntoQuery(schema, relationshipId, relationship, table, linkedTuples.second);
  }

  addQuery(std::move(m_insertQuery));

  if (m_bIsReturningInsertedIds)
  {
    addQuery(std::make_unique<QueryInsertedIds>(m_tableId));
  }

  addLinkTuplesQueriesForRelationshipIds(specialInsertionRelationshipIds);
}

InsertIntoExt::InsertIntoReferences::InsertIntoReferences(Schema::Id tableId)
  : InsertInto(tableId)
{
}

InsertIntoExt::InsertIntoReferences::~InsertIntoReferences() = default;

void InsertIntoExt::InsertIntoReferences::addForeignKeyValue(const QVariant& value)
{
  m_foreignKeyValues.emplace_back(value);
}

void InsertIntoExt::InsertIntoReferences::bindQueryValues(QSqlQuery& query) const
{
  InsertInto::bindQueryValues(query);

  for (const auto& value : m_foreignKeyValues)
  {
    query.addBindValue(value);
  }
}

InsertIntoExt::QueryInsertedIds::QueryInsertedIds(Schema::Id tableId)
  : Query()
  , m_tableId(tableId)
{
}

InsertIntoExt::QueryInsertedIds::~QueryInsertedIds() = default;

API::IQuery::SqlQuery InsertIntoExt::QueryInsertedIds::getSqlQuery(Schema& schema, QueryResults& previousQueryResults)
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

API::IQuery::QueryResults InsertIntoExt::QueryInsertedIds::getQueryResults(Schema& schema, QSqlQuery& query) const
{
  const auto& table = schema.getTables().at(m_tableId);

  if (!query.next())
  {
    throw DatabaseException(DatabaseException::Type::QueryError, 
      QString("Could not query last inserted id from table '%1'.").arg(table.name));
  }

  Schema::TableColumnValuesMap resultsMap;

  auto currentValue = 1;
  for (const auto& primaryKey : table.primaryKeys)
  {
    resultsMap[{ m_tableId, primaryKey }] = query.value(currentValue);
    currentValue++;
  }

  return { QueryResults::Validity::Valid, { resultsMap } };
}

void InsertIntoExt::throwIdLinkedTupleAlreadyExisting(Schema::Id relationshipId) const
{
  if (m_linkedTuplesMap.count(relationshipId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("More than one linked tuple of same relationship with id %1 specified.").arg(relationshipId));
  }
}

std::unique_ptr<InsertIntoExt::InsertIntoReferences>& InsertIntoExt::getOrCreateInsertQuery()
{
  if (!m_insertQuery)
  {
    m_insertQuery = std::make_unique<InsertIntoReferences>(m_tableId);
  }
  return m_insertQuery;
}

bool InsertIntoExt::isSeparateLinkTuplesQueryNeeded(const Schema::Relationship& relationship) const
{
  return ((relationship.type == Schema::RelationshipType::ManyToMany) ||
    ((relationship.type == Schema::RelationshipType::OneToMany) && (relationship.tableFromId == m_tableId)) ||
    ((relationship.type == Schema::RelationshipType::ManyToOne) && (relationship.tableToId == m_tableId)));
}

void InsertIntoExt::addUpdateForeignKeyColumnsToInsertIntoQuery(Schema& schema, Schema::Id relationshipId,
                                                                const Schema::Relationship& relationship,
                                                                const Schema::Table& childTable,
                                                                const LinkedTuples& linkedTuples) const
{
  const auto parentTableId = (relationship.type == Schema::RelationshipType::OneToMany ? relationship.tableFromId : relationship.tableToId);
  const auto& parentTable = schema.getTables().at(parentTableId);
  const auto& linkedPrimaryKeys = linkedTuples.linkedPrimaryKeys.at(0);

  const auto& foreignKeyReferences = childTable.mapRelationshipToForeignKeyReferences.at({ relationshipId, parentTableId });
  for (const auto& parentKeyCol : parentTable.primaryKeys)
  {
    if (linkedPrimaryKeys.count({ parentTableId, parentKeyCol }) == 0)
    {
      throw DatabaseException(DatabaseException::Type::QueryError,
        QString("Missing primary key of tuple hat should be linked ('%1').").arg(parentTable.columns.at(parentKeyCol).name));
    }

    m_insertQuery->addColumnId(foreignKeyReferences.primaryForeignKeyColIdMap.at({ parentTableId, parentKeyCol }));
    m_insertQuery->addForeignKeyValue(linkedPrimaryKeys.at({ parentTableId, parentKeyCol }));
  }
}

void InsertIntoExt::addLinkTuplesQueriesForRelationshipIds(const std::set<Schema::Id>& relationshipIds)
{
  for (const auto& relationshipId : relationshipIds)
  {
    auto& linkedTuples = m_linkedTuplesMap.at(relationshipId);

    auto linkTupleQuery = std::make_unique<LinkTuples>(relationshipId);
    linkTupleQuery->fromRemainingKey();

    (linkedTuples.linkType == LinkType::ToOne
      ? linkTupleQuery->toOne(linkedTuples.linkedPrimaryKeys.at(0))
      : linkTupleQuery->toMany(linkedTuples.linkedPrimaryKeys));

    addQuery(std::move(linkTupleQuery));
  }
}

}
