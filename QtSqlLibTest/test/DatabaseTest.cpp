#include <gtest/gtest.h>

#include <QtSqlLib/Database.h>
#include <QtSqlLib/DatabaseException.h>
#include <QtSqlLib/InsertIntoExt.h>
#include <QtSqlLib/BatchInsertInto.h>
#include <QtSqlLib/FromTable.h>
#include <QtSqlLib/Expr.h>
#include <QtSqlLib/UpdateTable.h>
#include <QtSqlLib/LinkTuples.h>

#include <utilsLib/Utils.h>

#include <QFile>

#define ul utilsLib::underlying

using namespace QtSqlLib;

using DataType = Schema::DataType;

static const QString s_dbFilename = "test.db";

class TestDb : public Database
{
public:
  enum class TableIds
  {
    Table1,
    Table2,
    Students,
    Projects,
    Lectures
  };

  enum class Table1Cols
  {
    Id,
    Text,
    Mandatory
  };

  enum class Table2Cols
  {
    Id = 0,
    Text = 1,
    Mandatory = 2
  };

  enum class StudentsCols
  {
    Id = 3,
    Name = 4
  };

  enum class ProjectsCols
  {
    Id = 5,
    Title = 6
  };

  enum class LecturesCols
  {
    Id = 7,
    Topic = 8
  };

  enum class Relationships
  {
    RelationshipStudentsProjects,
    RelationshipStudentsLectures
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
using StudentsCols = TestDb::StudentsCols;
using ProjectsCols = TestDb::ProjectsCols;
using LecturesCols = TestDb::LecturesCols;
using TIds = TestDb::TableIds;
using Rs = TestDb::Relationships;

class DatabaseTest : public testing::Test
{
public:
  DatabaseTest()
    : m_db(std::make_unique<TestDb>())
  {
  }

  ~DatabaseTest()
  {
    m_db->close();
    m_db.reset();

    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

    QFile::remove(s_dbFilename);
  }

  std::unique_ptr<TestDb> m_db;
};

TEST_F(DatabaseTest, createTablesInsertAndQueryValues)
{
  m_db->setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    configurator.configureTable(ul(TIds::Table1), "table1")
      .column(ul(T1Cols::Id), "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(ul(T1Cols::Text), "text", DataType::Varchar, 128)
      .column(ul(T1Cols::Mandatory), "mandatory", DataType::Real).notNull();
  });

  m_db->initialize(s_dbFilename);

  m_db->execQuery(InsertIntoExt(ul(TIds::Table1))
    .value(ul(T1Cols::Text), "test")
    .value(ul(T1Cols::Mandatory), 0.5));

  m_db->execQuery(BatchInsertInto(ul(TIds::Table1))
    .values(ul(T1Cols::Text), QVariantList() << "test1" << "test2" << "test3")
    .values(ul(T1Cols::Mandatory), QVariantList() << 0.6 << 0.7 << 0.8));

  const auto results1 = m_db->execQuery(FromTable(ul(TIds::Table1))
    .select(ul(T1Cols::Text), ul(T1Cols::Mandatory))
    .where(Expr().equal(ul(T1Cols::Text), "test1")));

  EXPECT_EQ(results1.size(), 1);

  const auto result1ColText = results1[0].at({ ul(TIds::Table1), ul(T1Cols::Text) });
  const auto result1ColMand = results1[0].at({ ul(TIds::Table1), ul(T1Cols::Mandatory) });

  EXPECT_EQ(result1ColText.userType(), QMetaType::QString);
  EXPECT_EQ(result1ColMand.userType(), QMetaType::Double);

  EXPECT_EQ(result1ColText.toString(), "test1");
  EXPECT_DOUBLE_EQ(result1ColMand.toDouble(), 0.6);

  const auto results2 = m_db->execQuery(FromTable(ul(TIds::Table1))
    .select(ul(T1Cols::Id), ul(T1Cols::Text), ul(T1Cols::Mandatory))
    .where(Expr().less(ul(T1Cols::Mandatory), QVariant(0.75))));

  EXPECT_EQ(results2.size(), 3);

  for (const auto& result : results2)
  {
    const auto result2ColId = result.at({ ul(TIds::Table1), ul(T1Cols::Id) });
    const auto result2ColText = result.at({ ul(TIds::Table1), ul(T1Cols::Text) });
    const auto result2ColMand = result.at({ ul(TIds::Table1), ul(T1Cols::Mandatory) });

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
    configurator.configureTable(ul(TIds::Table1), "table1")
      .column(ul(T1Cols::Id), "id", DataType::Integer);

    configurator.configureTable(ul(TIds::Table1), "table2")
      .column(ul(T2Cols::Id), "id", DataType::Integer);
  });

