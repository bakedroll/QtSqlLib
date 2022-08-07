#include <gtest/gtest.h>

#include <Common.h>

namespace QtSqlLibTest
{

/**
 * @test: Constructs various Expr and checks the resulting strings.
 * @expected: The strings match excactly.
 */
TEST(ExpressionTest, validity)
{
  Schema schema;

  auto& table = schema.getTables()[TableIds::Table1];
  table.name = "test";
  table.columns[Table1Cols::Id].name = "id";
  table.columns[Table1Cols::Text].name = "test";

  Expr expr1;
  expr1
    .less(Table1Cols::Id, QVariant(2))
    .and()
    .equal(Table1Cols::Text, "test1");

  EXPECT_EQ(expr1.toQString(schema, TableIds::Table1), "'test'.'id' < 2 AND 'test'.'test' == 'test1'");

  Expr expr2;
  expr2
    .braces(Expr().unequal(Table1Cols::Id, QVariant(2)).and().unequal(Table1Cols::Text, "test1"))
    .or()
    .braces(Expr().greater(Table1Cols::Id, QVariant(3)));

  EXPECT_EQ(expr2.toQString(schema, TableIds::Table1), "('test'.'id' != 2 AND 'test'.'test' != 'test1') OR ('test'.'id' > 3)");
}

/**
 * @test: Constructs various invalid expressions.
 * @expected: Exceptions will be thrown.
 */
TEST(ExpressionTest, exceptions)
{
  Schema schema;

  auto& table = schema.getTables()[TableIds::Table1];
  table.name = "test";
  table.columns[Table1Cols::Id].name = "id";
  table.columns[Table1Cols::Text].name = "test";

  const auto assembleExpr1 = []()
  {
    // Conditions must not be consecutive
    Expr expr;
    expr
      .less(Table1Cols::Id, QVariant(2))
      .equal(Table1Cols::Text, "test1");
  };

  const auto assembleExpr2 = []()
  {
    // Expression must not begin with a logical operator
    Expr expr;
    expr
      .or()
      .less(Table1Cols::Id, QVariant(2));
  };

  const auto assembleExpr3 = [&schema]()
  {
    // Expressions must not end with a logical operator
    Expr expr;
    expr
      .less(Table1Cols::Id, QVariant(2))
      .and();

    expr.toQString(schema, TableIds::Table1);
  };

  const auto assembleExpr4 = [&schema]()
  {
    // Expressions must not be empty
    const Expr expr;
    expr.toQString(schema, TableIds::Table1);
  };

  EXPECT_THROW(assembleExpr1(), DatabaseException);
  EXPECT_THROW(assembleExpr2(), DatabaseException);
  EXPECT_THROW(assembleExpr3(), DatabaseException);
  EXPECT_THROW(assembleExpr4(), DatabaseException);
}

}
