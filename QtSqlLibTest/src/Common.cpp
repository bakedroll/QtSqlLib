#include "Common.h"

#include <gtest/gtest.h>

namespace QtSqlLibTest
{

QString Funcs::getDefaultDatabaseFilename()
{
  return "test.db";
}

bool Funcs::isResultTuplesContaining(const QtSqlLib::ResultSet& results, const IID& tableId,
                                     const IID& columnId, QVariant value)
{
  results.resetIteration();
  const ISchema::TableColumnId colId{ tableId.get(), columnId.get() };
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

void Funcs::expectRelations(const QtSqlLib::ResultSet& results, const IID& relationshipId,
                            const IID& fromTableId, const IID& fromColId, const IID& toTableId, const IID& toColId,
                            const QVariant& fromValue, const QVariantList& toValues)
{
  ASSERT_TRUE(isResultTuplesContaining(results, fromTableId, fromColId, fromValue));

  results.resetIteration();

  std::set<int> matchingToValuesIndices;

  const ISchema::TableColumnId colId{ fromTableId.get(), fromColId.get() };
  const ISchema::TableColumnId valueColId{ toTableId.get(), toColId.get() };

  while (results.hasNext())
  {
    const auto& next = results.next();
    if (next.at(colId) == fromValue)
    {
      EXPECT_EQ(results.getCurrentNumJoinedResults(relationshipId.get()), toValues.size());
      while (results.hasNextJoinedTuple(relationshipId.get()))
      {
        const auto& joinedTuple = results.nextJoinedTuple(relationshipId.get());
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
