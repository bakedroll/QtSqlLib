#include <gtest/gtest.h>

#include <Common.h>

#include <mocks/MockSanityChecker.h>

#include <QtSqlLib/QueryIdentifiers.h>

namespace QtSqlLibTest
{

/**
 * @test: Constructs various Expr and checks the resulting strings.
 * @expected: The strings match excactly.
 */
TEST(TextExpression, validity)
{
  auto pMockSanityChecker = std::make_unique<NiceMock<MockSanityChecker>>();
  Schema schema;
  schema.setSanityChecker(std::move(pMockSanityChecker));

  auto& table = schema.getTables()[static_cast<int>(TableIds::Table1)];
  table.name = "test";
  table.columns[static_cast<int>(Table1Cols::Id)].name = "id";
  table.columns[static_cast<int>(Table1Cols::Text)].name = "test";
  table.columns[static_cast<int>(Table1Cols::Number)].name = "number";

  QtSqlLib::QueryIdentifiers queryIdentifiers;
  queryIdentifiers.addTableIdentifier(std::nullopt, static_cast<QtSqlLib::API::IID::Type>(TableIds::Table1));

  Expr expr1;
  expr1.LESS(Table1Cols::Id, 2).AND.EQUAL(Table1Cols::Text, "test1");

  std::vector<QVariant> boundValues1;
  EXPECT_EQ(expr1.toQueryString(schema, queryIdentifiers, boundValues1), "'test'.'id' < ? AND 'test'.'test' = ?");

  ASSERT_EQ(boundValues1.size(), 2);
  EXPECT_EQ(boundValues1.at(0), 2);
  EXPECT_EQ(boundValues1.at(1), "test1");

  Expr expr2;
  expr2._(UNEQUAL(Table1Cols::Id, 2).AND.UNEQUAL(Table1Cols::Text, "test1")).OR._(GREATER(Table1Cols::Id, 3));

  std::vector<QVariant> boundValues2;
  EXPECT_EQ(expr2.toQueryString(schema, queryIdentifiers, boundValues2), "('test'.'id' != ? AND 'test'.'test' != ?) OR ('test'.'id' > ?)");

  ASSERT_EQ(boundValues2.size(), 3);
  EXPECT_EQ(boundValues2.at(0), 2);
  EXPECT_EQ(boundValues2.at(1), "test1");
  EXPECT_EQ(boundValues2.at(2), 3);

  Expr expr3;
  expr3.EQUAL(Table1Cols::Text, "test1").NOCASE;

  std::vector<QVariant> boundValues3;
  EXPECT_EQ(expr3.toQueryString(schema, queryIdentifiers, boundValues3), "'test'.'test' = ? COLLATE NOCASE");

  ASSERT_EQ(boundValues3.size(), 1);
  EXPECT_EQ(boundValues3.at(0), "test1");

  Expr expr4;
  expr4.LESS_COL(Table1Cols::Id, Table1Cols::Number);

  std::vector<QVariant> boundValues4;
  EXPECT_EQ(expr4.toQueryString(schema, queryIdentifiers, boundValues4), "'test'.'id' < 'test'.'number'");

  EXPECT_TRUE(boundValues4.empty());

  Expr expr5;
  expr5.IS(Table1Cols::Number, NULL_VAL);

  std::vector<QVariant> boundValues5;
  EXPECT_EQ(expr5.toQueryString(schema, queryIdentifiers, boundValues5), "'test'.'number' IS NULL");

  EXPECT_TRUE(boundValues5.empty());

  Expr expr6;
  expr6.NOT(Table1Cols::Number, NULL_VAL);

  std::vector<QVariant> boundValues6;
  EXPECT_EQ(expr6.toQueryString(schema, queryIdentifiers, boundValues6), "'test'.'number' NOT NULL");

  EXPECT_TRUE(boundValues6.empty());

  Expr expr7;
  expr7.LIKE(Table1Cols::Text, "foo");

  std::vector<QVariant> boundValues7;
  EXPECT_EQ(expr7.toQueryString(schema, queryIdentifiers, boundValues7), "'test'.'test' LIKE ?");

  ASSERT_EQ(boundValues7.size(), 1);
  EXPECT_EQ(boundValues7.at(0), "foo");

  Expr expr8;
  expr8.IN(Table1Cols::Text, "foo");

  std::vector<QVariant> boundValues8;
  EXPECT_EQ(expr8.toQueryString(schema, queryIdentifiers, boundValues8), "'test'.'test' IN ?");

  ASSERT_EQ(boundValues8.size(), 1);
  EXPECT_EQ(boundValues8.at(0), "foo");
}

/**w
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

  QtSqlLib::QueryIdentifiers queryIdentifiers;
  queryIdentifiers.addTableIdentifier(std::nullopt, static_cast<QtSqlLib::API::IID::Type>(TableIds::Table1));

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

  const auto assembleExpr3 = [&schema, &queryIdentifiers]()
  {
    // Expressions must not end with a logical operator
    Expr expr;
    expr.LESS(Table1Cols::Id, 2).AND;

    std::vector<QVariant> boundValues;
    expr.toQueryString(schema, queryIdentifiers, boundValues);
  };

  const auto assembleExpr4 = [&schema, &queryIdentifiers]()
  {
    // Expressions must not be empty
    const Expr expr;
    QtSqlLib::ID tid(TableIds::Table1);
    std::vector<QVariant> boundValues;
    expr.toQueryString(schema, queryIdentifiers, boundValues);
  };

  EXPECT_THROW(assembleExpr1(), DatabaseException);
  EXPECT_THROW(assembleExpr2(), DatabaseException);
  EXPECT_THROW(assembleExpr3(), DatabaseException);
  EXPECT_THROW(assembleExpr4(), DatabaseException);
}

}
