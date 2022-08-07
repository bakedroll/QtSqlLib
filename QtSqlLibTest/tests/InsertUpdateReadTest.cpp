#include <gtest/gtest.h>

#include <Common.h>

namespace QtSqlLibTest
{

/**
 * @test: Creates a single table and executes each of the insert queries InsertInto and BatchInsertInto to insert four tuples.
 *        Then uses FromTable to query all inserted tuples in one case and all tuples where number is less than 0.7 in another case.
 * @expected: No exceptions occur.
 *            The first FromTable query delivers four results with correct values.
 *            The second FromTable query delivers three results with correct values.
 */
TEST(InsertUpdateReadTest, insertAndRead)
{
  SchemaConfigurator configurator;
  configurator.configureTable(TableIds::Table1, "table1")
    .column(Table1Cols::Id, "id", DataType::Integer).primaryKey().autoIncrement().notNull()
    .column(Table1Cols::Text, "text", DataType::Varchar, 128)
    .column(Table1Cols::Number, "number", DataType::Real).notNull();

  TestDatabase db;
  db.initialize(configurator, Funcs::getDefaultDatabaseFilename());

  db.execQuery(InsertInto(TableIds::Table1)
    .value(Table1Cols::Text, "test")
    .value(Table1Cols::Number, 0.5));

  db.execQuery(BatchInsertInto(TableIds::Table1)
    .values(Table1Cols::Text, QVariantList() << "test1" << "test2" << "test3")
    .values(Table1Cols::Number, QVariantList() << 0.6 << 0.7 << 0.8));

  auto results = db.execQuery(FromTable(TableIds::Table1)
    .select({ Table1Cols::Text, Table1Cols::Number }));

  EXPECT_EQ(results.resultTuples.size(), 4);

  EXPECT_TRUE(Funcs::isResultTuplesContaining(results.resultTuples, TableIds::Table1, Table1Cols::Text, "test"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results.resultTuples, TableIds::Table1, Table1Cols::Text, "test1"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results.resultTuples, TableIds::Table1, Table1Cols::Text, "test2"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results.resultTuples, TableIds::Table1, Table1Cols::Text, "test3"));

  auto resultColText = results.resultTuples[0].values.at({ TableIds::Table1, Table1Cols::Text });
  auto resultColNr = results.resultTuples[0].values.at({ TableIds::Table1, Table1Cols::Number });

  EXPECT_EQ(resultColText.userType(), QMetaType::QString);
  EXPECT_EQ(resultColNr.userType(), QMetaType::Double);

  EXPECT_EQ(resultColText.toString(), "test");
  EXPECT_DOUBLE_EQ(resultColNr.toDouble(), 0.5);

  results = db.execQuery(FromTable(TableIds::Table1)
    .select({ Table1Cols::Id, Table1Cols::Text, Table1Cols::Number })
    .where(Expr().less(Table1Cols::Number, QVariant(0.75))));

  EXPECT_EQ(results.resultTuples.size(), 3);

  EXPECT_TRUE(Funcs::isResultTuplesContaining(results.resultTuples, TableIds::Table1, Table1Cols::Text, "test"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results.resultTuples, TableIds::Table1, Table1Cols::Text, "test1"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results.resultTuples, TableIds::Table1, Table1Cols::Text, "test2"));

  for (const auto& result : results.resultTuples)
  {
    const auto resultColId = result.values.at({ TableIds::Table1, Table1Cols::Id });
    resultColText = result.values.at({ TableIds::Table1, Table1Cols::Text });
    resultColNr = result.values.at({ TableIds::Table1, Table1Cols::Number });

    EXPECT_EQ(resultColId.userType(), QMetaType::LongLong);
    EXPECT_EQ(resultColText.userType(), QMetaType::QString);
    EXPECT_EQ(resultColNr.userType(), QMetaType::Double);

    EXPECT_LT(resultColNr.toDouble(), 0.75);
  }
}

/**
 * @test: Creates a single table and inserts some values, then updates a single value with UpdateTable.
 * @expected: No exceptions occur.
 *            The table contains the updated value.
 */
TEST(InsertUpdateReadTest, insertUpdateAndRead)
{
  SchemaConfigurator configurator;
  configurator.configureTable(TableIds::Table1, "table1")
    .column(Table1Cols::Id, "id", DataType::Integer).primaryKey().autoIncrement().notNull()
    .column(Table1Cols::Text, "text", DataType::Varchar, 128)
    .column(Table1Cols::Number, "number", DataType::Integer);

  TestDatabase db;
  db.initialize(configurator, Funcs::getDefaultDatabaseFilename());

  db.execQuery(BatchInsertInto(TableIds::Table1)
    .values(Table1Cols::Text, QVariantList() << "value1" << "value2" << "value3")
    .values(Table1Cols::Number, QVariantList() << 1 << 2 << 3));

  db.execQuery(UpdateTable(TableIds::Table1)
    .set(Table1Cols::Text, "value2_updated")
    .where(Expr().equal(Table1Cols::Number, QVariant(2))));

  const auto results = db.execQuery(FromTable(TableIds::Table1)
    .select({ Table1Cols::Text, Table1Cols::Number }));

  EXPECT_EQ(results.resultTuples.size(), 3);

  EXPECT_TRUE(Funcs::isResultTuplesContaining(results.resultTuples, TableIds::Table1, Table1Cols::Text, "value1"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results.resultTuples, TableIds::Table1, Table1Cols::Text, "value2_updated"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results.resultTuples, TableIds::Table1, Table1Cols::Text, "value3"));
}

/**
 * @test: Creates a single table with a primary key that is defined by two columns and inserts a value.
 * @expected: No exceptions occur.
 *            The resulting tuple key after the insert query operation is valid.
 */
TEST(InsertUpdateReadTest, multiplePrimaryKeysTable)
{
  SchemaConfigurator configurator;
  configurator.configureTable(TableIds::Table1, "table1")
    .column(Table1Cols::Id, "id", DataType::Integer).notNull()
    .column(Table1Cols::Text, "text", DataType::Varchar, 128).notNull()
    .primaryKeys({ Table1Cols::Id, Table1Cols::Text });

  TestDatabase db;
  db.initialize(configurator, Funcs::getDefaultDatabaseFilename());

  const auto results = db.execQuery(InsertIntoExt(TableIds::Table1)
    .value(Table1Cols::Id, 1)
    .value(Table1Cols::Text, "text")
    .returnIds());

  EXPECT_EQ(results.resultTuples.size(), 1);

  const auto resultId = results.resultTuples[0].values.at({ TableIds::Table1, Table1Cols::Id });
  const auto resultText = results.resultTuples[0].values.at({ TableIds::Table1, Table1Cols::Text });

  EXPECT_EQ(resultId.userType(), QMetaType::LongLong);
  EXPECT_EQ(resultText.userType(), QMetaType::QString);

  EXPECT_EQ(resultId.toLongLong(), 1);
  EXPECT_EQ(resultText.toString(), "text");
}

/**
 * @test: Initializes some databases with invalid schema configurations
 * @expected: Exceptions will be thrown.
 */
TEST(InsertUpdateReadTest, databaseImitializationExceptions)
{
  // Two different tables must not have the same id
  const auto case1 = []()
  {
    SchemaConfigurator configurator;
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "id", DataType::Integer);

    configurator.configureTable(TableIds::Table1, "table2")
      .column(Table2Cols::Id, "id", DataType::Integer);

    TestDatabase db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // Two different tables must not have the same name
  const auto case2 = []()
  {
    SchemaConfigurator configurator;
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "id", DataType::Integer);

    configurator.configureTable(TableIds::Table2, "table1")
      .column(Table2Cols::Id, "id", DataType::Integer);

    TestDatabase db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // A table name must not start with 'sqlite_'
  const auto case3 = []()
  {
    SchemaConfigurator configurator;
    configurator.configureTable(TableIds::Table1, "sqlite_table")
      .column(Table1Cols::Id, "id", DataType::Integer);

    TestDatabase db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // Two columns of the same table must not have the same id
  const auto case4 = []()
  {
    SchemaConfigurator configurator;
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "id1", DataType::Integer)
      .column(Table1Cols::Id, "id2", DataType::Integer);

    TestDatabase db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // Two columns of the same table must not have the same name
  const auto case5 = []()
  {
    SchemaConfigurator configurator;
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "col", DataType::Integer)
      .column(Table1Cols::Text, "col", DataType::Integer);

    TestDatabase db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // The varchar length parameter must be greater than 0 for varchar attribute
  const auto case6 = []()
  {
    SchemaConfigurator configurator;
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Text, "text", DataType::Varchar, 0);

    TestDatabase db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // Only one attribute can be the primary key
  const auto case7 = []()
  {
    SchemaConfigurator configurator;
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "Id", DataType::Integer).primaryKey()
      .column(Table1Cols::Text, "text", DataType::Varchar).primaryKey();

    TestDatabase db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // primaryKey() must not be called multiple times for a column
  const auto case8 = []()
  {
    SchemaConfigurator configurator;
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "Id", DataType::Integer).primaryKey().primaryKey();

    TestDatabase db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // autoIncrement() must not be called multiple times for a column
  const auto case9 = []()
  {
    SchemaConfigurator configurator;
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "Id", DataType::Integer).autoIncrement().autoIncrement();

    TestDatabase db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // notNull() must not be called multiple times for a column
  const auto case10 = []()
  {
    SchemaConfigurator configurator;
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "Id", DataType::Integer).notNull().notNull();

    TestDatabase db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // primaryKey(), autoIncrement() and notNoll() must be called after column()
  const auto case11 = []()
  {
    SchemaConfigurator configurator;
    configurator.configureTable(TableIds::Table1, "table1")
      .primaryKey();

    TestDatabase db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // column name must not ne empty
  const auto case12 = []()
  {
    SchemaConfigurator configurator;
    configurator.configureTable(TableIds::Table1, "table1").
      column(Table1Cols::Id, "", DataType::Integer);

    TestDatabase db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // table name must not ne empty
  const auto case13 = []()
  {
    SchemaConfigurator configurator;
    configurator.configureTable(TableIds::Table1, "");

    TestDatabase db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  EXPECT_THROW(case1(), DatabaseException);
  EXPECT_THROW(case2(), DatabaseException);
  EXPECT_THROW(case3(), DatabaseException);
  EXPECT_THROW(case4(), DatabaseException);
  EXPECT_THROW(case5(), DatabaseException);
  EXPECT_THROW(case6(), DatabaseException);
  EXPECT_THROW(case7(), DatabaseException);
  EXPECT_THROW(case8(), DatabaseException);
  EXPECT_THROW(case9(), DatabaseException);
  EXPECT_THROW(case10(), DatabaseException);
  EXPECT_THROW(case11(), DatabaseException);
  EXPECT_THROW(case12(), DatabaseException);
  EXPECT_THROW(case13(), DatabaseException);
}

/**
 * @test: Creates some invalid FromTable queries.
 * @expected: Exceptions will be thrown.
 */
TEST(InsertUpdateReadTest, fromTableExceptions)
{
  EXPECT_THROW(
    FromTable(TableIds::Students).select({ StudentsCols::Name }).select({ StudentsCols::Id }),
    DatabaseException);

  EXPECT_THROW(
    FromTable(TableIds::Students).select({ StudentsCols::Name }).selectAll(),
    DatabaseException);

  EXPECT_THROW(
    FromTable(TableIds::Students).selectAll().selectAll(),
    DatabaseException);

  EXPECT_THROW(
    FromTable(TableIds::Students)
      .joinColumns(Relationships::StudentsProjects, { StudentsCols::Name })
      .joinColumns(Relationships::StudentsProjects, { StudentsCols::Id }),
    DatabaseException);

  EXPECT_THROW(
    FromTable(TableIds::Students)
      .joinColumns(Relationships::StudentsProjects, { StudentsCols::Name })
      .joinAll(Relationships::StudentsProjects),
    DatabaseException);

  EXPECT_THROW(
    FromTable(TableIds::Students)
      .joinAll(Relationships::StudentsProjects)
      .joinAll(Relationships::StudentsProjects),
    DatabaseException);
}

}
