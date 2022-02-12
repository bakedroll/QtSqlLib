#include "QtSqlLib/LinkTuples.h"

#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/UpdateTable.h"
#include "QtSqlLib/BatchInsertInto.h"

namespace QtSqlLib
{

LinkTuples::LinkTuples(Schema::Id relationshipId)
  : QuerySequence()
  , m_relationshipId(relationshipId)
  , m_expectedCall(ExpectedCall::From)
  , m_type(RelationshipType::ToOne)
{
}

LinkTuples::~LinkTuples() = default;

LinkTuples& LinkTuples::fromOne(const Schema::TableColumnValuesMap& rowIds)
{
  if (m_expectedCall != ExpectedCall::From)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "fromOne() call not expected.");
  }

  m_fromRowIds = rowIds;
  m_expectedCall = ExpectedCall::To;
  return *this;
}

LinkTuples& LinkTuples::toOne(const Schema::TableColumnValuesMap& rowIds)
{
  if (m_expectedCall != ExpectedCall::To)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "toOne() call not expected.");
  }

  m_type = RelationshipType::ToOne;
  m_toRowIdsList.emplace_back(rowIds);
  m_expectedCall = ExpectedCall::Complete;
  return *this;
}

LinkTuples& LinkTuples::toMany(const std::vector<Schema::TableColumnValuesMap>& rowIdsList)
{
  if (m_expectedCall != ExpectedCall::To)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "toMany() call not expected.");
  }

  m_type = RelationshipType::ToMany;
  m_toRowIdsList = rowIdsList;
  m_expectedCall = ExpectedCall::Complete;
  return *this;
}

void LinkTuples::prepare(Schema& schema)
{
  schema.throwIfRelationshipIdNotExisting(m_relationshipId);
  const auto& relationship = schema.getRelationships().at(m_relationshipId);

  const auto linkedTableIds = validateAndGetLinkedTableIds(schema);

  const auto isOneToMany = (relationship.type == Schema::RelationshipType::OneToMany);
  const auto isManyToOne = (relationship.type == Schema::RelationshipType::ManyToOne);

  if (m_type == RelationshipType::ToMany)
  {
    if ((relationship.type != Schema::RelationshipType::ManyToMany) &&
      ((isManyToOne && (relationship.tableFromId == linkedTableIds.tableFromId)) || 
        (isOneToMany && (relationship.tableFromId != linkedTableIds.tableFromId))))
    {
      throw DatabaseException(DatabaseException::Type::InvalidSyntax,
        "toMany() not compatible with relationship.");
    }
  }

  if (!isTableIdsMatching(relationship, linkedTableIds))
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "Given entity keys not are not matching relationship table ids.");
  }

  if (isOneToMany || isManyToOne)
  {
    auto parentTableId = relationship.tableFromId;
    auto childTableId = relationship.tableToId;
    if (isManyToOne)
    {
      std::swap(parentTableId, childTableId);
    }

    const auto idsToInsert = (parentTableId == linkedTableIds.tableFromId ? m_fromRowIds : m_toRowIdsList[0]);
    const auto affectedChildRowIds = (childTableId == linkedTableIds.tableFromId
      ? std::vector<Schema::TableColumnValuesMap>{ m_fromRowIds }
      : m_toRowIdsList);

    const auto& childTable = schema.getTables().at(childTableId);
    const auto& foreignKeyRefs = childTable.mapRelationshioToForeignKeyReferences.at(m_relationshipId);

    for (const auto& affectedChildRowId : affectedChildRowIds)
    {
      auto updateQuery = std::make_unique<UpdateTable>(childTableId);

      for (const auto& primaryKeyId : idsToInsert)
      {
        const auto parentRefColId = primaryKeyId.first.second;
        const auto childColId = foreignKeyRefs.mapReferenceParentColIdToChildColId.at({ parentTableId, parentRefColId });

        updateQuery->set(childColId, primaryKeyId.second);
      }

      Expr whereExpr;
      for (const auto& childCol : affectedChildRowId)
      {
        if (childCol.first != affectedChildRowId.begin()->first)
        {
          whereExpr.and();
        }
        whereExpr.equal(childCol.first.second, childCol.second);
      }

      updateQuery->where(whereExpr);
      addQuery(std::move(updateQuery));
    }
  }
  else
  {
    const auto linkTableId = schema.getManyToManyLinkTableId(m_relationshipId);
    const auto& linkTable = schema.getTables().at(linkTableId);
    const auto& foreignKeyRefs = linkTable.mapRelationshioToForeignKeyReferences.at(m_relationshipId);

    std::map<Schema::Id, QVariantList> columnValuesMap;

    const auto appendValues = [&columnValuesMap, &foreignKeyRefs](const Schema::TableColumnValuesMap& values)
    {
      for (const auto& refColId : values)
      {
        const auto& colId = foreignKeyRefs.mapReferenceParentColIdToChildColId.at(refColId.first);
        columnValuesMap[colId].append(refColId.second);
      }
    };

    for (const auto& toRowIds : m_toRowIdsList)
    {
      appendValues(m_fromRowIds);
      appendValues(toRowIds);
    }

    auto batchInsertQuery = std::make_unique<BatchInsertInto>(linkTableId);
    for (const auto& values : columnValuesMap)
    {
      batchInsertQuery->values(values.first, values.second);
    }

    addQuery(std::move(batchInsertQuery));
  }

  QuerySequence::prepare(schema);
}

LinkTuples::LinkedTableIds LinkTuples::validateAndGetLinkedTableIds(Schema& schema)
{
  if (m_expectedCall != ExpectedCall::Complete)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "LinkTuples query incomplete.");
  }

  const auto fromTableId = schema.validatePrimaryKeysAndGetTableId(m_fromRowIds);

  if (m_toRowIdsList.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "LinkTuples query missing rows ids.");
  }

  auto firstTableIdSet = false;
  Schema::Id toTableId = 0;
  for (const auto& toRowIds : m_toRowIdsList)
  {
    if (!firstTableIdSet)
    {
      toTableId = schema.validatePrimaryKeysAndGetTableId(toRowIds);
      firstTableIdSet = true;
      continue;
    }

    if (toTableId != schema.validatePrimaryKeysAndGetTableId(toRowIds))
    {
      throw DatabaseException(DatabaseException::Type::InvalidSyntax,
        "Linked row ids should belong to a single table.");
    }
  }

  schema.throwIfTableIdNotExisting(fromTableId);
  schema.throwIfTableIdNotExisting(toTableId);

  return { fromTableId, toTableId };
}

bool LinkTuples::isTableIdsMatching(const Schema::Relationship& relationship, const LinkedTableIds& linkedTableIds)
{
  return (((linkedTableIds.tableToId == relationship.tableToId) && (linkedTableIds.tableFromId == relationship.tableFromId)) ||
    ((linkedTableIds.tableToId == relationship.tableFromId) && (linkedTableIds.tableFromId == relationship.tableToId)));
}

}
