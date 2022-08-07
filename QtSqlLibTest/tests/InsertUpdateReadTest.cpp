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
  DatabaseDummy db;

  db.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(Table1Cols::Text, "text", DataType::Varchar, 128)
      .column(Table1Cols::Number, "number", DataType::Real).notNull();
  });

  db.initialize(Funcs::getDefaultDatabaseFilename());

  db.execQuery(InsertInto(TableIds::Table1)
    .value(Table1Cols::Text, "test")
    .value(Table1Cols::Number, 0.5));

  db.execQuery(BatchInsertInto(TableIds::Table1)
    .values(Table1Cols::Text, QVariantList() << "test1" << "test2" << "test3")
    .values(Table1Cols::Number, QVariantList() << 0.6 << 0.7 << 0.8));

  auto results = db.execQuery(FromTable(TableIds::Table1)
    .select(Table1Cols::Text, Table1Cols::Number));

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
    .select(Table1Cols::Id, Table1Cols::Text, Table1Cols::Number)
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
  DatabaseDummy db;

  db.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(Table1Cols::Text, "text", DataType::Varchar, 128)
      .column(Table1Cols::Number, "number", DataType::Integer);
  });

  db.initialize(Funcs::getDefaultDatabaseFilename());

  db.execQuery(BatchInsertInto(TableIds::Table1)
    .values(Table1Cols::Text, QVariantList() << "value1" << "value2" << "value3")
    .values(Table1Cols::Number, QVariantList() << 1 << 2 << 3));

  db.execQuery(UpdateTable(TableIds::Table1)
    .set(Table1Cols::Text, "value2_updated")
    .where(Expr().equal(Table1Cols::Number, QVariant(2))));

  const auto results = db.execQuery(FromTable(TableIds::Table1)
    .select(Table1Cols::Text, Table1Cols::Number));

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
  DatabaseDummy db;

  db.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "id", DataType::Integer).notNull()
      .column(Table1Cols::Text, "text", DataType::Varchar, 128).notNull()
      .primaryKeys(Table1Cols::Id, Table1Cols::Text);
  });

  db.initialize(Funcs::getDefaultDatabaseFilename());

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
  DatabaseDummy db1;
  db1.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // Two different tables must not have the same id
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "id", DataType::Integer);

    configurator.configureTable(TableIds::Table1, "table2")
      .column(Table2Cols::Id, "id", DataType::Integer);
  });

  DatabaseDummy db2;
  db2.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // Two different tables must not have the same name
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "id", DataType::Integer);

    configurator.configureTable(TableIds::Table2, "table1")
      .column(Table2Cols::Id, "id", DataType::Integer);
  });

  DatabaseDummy db3;
  db3.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // A table name must not start with 'sqlite_'
    configurator.configureTable(TableIds::Table1, "sqlite_table")
      .column(Table1Cols::Id, "id", DataType::Integer);
  });

  DatabaseDummy db4;
  db4.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // Two columns of the same table must not have the same id
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "id1", DataType::Integer)
      .column(Table1Cols::Id, "id2", DataType::Integer);
  });

  DatabaseDummy db5;
  db5.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // Two columns of the same table must not have the same name
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "col", DataType::Integer)
      .column(Table1Cols::Text, "col", DataType::Integer);
  });

  DatabaseDummy db6;
  db6.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // The varchar length parameter must be greater than 0 for varchar attribute
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Text, "text", DataType::Varchar, 0);
  });

  DatabaseDummy db7;
  db7.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // Only one attribute can be the primary key
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "Id", DataType::Integer).primaryKey()
      .column(Table1Cols::Text, "text", DataType::Varchar).primaryKey();
  });

  DatabaseDummy db8;
  db8.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // primaryKey() must not be called multiple times for a column
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "Id", DataType::Integer).primaryKey().primaryKey();
  });

  DatabaseDummy db9;
  db9.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // autoIncrement() must not be called multiple times for a column
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "Id", DataType::Integer).autoIncrement().autoIncrement();
  });

  DatabaseDummy db10;
  db10.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // notNull() must not be called multiple times for a column
    configurator.configureTable(TableIds::Table1, "table1")
      .column(Table1Cols::Id, "Id", DataType::Integer).notNull().notNull();
  });

  DatabaseDummy db11;
  db11.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // primaryKey(), autoIncrement() and notNoll() must be called after column()
    configurator.configureTable(TableIds::Table1, "table1")
      .primaryKey();
  });

  DatabaseDummy db12;
  db12.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // column name must not ne empty
    configurator.configureTable(TableIds::Table1, "table1").
      column(Table1Cols::Id, "", DataType::Integer);
  });

  DatabaseDummy db13;
  db13.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // table name must not ne empty
    configurator.configureTable(TableIds::Table1, "");
  });

  const auto filename = Funcs::getDefaultDatabaseFilename();

  EXPECT_THROW(db1.initialize(filename), DatabaseException);
  EXPECT_THROW(db2.initialize(filename), DatabaseException);
  EXPECT_THROW(db3.initialize(filename), DatabaseException);
  EXPECT_THROW(db4.initialize(filename), DatabaseException);
  EXPECT_THROW(db5.initialize(filename), DatabaseException);
  EXPECT_THROW(db6.initialize(filename), DatabaseException);
  EXPECT_THROW(db7.initialize(filename), DatabaseException);
  EXPECT_THROW(db8.initialize(filename), DatabaseException);
  EXPECT_THROW(db9.initialize(filename), DatabaseException);
  EXPECT_THROW(db10.initialize(filename), DatabaseException);
  EXPECT_THROW(db11.initialize(filename), DatabaseException);
  EXPECT_THROW(db12.initialize(filename), DatabaseException);
  EXPECT_THROW(db13.initialize(filename), DatabaseException);
}

/**
 * @test: Creates some invalid FromTable queries.
 * @expected: Exceptions will be thrown.
 */
TEST(InsertUpdateReadTest, fromTableExceptions)
{
  EXPECT_THROW(
    FromTable(TableIds::Students).select(StudentsCols::Name).select(StudentsCols::Id),
    DatabaseException);

  EXPECT_THROW(
    FromTable(TableIds::Students).select(StudentsCols::Name).selectAll(),
    DatabaseException);

  EXPECT_THROW(
    FromTable(TableIds::Students).selectAll().selectAll(),
    DatabaseException);

  EXPECT_THROW(
    FromTable(TableIds::Students)
      .joinColumns(Relationships::StudentsProjects, StudentsCols::Name)
      .joinColumns(Relationships::StudentsProjects, StudentsCols::Id),
    DatabaseException);

  EXPECT_THROW(
    FromTable(TableIds::Students)
      .joinColumns(Relationships::StudentsProjects, StudentsCols::Name)
      .joinAll(Relationships::StudentsProjects),
    DatabaseException);

  EXPECT_THROW(
    FromTable(TableIds::Students)
      .joinAll(Relationships::StudentsProjects)
      .joinAll(Relationships::StudentsProjects),
    DatabaseException);
}

}
