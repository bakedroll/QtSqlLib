#include <gtest/gtest.h>

#include <QtSqlLib/API/IQueryVisitor.h>
#include <QtSqlLib/Database.h>
#include <QtSqlLib/DatabaseException.h>
#include <QtSqlLib/Query/InsertIntoExt.h>
#include <QtSqlLib/Query/BatchInsertInto.h>
#include <QtSqlLib/Query/FromTable.h>
#include <QtSqlLib/Expr.h>
#include <QtSqlLib/Query/UpdateTable.h>
#include <QtSqlLib/Query/LinkTuples.h>

#include <utilsLib/Utils.h>

#include <QFile>

#define ul utilsLib::underlying

using namespace QtSqlLib;
using namespace QtSqlLib::Query;

using DataType = Schema::DataType;

static const QString s_dbFilename = "test.db";

class TestDb : public Database
{
public:
  enum class TableIds : unsigned int
  {
    Table1,
    Table2,
    Students,
    Projects,
    Lectures
  };

  enum class Table1Cols : unsigned int
  {
    Id,
    Text,
    Mandatory
  };

  enum class Table2Cols : unsigned int
  {
    Id = 0,
    Text = 1,
    Mandatory = 2
  };

  enum class StudentsCols : unsigned int
  {
    Id = 3,
    Name = 4
  };

  enum class ProjectsCols : unsigned int
  {
    Id = 5,
    Title = 6
  };

  enum class LecturesCols : unsigned int
  {
    Id = 7,
    Topic = 8
  };

  enum class Relationships : unsigned int
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

  void setupReplationshipTestsSchema() const
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

  EXPECT_EQ(results1.values.size(), 1);

  const auto result1ColText = results1.values[0].at({ ul(TIds::Table1), ul(T1Cols::Text) });
  const auto result1ColMand = results1.values[0].at({ ul(TIds::Table1), ul(T1Cols::Mandatory) });

  EXPECT_EQ(result1ColText.userType(), QMetaType::QString);
  EXPECT_EQ(result1ColMand.userType(), QMetaType::Double);

  EXPECT_EQ(result1ColText.toString(), "test1");
  EXPECT_DOUBLE_EQ(result1ColMand.toDouble(), 0.6);

  const auto results2 = m_db->execQuery(FromTable(ul(TIds::Table1))
    .select(ul(T1Cols::Id), ul(T1Cols::Text), ul(T1Cols::Mandatory))
    .where(Expr().less(ul(T1Cols::Mandatory), QVariant(0.75))));

  EXPECT_EQ(results2.values.size(), 3);

  for (const auto& result : results2.values)
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

  EXPECT_EQ(expr1.toQString(schema, ul(TIds::Table1)), "'test'.'id' < 2 AND 'test'.'test' == 'test1'");

  Expr expr2;
  expr2
    .braces(Expr().unequal(ul(T1Cols::Id), QVariant(2)).and().unequal(ul(T1Cols::Text), "test1"))
    .or()
    .braces(Expr().greater(ul(T1Cols::Id), QVariant(3)));

  EXPECT_EQ(expr2.toQString(schema, ul(TIds::Table1)), "('test'.'id' != 2 AND 'test'.'test' != 'test1') OR ('test'.'id' > 3)");
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

  EXPECT_EQ(results.values[0].at({ ul(TIds::Table1), ul(T1Cols::Text) }).toString(), "unchanged");
  EXPECT_EQ(results.values[1].at({ ul(TIds::Table1), ul(T1Cols::Text) }).toString(), "updated");
  EXPECT_EQ(results.values[2].at({ ul(TIds::Table1), ul(T1Cols::Text) }).toString(), "unchanged");
}

TEST_F(DatabaseTest, fromTableThrows)
{
  EXPECT_THROW(
    FromTable(ul(TIds::Students)).select(ul(StudentsCols::Name)).select(ul(StudentsCols::Id)),
    DatabaseException);

  EXPECT_THROW(
    FromTable(ul(TIds::Students)).select(ul(StudentsCols::Name)).selectAll(),
    DatabaseException);

  EXPECT_THROW(
    FromTable(ul(TIds::Students)).selectAll().selectAll(),
    DatabaseException);

  EXPECT_THROW(
    FromTable(ul(TIds::Students))
      .joinColumns(ul(Rs::RelationshipStudentsProjects), ul(StudentsCols::Name))
      .joinColumns(ul(Rs::RelationshipStudentsProjects), ul(StudentsCols::Id)),
    DatabaseException);

  EXPECT_THROW(
    FromTable(ul(TIds::Students))
      .joinColumns(ul(Rs::RelationshipStudentsProjects), ul(StudentsCols::Name))
      .joinAll(ul(Rs::RelationshipStudentsProjects)),
    DatabaseException);

  EXPECT_THROW(
    FromTable(ul(TIds::Students))
      .joinAll(ul(Rs::RelationshipStudentsProjects))
      .joinAll(ul(Rs::RelationshipStudentsProjects)),
    DatabaseException);
}

