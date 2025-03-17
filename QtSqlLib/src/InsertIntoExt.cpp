#include "QtSqlLib/Query/InsertIntoExt.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/API/ISchema.h"
#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/ID.h"
#include "QtSqlLib/Query/LinkTuples.h"

#include "InsertIntoReferences.h"
#include "QueryInsertedIDs.h"

namespace QtSqlLib::Query
{

InsertIntoExt::InsertIntoExt(const API::IID& tableId)
  : QuerySequence()
  , m_tableId(tableId.get())
  , m_bIsReturningInsertedIds(false)
{
}

InsertIntoExt::~InsertIntoExt() = default;

InsertIntoExt& InsertIntoExt::value(const API::IID& columnId, const QVariant& value)
{
  getOrCreateInsertQuery()->value(columnId, value);
  return *this;
}

InsertIntoExt& InsertIntoExt::linkToOneTuple(const API::IID& relationshipId, const API::TupleValues& tupleKeyValues)
{
  throwIdLinkedTupleAlreadyExisting(relationshipId.get());

  m_linkedTuplesMap[relationshipId.get()] = { LinkType::ToOne, { tupleKeyValues } };
  return *this;
}

InsertIntoExt& InsertIntoExt::linkToManyTuples(const API::IID& relationshipId,
                                               const std::vector<API::TupleValues>& tupleKeyValuesList)
{
  throwIdLinkedTupleAlreadyExisting(relationshipId.get());

  m_linkedTuplesMap[relationshipId.get()] = { LinkType::ToMany, tupleKeyValuesList };
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

void InsertIntoExt::prepare(API::ISchema& schema)
{
  getOrCreateInsertQuery();

  schema.getSanityChecker().throwIfTableIdNotExisting(m_tableId);

  const auto& table = schema.getTables().at(m_tableId);

  std::set<API::IID::Type> specialInsertionRelationshipIds;

  const auto& relationships = schema.getRelationships();
  for (const auto& linkedTuples : m_linkedTuplesMap)
  {
    const auto relationshipId = linkedTuples.first;

    schema.getSanityChecker().throwIfRelationshipIsNotExisting(relationshipId);
    const auto& relationship = relationships.at(relationshipId);

    if ((relationship.tableFromId != m_tableId) && (relationship.tableToId != m_tableId))
    {
      throw DatabaseException(DatabaseException::Type::InvalidId,
        "Invalid relationship ID.");
    }

    if (linkedTuples.second.linkType == LinkType::ToOne)
    {
      schema.verifyOneToOneRelationshipPrimaryKeysAndGetTableIds(relationshipId, {}, linkedTuples.second.linkedPrimaryKeys.at(0));
    }
    else
    {
      schema.verifyOneToManyRelationshipPrimaryKeysAndGetTableIds(relationshipId, {}, linkedTuples.second.linkedPrimaryKeys);
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
    addQuery(std::make_unique<QueryInsertedIDs>(m_tableId));
  }

  addLinkTuplesQueriesForRelationshipIds(specialInsertionRelationshipIds);
}

void InsertIntoExt::throwIdLinkedTupleAlreadyExisting(API::IID::Type relationshipId) const
{
  if (m_linkedTuplesMap.count(relationshipId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("More than one linked tuple of same relationship with id %1 specified.").arg(relationshipId));
  }
}

std::unique_ptr<InsertIntoReferences>& InsertIntoExt::getOrCreateInsertQuery()
{
  if (!m_insertQuery)
  {
    m_insertQuery = std::make_unique<InsertIntoReferences>(m_tableId);
  }
  return m_insertQuery;
}

bool InsertIntoExt::isSeparateLinkTuplesQueryNeeded(const API::Relationship& relationship) const
{
  return ((relationship.type == API::RelationshipType::ManyToMany) ||
    ((relationship.type == API::RelationshipType::OneToMany) && (relationship.tableFromId == m_tableId)) ||
    ((relationship.type == API::RelationshipType::ManyToOne) && (relationship.tableToId == m_tableId)));
}

void InsertIntoExt::addUpdateForeignKeyColumnsToInsertIntoQuery(API::ISchema& schema, API::IID::Type relationshipId,
                                                                const API::Relationship& relationship,
                                                                const API::Table& childTable,
                                                                const LinkedTuples& linkedTuples) const
{
  const auto parentTableId = (relationship.type == API::RelationshipType::OneToMany ? relationship.tableFromId : relationship.tableToId);
  const auto& parentTable = schema.getTables().at(parentTableId);
  const auto& linkedPrimaryKeys = linkedTuples.linkedPrimaryKeys.at(0);

  const auto& foreignKeyReferences = childTable.relationshipToForeignKeyReferencesMap.at({ relationshipId, parentTableId });
  if (foreignKeyReferences.size() != 1)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError,
      "Foreign key references table seems to be corrupted.");
  }

  for (const auto& parentKeyCol : parentTable.primaryKeys)
  {
    if (linkedPrimaryKeys.count({ parentTableId, parentKeyCol }) == 0)
    {
      throw DatabaseException(DatabaseException::Type::QueryError,
        QString("Missing primary key of tuple hat should be linked ('%1').").arg(parentTable.columns.at(parentKeyCol).name));
    }

    m_insertQuery->addColumn(ID(foreignKeyReferences[0].primaryForeignKeyColIdMap.at({ parentTableId, parentKeyCol })));
    m_insertQuery->addForeignKeyValue(linkedPrimaryKeys.at({ parentTableId, parentKeyCol }));
  }
}

void InsertIntoExt::addLinkTuplesQueriesForRelationshipIds(const std::set<API::IID::Type>& relationshipIds)
{
  for (const auto& relationshipId : relationshipIds)
  {
    auto& linkedTuples = m_linkedTuplesMap.at(relationshipId);

    auto linkTupleQuery = std::make_unique<LinkTuples>(ID(relationshipId));
    linkTupleQuery->fromRemainingKey();

    (linkedTuples.linkType == LinkType::ToOne
      ? linkTupleQuery->toOne(linkedTuples.linkedPrimaryKeys.at(0))
      : linkTupleQuery->toMany(linkedTuples.linkedPrimaryKeys));

    addQuery(std::move(linkTupleQuery));
  }
}

}
