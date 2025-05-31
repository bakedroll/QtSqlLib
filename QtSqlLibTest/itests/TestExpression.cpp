#include <gtest/gtest.h>

#include <Common.h>

#include <mocks/MockSanityChecker.h>

namespace QtSqlLibTest
{

/**
 * @test: Constructs various Expr and checks the resulting strings.
 * @expected: The strings match excactly.
 */
TEST(TextExpression, validity)
{
  auto sanityChecker = std::make_unique<NiceMock<MockSanityChecker>>();
  Schema schema;
  schema.setSanityChecker(std::move(sanityChecker));

  auto& table = schema.getTables()[static_cast<int>(TableIds::Table1)];
  table.name = "test";
  table.columns[static_cast<int>(Table1Cols::Id)].name = "id";
  table.columns[static_cast<int>(Table1Cols::Text)].name = "test";
  table.columns[static_cast<int>(Table1Cols::Number)].name = "number";

  Expr expr1;
  expr1.LESS(Table1Cols::Id, 2).AND.EQUAL(Table1Cols::Text, "test1");

  QtSqlLib::ID tid(TableIds::Table1);
  std::vector<QVariant> boundValues1;
  EXPECT_EQ(expr1.toQueryString(schema, boundValues1, tid), "'test'.'id' < ? AND 'test'.'test' == ?");

  ASSERT_EQ(boundValues1.size(), 2);
  EXPECT_EQ(boundValues1.at(0), 2);
  EXPECT_EQ(boundValues1.at(1), "test1");

  Expr expr2;
  expr2._(UNEQUAL(Table1Cols::Id, 2).AND.UNEQUAL(Table1Cols::Text, "test1")).OR._(GREATER(Table1Cols::Id, 3));

  std::vector<QVariant> boundValues2;
  EXPECT_EQ(expr2.toQueryString(schema, boundValues2, tid), "('test'.'id' != ? AND 'test'.'test' != ?) OR ('test'.'id' > ?)");

  ASSERT_EQ(boundValues2.size(), 3);
  EXPECT_EQ(boundValues2.at(0), 2);
  EXPECT_EQ(boundValues2.at(1), "test1");
  EXPECT_EQ(boundValues2.at(2), 3);

  Expr expr3;
  expr3.LESS_COL(Table1Cols::Id, Table1Cols::Number);

  std::vector<QVariant> boundValues3;
  EXPECT_EQ(expr3.toQueryString(schema, boundValues3, tid), "'test'.'id' < 'test'.'number'");

  EXPECT_TRUE(boundValues3.empty());
}

/**
 * @test: Constructs various invalid expressions.
 * @expected: Exceptions will be thrown.
 */
TEST(TextExpression, exceptions)
{
  auto sanityChecker = std::make_unique<NiceMock<MockSanityChecker>>();
  Schema schema;
  schema.setSanityChecker(std::move(sanityChecker));

  auto& table = schema.getTables()[static_cast<int>(TableIds::Table1)];
  table.name = "test";
  table.columns[static_cast<int>(Table1Cols::Id)].name = "id";
  table.columns[static_cast<int>(Table1Cols::Text)].name = "test";

  const auto assembleExpr1 = []()
  {
    // Conditions must not be consecutive
    Expr expr;
    expr.LESS(Table1Cols::Id, 2).EQUAL(Table1Cols::Text, "test1");
  };

  const auto assembleExpr2 = []()
  {
    // Expression must not begin with a logical operator
    Expr expr;
    expr.OR.LESS(Table1Cols::Id, 2);
  };

  const auto assembleExpr3 = [&schema]()
  {
    // Expressions must not end with a logical operator
    Expr expr;
    expr.LESS(Table1Cols::Id, 2).AND;

    QtSqlLib::ID tid(TableIds::Table1);
    std::vector<QVariant> boundValues;
    expr.toQueryString(schema, boundValues, tid);
  };

  const auto assembleExpr4 = [&schema]()
  {
    // Expressions must not be empty
    const Expr expr;
    QtSqlLib::ID tid(TableIds::Table1);
    std::vector<QVariant> boundValues;
    expr.toQueryString(schema, boundValues, tid);
  };

  EXPECT_THROW(assembleExpr1(), DatabaseException);
  EXPECT_THROW(assembleExpr2(), DatabaseException);
  EXPECT_THROW(assembleExpr3(), DatabaseException);
  EXPECT_THROW(assembleExpr4(), DatabaseException);
}

}
