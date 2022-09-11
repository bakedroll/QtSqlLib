#include "QtSqlLib/Query/LinkTuples.h"

#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/Query/BatchInsertInto.h"

namespace QtSqlLib::Query
{

LinkTuples::LinkTuples(API::ISchema::Id relationshipId)
  : QuerySequence()
  , m_relationshipId(relationshipId)
  , m_expectedCall(ExpectedCall::From)
  , m_type(RelationshipType::ToOne)
  , m_bRemainingFromKeys(false)
{
}

LinkTuples::~LinkTuples() = default;

LinkTuples& LinkTuples::fromOne(const API::ISchema::TupleValues& tupleKeyValues)
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

LinkTuples& LinkTuples::toOne(const API::ISchema::TupleValues& tupleKeyValues)
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

LinkTuples& LinkTuples::toMany(const std::vector<API::ISchema::TupleValues>& tupleKeyValuesList)
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

void LinkTuples::prepare(API::ISchema& schema)
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

  if (relationship.type == API::ISchema::RelationshipType::ManyToMany)
  {
    prepareToManyLinkQuery(schema, relationship, tableFromId, tableToId);
  }
  else
  {
    prepareToOneLinkQuery(schema, relationship, tableFromId, tableToId);
  }
}

LinkTuples::UpdateTableForeignKeys::UpdateTableForeignKeys(
  API::ISchema::Id tableId,
  const API::ISchema::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap)
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

void LinkTuples::UpdateTableForeignKeys::setForeignKeyValues(const API::ISchema::TupleValues& parentKeyValues)
{
  for (const auto& parentKeyValue : parentKeyValues)
  {
    const auto childColId = m_primaryForeignKeyColIdMap.at(parentKeyValue.first);
    set(childColId, parentKeyValue.second);
  }
}

void LinkTuples::UpdateTableForeignKeys::makeAndAddWhereExpr(const API::ISchema::TupleValues& affectedChildKeyValues)
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

API::IQuery::SqlQuery LinkTuples::UpdateTableForeignKeys::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema,
                                                                      QueryResults& previousQueryResults)
{
  const auto throwIfInvalidPreviousQueryResults = [&previousQueryResults]()
  {
    if ((previousQueryResults.validity == QueryResults::Validity::Invalid) || previousQueryResults.resultTuples.empty())
    {
      throw DatabaseException(DatabaseException::Type::InvalidSyntax,
        "Expected previous query results.");
    }
  };

  if (m_mode == Mode::ForeignKeyValuesRemaining)
  {
    throwIfInvalidPreviousQueryResults();
    setForeignKeyValues(previousQueryResults.resultTuples[0].values);
  }
  else if (m_mode == Mode::AffectedChildKeyValuesRemaining)
  {
    throwIfInvalidPreviousQueryResults();
    makeAndAddWhereExpr(previousQueryResults.resultTuples[0].values);
  }

  return UpdateTable::getSqlQuery(db, schema, previousQueryResults);
}

LinkTuples::BatchInsertRemainingKeys::BatchInsertRemainingKeys(API::ISchema::Id tableId,
  int numRelations,
  const API::ISchema::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap)
  : BatchInsertInto(tableId)
  , m_numRelations(numRelations)
  , m_primaryForeignKeyColIdMap(primaryForeignKeyColIdMap)
{
}

LinkTuples::BatchInsertRemainingKeys::~BatchInsertRemainingKeys() = default;

API::IQuery::SqlQuery LinkTuples::BatchInsertRemainingKeys::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema,
                                                                        QueryResults& previousQueryResults)
{
  if (previousQueryResults.validity != QueryResults::Validity::Valid || previousQueryResults.resultTuples.empty())
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Expected previous query results.");
  }

  const auto& prevValues = previousQueryResults.resultTuples[0].values;
  for (const auto& value : prevValues)
  {
    if (m_primaryForeignKeyColIdMap.count(value.first) == 0)
    {
      throw DatabaseException(DatabaseException::Type::UnexpectedError, "Missing foreign key ref.");
    }
    QVariantList list;
    for (auto i=0; i<m_numRelations; ++i)
    {
      list << value.second;
    }

    values(m_primaryForeignKeyColIdMap.at(value.first), list);
  }

  return BatchInsertInto::getSqlQuery(db, schema, previousQueryResults);
}

void LinkTuples::prepareToOneLinkQuery(API::ISchema& schema, const API::ISchema::Relationship& relationship,
                                       API::ISchema::Id fromTableId, API::ISchema::Id toTableId)
{
  auto parentTableId = relationship.tableFromId;
  auto childTableId = relationship.tableToId;

  if (relationship.type == API::ISchema::RelationshipType::ManyToOne)
  {
    std::swap(parentTableId, childTableId);
  }

  const auto isCorrectTableRelationDirection = (parentTableId == fromTableId || fromTableId == toTableId);
  const auto keyValuesToInsert = (isCorrectTableRelationDirection ? m_fromTupleKeyValues : m_toTupleKeyValuesList[0]);
  const auto affectedChildTupleKeys = (isCorrectTableRelationDirection
    ? m_toTupleKeyValuesList
    : std::vector<API::ISchema::TupleValues>{ m_fromTupleKeyValues });

  const auto& childTable = schema.getTables().at(childTableId);
  const auto& foreignKeyRefs = childTable.relationshipToForeignKeyReferencesMap.at({ m_relationshipId, parentTableId });

  const auto isRemainingForeignKeyValues = (m_bRemainingFromKeys && ((parentTableId == fromTableId) || (toTableId == fromTableId)));
  const auto isRemainingAffectedChildKeyValues = (m_bRemainingFromKeys && ((childTableId == fromTableId) && (toTableId != fromTableId)));

  if (foreignKeyRefs.size() != 1)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError,
      "Foreign key references table seems to be corrupted.");
  }

  for (const auto& affectedChildRowId : affectedChildTupleKeys)
  {
    auto updateQuery = std::make_unique<UpdateTableForeignKeys>(childTableId, foreignKeyRefs[0].primaryForeignKeyColIdMap);

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

void LinkTuples::prepareToManyLinkQuery(API::ISchema& schema, const API::ISchema::Relationship& relationship,
                                        API::ISchema::Id fromTableId, API::ISchema::Id toTableId)
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

  std::map<API::ISchema::Id, QVariantList> columnValuesMap;

  const auto appendValues = [&columnValuesMap](
    const API::ISchema::TupleValues& values,
    const API::ISchema::ForeignKeyReference& foreignKeyRef)
  {
    for (const auto& refColId : values)
    {
      if (foreignKeyRef.primaryForeignKeyColIdMap.count(refColId.first) > 0)
      {
        const auto& colId = foreignKeyRef.primaryForeignKeyColIdMap.at(refColId.first);
        columnValuesMap[colId].append(refColId.second);
      }
    }
  };

  for (const auto& toRowIds : m_toTupleKeyValuesList)
  {
    appendValues(m_fromTupleKeyValues, foreignKeyRefsFrom);
    appendValues(toRowIds, foreignKeyRefsTo);
  }

  auto batchInsertQuery = m_bRemainingFromKeys
    ? std::make_unique<BatchInsertRemainingKeys>(linkTableId, static_cast<int>(m_toTupleKeyValuesList.size()),
      foreignKeyRefsFrom.primaryForeignKeyColIdMap)
    : std::make_unique<BatchInsertInto>(linkTableId);

  for (const auto& values : columnValuesMap)
  {
    batchInsertQuery->values(values.first, values.second);
  }

  addQuery(std::move(batchInsertQuery));
}

}
