#include <gtest/gtest.h>

#include <Common.h>

namespace QtSqlLibTest
{

static void expectSpecialRelation1Students(QtSqlLib::API::IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::SpecialRel1,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Student1", QVariantList() << "Project1" << "Project2");

  Funcs::expectRelations(tuples, Relationships::SpecialRel1,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Student2", QVariantList());

  Funcs::expectRelations(tuples, Relationships::SpecialRel1,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Student3", QVariantList());
}

static void expectSpecialRelation2Projects(QtSqlLib::API::IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::SpecialRel2,
    TableIds::Projects, ProjectsCols::Title, TableIds::Students, StudentsCols::Name,
    "Project1", QVariantList() << "Student1");

  Funcs::expectRelations(tuples, Relationships::SpecialRel2,
    TableIds::Projects, ProjectsCols::Title, TableIds::Students, StudentsCols::Name,
    "Project2", QVariantList() << "Student1");

  Funcs::expectRelations(tuples, Relationships::SpecialRel2,
    TableIds::Projects, ProjectsCols::Title, TableIds::Students, StudentsCols::Name,
    "Project3", QVariantList() << "Student2");
}

static void expectSpecialRelation2Students(QtSqlLib::API::IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::SpecialRel2,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Student1", QVariantList() << "Project1" << "Project2");

  Funcs::expectRelations(tuples, Relationships::SpecialRel2,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Student2", QVariantList() << "Project3");

  Funcs::expectRelations(tuples, Relationships::SpecialRel2,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Student3", QVariantList());
}

static void expectSpecialRelation3Students(QtSqlLib::API::IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::SpecialRel3,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Student1", QVariantList() << "Project1" << "Project2" << "Project3");

  Funcs::expectRelations(tuples, Relationships::SpecialRel3,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Student2", QVariantList() << "Project1");

  Funcs::expectRelations(tuples, Relationships::SpecialRel3,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Student3", QVariantList() << "Project1" << "Project2");
}

static void expectSpecialRelation4Students(QtSqlLib::API::IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::SpecialRel4,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Student1", QVariantList() << "Project3");

  Funcs::expectRelations(tuples, Relationships::SpecialRel4,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Student2", QVariantList());

  Funcs::expectRelations(tuples, Relationships::SpecialRel4,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Student3", QVariantList() << "Project1" << "Project2" << "Project3");
}

static void expectSpecialRelation5Students(QtSqlLib::API::IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::SpecialRel5,
    TableIds::Students, StudentsCols::Name, TableIds::Students, StudentsCols::Name,
    "Student1", QVariantList() << "Student1" << "Student2");

  Funcs::expectRelations(tuples, Relationships::SpecialRel5,
    TableIds::Students, StudentsCols::Name, TableIds::Students, StudentsCols::Name,
    "Student2", QVariantList() << "Student3");

  Funcs::expectRelations(tuples, Relationships::SpecialRel5,
    TableIds::Students, StudentsCols::Name, TableIds::Students, StudentsCols::Name,
    "Student3", QVariantList());
}

static void expectSpecialRelation6Students(QtSqlLib::API::IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::SpecialRel6,
    TableIds::Students, StudentsCols::Name, TableIds::Students, StudentsCols::Name,
    "Student1", QVariantList() << "Student2" << "Student3");

  Funcs::expectRelations(tuples, Relationships::SpecialRel6,
    TableIds::Students, StudentsCols::Name, TableIds::Students, StudentsCols::Name,
    "Student2", QVariantList() << "Student3");

  Funcs::expectRelations(tuples, Relationships::SpecialRel6,
    TableIds::Students, StudentsCols::Name, TableIds::Students, StudentsCols::Name,
    "Student3", QVariantList());
}

static void setupReplationshipTestsSchema(DatabaseDummy& db)
{
  db.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    configurator.configureTable(TableIds::Students, "students")
      .column(StudentsCols::Id, "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(StudentsCols::Name, "name", DataType::Varchar, 128);

    configurator.configureTable(TableIds::Projects, "projects")
      .column(ProjectsCols::Id, "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(ProjectsCols::Title, "title", DataType::Varchar, 128);

    configurator.configureTable(TableIds::Lectures, "lectures")
      .column(LecturesCols::Id, "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(LecturesCols::Topic, "topic", DataType::Varchar, 128);

    configurator.configureRelationship(Relationships::RelationshipStudentsProjects, TableIds::Students, TableIds::Projects,
      Schema::RelationshipType::OneToMany).onDelete(Schema::ForeignKeyAction::Cascade);

    configurator.configureRelationship(Relationships::RelationshipStudentsLectures, TableIds::Students, TableIds::Lectures,
      Schema::RelationshipType::ManyToMany);
  });
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
              >    Programming
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
TEST(RelationshipTest, linkTuplesOnInsertTest)
{
  DatabaseDummy db;

  setupReplationshipTestsSchema(db);

  db.initialize(Funcs::getDefaultDatabaseFilename());

  // Insert Lecture "Math"
  const auto lectureMath = db.execQuery(InsertIntoExt(TableIds::Lectures)
    .value(LecturesCols::Topic, "Math")
    .returnIds()).resultTuples[0].values;

  // Insert Project "Game Programming"
  const auto projectGameProgramming = db.execQuery(InsertIntoExt(TableIds::Projects)
    .value(ProjectsCols::Title, "Game Programming")
    .returnIds()).resultTuples[0].values;

  // Insert Project "Modeling"
  const auto projectModeling = db.execQuery(InsertIntoExt(TableIds::Projects)
    .value(ProjectsCols::Title, "Modeling")
    .returnIds()).resultTuples[0].values;

  // Insert Project "Machine Learning"
  const auto projectMachineLearning = db.execQuery(InsertIntoExt(TableIds::Projects)
    .value(ProjectsCols::Title, "Machine Learning")
    .returnIds()).resultTuples[0].values;
  
  // (1) Insert Student "John" and link directly to Lecture "Math"
  const auto studentJohn = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "John")
    .linkToOneTuple(Relationships::RelationshipStudentsLectures, lectureMath)
    .returnIds()).resultTuples[0].values;

  // (7) Insert Project "Computer vision" and link direktly to Student "John"
  const auto projectComputerVision = db.execQuery(InsertIntoExt(TableIds::Projects)
    .value(ProjectsCols::Title, "Computer Vision")
    .linkToOneTuple(Relationships::RelationshipStudentsProjects, studentJohn)
    .returnIds()).resultTuples[0].values;

  // (3) Insert Lecture "Programming" and link directly to Student "John"
  const auto lectureProgramming = db.execQuery(InsertIntoExt(TableIds::Lectures)
    .value(LecturesCols::Topic, "Programming")
    .linkToOneTuple(Relationships::RelationshipStudentsLectures, studentJohn)
    .returnIds()).resultTuples[0].values;

  // (1) (5) Insert Student Mary and link directly to Lecture "Programming" and to Project "Game Programming"
  const auto studentMary = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "Mary")
    .linkToOneTuple(Relationships::RelationshipStudentsLectures, lectureProgramming)
    .linkToOneTuple(Relationships::RelationshipStudentsProjects, projectGameProgramming)
    .returnIds()).resultTuples[0].values;
  
  // (4) Insert Lecture "Database Systems" and link directly to Students "John" and "Mary"
  const auto lectureDbSystems = db.execQuery(InsertIntoExt(TableIds::Lectures)
    .value(LecturesCols::Topic, "Database Systems")
    .linkToManyTuples(Relationships::RelationshipStudentsLectures, { studentJohn, studentMary })
    .returnIds()).resultTuples[0].values;
  
  // (2) (6) Insert Student Paul and link directly to Lectures "Math" and "Programming"
  //         AND to Projects "Game Programming" AND "Machine Learning"
  const auto studentPaul = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "Paul")
    .linkToManyTuples(Relationships::RelationshipStudentsLectures, { lectureMath, lectureProgramming })
    .linkToManyTuples(Relationships::RelationshipStudentsProjects, { projectModeling, projectMachineLearning })
    .returnIds()).resultTuples[0].values;

  // Expectations:
  auto results = db.execQuery(FromTable(TableIds::Students)
    .select(StudentsCols::Name)
    .joinColumns(Relationships::RelationshipStudentsLectures, LecturesCols::Topic));

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "John", QVariantList() << "Math" << "Programming" << "Database Systems");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Mary", QVariantList() << "Programming" << "Database Systems");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Paul", QVariantList() << "Math" << "Programming");