/**
 * @test:
 *      Students
 *        1  N
 *       /    \
 *      /      \
 *     N        M
 * Projects   Lectures
 *
 * (1) Insert Student and link directly to ONE Lecture
 * (2) Insert Student and link directly to MANY Lectures
 * (3) Insert Lecture and link directly to ONE Student
 * (4) Insert Lecture and link directly to MANY Students
 * (5) Insert Student and link directly to ONE Project
 * (6) Insert Student and link directly to MANY Projects
 * (7) Insert Project and link directly to ONE Student
 *
 * @expected:
 * Relations Students-Lectures:
 * John    <-->    Math
 *            >    Database Systems
 * Mary    <-->    Programming
 *            >    Database Systems
 * Paul    <-->    Math
 *            >    Programming
 *
 * Relations Students-Projects:
 * John    <-->    Computer Vision
 * Mary    <-->    Game Programming
 * Paul    <-->    Modeling
 *            >    Machine Learning
 */
TEST_F(DatabaseTest, linkTuplesOnInsertTest)
{
  setupReplationshipTestsSchema();

  m_db->initialize(s_dbFilename);

  // Insert Lecture "Math"
  const auto lectureMath = m_db->execQuery(InsertIntoExt(ul(TIds::Lectures))
    .value(ul(LecturesCols::Topic), "Math")
    .returnIds()).values[0];

  // Insert Project "Game Programming"
  const auto projectGameProgramming = m_db->execQuery(InsertIntoExt(ul(TIds::Projects))
    .value(ul(ProjectsCols::Title), "Game Programming")
    .returnIds()).values[0];

  // Insert Project "Modeling"
  const auto projectModeling = m_db->execQuery(InsertIntoExt(ul(TIds::Projects))
    .value(ul(ProjectsCols::Title), "Modeling")
    .returnIds()).values[0];

  // Insert Project "Machine Learning"
  const auto projectMachineLearning = m_db->execQuery(InsertIntoExt(ul(TIds::Projects))
    .value(ul(ProjectsCols::Title), "Machine Learning")
    .returnIds()).values[0];
  
  // (1) Insert Student "John" and link directly to Lecture "Math"
  const auto studentJohn = m_db->execQuery(InsertIntoExt(ul(TIds::Students))
    .value(ul(StudentsCols::Name), "John")
    .linkToOneTuple(ul(Rs::RelationshipStudentsLectures), lectureMath)
    .returnIds()).values[0];

  // (7) Insert Project "Computer vision" and link direktly to Student "John"
  const auto projectComputerVision = m_db->execQuery(InsertIntoExt(ul(TIds::Projects))
    .value(ul(ProjectsCols::Title), "Computer Vision")
    .linkToOneTuple(ul(Rs::RelationshipStudentsProjects), studentJohn)
    .returnIds()).values[0];

  // (3) Insert Lecture "Programming" and link directly to Student "John"
  const auto lectureProgramming = m_db->execQuery(InsertIntoExt(ul(TIds::Lectures))
    .value(ul(LecturesCols::Topic), "Programming")
    .linkToOneTuple(ul(Rs::RelationshipStudentsLectures), studentJohn)
    .returnIds()).values[0];

  // (1) (5) Insert Student Mary and link directly to Lecture "Programming" and to Project "Game Programming"
  const auto studentMary = m_db->execQuery(InsertIntoExt(ul(TIds::Students))
    .value(ul(StudentsCols::Name), "Mary")
    .linkToOneTuple(ul(Rs::RelationshipStudentsLectures), lectureProgramming)
    .linkToOneTuple(ul(Rs::RelationshipStudentsProjects), projectGameProgramming)
    .returnIds()).values[0];
  
  // (4) Insert Lecture "Database Systems" and link directly to Students "John" and "Mary"
  const auto lectureDbSystems = m_db->execQuery(InsertIntoExt(ul(TIds::Lectures))
    .value(ul(LecturesCols::Topic), "Database Systems")
    .linkToManyTuples(ul(Rs::RelationshipStudentsLectures), { studentJohn, studentMary })
    .returnIds()).values[0];
  
  // (2) (6) Insert Student Paul and link directly to Lectures "Math" and "Programming"
  //         AND to Projects "Game Programming" AND "Machine Learning"
  const auto studentPaul = m_db->execQuery(InsertIntoExt(ul(TIds::Students))
    .value(ul(StudentsCols::Name), "Paul")
    .linkToManyTuples(ul(Rs::RelationshipStudentsLectures), { lectureMath, lectureProgramming })
    .linkToManyTuples(ul(Rs::RelationshipStudentsProjects), { projectModeling, projectMachineLearning })
    .returnIds()).values[0];

  const auto results = m_db->execQuery(FromTable(ul(TIds::Students))
    .select(ul(StudentsCols::Name))
    .joinAll(ul(Rs::RelationshipStudentsProjects)));
}

