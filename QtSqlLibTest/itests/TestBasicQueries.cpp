#include <gtest/gtest.h>

#include <Common.h>

#include <QFile>

namespace QtSqlLibTest
{

class TestBasicQueries : public testing::Test
{
public:
  ~TestBasicQueries() override
  {
    m_db.close();
    QFile::remove(Funcs::getDefaultDatabaseFilename());
  }

  QtSqlLib::Database m_db;

};

/**
 * @test: Creates a single table and executes each of the insert queries InsertInto and BatchInsertInto to insert four tuples.
 *        Then uses FromTable to query all inserted tuples in one case and all tuples where number is less than 0.7 in another case.
 * @expected: No exceptions occur.
 *            The first FromTable query delivers four results with correct values.
 *            The second FromTable query delivers three results with correct values.
 */
TEST_F(TestBasicQueries, insertAndRead)
{
  SchemaConfigurator configurator;
  configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
    .COLUMN(Table1Cols::Id, "id", DataType::Integer).primaryKey().autoIncrement().notNull()
    .COLUMN_VARCHAR(Table1Cols::Text, "text", 128)
    .COLUMN(Table1Cols::Number, "number", DataType::Real).notNull();

  m_db.initialize(configurator, Funcs::getDefaultDatabaseFilename());

  m_db.execQuery(INSERT_INTO(TableIds::Table1)
    .VALUE(Table1Cols::Text, "test")
    .VALUE(Table1Cols::Number, 0.5));

  m_db.execQuery(BATCH_INSERT_INTO(TableIds::Table1)
    .VALUES(Table1Cols::Text, QVariantList() << "test1" << "test2" << "test3")
    .VALUES(Table1Cols::Number, QVariantList() << 0.6 << 0.7 << 0.8));

  auto results = m_db.execQuery(FROM_TABLE(TableIds::Table1)
    .SELECT(Table1Cols::Text, Table1Cols::Number));

  EXPECT_EQ(results.getNumResults(), 4);

  EXPECT_TRUE(Funcs::isResultTuplesContaining(results, TableIds::Table1, Table1Cols::Text, "test"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results, TableIds::Table1, Table1Cols::Text, "test1"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results, TableIds::Table1, Table1Cols::Text, "test2"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results, TableIds::Table1, Table1Cols::Text, "test3"));

  results.resetIteration();
  const auto& tuple = results.next();
  auto resultColText = tuple.at({ static_cast<int>(TableIds::Table1), static_cast<int>(Table1Cols::Text) });
  auto resultColNr = tuple.at({ static_cast<int>(TableIds::Table1), static_cast<int>(Table1Cols::Number) });

  EXPECT_EQ(resultColText.userType(), QMetaType::QString);
  EXPECT_EQ(resultColNr.userType(), QMetaType::Double);

  EXPECT_EQ(resultColText.toString(), "test");
  EXPECT_DOUBLE_EQ(resultColNr.toDouble(), 0.5);

  results = m_db.execQuery(FROM_TABLE(TableIds::Table1)
    .SELECT(Table1Cols::Id, Table1Cols::Text, Table1Cols::Number)
    .WHERE(LESS(Table1Cols::Number, 0.75)));

  EXPECT_EQ(results.getNumResults(), 3);

  EXPECT_TRUE(Funcs::isResultTuplesContaining(results, TableIds::Table1, Table1Cols::Text, "test"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results, TableIds::Table1, Table1Cols::Text, "test1"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results, TableIds::Table1, Table1Cols::Text, "test2"));

  results.resetIteration();
  while (results.hasNext())
  {
    const auto& nextTuple = results.next();

    const auto resultColId = nextTuple.at({ static_cast<int>(TableIds::Table1), static_cast<int>(Table1Cols::Id) });
    resultColText = nextTuple.at({ static_cast<int>(TableIds::Table1), static_cast<int>(Table1Cols::Text) });
    resultColNr = nextTuple.at({ static_cast<int>(TableIds::Table1), static_cast<int>(Table1Cols::Number) });

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
TEST_F(TestBasicQueries, insertUpdateAndRead)
{
  SchemaConfigurator configurator;
  configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
    .COLUMN(Table1Cols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(Table1Cols::Text, "text", 128)
    .COLUMN(Table1Cols::Number, "number", DataType::Integer);

  m_db.initialize(configurator, Funcs::getDefaultDatabaseFilename());

  m_db.execQuery(BATCH_INSERT_INTO(TableIds::Table1)
    .VALUES(Table1Cols::Text, QVariantList() << "value1" << "value2" << "value3")
    .VALUES(Table1Cols::Number, QVariantList() << 1 << 2 << 3));

  m_db.execQuery(UPDATE_TABLE(TableIds::Table1)
    .SET(Table1Cols::Text, "value2_updated")
    .WHERE(EQUAL(Table1Cols::Number, 2)));

  const auto results = m_db.execQuery(FROM_TABLE(TableIds::Table1)
    .SELECT(Table1Cols::Text, Table1Cols::Number));

  EXPECT_EQ(results.getNumResults(), 3);

  EXPECT_TRUE(Funcs::isResultTuplesContaining(results, TableIds::Table1, Table1Cols::Text, "value1"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results, TableIds::Table1, Table1Cols::Text, "value2_updated"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results, TableIds::Table1, Table1Cols::Text, "value3"));
}

/**
 * @test: Creates a single table and inserts some values, then some values will be deletes.
 * @expected: No exceptions occur.
 *            The tuples were deleted as expected.
 */
TEST_F(TestBasicQueries, insertAndDelete)
{
  SchemaConfigurator configurator;
  configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
    .COLUMN(Table1Cols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(Table1Cols::Text, "text", 128)
    .COLUMN(Table1Cols::Number, "number", DataType::Integer);

  m_db.initialize(configurator, Funcs::getDefaultDatabaseFilename());

  m_db.execQuery(BATCH_INSERT_INTO(TableIds::Table1)
    .VALUES(Table1Cols::Text, QVariantList() << "value1" << "value2" << "value3" << "value3")
    .VALUES(Table1Cols::Number, QVariantList() << 1 << 2 << 3 << 4));

  m_db.execQuery(DELETE_FROM(TableIds::Table1)
    .WHERE(EQUAL(Table1Cols::Text, "value1").OR.GREATEREQUAL(Table1Cols::Number, 3)));

  const auto results = m_db.execQuery(FROM_TABLE(TableIds::Table1)
    .SELECT(Table1Cols::Text, Table1Cols::Number));

  EXPECT_EQ(results.getNumResults(), 1);

  EXPECT_FALSE(Funcs::isResultTuplesContaining(results, TableIds::Table1, Table1Cols::Text, "value1"));
  EXPECT_TRUE(Funcs::isResultTuplesContaining(results, TableIds::Table1, Table1Cols::Text, "value2"));
  EXPECT_FALSE(Funcs::isResultTuplesContaining(results, TableIds::Table1, Table1Cols::Text, "value3"));
  EXPECT_FALSE(Funcs::isResultTuplesContaining(results, TableIds::Table1, Table1Cols::Text, "value4"));
}

/**
 * @test: Creates a single table with a primary key that is defined by two columns and inserts a value.
 * @expected: No exceptions occur.
 *            The resulting tuple key after the insert query operation is valid.
 */
TEST_F(TestBasicQueries, multiplePrimaryKeysTable)
{
  SchemaConfigurator configurator;
  configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
    .COLUMN(Table1Cols::Id, "id", DataType::Integer).NOT_NULL
    .COLUMN_VARCHAR(Table1Cols::Text, "text", 128).NOT_NULL
    .PRIMARY_KEYS(Table1Cols::Id, Table1Cols::Text);

  m_db.initialize(configurator, Funcs::getDefaultDatabaseFilename());

  const auto results = m_db.execQuery(INSERT_INTO_EXT(TableIds::Table1)
    .VALUE(Table1Cols::Id, 1)
    .VALUE(Table1Cols::Text, "text")
    .RETURN_IDS);

  EXPECT_EQ(results.getNumResults(), 1);

  const auto& tuple = results.next();
  const auto resultId = tuple.at({ static_cast<int>(TableIds::Table1), static_cast<int>(Table1Cols::Id) });
  const auto resultText = tuple.at({ static_cast<int>(TableIds::Table1), static_cast<int>(Table1Cols::Text) });

  EXPECT_EQ(resultId.userType(), QMetaType::LongLong);
  EXPECT_EQ(resultText.userType(), QMetaType::QString);

  EXPECT_EQ(resultId.toLongLong(), 1);
  EXPECT_EQ(resultText.toString(), "text");
}

/**
 * @test: Initializes some databases with invalid schema configurations
 * @expected: Exceptions will be thrown.
 */
TEST_F(TestBasicQueries, databaseInitializationExceptions)
{
  // Two different tables must not have the same id
  const auto case1 = []()
  {
    SchemaConfigurator configurator;
    configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
      .COLUMN(Table1Cols::Id, "id", DataType::Integer);

    configurator.CONFIGURE_TABLE(TableIds::Table1, "table2")
      .COLUMN(Table2Cols::Id, "id", DataType::Integer);

    QtSqlLib::Database db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // Two different tables must not have the same name
  const auto case2 = []()
  {
    SchemaConfigurator configurator;
    configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
      .COLUMN(Table1Cols::Id, "id", DataType::Integer);

    configurator.CONFIGURE_TABLE(TableIds::Table2, "table1")
      .COLUMN(Table2Cols::Id, "id", DataType::Integer);

    QtSqlLib::Database db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // A table name must not start with 'sqlite_'
  const auto case3 = []()
  {
    SchemaConfigurator configurator;
    configurator.CONFIGURE_TABLE(TableIds::Table1, "sqlite_table")
      .COLUMN(Table1Cols::Id, "id", DataType::Integer);

    QtSqlLib::Database db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // Two columns of the same table must not have the same id
  const auto case4 = []()
  {
    SchemaConfigurator configurator;
    configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
      .COLUMN(Table1Cols::Id, "id1", DataType::Integer)
      .COLUMN(Table1Cols::Id, "id2", DataType::Integer);

    QtSqlLib::Database db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // Two columns of the same table must not have the same name
  const auto case5 = []()
  {
    SchemaConfigurator configurator;
    configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
      .COLUMN(Table1Cols::Id, "col", DataType::Integer)
      .COLUMN(Table1Cols::Text, "col", DataType::Integer);

    QtSqlLib::Database db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // The varchar length parameter must be greater than 0 for varchar attribute
  const auto case6 = []()
  {
    SchemaConfigurator configurator;
    configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
      .COLUMN_VARCHAR(Table1Cols::Text, "text", 0);

    QtSqlLib::Database db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // Only one attribute can be the primary key
  const auto case7 = []()
  {
    SchemaConfigurator configurator;
    configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
      .COLUMN(Table1Cols::Id, "Id", DataType::Integer).PRIMARY_KEY
      .COLUMN(Table1Cols::Text, "text", DataType::Varchar).PRIMARY_KEY;

    QtSqlLib::Database db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // primaryKey() must not be called multiple times for a column
  const auto case8 = []()
  {
    SchemaConfigurator configurator;
    configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
      .COLUMN(Table1Cols::Id, "Id", DataType::Integer).primaryKey().PRIMARY_KEY;

    QtSqlLib::Database db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // autoIncrement() must not be called multiple times for a column
  const auto case9 = []()
  {
    SchemaConfigurator configurator;
    configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
      .COLUMN(Table1Cols::Id, "Id", DataType::Integer).AUTO_INCREMENT.AUTO_INCREMENT;

    QtSqlLib::Database db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // notNull() must not be called multiple times for a column
  const auto case10 = []()
  {
    SchemaConfigurator configurator;
    configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
      .COLUMN(Table1Cols::Id, "Id", DataType::Integer).NOT_NULL.NOT_NULL;

    QtSqlLib::Database db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // primaryKey(), autoIncrement() and notNoll() must be called after column()
  const auto case11 = []()
  {
    SchemaConfigurator configurator;
    configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
      .primaryKey();

    QtSqlLib::Database db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // column name must not ne empty
  const auto case12 = []()
  {
    SchemaConfigurator configurator;
    configurator.CONFIGURE_TABLE(TableIds::Table1, "table1").
      COLUMN(Table1Cols::Id, "", DataType::Integer);

    QtSqlLib::Database db;
    db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
  };

  // table name must not ne empty
  const auto case13 = []()
  {
    SchemaConfigurator configurator;
    configurator.CONFIGURE_TABLE(TableIds::Table1, "");

    QtSqlLib::Database db;
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
TEST_F(TestBasicQueries, fromTableExceptions)
{
  EXPECT_THROW(
    FROM_TABLE(TableIds::Students).SELECT(StudentsCols::Name).SELECT(StudentsCols::Id),
    DatabaseException);

  EXPECT_THROW(
    FROM_TABLE(TableIds::Students).SELECT(StudentsCols::Name).SELECT_ALL,
    DatabaseException);

  EXPECT_THROW(
    FROM_TABLE(TableIds::Students).SELECT_ALL.SELECT_ALL,
    DatabaseException);

  EXPECT_THROW(
    FROM_TABLE(TableIds::Students)
      .JOIN(Relationships::StudentsConfidant, StudentsCols::Name)
      .JOIN(Relationships::StudentsConfidant, StudentsCols::Id),
    DatabaseException);

  EXPECT_THROW(
    FROM_TABLE(TableIds::Students)
      .JOIN(Relationships::StudentsConfidant, StudentsCols::Name)
      .JOIN_ALL(Relationships::StudentsConfidant),
    DatabaseException);

  EXPECT_THROW(
    FROM_TABLE(TableIds::Students)
      .JOIN_ALL(Relationships::StudentsConfidant)
      .JOIN_ALL(Relationships::StudentsConfidant),
    DatabaseException);
}

}
