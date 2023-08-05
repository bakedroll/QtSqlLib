#include "QtSqlLib/Query/LinkTuples.h"

#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/ID.h"
#include "QtSqlLib/Query/BatchInsertInto.h"

#include <QVariantList>

namespace QtSqlLib::Query
{

LinkTuples::LinkTuples(const API::IID& relationshipId) :
  QuerySequence(),
  m_relationshipPreparationData(relationshipId)
{
}

LinkTuples::~LinkTuples() = default;

LinkTuples& LinkTuples::fromOne(const API::ISchema::TupleValues& tupleKeyValues)
{
  m_relationshipPreparationData.fromOne(tupleKeyValues);
  return *this;
}

LinkTuples& LinkTuples::fromRemainingKey()
{
  m_relationshipPreparationData.fromRemainingKey();
  return *this;
}

LinkTuples& LinkTuples::toOne(const API::ISchema::TupleValues& tupleKeyValues)
{
  m_relationshipPreparationData.toOne(tupleKeyValues);
  return *this;
}

LinkTuples& LinkTuples::toMany(const std::vector<API::ISchema::TupleValues>& tupleKeyValuesList)
{
  m_relationshipPreparationData.toMany(tupleKeyValuesList);
  return *this;
}

void LinkTuples::prepare(API::ISchema& schema)
{
  const auto affectedData = m_relationshipPreparationData.resolveAffectedTableData(schema);
  if (affectedData.isLinkTable)
  {
    auto batchInsertQuery = affectedData.remainingKeysMode == RelationshipPreparationData::RemainingKeysMode::RemainingForeignKeys
      ? std::make_unique<BatchInsertRemainingKeys>(affectedData.tableId, static_cast<int>(affectedData.affectedTuples.size()),
        affectedData.primaryForeignKeyColIdMap)
      : std::make_unique<BatchInsertInto>(ID(affectedData.tableId));

    std::map<API::IID::Type, QVariantList> colValuesMap;
    for (const auto& tuple : affectedData.affectedTuples)
    {
      for (const auto& col : tuple.childKeyValues)
      {
        colValuesMap[col.first.columnId].append(col.second);
      }
    }

    for (const auto& column : colValuesMap)
    {
      batchInsertQuery->values(ID(column.first), column.second);
    }

    addQuery(std::move(batchInsertQuery));
  }
  else
  {
    for (const auto& affectedTuple : affectedData.affectedTuples)
    {
      auto updateQuery = std::make_unique<UpdateTableForeignKeys>(affectedData.tableId, affectedData.primaryForeignKeyColIdMap);
      updateQuery->setRemainingKeysMode(affectedData.remainingKeysMode);

      if (affectedData.remainingKeysMode != RelationshipPreparationData::RemainingKeysMode::RemainingForeignKeys)
      {
        updateQuery->setForeignKeyValues(affectedTuple.foreignKeyValues);
      }
      if (affectedData.remainingKeysMode != RelationshipPreparationData::RemainingKeysMode::RemainingPrimaryKeys)
      {
        updateQuery->makeAndAddWhereExpr(affectedTuple.childKeyValues);
      }
      addQuery(std::move(updateQuery));
    }
  }
}

LinkTuples::UpdateTableForeignKeys::UpdateTableForeignKeys(
  API::IID::Type tableId,
  const API::ISchema::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap)
  : UpdateTable(ID(tableId))
  , m_remainingKeysMode(RelationshipPreparationData::RemainingKeysMode::NoRemainingKeys)
  , m_primaryForeignKeyColIdMap(primaryForeignKeyColIdMap)
{
}

LinkTuples::UpdateTableForeignKeys::~UpdateTableForeignKeys() = default;

void LinkTuples::UpdateTableForeignKeys::setRemainingKeysMode(RelationshipPreparationData::RemainingKeysMode mode)
{
  m_remainingKeysMode = mode;
}

void LinkTuples::UpdateTableForeignKeys::setForeignKeyValues(const API::ISchema::TupleValues& parentKeyValues)
{
  for (const auto& parentKeyValue : parentKeyValues)
  {
    const auto childColId = m_primaryForeignKeyColIdMap.at(parentKeyValue.first);
    set(ID(childColId), parentKeyValue.second);
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
    whereExpr.equal(ID(childKeyValue.first.columnId), childKeyValue.second);
  }

  where(whereExpr);
}

API::IQuery::SqlQuery LinkTuples::UpdateTableForeignKeys::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema,
                                                                      const ResultSet& previousQueryResults)
{
  const auto throwIfInvalidPreviousQueryResults = [&previousQueryResults]()
  {
    if (!previousQueryResults.isValid() || !previousQueryResults.hasNext())
    {
      throw DatabaseException(DatabaseException::Type::InvalidSyntax,
        "Expected previous query results.");
    }
  };

  if (m_remainingKeysMode == RelationshipPreparationData::RemainingKeysMode::RemainingForeignKeys)
  {
    throwIfInvalidPreviousQueryResults();
    setForeignKeyValues(previousQueryResults.next());
  }
  else if (m_remainingKeysMode == RelationshipPreparationData::RemainingKeysMode::RemainingPrimaryKeys)
  {
    throwIfInvalidPreviousQueryResults();
    makeAndAddWhereExpr(previousQueryResults.next());
  }

  previousQueryResults.resetIteration();
  return UpdateTable::getSqlQuery(db, schema, previousQueryResults);
}

LinkTuples::BatchInsertRemainingKeys::BatchInsertRemainingKeys(API::IID::Type tableId,
  int numRelations,
  const API::ISchema::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap)
  : BatchInsertInto(ID(tableId))
  , m_numRelations(numRelations)
  , m_primaryForeignKeyColIdMap(primaryForeignKeyColIdMap)
{
}

LinkTuples::BatchInsertRemainingKeys::~BatchInsertRemainingKeys() = default;

API::IQuery::SqlQuery LinkTuples::BatchInsertRemainingKeys::getSqlQuery(const QSqlDatabase& db, API::ISchema& schema,
                                                                        const ResultSet& previousQueryResults)
{
  if (!previousQueryResults.isValid() || !previousQueryResults.hasNext())
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Expected previous query results.");
  }

  const auto& prevValues = previousQueryResults.next();
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

    values(ID(m_primaryForeignKeyColIdMap.at(value.first)), list);
  }

  previousQueryResults.resetIteration();
  return BatchInsertInto::getSqlQuery(db, schema, previousQueryResults);
}

}
