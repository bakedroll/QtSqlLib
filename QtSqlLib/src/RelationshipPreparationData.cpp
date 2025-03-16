#include "QtSqlLib/RelationshipPreparationData.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/API/ISchema.h"
#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

RelationshipPreparationData::RelationshipPreparationData(const API::IID& relationshipId) :
  m_relationshipId(relationshipId.get()),
  m_type(RelationshipType::ToOne),
  m_expectedCall(ExpectedCall::From),
  m_bRemainingFromKeys(false)
{ 
}

RelationshipPreparationData::~RelationshipPreparationData() = default;

void RelationshipPreparationData::fromOne(const API::TupleValues& tupleKeyValues)
{
  if (m_expectedCall != ExpectedCall::From)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "fromOne() call not expected.");
  }

  m_fromTupleKeyValues = tupleKeyValues;
  m_expectedCall = ExpectedCall::To;
}

void RelationshipPreparationData::fromRemainingKey()
{
  if (m_expectedCall != ExpectedCall::From)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "fromOne() call not expected.");
  }

  m_bRemainingFromKeys = true;
  m_expectedCall = ExpectedCall::To;
}

void RelationshipPreparationData::toOne(const API::TupleValues& tupleKeyValues)
{
  if (m_expectedCall != ExpectedCall::To)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "toOne() call not expected.");
  }

  m_type = RelationshipType::ToOne;
  m_toTupleKeyValuesList.emplace_back(tupleKeyValues);
  m_expectedCall = ExpectedCall::Complete;
}

void RelationshipPreparationData::toMany(const std::vector<API::TupleValues>& tupleKeyValuesList)
{
  if (m_expectedCall != ExpectedCall::To)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "toMany() call not expected.");
  }

  m_type = RelationshipType::ToMany;
  m_toTupleKeyValuesList = tupleKeyValuesList;
  m_expectedCall = ExpectedCall::Complete;
}

RelationshipPreparationData::AffectedData RelationshipPreparationData::resolveAffectedTableData(API::ISchema& schema)
{
  if (m_expectedCall != ExpectedCall::Complete)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "LinkTuples query incomplete.");
  }

  if (!m_bRemainingFromKeys && m_fromTupleKeyValues.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "From key must not be empty.");
  }

  schema.getSanityChecker().throwIfRelationshipIsNotExisting(m_relationshipId);
  const auto& relationship = schema.getRelationships().at(m_relationshipId);

  const auto tableIds = (m_type == RelationshipType::ToOne
    ? schema.verifyOneToOneRelationshipPrimaryKeysAndGetTableIds(m_relationshipId,  m_fromTupleKeyValues, m_toTupleKeyValuesList[0])
    : schema.verifyOneToManyRelationshipPrimaryKeysAndGetTableIds(m_relationshipId, m_fromTupleKeyValues, m_toTupleKeyValuesList));

  const auto tableFromId = tableIds.first;
  const auto tableToId = tableIds.second;

  if (relationship.type == API::RelationshipType::ManyToMany)
  {
    return determineAffectedLinkTableData(schema, tableFromId, tableToId);
  }

  return determineAffectedChildTableData(schema, relationship, tableFromId, tableToId);
}

