#include "Common.h"

#include <gtest/gtest.h>

namespace QtSqlLibTest
{

QString Funcs::getDefaultDatabaseFilename()
{
  return "test.db";
}

bool Funcs::isResultTuplesContaining(const std::vector<IQuery::ResultTuple>& results, const IID& tableId,
                                     const IID& columnId, QVariant value)
{
  const ISchema::TableColumnId colId{ tableId.get(), columnId.get() };
  for (const auto& result : results)
  {
    if (result.values.count(colId) == 0)
    {
      return false;
    }
    if (result.values.at(colId) == value)
    {
      return true;
    }
  }
  return false;
}

bool Funcs::isResultTuplesContaining(const IQuery::TupleValuesList& results, const IID& tableId,
                                     const IID& columnId, QVariant value)
{
  const ISchema::TableColumnId colId{ tableId.get(), columnId.get() };
  for (const auto& result : results)
  {
    if (result.count(colId) == 0)
    {
      return false;
    }
    if (result.at(colId) == value)
    {
      return true;
    }
  }
  return false;
}

IQuery::TupleValuesList& Funcs::getJoinedTuples(std::vector<IQuery::ResultTuple>& results,
                                                               const IID& tableId, const IID& columnId, const QVariant& value,
                                                               const IID& relationshipId)
{
  const ISchema::TableColumnId colId{ tableId.get(), columnId.get() };
  for (auto& result : results)
  {
    if (result.values.at(colId) == value)
    {
      return result.joinedTuples.at(relationshipId.get());
    }
  }
  return s_nullTupleValuesList;
}

void Funcs::expectRelations(std::vector<IQuery::ResultTuple>& results, const IID& relationshipId,
                            const IID& fromTableId, const IID& fromColId, const IID& toTableId, const IID& toColId,
                            const QVariant& fromValue, const QVariantList& toValues)
{
  ASSERT_TRUE(isResultTuplesContaining(results, fromTableId, fromColId, fromValue));

  auto joinedTuples = getJoinedTuples(results, fromTableId, fromColId, fromValue, relationshipId);
  EXPECT_EQ(joinedTuples.size(), toValues.size());

  for (const auto& value : toValues)
  {
    EXPECT_TRUE(isResultTuplesContaining(joinedTuples, toTableId, toColId, value));
  }
}

IQuery::TupleValuesList Funcs::s_nullTupleValuesList;

}