  TestDb db2;
  db2.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // Two different tables must not have the same name
    configurator.configureTable(ul(TIds::Table1), "table1")
      .column(ul(T1Cols::Id), "id", DataType::Integer);

    configurator.configureTable(ul(TIds::Table2), "table1")
      .column(ul(T2Cols::Id), "id", DataType::Integer);
  });

  TestDb db3;
  db3.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // A table name must not start with 'sqlite_'
    configurator.configureTable(ul(TIds::Table1), "sqlite_table")
      .column(ul(T1Cols::Id), "id", DataType::Integer);
  });

  TestDb db4;
  db4.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // Two columns of the same table must not have the same id
    configurator.configureTable(ul(TIds::Table1), "table1")
      .column(ul(T1Cols::Id), "id1", DataType::Integer)
      .column(ul(T1Cols::Id), "id2", DataType::Integer);
  });

  TestDb db5;
  db5.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // Two columns of the same table must not have the same name
    configurator.configureTable(ul(TIds::Table1), "table1")
      .column(ul(T1Cols::Id), "col", DataType::Integer)
      .column(ul(T1Cols::Text), "col", DataType::Integer);
  });

  TestDb db6;
  db6.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // The varchar length parameter must be greater than 0 for varchar attribute
    configurator.configureTable(ul(TIds::Table1), "table1")
      .column(ul(T1Cols::Text), "text", DataType::Varchar, 0);
  });

  TestDb db7;
  db7.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // Only one attribute can be the primary key
    configurator.configureTable(ul(TIds::Table1), "table1")
      .column(ul(T1Cols::Id), "Id", DataType::Integer).primaryKey()
      .column(ul(T1Cols::Text), "text", DataType::Varchar).primaryKey();
  });

  TestDb db8;
  db8.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // primaryKey() must not be called multiple times for a column
    configurator.configureTable(ul(TIds::Table1), "table1")
      .column(ul(T1Cols::Id), "Id", DataType::Integer).primaryKey().primaryKey();
  });

  TestDb db9;
  db9.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // autoIncrement() must not be called multiple times for a column
    configurator.configureTable(ul(TIds::Table1), "table1")
      .column(ul(T1Cols::Id), "Id", DataType::Integer).autoIncrement().autoIncrement();
  });

  TestDb db10;
  db10.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // notNull() must not be called multiple times for a column
    configurator.configureTable(ul(TIds::Table1), "table1")
      .column(ul(T1Cols::Id), "Id", DataType::Integer).notNull().notNull();
  });

  TestDb db11;
  db11.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // primaryKey(), autoIncrement() and notNoll() must be called after column()
    configurator.configureTable(ul(TIds::Table1), "table1")
      .primaryKey();
  });

  TestDb db12;
  db12.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // column name must not ne empty
    configurator.configureTable(ul(TIds::Table1), "table1").
      column(ul(T1Cols::Id), "", DataType::Integer);
  });

  TestDb db13;
  db13.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    // table name must not ne empty
    configurator.configureTable(ul(TIds::Table1), "");
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
  EXPECT_THROW(db12.initialize(s_dbFilename), DatabaseException);
  EXPECT_THROW(db13.initialize(s_dbFilename), DatabaseException);
}

TEST_F(DatabaseTest, expressionStringsCorrectTest)
{
  Schema schema;

  auto& table = schema.getTables()[ul(TIds::Table1)];
  table.name = "test";
  table.columns[ul(T1Cols::Id)].name = "id";
  table.columns[ul(T1Cols::Text)].name = "test";

  Expr expr1;
  expr1
    .less(ul(T1Cols::Id), QVariant(2))
    .and()
    .equal(ul(T1Cols::Text), "test1");

  EXPECT_EQ(expr1.toQString(schema, ul(TIds::Table1)), "'test'.id < 2 AND 'test'.test == 'test1'");

  Expr expr2;
  expr2
    .braces(Expr().unequal(ul(T1Cols::Id), QVariant(2)).and().unequal(ul(T1Cols::Text), "test1"))
    .or()
    .braces(Expr().greater(ul(T1Cols::Id), QVariant(3)));

  EXPECT_EQ(expr2.toQString(schema, ul(TIds::Table1)), "('test'.id != 2 AND 'test'.test != 'test1') OR ('test'.id > 3)");
}

