#include "QtSqlLib/Query/LinkTuples.h"

#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/Query/BatchInsertInto.h"

namespace QtSqlLib::Query
{

LinkTuples::LinkTuples(Schema::Id relationshipId)
  : QuerySequence()
  , m_relationshipId(relationshipId)
  , m_expectedCall(ExpectedCall::From)
  , m_type(RelationshipType::ToOne)
  , m_bRemainingFromKeys(false)
{
}

LinkTuples::~LinkTuples() = default;

LinkTuples& LinkTuples::fromOne(const Schema::TupleValues& tupleKeyValues)
{
  if (m_expectedCall != ExpectedCall::From)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "fromOne() call not expected.");
  }

  m_fromTupleKeyValues = tupleKeyValues;
  m_expectedCall = ExpectedCall::To;
  return *this;
}

LinkTuples& LinkTuples::fromRemainingKey()
{
  if (m_expectedCall != ExpectedCall::From)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "fromOne() call not expected.");
  }

  m_bRemainingFromKeys = true;
  m_expectedCall = ExpectedCall::To;
  return *this;
}

LinkTuples& LinkTuples::toOne(const Schema::TupleValues& tupleKeyValues)
{
  if (m_expectedCall != ExpectedCall::To)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "toOne() call not expected.");
  }

  m_type = RelationshipType::ToOne;
  m_toTupleKeyValuesList.emplace_back(tupleKeyValues);
  m_expectedCall = ExpectedCall::Complete;
  return *this;
}

LinkTuples& LinkTuples::toMany(const std::vector<Schema::TupleValues>& tupleKeyValuesList)
{
  if (m_expectedCall != ExpectedCall::To)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "toMany() call not expected.");
  }

  m_type = RelationshipType::ToMany;
  m_toTupleKeyValuesList = tupleKeyValuesList;
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

  if (!m_bRemainingFromKeys && m_fromTupleKeyValues.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "From key must not be empty.");
  }

  schema.throwIfRelationshipIsNotExisting(m_relationshipId);
  const auto& relationship = schema.getRelationships().at(m_relationshipId);

  const auto tableIds = (m_type == RelationshipType::ToOne
    ? schema.verifyOneToOneRelationshipPrimaryKeysAndGetTableIds(m_relationshipId,  m_fromTupleKeyValues, m_toTupleKeyValuesList[0])
    : schema.verifyOneToManyRelationshipPrimaryKeysAndGetTableIds(m_relationshipId, m_fromTupleKeyValues, m_toTupleKeyValuesList));

  const auto tableFromId = tableIds.first;
  const auto tableToId = tableIds.second;

  if (relationship.type == Schema::RelationshipType::ManyToMany)
  {
    prepareToManyLinkQuery(schema, relationship, tableFromId, tableToId);
  }
  else
  {
    prepareToOneLinkQuery(schema, relationship, tableFromId, tableToId);
  }
}

LinkTuples::UpdateTableForeignKeys::UpdateTableForeignKeys(
  Schema::Id tableId,
  const Schema::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap)
  : UpdateTable(tableId)
  , m_mode(Mode::Default)
  , m_primaryForeignKeyColIdMap(primaryForeignKeyColIdMap)
{
}

LinkTuples::UpdateTableForeignKeys::~UpdateTableForeignKeys() = default;

void LinkTuples::UpdateTableForeignKeys::setMode(Mode mode)
{
  m_mode = mode;
}

void LinkTuples::UpdateTableForeignKeys::setForeignKeyValues(const Schema::TupleValues& parentKeyValues)
{
  for (const auto& parentKeyValue : parentKeyValues)
  {
    const auto childColId = m_primaryForeignKeyColIdMap.at(parentKeyValue.first);
    set(childColId, parentKeyValue.second);
  }
}

void LinkTuples::UpdateTableForeignKeys::makeAndAddWhereExpr(const Schema::TupleValues& affectedChildKeyValues)
{
  Expr whereExpr;
  for (const auto& childKeyValue : affectedChildKeyValues)
  {
    if (childKeyValue.first != affectedChildKeyValues.begin()->first)
    {
      whereExpr.and();
    }
    whereExpr.equal(childKeyValue.first.columnId, childKeyValue.second);
  }

  where(whereExpr);
}

