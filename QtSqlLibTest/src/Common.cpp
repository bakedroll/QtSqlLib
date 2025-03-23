#include "Common.h"

#include <gtest/gtest.h>

namespace QtSqlLibTest
{

QString Funcs::getDefaultDatabaseFilename()
{
  return "test.db";
}

bool Funcs::isResultTuplesContaining(
  const QtSqlLib::ResultSet_bak& results, IID::Type tableId,
  IID::Type columnId, QVariant value)
{
  results.resetIteration();
  const QtSqlLib::API::TableColumnId colId{ tableId, columnId };
  while (results.hasNext())
  {
    const auto& next = results.next();
    if (next.count(colId) == 0)
    {
      return false;
    }
    if (next.at(colId) == value)
    {
      return true;
    }
  }
  return false;
}

void Funcs::expectRelations(
  const QtSqlLib::ResultSet_bak& results, IID::Type relationshipId,
  IID::Type fromTableId, IID::Type fromColId, IID::Type toTableId, IID::Type toColId,
  const QVariant& fromValue, const QVariantList& toValues)
{
  ASSERT_TRUE(isResultTuplesContaining(results, fromTableId, fromColId, fromValue));

  results.resetIteration();

  std::set<int> matchingToValuesIndices;

  const QtSqlLib::API::TableColumnId colId{ fromTableId, fromColId };
  const QtSqlLib::API::TableColumnId valueColId{ toTableId, toColId };

  while (results.hasNext())
  {
    const auto& next = results.next();
    if (next.at(colId) == fromValue)
    {
      EXPECT_EQ(results.getCurrentNumJoinedResults(relationshipId), toValues.size());
      while (results.hasNextJoinedTuple(relationshipId))
      {
        const auto& joinedTuple = results.nextJoinedTuple(relationshipId);
        const auto& value = joinedTuple.at(valueColId);

        for (int i=0; i<toValues.size(); ++i)
        {
          if (value == toValues.at(i))
          {
            matchingToValuesIndices.insert(i);
            break;
          }
        }
      }

      break;
    }
  }

  EXPECT_EQ(toValues.size(), matchingToValuesIndices.size());
}

}
