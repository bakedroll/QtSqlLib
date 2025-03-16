#include "Common.h"

#include <gtest/gtest.h>

namespace QtSqlLibTest
{

QString Funcs::getDefaultDatabaseFilename()
{
  return "test.db";
}

size_t Funcs::numResults(QtSqlLib::ResultSet& results)
{
  size_t counter = 0;
  results.resetIteration();
  while (results.hasNextTuple())
  {
    static_cast<void>(results.nextTuple());
    counter++;
  }
  return counter;
}

bool Funcs::isResultTuplesContaining(
  QtSqlLib::ResultSet& results, IID::Type tableId,
  IID::Type columnId, QVariant value)
{
  results.resetIteration();
  while (results.hasNextTuple())
  {
    const auto next = results.nextTuple();
    if (next.tableId() == tableId && next.hasColumnValue(columnId))
    {
      if (next.columnValue(columnId) == value)
      {
        return true;
      }
    }
  }
  return false;
}

void Funcs::expectRelations(
  QtSqlLib::ResultSet& results, IID::Type relationshipId,
  IID::Type fromTableId, IID::Type fromColId, IID::Type toTableId, IID::Type toColId,
  const QVariant& fromValue, const QVariantList& toValues)
{
  ASSERT_TRUE(isResultTuplesContaining(results, fromTableId, fromColId, fromValue));

  results.resetIteration();

  std::set<int> matchingToValuesIndices;
  size_t totalJoinedTuples = 0;

  while (results.hasNextTuple())
  {
    const auto next = results.nextTuple();
    if (next.tableId() == fromTableId && next.columnValue(fromColId) == fromValue)
    {
      while (results.hasNextJoinedTuple())
      {
        const auto nextJoined = results.nextJoinedTuple();
        if (nextJoined.relationshipId() == relationshipId && nextJoined.tableId() == toTableId)
        {
          totalJoinedTuples++;
          for (int i=0; i<toValues.size(); ++i)
          {
            if (nextJoined.columnValue(toColId) == toValues.at(i))
            {
              matchingToValuesIndices.insert(i);
              break;
            }
          }
        }
      }

      break;
    }
  }

  EXPECT_EQ(static_cast<size_t>(toValues.size()), matchingToValuesIndices.size());
  EXPECT_EQ(static_cast<size_t>(toValues.size()), totalJoinedTuples);
}

}