API::IQuery::SqlQuery LinkTuples::UpdateTableForeignKeys::getSqlQuery(Schema& schema,
                                                                      QueryResults& previousQueryResults)
{
  const auto throwIfInvalidPreviousQueryResults = [&previousQueryResults]()
  {
    if ((previousQueryResults.validity == QueryResults::Validity::Invalid) || previousQueryResults.values.empty())
    {
      throw DatabaseException(DatabaseException::Type::InvalidSyntax,
        "Expected previous query results.");
    }
  };

  if (m_mode == Mode::ForeignKeyValuesRemaining)
  {
    throwIfInvalidPreviousQueryResults();
    setForeignKeyValues(previousQueryResults.values[0]);
  }
  else if (m_mode == Mode::AffectedChildKeyValuesRemaining)
  {
    throwIfInvalidPreviousQueryResults();
    makeAndAddWhereExpr(previousQueryResults.values[0]);
  }

  return UpdateTable::getSqlQuery(schema, previousQueryResults);
}

void LinkTuples::prepareToOneLinkQuery(Schema& schema, const Schema::Relationship& relationship,
                                       Schema::Id fromTableId, Schema::Id toTableId)
{
  auto parentTableId = relationship.tableFromId;
  auto childTableId = relationship.tableToId;
  if (relationship.type == Schema::RelationshipType::ManyToOne)
  {
    std::swap(parentTableId, childTableId);
  }

  const auto keyValuesToInsert = (parentTableId == fromTableId ? m_fromTupleKeyValues : m_toTupleKeyValuesList[0]);
  const auto affectedChildTupleKeys = (childTableId == fromTableId
    ? std::vector<Schema::TupleValues>{ m_fromTupleKeyValues }
    : m_toTupleKeyValuesList);

  const auto& childTable = schema.getTables().at(childTableId);
  const auto& foreignKeyRefs = childTable.relationshipToForeignKeyReferencesMap.at({ m_relationshipId, parentTableId });

  const auto isRemainingForeignKeyValues = (m_bRemainingFromKeys && (parentTableId == fromTableId));
  const auto isRemainingAffectedChildKeyValues = (m_bRemainingFromKeys && (childTableId == fromTableId));

  for (const auto& affectedChildRowId : affectedChildTupleKeys)
  {
    auto updateQuery = std::make_unique<UpdateTableForeignKeys>(childTableId, foreignKeyRefs.primaryForeignKeyColIdMap);

    if (isRemainingForeignKeyValues)
    {
      updateQuery->setMode(UpdateTableForeignKeys::Mode::ForeignKeyValuesRemaining);
    }
    else
    {
      updateQuery->setForeignKeyValues(keyValuesToInsert);
    }

    if (isRemainingAffectedChildKeyValues)
    {
      updateQuery->setMode(UpdateTableForeignKeys::Mode::AffectedChildKeyValuesRemaining);
    }
    else
    {
      updateQuery->makeAndAddWhereExpr(affectedChildRowId);
    }

    addQuery(std::move(updateQuery));
  }
}

void LinkTuples::prepareToManyLinkQuery(Schema& schema, const Schema::Relationship& relationship,
                                        Schema::Id fromTableId, Schema::Id toTableId)
{
  const auto linkTableId = schema.getManyToManyLinkTableId(m_relationshipId);
  const auto& linkTable = schema.getTables().at(linkTableId);
  const auto& foreignKeyRefsFrom = linkTable.relationshipToForeignKeyReferencesMap.at({ m_relationshipId, fromTableId });
  const auto& foreignKeyRefsTo = linkTable.relationshipToForeignKeyReferencesMap.at({ m_relationshipId, toTableId });

  std::map<Schema::Id, QVariantList> columnValuesMap;

  const auto appendValues = [&columnValuesMap](
    const Schema::TupleValues& values,
    const Schema::ForeignKeyReference& foreignKeyRef)
  {
    for (const auto& refColId : values)
    {
      if (foreignKeyRef.primaryForeignKeyColIdMap.count(refColId.first) > 0)
      {
        const auto& colId = foreignKeyRef.primaryForeignKeyColIdMap.at(refColId.first);
        columnValuesMap[colId].append(refColId.second);
        break;
      }
    }
  };

  for (const auto& toRowIds : m_toTupleKeyValuesList)
  {
    appendValues(m_fromTupleKeyValues, foreignKeyRefsFrom);
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
