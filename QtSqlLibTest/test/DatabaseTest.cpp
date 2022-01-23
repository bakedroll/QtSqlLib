#include <gtest/gtest.h>

#include <QtSqlLib/Database.h>
#include <QtSqlLib/DatabaseException.h>
#include <QtSqlLib/InsertInto.h>
#include <QtSqlLib/BatchInsertInto.h>
#include <QtSqlLib/FromTable.h>
#include <QtSqlLib/Expr.h>

#include <utilsLib/Utils.h>

#include <QFile>

using namespace QtSqlLib;
using namespace utilsLib;

using DataType = TableConfigurator::DataType;

static const QString s_dbFilename = "test.db";

class TestDb : public Database
{
public:
  enum class TableIds
  {
    Table1,
    Table2
  };

  enum class Table1Cols
  {
    Id,
    Text,
    Mandatory
  };

  enum class Table2Cols
  {
    Id,
    Text,
    Mandatory
  };

  using ConfigFunc = std::function<void(SchemaConfigurator&)>;

  void setConfigureSchemaFunc(const ConfigFunc& func)
  {
    m_configureSchemaFunc = func;
  }

protected:
  void configureSchema(SchemaConfigurator& configurator) override
  {
    ASSERT_TRUE(m_configureSchemaFunc);
    m_configureSchemaFunc(configurator);
  }

private:
  ConfigFunc m_configureSchemaFunc;

};

using T1Cols = TestDb::Table1Cols;
using T2Cols = TestDb::Table2Cols;
using TIds = TestDb::TableIds;

class DatabaseTest : public testing::Test
{
public:
  ~DatabaseTest()
  {
    m_db.close();
    QFile::remove(s_dbFilename);
  }

  TestDb m_db;
};

TEST_F(DatabaseTest, createTablesInsertAndQueryValues)
{
  m_db.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    configurator.configureTable(underlying(TIds::Table1), "table1")
      .column(underlying(T1Cols::Id), "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(underlying(T1Cols::Text), "text", DataType::Varchar, 128)
      .column(underlying(T1Cols::Mandatory), "mandatory", DataType::Real).notNull();
  });

  m_db.initialize(s_dbFilename);

  m_db.execQuery(InsertInto(underlying(TIds::Table1))
    .value(underlying(T1Cols::Text), "test")
    .value(underlying(T1Cols::Mandatory), 0.5));

  m_db.execQuery(BatchInsertInto(underlying(TIds::Table1))
    .values(underlying(T1Cols::Text), QVariantList() << "test1" << "test2" << "test3")
    .values(underlying(T1Cols::Mandatory), QVariantList() << 0.6 << 0.7 << 0.8));

  const auto results1 = m_db.execQuery(FromTable(underlying(TIds::Table1))
    .select(underlying(T1Cols::Text), underlying(T1Cols::Mandatory))
    .where(Expr().equal(underlying(T1Cols::Text), "test1")));

  EXPECT_EQ(results1.size(), 1);

  const auto result1ColText = results1[0].at({ underlying(TIds::Table1), underlying(T1Cols::Text) });
  const auto result1ColMand = results1[0].at({ underlying(TIds::Table1), underlying(T1Cols::Mandatory) });

  EXPECT_EQ(result1ColText.userType(), QMetaType::QString);
  EXPECT_EQ(result1ColMand.userType(), QMetaType::Double);

  EXPECT_EQ(result1ColText.toString(), "test1");
  EXPECT_DOUBLE_EQ(result1ColMand.toDouble(), 0.6);

  const auto results2 = m_db.execQuery(FromTable(underlying(TIds::Table1))
    .select(underlying(T1Cols::Id), underlying(T1Cols::Text), underlying(T1Cols::Mandatory))
    .where(Expr().less(underlying(T1Cols::Mandatory), QVariant(0.75))));

  EXPECT_EQ(results2.size(), 3);

  for (const auto& result : results2)
  {
    const auto result2ColId = result.at({ underlying(TIds::Table1), underlying(T1Cols::Id) });
    const auto result2ColText = result.at({ underlying(TIds::Table1), underlying(T1Cols::Text) });
    const auto result2ColMand = result.at({ underlying(TIds::Table1), underlying(T1Cols::Mandatory) });

    EXPECT_EQ(result2ColId.userType(), QMetaType::LongLong);
    EXPECT_EQ(result2ColText.userType(), QMetaType::QString);
    EXPECT_EQ(result2ColMand.userType(), QMetaType::Double);

    EXPECT_LT(result1ColMand.toDouble(), 0.75);
  }
}