RelationshipPreparationData::AffectedData RelationshipPreparationData::determineAffectedChildTableData(
  API::ISchema& schema, const API::Relationship& relationship,
  API::IID::Type fromTableId, API::IID::Type toTableId)
{
  auto parentTableId = relationship.tableFromId;
  auto childTableId = relationship.tableToId;

  if (relationship.type == API::RelationshipType::ManyToOne)
  {
    std::swap(parentTableId, childTableId);
  }

  const auto isCorrectTableRelationDirection = (parentTableId == fromTableId || fromTableId == toTableId);
  const auto keyValuesToInsert = (isCorrectTableRelationDirection ? m_fromTupleKeyValues : m_toTupleKeyValuesList[0]);
  const auto affectedChildTupleKeys = (isCorrectTableRelationDirection
    ? m_toTupleKeyValuesList
    : std::vector<API::TupleValues>{ m_fromTupleKeyValues });

  const auto& childTable = schema.getTables().at(childTableId);
  const auto& foreignKeyRefs = childTable.relationshipToForeignKeyReferencesMap.at({ m_relationshipId, parentTableId });

  const auto isRemainingForeignKeyValues = (m_bRemainingFromKeys && ((parentTableId == fromTableId) || (toTableId == fromTableId)));
  const auto isRemainingAffectedChildKeyValues = (m_bRemainingFromKeys && ((childTableId == fromTableId) && (toTableId != fromTableId)));

  if (foreignKeyRefs.size() != 1)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError,
      "Foreign key references table seems to be corrupted.");
  }

  const auto remainingKeysMode = isRemainingForeignKeyValues
    ? RemainingKeysMode::RemainingForeignKeys
    : (isRemainingAffectedChildKeyValues
      ? RemainingKeysMode::RemainingPrimaryKeys
      : RemainingKeysMode::NoRemainingKeys);

  AffectedData affectedData { childTableId, false, remainingKeysMode, foreignKeyRefs[0].primaryForeignKeyColIdMap, {} };
  for (const auto& affectedChildRowId : affectedChildTupleKeys)
  {
    AffectedTuple affectedTuple;

    if (!isRemainingForeignKeyValues)
    {
      affectedTuple.foreignKeyValues = keyValuesToInsert;
    }
    if (!isRemainingAffectedChildKeyValues)
    {
      affectedTuple.childKeyValues = affectedChildRowId;
    }

    affectedData.affectedTuples.emplace_back(affectedTuple);
  }

  return affectedData;
}

RelationshipPreparationData::AffectedData RelationshipPreparationData::determineAffectedLinkTableData(
  API::ISchema& schema, API::IID::Type fromTableId, API::IID::Type toTableId)
{
  const auto linkTableId = schema.getManyToManyLinkTableId(m_relationshipId);
  const auto& linkTable = schema.getTables().at(linkTableId);

  const auto isSelfRelationship = (fromTableId == toTableId);

  const auto& foreignKeyRefsFromList = linkTable.relationshipToForeignKeyReferencesMap.at({ m_relationshipId, fromTableId });
  const auto& foreignKeyRefsToList = linkTable.relationshipToForeignKeyReferencesMap.at({ m_relationshipId, toTableId });

  if ((isSelfRelationship && foreignKeyRefsFromList.size() != 2) ||
    (!isSelfRelationship && (foreignKeyRefsFromList.size() != 1 || foreignKeyRefsToList.size() != 1)))
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError,
      "Foreign key references table seems to be corrupted.");
  }

  const auto& foreignKeyRefsFrom =  foreignKeyRefsFromList[0];
  const auto& foreignKeyRefsTo = (isSelfRelationship ? foreignKeyRefsToList[1] : foreignKeyRefsToList[0]);

  const auto appendTuple = [&linkTableId](
    AffectedTuple& tuple,
    const API::TupleValues& values,
    const API::ForeignKeyReference& foreignKeyRef)
  {
    for (const auto& refColId : values)
    {
      if (foreignKeyRef.primaryForeignKeyColIdMap.count(refColId.first) > 0)
      {
        const auto& colId = foreignKeyRef.primaryForeignKeyColIdMap.at(refColId.first);
        tuple.childKeyValues[API::TableColumnId{ linkTableId, colId }] = refColId.second;
      }
    }
  };

  const auto remainingKeysMode = m_bRemainingFromKeys
    ? RemainingKeysMode::RemainingForeignKeys
    : RemainingKeysMode::NoRemainingKeys;

  AffectedData affectedData { linkTableId, true, remainingKeysMode, foreignKeyRefsFrom.primaryForeignKeyColIdMap, {} };

  for (const auto& toRowIds : m_toTupleKeyValuesList)
  {
    AffectedTuple affectedTuple;
    appendTuple(affectedTuple, m_fromTupleKeyValues, foreignKeyRefsFrom);
    appendTuple(affectedTuple, toRowIds, foreignKeyRefsTo);
    affectedData.affectedTuples.emplace_back(affectedTuple);
  }

  return affectedData;
}

}