  results = db.execQuery(FromTable(TableIds::Lectures)
    .selectAll()
    .joinColumns(Relationships::RelationshipStudentsLectures, StudentsCols::Name));

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Students, StudentsCols::Name,
    "Math", QVariantList() << "John" << "Paul");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Students, StudentsCols::Name,
    "Programming", QVariantList() << "John" << "Mary" << "Paul");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Students, StudentsCols::Name,
    "Database Systems", QVariantList() << "John" << "Mary");

  results = db.execQuery(FromTable(TableIds::Students)
    .select(StudentsCols::Name)
    .joinAll(Relationships::RelationshipStudentsProjects));

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "John", QVariantList() << "Computer Vision");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Mary", QVariantList() << "Game Programming");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Paul", QVariantList() << "Modeling" << "Machine Learning");

  results = db.execQuery(FromTable(TableIds::Projects)
    .selectAll()
    .joinAll(Relationships::RelationshipStudentsProjects));

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Projects, ProjectsCols::Title,TableIds::Students, StudentsCols::Name,
    "Computer Vision", QVariantList() << "John");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Projects, ProjectsCols::Title,TableIds::Students, StudentsCols::Name,
    "Game Programming", QVariantList() << "Mary");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Projects, ProjectsCols::Title,TableIds::Students, StudentsCols::Name,
    "Modeling", QVariantList() << "Paul");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Projects, ProjectsCols::Title,TableIds::Students, StudentsCols::Name,
    "Machine Learning", QVariantList() << "Paul");

  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::RelationshipStudentsProjects)
    .joinAll(Relationships::RelationshipStudentsLectures));

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "John", QVariantList() << "Math" << "Programming" << "Database Systems");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Mary", QVariantList() << "Programming" << "Database Systems");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Paul", QVariantList() << "Math" << "Programming");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "John", QVariantList() << "Computer Vision");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Mary", QVariantList() << "Game Programming");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Paul", QVariantList() << "Modeling" << "Machine Learning");
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
TEST(RelationshipTest, linkTuplesQueryTest)
{
  DatabaseDummy db;
  setupReplationshipTestsSchema(db);

  db.initialize(Funcs::getDefaultDatabaseFilename());

  // Insert tuples
  const auto studentJohn = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "John")
    .returnIds()).resultTuples[0].values;

  const auto studentMary = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "Mary")
    .returnIds()).resultTuples[0].values;

  const auto studentPaul = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "Paul")
    .returnIds()).resultTuples[0].values;

  const auto lectureMath = db.execQuery(InsertIntoExt(TableIds::Lectures)
    .value(LecturesCols::Topic, "Math")
    .returnIds()).resultTuples[0].values;

  const auto lectureProgramming = db.execQuery(InsertIntoExt(TableIds::Lectures)
    .value(LecturesCols::Topic, "Programming")
    .returnIds()).resultTuples[0].values;

  const auto lectureDbSystems = db.execQuery(InsertIntoExt(TableIds::Lectures)
    .value(LecturesCols::Topic, "Database Systems")
    .returnIds()).resultTuples[0].values;

  const auto projectGameProgramming = db.execQuery(InsertIntoExt(TableIds::Projects)
    .value(ProjectsCols::Title, "Game Programming")
    .returnIds()).resultTuples[0].values;

  const auto projectModeling = db.execQuery(InsertIntoExt(TableIds::Projects)
    .value(ProjectsCols::Title, "Modeling")
    .returnIds()).resultTuples[0].values;

  const auto projectMachineLearning = db.execQuery(InsertIntoExt(TableIds::Projects)
    .value(ProjectsCols::Title, "Machine Learning")
    .returnIds()).resultTuples[0].values;

  const auto projectComputerVision = db.execQuery(InsertIntoExt(TableIds::Projects)
    .value(ProjectsCols::Title, "Computer Vision")
    .returnIds()).resultTuples[0].values;

  // (1)
  db.execQuery(LinkTuples(Relationships::RelationshipStudentsLectures)
    .fromOne(studentMary)
    .toOne(lectureProgramming));

  // (2)
  db.execQuery(LinkTuples(Relationships::RelationshipStudentsLectures)
    .fromOne(studentPaul)
    .toMany({ lectureMath, lectureProgramming }));

  // (3)
  db.execQuery(LinkTuples(Relationships::RelationshipStudentsLectures)
    .fromOne(lectureMath)
    .toOne(studentJohn));

  // (4)
  db.execQuery(LinkTuples(Relationships::RelationshipStudentsLectures)
    .fromOne(lectureDbSystems)
    .toMany({ studentJohn, studentMary }));

  // (5)
  db.execQuery(LinkTuples(Relationships::RelationshipStudentsProjects)
    .fromOne(studentMary)
    .toOne(projectGameProgramming));

  // (6)
  db.execQuery(LinkTuples(Relationships::RelationshipStudentsProjects)
    .fromOne(studentPaul)
    .toMany({ projectModeling, projectMachineLearning }));

  // (7)
  db.execQuery(LinkTuples(Relationships::RelationshipStudentsProjects)
    .fromOne(projectComputerVision)
    .toOne(studentJohn));

  // (8)
  EXPECT_THROW(db.execQuery(LinkTuples(Relationships::RelationshipStudentsProjects)
    .fromOne(projectComputerVision)
    .toMany({ studentJohn, studentMary }))
    , DatabaseException);

  EXPECT_THROW(db.execQuery(InsertIntoExt(TableIds::Projects)
    .value(ProjectsCols::Title, "Dummy")
    .linkToManyTuples(Relationships::RelationshipStudentsProjects, { studentJohn, studentMary }))
    , DatabaseException);

  // (9)
  EXPECT_THROW(db.execQuery(InsertIntoExt(TableIds::Projects)
    .value(ProjectsCols::Title, "Dummy")
    .linkToOneTuple(Relationships::RelationshipStudentsLectures, studentJohn))
    , DatabaseException);

  // (10)
  EXPECT_THROW(db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "Dummy")
    .linkToOneTuple(Relationships::RelationshipStudentsLectures, projectMachineLearning))
    , DatabaseException);

  // (11)
  EXPECT_THROW(db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "Dummy")
    .linkToManyTuples(Relationships::RelationshipStudentsLectures, { projectMachineLearning, lectureDbSystems }))
    , DatabaseException);

  // (12)
  EXPECT_THROW(db.execQuery(LinkTuples(Relationships::RelationshipStudentsLectures)
    .fromOne(projectComputerVision)
    .toOne(lectureProgramming))
    , DatabaseException);

  // (13)
  EXPECT_THROW(db.execQuery(LinkTuples(Relationships::RelationshipStudentsLectures)
    .fromOne(studentMary)
    .toOne(projectComputerVision))
    , DatabaseException);

  // (14)
  EXPECT_THROW(db.execQuery(LinkTuples(Relationships::RelationshipStudentsLectures)
    .fromOne(studentMary)
    .toMany({ projectComputerVision, lectureProgramming }))
    , DatabaseException);

  // Expectations:
  auto results = db.execQuery(FromTable(TableIds::Students)
    .select(StudentsCols::Name)
    .joinColumns(Relationships::RelationshipStudentsLectures, LecturesCols::Topic));

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "John", QVariantList() << "Math" << "Database Systems");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Mary", QVariantList() << "Programming" << "Database Systems");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Paul", QVariantList() << "Math" << "Programming");

  results = db.execQuery(FromTable(TableIds::Lectures)
    .selectAll()
    .joinColumns(Relationships::RelationshipStudentsLectures, StudentsCols::Name));

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Students, StudentsCols::Name,
    "Math", QVariantList() << "John" << "Paul");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Students, StudentsCols::Name,
    "Programming", QVariantList() << "Mary" << "Paul");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Students, StudentsCols::Name,
    "Database Systems", QVariantList() << "John" << "Mary");

  results = db.execQuery(FromTable(TableIds::Students)
    .select(StudentsCols::Name)
    .joinAll(Relationships::RelationshipStudentsProjects));

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "John", QVariantList() << "Computer Vision");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Mary", QVariantList() << "Game Programming");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Paul", QVariantList() << "Modeling" << "Machine Learning");

  results = db.execQuery(FromTable(TableIds::Projects)
    .selectAll()
    .joinAll(Relationships::RelationshipStudentsProjects));

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Projects, ProjectsCols::Title,TableIds::Students, StudentsCols::Name,
    "Computer Vision", QVariantList() << "John");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Projects, ProjectsCols::Title,TableIds::Students, StudentsCols::Name,
    "Game Programming", QVariantList() << "Mary");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Projects, ProjectsCols::Title,TableIds::Students, StudentsCols::Name,
    "Modeling", QVariantList() << "Paul");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Projects, ProjectsCols::Title,TableIds::Students, StudentsCols::Name,
    "Machine Learning", QVariantList() << "Paul");

  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::RelationshipStudentsProjects)
    .joinAll(Relationships::RelationshipStudentsLectures));

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "John", QVariantList() << "Math" << "Database Systems");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Mary", QVariantList() << "Programming" << "Database Systems");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsLectures,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Paul", QVariantList() << "Math" << "Programming");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "John", QVariantList() << "Computer Vision");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Mary", QVariantList() << "Game Programming");

  Funcs::expectRelations(results.resultTuples, Relationships::RelationshipStudentsProjects,
    TableIds::Students, StudentsCols::Name, TableIds::Projects, ProjectsCols::Title,
    "Paul", QVariantList() << "Modeling" << "Machine Learning");
}