TEST_F(DatabaseTest, initializeThrowsOnInvalidConfig)
{
  TestDb db1;
  db1.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // Two different tables must not have the same id
    configurator.configureTable(underlying(TIds::Table1), "table1")
      .column(underlying(T1Cols::Id), "id", DataType::Integer);

    configurator.configureTable(underlying(TIds::Table1), "table2")
      .column(underlying(T2Cols::Id), "id", DataType::Integer);
  });

  TestDb db2;
  db2.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // Two different tables must not have the same name
    configurator.configureTable(underlying(TIds::Table1), "table1")
      .column(underlying(T1Cols::Id), "id", DataType::Integer);

    configurator.configureTable(underlying(TIds::Table2), "table1")
      .column(underlying(T2Cols::Id), "id", DataType::Integer);
  });

  TestDb db3;
  db3.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // A table name must not start with 'sqlite_'
    configurator.configureTable(underlying(TIds::Table1), "sqlite_table")
      .column(underlying(T1Cols::Id), "id", DataType::Integer);
  });

  TestDb db4;
  db4.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // Two columns of the same table must not have the same id
    configurator.configureTable(underlying(TIds::Table1), "table1")
      .column(underlying(T1Cols::Id), "id1", DataType::Integer)
      .column(underlying(T1Cols::Id), "id2", DataType::Integer);
  });

  TestDb db5;
  db5.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // Two columns of the same table must not have the same name
    configurator.configureTable(underlying(TIds::Table1), "table1")
      .column(underlying(T1Cols::Id), "col", DataType::Integer)
      .column(underlying(T1Cols::Text), "col", DataType::Integer);
  });

  TestDb db6;
  db6.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // The varchar length parameter must be greater than 0 for varchar attribute
    configurator.configureTable(underlying(TIds::Table1), "table1")
      .column(underlying(T1Cols::Text), "text", DataType::Varchar, 0);
  });

  TestDb db7;
  db7.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // Only one attribute can be the primary key
    configurator.configureTable(underlying(TIds::Table1), "table1")
      .column(underlying(T1Cols::Id), "Id", DataType::Integer).primaryKey()
      .column(underlying(T1Cols::Text), "text", DataType::Varchar).primaryKey();
  });

  TestDb db8;
  db8.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // primaryKey() must not be called multiple times for a column
    configurator.configureTable(underlying(TIds::Table1), "table1")
      .column(underlying(T1Cols::Id), "Id", DataType::Integer).primaryKey().primaryKey();
  });

  TestDb db9;
  db9.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // autoIncrement() must not be called multiple times for a column
    configurator.configureTable(underlying(TIds::Table1), "table1")
      .column(underlying(T1Cols::Id), "Id", DataType::Integer).autoIncrement().autoIncrement();
  });

  TestDb db10;
  db10.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // notNull() must not be called multiple times for a column
    configurator.configureTable(underlying(TIds::Table1), "table1")
      .column(underlying(T1Cols::Id), "Id", DataType::Integer).notNull().notNull();
  });

  TestDb db11;
  db11.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // primaryKey(), autoIncrement() and notNoll() must be called after column()
    configurator.configureTable(underlying(TIds::Table1), "table1")
      .primaryKey();
  });

  EXPECT_THROW(db1.initialize(s_dbFilename), DatabaseException);
  EXPECT_THROW(db2.initialize(s_dbFilename), DatabaseException);
  EXPECT_THROW(db3.initialize(s_dbFilename), DatabaseException);
  EXPECT_THROW(db4.initialize(s_dbFilename), DatabaseException);
  EXPECT_THROW(db5.initialize(s_dbFilename), DatabaseException);
  EXPECT_THROW(db6.initialize(s_dbFilename), DatabaseException);
  EXPECT_THROW(db7.initialize(s_dbFilename), DatabaseException);
  EXPECT_THROW(db8.initialize(s_dbFilename), DatabaseException);
  EXPECT_THROW(db9.initialize(s_dbFilename), DatabaseException);
  EXPECT_THROW(db10.initialize(s_dbFilename), DatabaseException);
  EXPECT_THROW(db11.initialize(s_dbFilename), DatabaseException);
}

TEST_F(DatabaseTest, expressionStringsCorrectTest)
{
  SchemaConfigurator::Schema schema;

  auto& table = schema.tables[underlying(TIds::Table1)];
  table.name = "test";
  table.columns[underlying(T1Cols::Id)].name = "id";
  table.columns[underlying(T1Cols::Text)].name = "test";

  Expr expr1;
  expr1
    .less(underlying(T1Cols::Id), QVariant(2))
    .and()
    .equal(underlying(T1Cols::Text), "test1");

  EXPECT_EQ(expr1.toQString(schema, underlying(TIds::Table1)), "test.id < 2 AND test.test == 'test1'");

  Expr expr2;
  expr2
    .braces(Expr().unequal(underlying(T1Cols::Id), QVariant(2)).and().unequal(underlying(T1Cols::Text), "test1"))
    .or()
    .braces(Expr().greater(underlying(T1Cols::Id), QVariant(3)));

  EXPECT_EQ(expr2.toQString(schema, underlying(TIds::Table1)), "(test.id != 2 AND test.test != 'test1') OR (test.id > 3)");
}

TEST_F(DatabaseTest, expressionThrowsTest)
{
  SchemaConfigurator::Schema schema;

  auto& table = schema.tables[underlying(TIds::Table1)];
  table.name = "test";
  table.columns[underlying(T1Cols::Id)].name = "id";
  table.columns[underlying(T1Cols::Text)].name = "test";

  const auto assembleExpr1 = []()
  {
    // Conditions must not be consecutive
    Expr expr;
    expr
      .less(underlying(T1Cols::Id), QVariant(2))
      .equal(underlying(T1Cols::Text), "test1");
  };

  const auto assembleExpr2 = []()
  {
    // Expression must not begin with a logical operator
    Expr expr;
    expr
      .or()
      .less(underlying(T1Cols::Id), QVariant(2));
  };

  const auto assembleExpr3 = [&schema]()
  {
    // Expressions must not end with a logical operator
    Expr expr;
    expr
      .less(underlying(T1Cols::Id), QVariant(2))
      .and();

    expr.toQString(schema, underlying(TIds::Table1));
  };

  const auto assembleExpr4 = [&schema]()
  {
    // Expressions must not be empty
    const Expr expr;
    expr.toQString(schema, underlying(TIds::Table1));
  };

  EXPECT_THROW(assembleExpr1(), DatabaseException);
  EXPECT_THROW(assembleExpr2(), DatabaseException);
  EXPECT_THROW(assembleExpr3(), DatabaseException);
  EXPECT_THROW(assembleExpr4(), DatabaseException);
}
