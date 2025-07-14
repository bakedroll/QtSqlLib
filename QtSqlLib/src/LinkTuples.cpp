#include "QtSqlLib/Query/LinkTuples.h"

#include "QtSqlLib/Expr.h"
#include "QtSqlLib/ID.h"
#include "QtSqlLib/Query/BatchInsertInto.h"

#include "BatchInsertRemainingKeys.h"
#include "UpdateTableForeignKeys.h"

#include <QVariantList>

namespace QtSqlLib::Query
{

LinkTuples::LinkTuples(const API::IID& relationshipId) :
  QuerySequence(),
  m_relationshipPreparationData(relationshipId)
{
}

LinkTuples::~LinkTuples() = default;

LinkTuples& LinkTuples::fromOne(const PrimaryKey& tupleKeyValues)
{
  m_relationshipPreparationData.fromOne(tupleKeyValues);
  return *this;
}

LinkTuples& LinkTuples::fromRemainingKey()
{
  m_relationshipPreparationData.fromRemainingKey();
  return *this;
}

LinkTuples& LinkTuples::toOne(const PrimaryKey& tupleKeyValues)
{
  m_relationshipPreparationData.toOne(tupleKeyValues);
  return *this;
}

LinkTuples& LinkTuples::toMany(const std::vector<PrimaryKey>& tupleKeyValuesList)
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
      for (const auto& col : tuple.childKeyValues.values())
      {
        colValuesMap[col.columnId].append(col.value);
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

}