TEST(RelationshipTest, specialRelationships)
{
  DatabaseDummy db;

  db.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    configurator.configureTable(TableIds::Students, "students")
      .column(StudentsCols::Id, "id", DataType::Integer).autoIncrement().notNull()
      .column(StudentsCols::Name, "name", DataType::Varchar, 128)
      .primaryKeys(StudentsCols::Id);

    configurator.configureTable(TableIds::Projects, "projects")
      .column(ProjectsCols::Id, "id", DataType::Integer).autoIncrement().notNull()
      .column(ProjectsCols::Title, "title", DataType::Varchar, 128)
      .primaryKeys(ProjectsCols::Id);

    configurator.configureRelationship(Relationships::SpecialRel1, TableIds::Students, TableIds::Projects, Schema::RelationshipType::OneToMany);
    configurator.configureRelationship(Relationships::SpecialRel2, TableIds::Students, TableIds::Projects, Schema::RelationshipType::OneToMany);
    configurator.configureRelationship(Relationships::SpecialRel3, TableIds::Students, TableIds::Projects, Schema::RelationshipType::ManyToMany);
    configurator.configureRelationship(Relationships::SpecialRel4, TableIds::Students, TableIds::Projects, Schema::RelationshipType::ManyToMany);
    configurator.configureRelationship(Relationships::SpecialRel5, TableIds::Students, TableIds::Students, Schema::RelationshipType::OneToMany);
    configurator.configureRelationship(Relationships::SpecialRel6, TableIds::Students, TableIds::Students, Schema::RelationshipType::ManyToMany);
  });

  db.initialize(Funcs::getDefaultDatabaseFilename());

  const auto student1 = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "Student1")
    .returnIds()).resultTuples[0].values;

  const auto student2 = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "Student2")
    .returnIds()).resultTuples[0].values;

  const auto student3 = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "Student3")
    .returnIds()).resultTuples[0].values;

  const auto project1 = db.execQuery(InsertIntoExt(TableIds::Projects)
    .value(ProjectsCols::Title, "Project1")
    .returnIds()).resultTuples[0].values;

  const auto project2 = db.execQuery(InsertIntoExt(TableIds::Projects)
    .value(ProjectsCols::Title, "Project2")
    .returnIds()).resultTuples[0].values;

  const auto project3 = db.execQuery(InsertIntoExt(TableIds::Projects)
    .value(ProjectsCols::Title, "Project3")
    .returnIds()).resultTuples[0].values;

  db.execQuery(LinkTuples(Relationships::SpecialRel1)
    .fromOne(student1)
    .toOne(project1));

  db.execQuery(LinkTuples(Relationships::SpecialRel2)
    .fromOne(student1)
    .toMany({ project1, project2 }));

  db.execQuery(LinkTuples(Relationships::SpecialRel1)
    .fromOne(project2)
    .toOne({ student1 }));

  db.execQuery(LinkTuples(Relationships::SpecialRel2)
    .fromOne(project3)
    .toOne({ student2 }));

  db.execQuery(LinkTuples(Relationships::SpecialRel3)
    .fromOne(project1)
    .toMany ({ student2, student3 }));

  db.execQuery(LinkTuples(Relationships::SpecialRel3)
    .fromOne(student1)
    .toMany ({ project1, project3 }));

  db.execQuery(LinkTuples(Relationships::SpecialRel3)
    .fromOne(project2)
    .toMany ({ student1, student3 }));

  db.execQuery(LinkTuples(Relationships::SpecialRel4)
    .fromOne(project3)
    .toMany ({ student1 }));

  db.execQuery(LinkTuples(Relationships::SpecialRel4)
    .fromOne(student3)
    .toMany ({ project1, project2, project3 }));

  db.execQuery(LinkTuples(Relationships::SpecialRel5)
    .fromOne(student1)
    .toMany ({ student1, student2 }));

  db.execQuery(LinkTuples(Relationships::SpecialRel5)
    .fromOne(student2)
    .toMany ({ student3 }));

  db.execQuery(LinkTuples(Relationships::SpecialRel6)
    .fromOne(student1)
    .toMany ({ student2, student3 }));

  db.execQuery(LinkTuples(Relationships::SpecialRel6)
    .fromOne(student2)
    .toOne ({ student3 }));

  auto results = db.execQuery(FromTable(TableIds::Students)
    .select(StudentsCols::Name)
    .joinColumns(Relationships::SpecialRel1, ProjectsCols::Title));

  expectSpecialRelation1Students(results.resultTuples);

  results = db.execQuery(FromTable(TableIds::Projects)
    .selectAll()
    .joinAll(Relationships::SpecialRel2));

  expectSpecialRelation2Projects(results.resultTuples);
  
  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::SpecialRel1)
    .joinAll(Relationships::SpecialRel2));

  expectSpecialRelation1Students(results.resultTuples);
  expectSpecialRelation2Students(results.resultTuples);

  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::SpecialRel3));

  expectSpecialRelation3Students(results.resultTuples);

  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::SpecialRel4));

  expectSpecialRelation4Students(results.resultTuples);

  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::SpecialRel3)
    .joinAll(Relationships::SpecialRel4));

  expectSpecialRelation3Students(results.resultTuples);
  expectSpecialRelation4Students(results.resultTuples);

  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::SpecialRel1)
    .joinAll(Relationships::SpecialRel2)
    .joinAll(Relationships::SpecialRel3)
    .joinAll(Relationships::SpecialRel4));

  expectSpecialRelation1Students(results.resultTuples);
  expectSpecialRelation2Students(results.resultTuples);
  expectSpecialRelation3Students(results.resultTuples);
  expectSpecialRelation4Students(results.resultTuples);

  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::SpecialRel5));

  expectSpecialRelation5Students(results.resultTuples);

  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::SpecialRel6));

  expectSpecialRelation6Students(results.resultTuples);

  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::SpecialRel1)
    .joinAll(Relationships::SpecialRel2)
    .joinAll(Relationships::SpecialRel3)
    .joinAll(Relationships::SpecialRel4)
    .joinAll(Relationships::SpecialRel5)
    .joinAll(Relationships::SpecialRel6));

  expectSpecialRelation1Students(results.resultTuples);
  expectSpecialRelation2Students(results.resultTuples);
  expectSpecialRelation3Students(results.resultTuples);
  expectSpecialRelation4Students(results.resultTuples);
  expectSpecialRelation5Students(results.resultTuples);
  expectSpecialRelation6Students(results.resultTuples);
}


}