TEST_F(DatabaseTest, expressionThrowsTest)
{
  Schema schema;

  auto& table = schema.getTables()[ul(TIds::Table1)];
  table.name = "test";
  table.columns[ul(T1Cols::Id)].name = "id";
  table.columns[ul(T1Cols::Text)].name = "test";

  const auto assembleExpr1 = []()
  {
    // Conditions must not be consecutive
    Expr expr;
    expr
      .less(ul(T1Cols::Id), QVariant(2))
      .equal(ul(T1Cols::Text), "test1");
  };

  const auto assembleExpr2 = []()
  {
    // Expression must not begin with a logical operator
    Expr expr;
    expr
      .or()
      .less(ul(T1Cols::Id), QVariant(2));
  };

  const auto assembleExpr3 = [&schema]()
  {
    // Expressions must not end with a logical operator
    Expr expr;
    expr
      .less(ul(T1Cols::Id), QVariant(2))
      .and();

    expr.toQString(schema, ul(TIds::Table1));
  };

  const auto assembleExpr4 = [&schema]()
  {
    // Expressions must not be empty
    const Expr expr;
    expr.toQString(schema, ul(TIds::Table1));
  };

  EXPECT_THROW(assembleExpr1(), DatabaseException);
  EXPECT_THROW(assembleExpr2(), DatabaseException);
  EXPECT_THROW(assembleExpr3(), DatabaseException);
  EXPECT_THROW(assembleExpr4(), DatabaseException);
}

TEST_F(DatabaseTest, updateTableTest)
{
  m_db->setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    configurator.configureTable(ul(TIds::Table1), "table1")
      .column(ul(T1Cols::Id), "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(ul(T1Cols::Text), "text", DataType::Varchar, 128)
      .column(ul(T1Cols::Mandatory), "mandatory", DataType::Integer);
  });

  m_db->initialize(s_dbFilename);

  m_db->execQuery(BatchInsertInto(ul(TIds::Table1))
    .values(ul(T1Cols::Text), QVariantList() << "unchanged" << "unchanged" << "unchanged")
    .values(ul(T1Cols::Mandatory), QVariantList() << 1 << 2 << 3));

  m_db->execQuery(UpdateTable(ul(TIds::Table1))
    .set(ul(T1Cols::Text), "updated")
    .where(Expr().equal(ul(T1Cols::Mandatory), QVariant(2))));

  const auto results = m_db->execQuery(FromTable(ul(TIds::Table1))
    .select(ul(T1Cols::Text), ul(T1Cols::Mandatory)));

  EXPECT_EQ(results[0].at({ ul(TIds::Table1), ul(T1Cols::Text) }).toString(), "unchanged");
  EXPECT_EQ(results[1].at({ ul(TIds::Table1), ul(T1Cols::Text) }).toString(), "updated");
  EXPECT_EQ(results[2].at({ ul(TIds::Table1), ul(T1Cols::Text) }).toString(), "unchanged");
}

