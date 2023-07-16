#include "QtSqlLib/Query/UnlinkTuples.h"

#include "QtSqlLib/Query/DeleteFrom.h"
#include "QtSqlLib/Query/UpdateTable.h"

namespace QtSqlLib::Query
{

Expr createWhereExpression(const API::ISchema::TupleValues& childKeyValues)
{
  Expr whereExpr;
  for (const auto& col : childKeyValues)
  {
    if (col.first != childKeyValues.begin()->first)
    {
      whereExpr.and();
    }

    whereExpr.equal(col.first, col.second);
  }
  return whereExpr;
}

UnlinkTuples::UnlinkTuples(const API::IID& relationshipId) :
  QuerySequence(),
  m_relationshipPreparationData(relationshipId)
{
}

UnlinkTuples::~UnlinkTuples() = default;

UnlinkTuples& UnlinkTuples::fromOne(const API::ISchema::TupleValues& tupleKeyValues)
{
  m_relationshipPreparationData.fromOne(tupleKeyValues);
  return *this;
}

UnlinkTuples& UnlinkTuples::toOne(const API::ISchema::TupleValues& tupleKeyValues)
{
  m_relationshipPreparationData.toOne(tupleKeyValues);
  return *this;
}

UnlinkTuples& UnlinkTuples::toMany(const std::vector<API::ISchema::TupleValues>& tupleKeyValuesList)
{
  m_relationshipPreparationData.toMany(tupleKeyValuesList);
  return *this;
}

void UnlinkTuples::prepare(API::ISchema& schema)
{
  const auto affectedData = m_relationshipPreparationData.resolveAffectedTableData(schema);
  if (affectedData.isLinkTable)
  {
    for (const auto& tuple : affectedData.affectedTuples)
    {
      auto deleteQuery = std::make_unique<DeleteFrom>(ID(affectedData.tableId));
      auto expr = createWhereExpression(tuple.childKeyValues);
      deleteQuery->where(expr);

      addQuery(std::move(deleteQuery));
    }
  }
  else
  {
    for (const auto& tuple : affectedData.affectedTuples)
    {
      auto updateQuery = std::make_unique<UpdateTable>(ID(affectedData.tableId));
      for (const auto& col : tuple.foreignKeyValues)
      {
        const auto childColId = affectedData.primaryForeignKeyColIdMap.at(col.first);
        updateQuery->set(ID(childColId), QVariant());

        auto expr = createWhereExpression(tuple.childKeyValues);
        updateQuery->where(expr);
      }

      addQuery(std::move(updateQuery));
    }
  }
}

}
