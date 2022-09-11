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
  table.columns[Table1Cols::Number].name = "number";

  Expr expr1;
  expr1.LESS(Table1Cols::Id, 2).AND.EQUAL(Table1Cols::Text, "test1");

  QtSqlLib::ID tid(TableIds::Table1);
  EXPECT_EQ(expr1.toQString(schema, tid), "'test'.'id' < 2 AND 'test'.'test' == 'test1'");

  Expr expr2;
  expr2._(UNEQUAL(Table1Cols::Id, 2).AND.UNEQUAL(Table1Cols::Text, "test1")).OR._(GREATER(Table1Cols::Id, 3));

  EXPECT_EQ(expr2.toQString(schema, tid), "('test'.'id' != 2 AND 'test'.'test' != 'test1') OR ('test'.'id' > 3)");

  Expr expr3;
  expr3.LESS_COL(Table1Cols::Id, Table1Cols::Number);

  EXPECT_EQ(expr3.toQString(schema, tid), "'test'.'id' < 'test'.'number'");
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
    expr.toQString(schema, tid);
  };

  const auto assembleExpr4 = [&schema]()
  {
    // Expressions must not be empty
    const Expr expr;
    QtSqlLib::ID tid(TableIds::Table1);
    expr.toQString(schema, tid);
  };

  EXPECT_THROW(assembleExpr1(), DatabaseException);
  EXPECT_THROW(assembleExpr2(), DatabaseException);
  EXPECT_THROW(assembleExpr3(), DatabaseException);
  EXPECT_THROW(assembleExpr4(), DatabaseException);
}

}
