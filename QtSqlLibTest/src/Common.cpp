#include "Common.h"

#include <gtest/gtest.h>

namespace QtSqlLibTest
{

QString Funcs::getDefaultDatabaseFilename()
{
  return "test.db";
}

bool Funcs::isResultTuplesContaining(const std::vector<IQuery::ResultTuple>& results, ISchema::Id tableId,
                                     ISchema::Id columnId, QVariant value)
{
  const ISchema::TableColumnId colId{ tableId, columnId };
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

bool Funcs::isResultTuplesContaining(const IQuery::TupleValuesList& results, ISchema::Id tableId,
                                     ISchema::Id columnId, QVariant value)
{
  const ISchema::TableColumnId colId{ tableId, columnId };
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
                                                               ISchema::Id tableId, ISchema::Id columnId, const QVariant& value,
                                                               ISchema::Id relationshipId)
{
  const ISchema::TableColumnId colId{ tableId, columnId };
  for (auto& result : results)
  {
    if (result.values.at(colId) == value)
    {
      return result.joinedTuples.at(relationshipId);
    }
  }
  return s_nullTupleValuesList;
}

void Funcs::expectRelations(std::vector<IQuery::ResultTuple>& results, ISchema::Id relationshipId,
                            ISchema::Id fromTableId, ISchema::Id fromColId, ISchema::Id toTableId, ISchema::Id toColId,
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
