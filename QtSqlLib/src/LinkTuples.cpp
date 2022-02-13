#include "QtSqlLib/Query/LinkTuples.h"

#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/Query/UpdateTable.h"
#include "QtSqlLib/Query/BatchInsertInto.h"

namespace QtSqlLib::Query
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
  if (m_expectedCall != ExpectedCall::Complete)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "LinkTuples query incomplete.");
  }

  schema.throwIfRelationshipIdNotExisting(m_relationshipId);
  const auto& relationship = schema.getRelationships().at(m_relationshipId);

  const auto tableIds = (m_type == RelationshipType::ToOne
    ? schema.validateOneToOneRelationshipPrimaryKeysAndGetTableIds(m_relationshipId,  m_fromRowIds, m_toRowIdsList[0])
    : schema.validateOneToManyRelationshipPrimaryKeysAndGetTableIds(m_relationshipId, m_fromRowIds, m_toRowIdsList));

  if (relationship.type == Schema::RelationshipType::ManyToMany)
  {
    prepareManyToManyLinkQuery(schema, relationship, tableIds.first, tableIds.second);
  }
  else
  {
    prepareOneToManyLinkQuery(schema, relationship, tableIds.first, tableIds.second);
  }
}

void LinkTuples::prepareOneToManyLinkQuery(Schema& schema, const Schema::Relationship& relationship,
                                           Schema::Id fromTableId, Schema::Id toTableId)
{
  auto parentTableId = relationship.tableFromId;
  auto childTableId = relationship.tableToId;
  if (relationship.type == Schema::RelationshipType::ManyToOne)
  {
    std::swap(parentTableId, childTableId);
  }

  const auto idsToInsert = (parentTableId == fromTableId ? m_fromRowIds : m_toRowIdsList[0]);
  const auto affectedChildRowIds = (childTableId == fromTableId
    ? std::vector<Schema::TableColumnValuesMap>{ m_fromRowIds }
  : m_toRowIdsList);

  const auto& childTable = schema.getTables().at(childTableId);
  const auto& foreignKeyRefs = childTable.mapRelationshioToForeignKeyReferences.at({ m_relationshipId, parentTableId });

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

void LinkTuples::prepareManyToManyLinkQuery(Schema& schema, const Schema::Relationship& relationship,
                                            Schema::Id fromTableId, Schema::Id toTableId)
{
  const auto linkTableId = schema.getManyToManyLinkTableId(m_relationshipId);
  const auto& linkTable = schema.getTables().at(linkTableId);
  const auto& foreignKeyRefsFrom = linkTable.mapRelationshioToForeignKeyReferences.at({ m_relationshipId, fromTableId });
  const auto& foreignKeyRefsTo = linkTable.mapRelationshioToForeignKeyReferences.at({ m_relationshipId, toTableId });

  std::map<Schema::Id, QVariantList> columnValuesMap;

  const auto appendValues = [&columnValuesMap](const Schema::TableColumnValuesMap& values,
    const Schema::ForeignKeyReference& foreignKeyRef)
  {
    for (const auto& refColId : values)
    {
      if (foreignKeyRef.mapReferenceParentColIdToChildColId.count(refColId.first) > 0)
      {
        const auto& colId = foreignKeyRef.mapReferenceParentColIdToChildColId.at(refColId.first);
        columnValuesMap[colId].append(refColId.second);
        break;
      }
    }
  };

  for (const auto& toRowIds : m_toRowIdsList)
  {
    appendValues(m_fromRowIds, foreignKeyRefsFrom);
    appendValues(toRowIds, foreignKeyRefsTo);
  }

  auto batchInsertQuery = std::make_unique<BatchInsertInto>(linkTableId);
  for (const auto& values : columnValuesMap)
  {
    batchInsertQuery->values(values.first, values.second);
  }

  addQuery(std::move(batchInsertQuery));
}

}