/**
 * @test:
 *      Students
 *        1  N
 *       /    \
 *      /      \
 *     N        M
 * Projects   Lectures
 *
 * (1) Link Student to ONE Lecture
 * (2) Link Student to MANY Lectures
 * (3) Link Lecture to ONE Student
 * (4) Link Lecture to MANY Students
 * (5) Link Student to ONE Project
 * (6) Link Student to MANY Projects
 * (7) Link Project to ONE Student
 *
 * Exception handling tests:
 * (8) Link Project to MANY Students
 * (9) Use invalid relationship ID
 * (10) Use InsertIntoExt::linkToOneTuple() with invalid tuple Key
 * (11) Use InsertIntoExt::linkToManyTuple() with invalid tuple Key
 * (12) Use LinkTuples::fromOne() with invalid tuple key
 * (13) Use LinkTuples::toOne() with invalid tuple key
 * (14) Use LinkTuples::fromOne() with invalid tuple key
 *
 * @expected:
 * Relations Students-Lectures:
 * John    <-->    Math
 *            >    Database Systems
 * Mary    <-->    Programming
 *            >    Database Systems
 * Paul    <-->    Math
 *            >    Programming
 *
 * Relations Students-Projects:
 * John    <-->    Computer Vision
 * Mary    <-->    Game Programming
 * Paul    <-->    Modeling
 *            >    Machine Learning
 */