TEST_F(DatabaseTest, relationshipTest)
{
  m_db->setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    configurator.configureTable(ul(TIds::Students), "students")
      .column(ul(StudentsCols::Id), "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(ul(StudentsCols::Name), "name", DataType::Varchar, 128);

    configurator.configureTable(ul(TIds::Projects), "projects")
      .column(ul(ProjectsCols::Id), "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(ul(ProjectsCols::Title), "title", DataType::Varchar, 128);

    configurator.configureTable(ul(TIds::Lectures), "lectures")
      .column(ul(LecturesCols::Id), "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(ul(LecturesCols::Topic), "topic", DataType::Varchar, 128);

    configurator.configureRelationship(ul(Rs::RelationshipStudentsProjects), ul(TIds::Students), ul(TIds::Projects),
      Schema::RelationshipType::OneToMany).onDelete(Schema::ForeignKeyAction::Cascade);

    configurator.configureRelationship(ul(Rs::RelationshipStudentsLectures), ul(TIds::Students), ul(TIds::Lectures),
      Schema::RelationshipType::ManyToMany);
  });

  m_db->initialize(s_dbFilename);

  const auto studentJohn = m_db->execQuery(InsertIntoExt(ul(TIds::Students))
    .value(ul(StudentsCols::Name), "John")
    .returnIds());

  const auto studentMary = m_db->execQuery(InsertIntoExt(ul(TIds::Students))
    .value(ul(StudentsCols::Name), "Mary")
    .returnIds());

  const auto projectGameProgramming = m_db->execQuery(InsertIntoExt(ul(TIds::Projects))
    .value(ul(ProjectsCols::Title), "Game Programming")
    //.linkTuple(ul(Rs::RelationshipStudentsProjects), studentJohn[0])
    .returnIds());

  const auto projectComputerVision = m_db->execQuery(InsertIntoExt(ul(TIds::Projects))
    .value(ul(ProjectsCols::Title), "Computer Vision")
    //.linkTuple(ul(Rs::RelationshipStudentsProjects), studentJohn[0])
    .returnIds());

  const auto projectMachineLearning = m_db->execQuery(InsertIntoExt(ul(TIds::Projects))
    .value(ul(ProjectsCols::Title), "Machine Learning")
    //.linkTuple(ul(Rs::RelationshipStudentsProjects), studentMary[0])
    .returnIds());

  const auto lectureMath = m_db->execQuery(InsertIntoExt(ul(TIds::Lectures))
    .value(ul(LecturesCols::Topic), "Math")
    .returnIds());

  const auto lectureProgramming = m_db->execQuery(InsertIntoExt(ul(TIds::Lectures))
    .value(ul(LecturesCols::Topic), "Programming")
    .returnIds());

  m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsProjects))
    .fromOne(studentJohn[0])
    .toMany({ projectGameProgramming[0], projectComputerVision[0] }));

  m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsProjects))
    .fromOne(projectMachineLearning[0])
    .toOne(studentMary[0]));


  //m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsLectures))
  //  .fromOne(studentJohn[0])
  //  .toOne(lectureMath[0]));



  //m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsLectures))
  //  .fromOne(studentJohn[0])
  //  .toMany({ lectureMath[0], lectureProgramming[0] }));
}

TEST_F(DatabaseTest, multiplePrimaryKeysTable)
{
  m_db->setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    configurator.configureTable(ul(TIds::Table1), "table1")
      .column(ul(T1Cols::Id), "id", DataType::Integer).notNull()
      .column(ul(T1Cols::Text), "text", DataType::Varchar, 128).notNull()
      .primaryKeys(ul(T1Cols::Id), ul(T1Cols::Text));
  });

  m_db->initialize(s_dbFilename);

  const auto results = m_db->execQuery(InsertIntoExt(ul(TIds::Table1))
    .value(ul(T1Cols::Id), 1)
    .value(ul(T1Cols::Text), "text")
    .returnIds());

  EXPECT_EQ(results.size(), 1);

  const auto resultId = results[0].at({ ul(TIds::Table1), ul(T1Cols::Id) });
  const auto resultText = results[0].at({ ul(TIds::Table1), ul(T1Cols::Text) });

  EXPECT_EQ(resultId.userType(), QMetaType::LongLong);
  EXPECT_EQ(resultText.userType(), QMetaType::QString);

  EXPECT_EQ(resultId.toLongLong(), 1);
  EXPECT_EQ(resultText.toString(), "text");
}

TEST_F(DatabaseTest, nestedQuerySequence)
{
  static auto currentQuery = 0;

  class DummyQuery : public IQuery
  {
  public:
    DummyQuery(int num) : m_num(num)
    {
    }

    QueryDefines::SqlQuery getSqlQuery(Schema& schema) override
    {
      EXPECT_EQ(currentQuery, m_num);
      currentQuery++;
      return { QSqlQuery(), QueryDefines::QueryMode::Single };
    }

  private:
    int m_num;
  };

  auto nestedSeq2 = std::make_unique<QuerySequence>();
  nestedSeq2->addQuery(std::make_unique<DummyQuery>(3));
  nestedSeq2->addQuery(std::make_unique<DummyQuery>(4));

  auto nestedSeq1 = std::make_unique<QuerySequence>();
  nestedSeq1->addQuery(std::make_unique<DummyQuery>(1));
  nestedSeq1->addQuery(std::make_unique<DummyQuery>(2));
  nestedSeq1->addQuery(std::move(nestedSeq2));
  nestedSeq1->addQuery(std::make_unique<DummyQuery>(5));

  QuerySequence seq;
  seq.addQuery(std::make_unique<DummyQuery>(0));
  seq.addQuery(std::move(nestedSeq1));

  seq.addQuery(std::make_unique<DummyQuery>(6));

  Schema s;
  seq.prepare(s);

  const auto num = seq.getNumQueries();
  for (auto i=0; i<num; i++)
  {
    seq.getSqlQuery(i, s);
  }
}


// TODO: foreignKeys() test