TEST_F(DatabaseTest, linkTuplesQueryTest)
{
  setupReplationshipTestsSchema();

  m_db->initialize(s_dbFilename);

  // Insert tuples
  const auto studentJohn = m_db->execQuery(InsertIntoExt(ul(TIds::Students))
    .value(ul(StudentsCols::Name), "John")
    .returnIds()).values[0];

  const auto studentMary = m_db->execQuery(InsertIntoExt(ul(TIds::Students))
    .value(ul(StudentsCols::Name), "Mary")
    .returnIds()).values[0];

  const auto studentPaul = m_db->execQuery(InsertIntoExt(ul(TIds::Students))
    .value(ul(StudentsCols::Name), "Paul")
    .returnIds()).values[0];

  const auto lectureMath = m_db->execQuery(InsertIntoExt(ul(TIds::Lectures))
    .value(ul(LecturesCols::Topic), "Math")
    .returnIds()).values[0];

  const auto lectureProgramming = m_db->execQuery(InsertIntoExt(ul(TIds::Lectures))
    .value(ul(LecturesCols::Topic), "Programming")
    .returnIds()).values[0];

  const auto lectureDbSystems = m_db->execQuery(InsertIntoExt(ul(TIds::Lectures))
    .value(ul(LecturesCols::Topic), "Database Systems")
    .returnIds()).values[0];

  const auto projectGameProgramming = m_db->execQuery(InsertIntoExt(ul(TIds::Projects))
    .value(ul(ProjectsCols::Title), "Game Programming")
    .returnIds()).values[0];

  const auto projectModeling = m_db->execQuery(InsertIntoExt(ul(TIds::Projects))
    .value(ul(ProjectsCols::Title), "Modeling")
    .returnIds()).values[0];

  const auto projectMachineLearning = m_db->execQuery(InsertIntoExt(ul(TIds::Projects))
    .value(ul(ProjectsCols::Title), "Machine Learning")
    .returnIds()).values[0];

  const auto projectComputerVision = m_db->execQuery(InsertIntoExt(ul(TIds::Projects))
    .value(ul(ProjectsCols::Title), "Computer Vision")
    .returnIds()).values[0];

  // (1)
  m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsLectures))
    .fromOne(studentMary)
    .toOne(lectureProgramming));

  // (2)
  m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsLectures))
    .fromOne(studentPaul)
    .toMany({ lectureMath, lectureProgramming }));

  // (3)
  m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsLectures))
    .fromOne(lectureProgramming)
    .toOne(studentJohn));

  // (4)
  m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsLectures))
    .fromOne(lectureDbSystems)
    .toMany({ studentJohn, studentMary }));

  // (5)
  m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsProjects))
    .fromOne(studentMary)
    .toOne(projectGameProgramming));

  // (6)
  m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsProjects))
    .fromOne(studentPaul)
    .toMany({ projectModeling, projectMachineLearning }));

  // (7)
  m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsProjects))
    .fromOne(projectComputerVision)
    .toOne(studentJohn));

  // (8)
  EXPECT_THROW(m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsProjects))
    .fromOne(projectComputerVision)
    .toMany({ studentJohn, studentMary }))
    , DatabaseException);

  EXPECT_THROW(m_db->execQuery(InsertIntoExt(ul(TIds::Projects))
    .value(ul(ProjectsCols::Title), "Dummy")
    .linkToManyTuples(ul(Rs::RelationshipStudentsProjects), { studentJohn, studentMary }))
    , DatabaseException);

  // (9)
  EXPECT_THROW(m_db->execQuery(InsertIntoExt(ul(TIds::Projects))
    .value(ul(ProjectsCols::Title), "Dummy")
    .linkToOneTuple(ul(Rs::RelationshipStudentsLectures), studentJohn))
    , DatabaseException);

  // (10)
  EXPECT_THROW(m_db->execQuery(InsertIntoExt(ul(TIds::Students))
    .value(ul(StudentsCols::Name), "Dummy")
    .linkToOneTuple(ul(Rs::RelationshipStudentsLectures), projectMachineLearning))
    , DatabaseException);

  // (11)
  EXPECT_THROW(m_db->execQuery(InsertIntoExt(ul(TIds::Students))
    .value(ul(StudentsCols::Name), "Dummy")
    .linkToManyTuples(ul(Rs::RelationshipStudentsLectures), { projectMachineLearning, lectureDbSystems }))
    , DatabaseException);

  // (12)
  EXPECT_THROW(m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsLectures))
    .fromOne(projectComputerVision)
    .toOne(lectureProgramming))
    , DatabaseException);

  // (13)
  EXPECT_THROW(m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsLectures))
    .fromOne(studentMary)
    .toOne(projectComputerVision))
    , DatabaseException);

  // (14)
  EXPECT_THROW(m_db->execQuery(LinkTuples(ul(Rs::RelationshipStudentsLectures))
    .fromOne(studentMary)
    .toMany({ projectComputerVision, lectureProgramming }))
    , DatabaseException);
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

  EXPECT_EQ(results.values.size(), 1);

  const auto resultId = results.values[0].at({ ul(TIds::Table1), ul(T1Cols::Id) });
  const auto resultText = results.values[0].at({ ul(TIds::Table1), ul(T1Cols::Text) });

  EXPECT_EQ(resultId.userType(), QMetaType::LongLong);
  EXPECT_EQ(resultText.userType(), QMetaType::QString);

  EXPECT_EQ(resultId.toLongLong(), 1);
  EXPECT_EQ(resultText.toString(), "text");
}

TEST_F(DatabaseTest, querySequenceVisitor)
{
  class DummyQuery : public Query
  {
  public:
    DummyQuery(int num)
      : Query()
      , m_num(num)
    {
    }

    int getNum() const
    {
      return m_num;
    }

    SqlQuery getSqlQuery(Schema& schema, QueryResults& previousQueryResults) override
    {
      return { QSqlQuery(), QueryMode::Single };
    }

  private:
    int m_num;
  };

  class TestVisitor : public API::IQueryVisitor
  {
  public:
    TestVisitor() : API::IQueryVisitor(), m_currentNum(0)
    {}

    ~TestVisitor() override = default;

    void visit(API::IQuery& query) override
    {
      EXPECT_EQ(m_currentNum, static_cast<DummyQuery&>(query).getNum());
      m_currentNum++;
    }

    void visit(API::IQuerySequence& query) override
    {
    }

  private:
    int m_currentNum;

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

  TestVisitor visitor;
  seq.accept(visitor);
}
