#include <gtest/gtest.h>

#include <Common.h>

namespace QtSqlLibTest
{

static void expectStudentsConfidantStudents(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::StudentsConfidant,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "John", QVariantList() << "Prof. Smith");

  Funcs::expectRelations(tuples, Relationships::StudentsConfidant,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "Mary", QVariantList() << "Prof. Smith");

  Funcs::expectRelations(tuples, Relationships::StudentsConfidant,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "Paul", QVariantList() << "Dr. Evans");

  Funcs::expectRelations(tuples, Relationships::StudentsConfidant,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "Sarah", QVariantList() << "Prof. Adams");
}

static void expectStudentsConfidantProfessors(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::StudentsConfidant,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "Prof. Smith", QVariantList() << "John" << "Mary");

  Funcs::expectRelations(tuples, Relationships::StudentsConfidant,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "Dr. Evans", QVariantList() << "Paul");

  Funcs::expectRelations(tuples, Relationships::StudentsConfidant,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "Prof. Adams", QVariantList() << "Sarah");
}

static void expectLecturerProfessors(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::Lecturer,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Prof. Smith", QVariantList() << "Programming");

  Funcs::expectRelations(tuples, Relationships::Lecturer,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Dr. Evans", QVariantList() << "Math");

  Funcs::expectRelations(tuples, Relationships::Lecturer,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Prof. Adams", QVariantList() << "Operating systems" << "Database systems");
}

static void expectLecturerLectures(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::Lecturer,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Professors, ProfessorsCols::Name,
    "Programming", QVariantList() << "Prof. Smith");

  Funcs::expectRelations(tuples, Relationships::Lecturer,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Professors, ProfessorsCols::Name,
    "Math", QVariantList() << "Dr. Evans");

  Funcs::expectRelations(tuples, Relationships::Lecturer,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Professors, ProfessorsCols::Name,
    "Operating systems", QVariantList() << "Prof. Adams");

  Funcs::expectRelations(tuples, Relationships::Lecturer,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Professors, ProfessorsCols::Name,
    "Database systems", QVariantList() << "Prof. Adams");
}

static void expectLectureParticipantLectures(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::LectureParticipant,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Students, StudentsCols::Name,
    "Operating systems", QVariantList() << "Sarah");

  Funcs::expectRelations(tuples, Relationships::LectureParticipant,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Students, StudentsCols::Name,
    "Database systems", QVariantList() << "Paul" << "Sarah");

  Funcs::expectRelations(tuples, Relationships::LectureParticipant,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Students, StudentsCols::Name,
    "Math", QVariantList() << "John" << "Mary" << "Paul");

  Funcs::expectRelations(tuples, Relationships::LectureParticipant,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Students, StudentsCols::Name,
    "Programming", QVariantList() << "John" << "Mary" << "Sarah");
}

static void expectLectureParticipantStudents(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "John", QVariantList() << "Math" << "Programming");

  Funcs::expectRelations(tuples, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Mary", QVariantList() << "Math" << "Programming");

  Funcs::expectRelations(tuples, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Paul", QVariantList() << "Database systems" << "Math");

  Funcs::expectRelations(tuples, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Sarah", QVariantList() << "Operating systems" << "Database systems" << "Programming");
}

static void expectSpecialRelation1Students(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::Special1,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "student1", QVariantList() << "professor1" << "professor2");

  Funcs::expectRelations(tuples, Relationships::Special1,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "student2", QVariantList());

  Funcs::expectRelations(tuples, Relationships::Special1,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "student3", QVariantList());
}

static void expectSpecialRelation1Professors(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::Special1,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "professor1", QVariantList() << "student1");

  Funcs::expectRelations(tuples, Relationships::Special1,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "professor2", QVariantList() << "student1");

  Funcs::expectRelations(tuples, Relationships::Special1,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "professor3", QVariantList());
}

static void expectSpecialRelation2Students(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::Special2,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "student1", QVariantList() << "professor1" << "professor2");

  Funcs::expectRelations(tuples, Relationships::Special2,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "student2", QVariantList() << "professor3");

  Funcs::expectRelations(tuples, Relationships::Special2,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "student3", QVariantList());
}

static void expectSpecialRelation2Professors(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::Special2,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "professor1", QVariantList() << "student1");

  Funcs::expectRelations(tuples, Relationships::Special2,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "professor2", QVariantList() << "student1");

  Funcs::expectRelations(tuples, Relationships::Special2,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "professor3", QVariantList() << "student2");
}

static void expectSpecialRelation3Students(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::Special3,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "student1", QVariantList() << "professor1" << "professor2" << "professor3");

  Funcs::expectRelations(tuples, Relationships::Special3,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "student2", QVariantList() << "professor1");

  Funcs::expectRelations(tuples, Relationships::Special3,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "student3", QVariantList() << "professor1" << "professor2");
}

static void expectSpecialRelation3Professors(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::Special3,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "professor1", QVariantList() << "student1" << "student2" << "student3");

  Funcs::expectRelations(tuples, Relationships::Special3,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "professor2", QVariantList() << "student1" << "student3");

  Funcs::expectRelations(tuples, Relationships::Special3,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "professor3", QVariantList() << "student1");
}

static void expectSpecialRelation4Students(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::Special4,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "student1", QVariantList() << "professor3");

  Funcs::expectRelations(tuples, Relationships::Special4,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "student2", QVariantList());

  Funcs::expectRelations(tuples, Relationships::Special4,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "student3", QVariantList() << "professor1" << "professor2" << "professor3");
}

static void expectSpecialRelation4Professors(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::Special4,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "professor1", QVariantList() << "student3");

  Funcs::expectRelations(tuples, Relationships::Special4,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "professor2", QVariantList() << "student3");

  Funcs::expectRelations(tuples, Relationships::Special4,
    TableIds::Professors, ProfessorsCols::Name, TableIds::Students, StudentsCols::Name,
    "professor3", QVariantList() << "student1" << "student3");
}

static void expectSpecialRelation5Students(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::Special5,
    TableIds::Students, StudentsCols::Name, TableIds::Students, StudentsCols::Name,
    "student1", QVariantList() << "student1" << "student2");

  Funcs::expectRelations(tuples, Relationships::Special5,
    TableIds::Students, StudentsCols::Name, TableIds::Students, StudentsCols::Name,
    "student2", QVariantList() << "student3");

  Funcs::expectRelations(tuples, Relationships::Special5,
    TableIds::Students, StudentsCols::Name, TableIds::Students, StudentsCols::Name,
    "student3", QVariantList());
}

static void expectSpecialRelation6Students(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, Relationships::Special6,
    TableIds::Students, StudentsCols::Name, TableIds::Students, StudentsCols::Name,
    "student1", QVariantList() << "student2" << "student3");

  Funcs::expectRelations(tuples, Relationships::Special6,
    TableIds::Students, StudentsCols::Name, TableIds::Students, StudentsCols::Name,
    "student2", QVariantList() << "student3");

  Funcs::expectRelations(tuples, Relationships::Special6,
    TableIds::Students, StudentsCols::Name, TableIds::Students, StudentsCols::Name,
    "student3", QVariantList());
}

/**
 * @description:
 * Configures the database schema with three tables (students, professors, lectures)
 * and three relationships as follows:
 *   (a) StudentsConfident:  students   N<---->1 professors (ManyToOne)
 *   (b) Lecturer:           professors 1<---->N lectures   (OneToMany)
 *   (c) LectureParticipant: lectures   N<---->M students   (ManyToMany)
 *
 * The test cases that use this schema will insert these tupels:
 *   - students:   [ John, Mary, Paul, Sarah ]
 *   - professors: [ Prof. Smith, Dr. Evans, Prof. Adams ]
 *   - lectures:   [ Operating systems, Database systems, Math, Programming ]
 *
 * and the following relations:
 *   (a) [ John, Mary ]  N<-->1 [ Prof. Smith ]
 *       [ Paul ]        N<-->1 [ Dr. Evans ]
 *       [ Sarah ]       N<-->1 [ Prof. Adams ]
 *   (b) [ Prof. Smith ] 1<-->N [ Programming ]
 *       [ Dr. Evans ]   1<-->N [ Math ]
 *       [ Prof. Adams ] 1<-->N [ Operating systems, Database systems ]
 *   (c) [ John ]        N<-->M [ Math, Programming ]
 *       [ Mary ]        N<-->M [ Math, Programming ]
 *       [ Paul ]        N<-->M [ Database systems, Math ]
 *       [ Sarah ]       N<-->M [ Operating systems, Database systems, Programming ]
 */
static void setupReplationshipTestsDatabase(TestDatabase& db)
{
  SchemaConfigurator configurator;
  configurator.configureTable(TableIds::Students, "students")
    .column(StudentsCols::Id, "id", DataType::Integer).primaryKey().autoIncrement().notNull()
    .column(StudentsCols::Name, "name", DataType::Varchar, 128);

  configurator.configureTable(TableIds::Professors, "professors")
    .column(ProfessorsCols::Id, "id", DataType::Integer).primaryKey().autoIncrement().notNull()
    .column(ProfessorsCols::Name, "name", DataType::Varchar, 128);

  configurator.configureTable(TableIds::Lectures, "lectures")
    .column(LecturesCols::Id, "id", DataType::Integer).primaryKey().autoIncrement().notNull()
    .column(LecturesCols::Topic, "topic", DataType::Varchar, 128);

  configurator.configureRelationship(Relationships::StudentsConfidant, TableIds::Students, TableIds::Professors,
    ISchema::RelationshipType::ManyToOne).onDelete(ISchema::ForeignKeyAction::Cascade);

  configurator.configureRelationship(Relationships::Lecturer, TableIds::Professors, TableIds::Lectures,
    ISchema::RelationshipType::OneToMany).onDelete(ISchema::ForeignKeyAction::Cascade);

  configurator.configureRelationship(Relationships::LectureParticipant, TableIds::Students, TableIds::Lectures,
    ISchema::RelationshipType::ManyToMany);

  db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
}

/**
 * @description:
 * This test adds links between tuples of related tables directly on data insertion
 * whithin the #InsertIntoExt query.
 * For more information about the created tables and relationships, see #setupReplationshipTestsDatabase().
 *
 * Create tuples and links between them as follows:
 *   (1) Insert student John
 *       Insert lecture Operating systems
 *       Insert lecture Database systems
 *   (2) Insert professor Prof. Smith
 *         Link toOne [ John ] (a)
 *   (3) Insert lecture Math
 *         Link toOne [ John ] (c)
 *   (4) Insert student Mary
 *         Link toOne [ Prof. Smith ] (a)
 *         Link toOne [ Math ] (c)
 *   (5) Insert professor Dr. Evans
 *         Link toOne [ Math ] (b)
 *   (6) Insert lecture Programming
 *         Link toMany [ John, Mary ] (c)
 *         Link toOne  [ Prof. Smith ] (b)
 *   (7) Insert student Paul
 *         Link toOne  [ Dr. Evans ] (a)
 *         Link toMany [ Database systems, Math ] (c)
 *   (8) Insert student Sarah
 *         Link toMany [ Operating systems, Database systems, Programming ] (c)
 *   (9) Insert professor Prof. Adams
 *         Link toOne  [ Sarah ] (a)
 *         Link toMany [ Operating systems, Database systems ] (b)
 *
 * Check the correctness of the inserted relations between tuples by querying the data and
 * compare the result tuples (including joined tuples) against our expectations.
 *   (10) Query students and related professors (a)
 *   (11) Query professors and related students (a)
 *   (12) Query professors and related lectures (b)
 *   (13) Query lectures and related professors (b)
 *   (14) Query lectures and related students (c)
 *   (15) Query students and related lectures (c)
 *   (16) Query students, related professors (a) and related lectures (c)
 *   (17) Query professors, related students (a) and related lectures (b)
 *   (18) Query lectures, related professors (b) and related students (a)
 */
TEST(RelationshipTest, linkTuplesOnInsertTest)
{
  TestDatabase db;
  setupReplationshipTestsDatabase(db);

  // (1)
  const auto studentJohn = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "John")
    .returnIds()).resultTuples[0].values;

  const auto lectureOs = db.execQuery(InsertIntoExt(TableIds::Lectures)
    .value(LecturesCols::Topic, "Operating systems")
    .returnIds()).resultTuples[0].values;

  const auto lectureDbs = db.execQuery(InsertIntoExt(TableIds::Lectures)
    .value(LecturesCols::Topic, "Database systems")
    .returnIds()).resultTuples[0].values;

  // (2)
  const auto profSmith = db.execQuery(InsertIntoExt(TableIds::Professors)
    .value(ProfessorsCols::Name, "Prof. Smith")
    .linkToOneTuple(Relationships::StudentsConfidant, studentJohn)
    .returnIds()).resultTuples[0].values;

  // (3)
  const auto lectureMath = db.execQuery(InsertIntoExt(TableIds::Lectures)
    .value(LecturesCols::Topic, "Math")
    .linkToOneTuple(Relationships::LectureParticipant, studentJohn)
    .returnIds()).resultTuples[0].values;

  // (4)
  const auto studentMary = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "Mary")
    .linkToOneTuple(Relationships::StudentsConfidant, profSmith)
    .linkToOneTuple(Relationships::LectureParticipant, lectureMath)
    .returnIds()).resultTuples[0].values;

  // (5)
  const auto profEvans = db.execQuery(InsertIntoExt(TableIds::Professors)
    .value(ProfessorsCols::Name, "Dr. Evans")
    .linkToOneTuple(Relationships::Lecturer, lectureMath)
    .returnIds()).resultTuples[0].values;

  // (6)
  const auto lectureProgramming = db.execQuery(InsertIntoExt(TableIds::Lectures)
    .value(LecturesCols::Topic, "Programming")
    .linkToManyTuples(Relationships::LectureParticipant, { studentJohn, studentMary })
    .linkToOneTuple(Relationships::Lecturer, profSmith)
    .returnIds()).resultTuples[0].values;

  // (7)
  const auto studentPaul = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "Paul")
    .linkToOneTuple(Relationships::StudentsConfidant, profEvans)
    .linkToManyTuples(Relationships::LectureParticipant, { lectureDbs, lectureMath })
    .returnIds()).resultTuples[0].values;

  // (8)
  const auto studentSarah = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "Sarah")
    .linkToManyTuples(Relationships::LectureParticipant, { lectureOs, lectureDbs, lectureProgramming })
    .returnIds()).resultTuples[0].values;

  // (9)
  const auto profAdams = db.execQuery(InsertIntoExt(TableIds::Professors)
    .value(ProfessorsCols::Name, "Prof. Adams")
    .linkToOneTuple(Relationships::StudentsConfidant, studentSarah)
    .linkToManyTuples(Relationships::Lecturer, { lectureOs, lectureDbs })
    .returnIds()).resultTuples[0].values;

  // (10)
  auto results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::StudentsConfidant));

  expectStudentsConfidantStudents(results.resultTuples);

  // (11)
  results = db.execQuery(FromTable(TableIds::Professors)
    .selectAll()
    .joinAll(Relationships::StudentsConfidant));

  expectStudentsConfidantProfessors(results.resultTuples);

  // (12)
  results = db.execQuery(FromTable(TableIds::Professors)
    .selectAll()
    .joinAll(Relationships::Lecturer));

  expectLecturerProfessors(results.resultTuples);

  // (13)
  results = db.execQuery(FromTable(TableIds::Lectures)
    .selectAll()
    .joinAll(Relationships::Lecturer));

  expectLecturerLectures(results.resultTuples);

  // (14)
  results = db.execQuery(FromTable(TableIds::Lectures)
    .selectAll()
    .joinAll(Relationships::LectureParticipant));

  expectLectureParticipantLectures(results.resultTuples);

  // (15)
  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::LectureParticipant));

  expectLectureParticipantStudents(results.resultTuples);

  // (16)
  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::StudentsConfidant)
    .joinAll(Relationships::LectureParticipant));;

  expectStudentsConfidantStudents(results.resultTuples);
  expectLectureParticipantStudents(results.resultTuples);

  // (17)
  results = db.execQuery(FromTable(TableIds::Professors)
    .selectAll()
    .joinAll(Relationships::StudentsConfidant)
    .joinAll(Relationships::Lecturer));;

  expectStudentsConfidantProfessors(results.resultTuples);
  expectLecturerProfessors(results.resultTuples);

  // (18)
  results = db.execQuery(FromTable(TableIds::Lectures)
    .selectAll()
    .joinAll(Relationships::Lecturer)
    .joinAll(Relationships::LectureParticipant));;

  expectLecturerLectures(results.resultTuples);
  expectLectureParticipantLectures(results.resultTuples);
}

/**
 *             students
 *               N  N
 *              /    \
 *             /      \
 *            1        M
 * professors 1 <----> N lectures
 *
 * @steps:
 * (1) Link student to ONE lectures
 * (2) Link student to MANY lectures
 * (3) Link lectures to ONE student
 * (4) Link lectures to MANY students
 * (5) Link student to ONE Project
 * (6) Link student to MANY professors
 * (7) Link Project to ONE student
 *
 * Exception handling tests:
 * (8) Link Project to MANY students
 * (9) Use invalid relationship ID
 * (10) Use InsertIntoExt::linkToOneTuple() with invalid tuple Key
 * (11) Use InsertIntoExt::linkToManyTuple() with invalid tuple Key
 * (12) Use LinkTuples::fromOne() with invalid tuple key
 * (13) Use LinkTuples::toOne() with invalid tuple key
 * (14) Use LinkTuples::fromOne() with invalid tuple key
 *
 * @expected:
 * Relations students-lectures:
 * John    <-->    Math
 *            >    Database Systems
 * Mary    <-->    Programming
 *            >    Database Systems
 * Paul    <-->    Math
 *            >    Programming
 *
 * Relations students-professors:
 * John    <-->    Prof. Smith
 * Mary    <-->    Game Programming
 * Paul    <-->    Modeling
 *            >    Dr. Evans
 */
TEST(RelationshipTest, linkTuplesQueryTest)
{
  /*TestDatabase db;
  setupReplationshipTestsDatabase(db);

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

  const auto projectGameProgramming = db.execQuery(InsertIntoExt(TableIds::Professors)
    .value(ProfessorsCols::Name, "Game Programming")
    .returnIds()).resultTuples[0].values;

  const auto projectModeling = db.execQuery(InsertIntoExt(TableIds::Professors)
    .value(ProfessorsCols::Name, "Modeling")
    .returnIds()).resultTuples[0].values;

  const auto projectMachineLearning = db.execQuery(InsertIntoExt(TableIds::Professors)
    .value(ProfessorsCols::Name, "Dr. Evans")
    .returnIds()).resultTuples[0].values;

  const auto projectComputerVision = db.execQuery(InsertIntoExt(TableIds::Professors)
    .value(ProfessorsCols::Name, "Prof. Smith")
    .returnIds()).resultTuples[0].values;

  // (1)
  db.execQuery(LinkTuples(Relationships::LectureParticipant)
    .fromOne(studentMary)
    .toOne(lectureProgramming));

  // (2)
  db.execQuery(LinkTuples(Relationships::LectureParticipant)
    .fromOne(studentPaul)
    .toMany({ lectureMath, lectureProgramming }));

  // (3)
  db.execQuery(LinkTuples(Relationships::LectureParticipant)
    .fromOne(lectureMath)
    .toOne(studentJohn));

  // (4)
  db.execQuery(LinkTuples(Relationships::LectureParticipant)
    .fromOne(lectureDbSystems)
    .toMany({ studentJohn, studentMary }));

  // (5)
  db.execQuery(LinkTuples(Relationships::StudentsConfidant)
    .fromOne(studentMary)
    .toOne(projectGameProgramming));

  // (6)
  db.execQuery(LinkTuples(Relationships::StudentsConfidant)
    .fromOne(studentPaul)
    .toMany({ projectModeling, projectMachineLearning }));

  // (7)
  db.execQuery(LinkTuples(Relationships::StudentsConfidant)
    .fromOne(projectComputerVision)
    .toOne(studentJohn));

  // (8)
  EXPECT_THROW(db.execQuery(LinkTuples(Relationships::StudentsConfidant)
    .fromOne(projectComputerVision)
    .toMany({ studentJohn, studentMary }))
    , DatabaseException);

  EXPECT_THROW(db.execQuery(InsertIntoExt(TableIds::Professors)
    .value(ProfessorsCols::Name, "Dummy")
    .linkToManyTuples(Relationships::StudentsConfidant, { studentJohn, studentMary }))
    , DatabaseException);

  // (9)
  EXPECT_THROW(db.execQuery(InsertIntoExt(TableIds::Professors)
    .value(ProfessorsCols::Name, "Dummy")
    .linkToOneTuple(Relationships::LectureParticipant, studentJohn))
    , DatabaseException);

  // (10)
  EXPECT_THROW(db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "Dummy")
    .linkToOneTuple(Relationships::LectureParticipant, projectMachineLearning))
    , DatabaseException);

  // (11)
  EXPECT_THROW(db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "Dummy")
    .linkToManyTuples(Relationships::LectureParticipant, { projectMachineLearning, lectureDbSystems }))
    , DatabaseException);

  // (12)
  EXPECT_THROW(db.execQuery(LinkTuples(Relationships::LectureParticipant)
    .fromOne(projectComputerVision)
    .toOne(lectureProgramming))
    , DatabaseException);

  // (13)
  EXPECT_THROW(db.execQuery(LinkTuples(Relationships::LectureParticipant)
    .fromOne(studentMary)
    .toOne(projectComputerVision))
    , DatabaseException);

  // (14)
  EXPECT_THROW(db.execQuery(LinkTuples(Relationships::LectureParticipant)
    .fromOne(studentMary)
    .toMany({ projectComputerVision, lectureProgramming }))
    , DatabaseException);

  // Expectations:
  auto results = db.execQuery(FromTable(TableIds::Students)
    .select({ StudentsCols::Name })
    .joinColumns(Relationships::LectureParticipant, { LecturesCols::Topic }));

  Funcs::expectRelations(results.resultTuples, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "John", QVariantList() << "Math" << "Database Systems");

  Funcs::expectRelations(results.resultTuples, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Mary", QVariantList() << "Programming" << "Database Systems");

  Funcs::expectRelations(results.resultTuples, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Paul", QVariantList() << "Math" << "Programming");

  results = db.execQuery(FromTable(TableIds::Lectures)
    .selectAll()
    .joinColumns(Relationships::LectureParticipant, { StudentsCols::Name }));

  Funcs::expectRelations(results.resultTuples, Relationships::LectureParticipant,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Students, StudentsCols::Name,
    "Math", QVariantList() << "John" << "Paul");

  Funcs::expectRelations(results.resultTuples, Relationships::LectureParticipant,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Students, StudentsCols::Name,
    "Programming", QVariantList() << "Mary" << "Paul");

  Funcs::expectRelations(results.resultTuples, Relationships::LectureParticipant,
    TableIds::Lectures, LecturesCols::Topic, TableIds::Students, StudentsCols::Name,
    "Database Systems", QVariantList() << "John" << "Mary");

  results = db.execQuery(FromTable(TableIds::Students)
    .select({ StudentsCols::Name })
    .joinAll(Relationships::StudentsConfidant));

  Funcs::expectRelations(results.resultTuples, Relationships::StudentsConfidant,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "John", QVariantList() << "Prof. Smith");

  Funcs::expectRelations(results.resultTuples, Relationships::StudentsConfidant,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "Mary", QVariantList() << "Game Programming");

  Funcs::expectRelations(results.resultTuples, Relationships::StudentsConfidant,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "Paul", QVariantList() << "Modeling" << "Dr. Evans");

  results = db.execQuery(FromTable(TableIds::Professors)
    .selectAll()
    .joinAll(Relationships::StudentsConfidant));

  Funcs::expectRelations(results.resultTuples, Relationships::StudentsConfidant,
    TableIds::Professors, ProfessorsCols::Name,TableIds::Students, StudentsCols::Name,
    "Prof. Smith", QVariantList() << "John");

  Funcs::expectRelations(results.resultTuples, Relationships::StudentsConfidant,
    TableIds::Professors, ProfessorsCols::Name,TableIds::Students, StudentsCols::Name,
    "Game Programming", QVariantList() << "Mary");

  Funcs::expectRelations(results.resultTuples, Relationships::StudentsConfidant,
    TableIds::Professors, ProfessorsCols::Name,TableIds::Students, StudentsCols::Name,
    "Modeling", QVariantList() << "Paul");

  Funcs::expectRelations(results.resultTuples, Relationships::StudentsConfidant,
    TableIds::Professors, ProfessorsCols::Name,TableIds::Students, StudentsCols::Name,
    "Dr. Evans", QVariantList() << "Paul");

  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::StudentsConfidant)
    .joinAll(Relationships::LectureParticipant));

  Funcs::expectRelations(results.resultTuples, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "John", QVariantList() << "Math" << "Database Systems");

  Funcs::expectRelations(results.resultTuples, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Mary", QVariantList() << "Programming" << "Database Systems");

  Funcs::expectRelations(results.resultTuples, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Paul", QVariantList() << "Math" << "Programming");

  Funcs::expectRelations(results.resultTuples, Relationships::StudentsConfidant,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "John", QVariantList() << "Prof. Smith");

  Funcs::expectRelations(results.resultTuples, Relationships::StudentsConfidant,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "Mary", QVariantList() << "Game Programming");

  Funcs::expectRelations(results.resultTuples, Relationships::StudentsConfidant,
    TableIds::Students, StudentsCols::Name, TableIds::Professors, ProfessorsCols::Name,
    "Paul", QVariantList() << "Modeling" << "Dr. Evans");*/
}

/**
 * @description:
 * Create two tables with three tuples respectively:
 *   - Table students, containing tuples: [ student1, student2, student3 ]
 *   - Table professors, containing tuples: [ professor1, professor2, professor3 ]
 *
 * Create 6 relationships and link tuples as follows:
 *   (a) students 1<---->N professors
 *       [ student1 ] 1<-->N [ professor1, professor2 ]
 *   (b) students 1<---->N professors
 *       [ student1 ] 1<-->N [ professor1, professor2 ]
 *       [ student2 ] 1<-->N [ professor3 ]
 *   (c) students N<---->M professors
 *       [ student1 ] 1<-->M [ professor1, professor2,  professor3 ]
 *       [ student2 ] 1<-->M [ professor1 ]
 *       [ student3 ] 1<-->M [ professor1, professor2 ]
 *   (d) students N<---->M professors
 *       [ student1 ] 1<-->M [ professor3 ]
 *       [ student3 ] 1<-->M [ professor1, professor2,  professor3 ]
 *   (e) students 1<---->N students
 *       [ student1 ] 1<-->N [ student1, student2 ]
 *       [ student2 ] 1<-->N [ student3 ]
 *   (f) students N<---->M students
 *       [ student1 ] 1<-->M [ student2, student3 ]
 *       [ student2 ] 1<-->M [ student3 ]
 *
 * We want to check if it is possible to have multiple relationships of the same type
 * linking the exact same tables - so we have two One-To-Many relationships (a), (b) linking students and
 * professors and we have two Many-to-Many relationships (c), (d) also linking students and professors.
 * Addditionally we have two relationahips linking the table students with itself, a One-To-Many relationship (e) and
 * a Many-To-Many relationship.
 *
 * First off, we perform verious LinkTuple queries to create links for each of the six relationships.
 * No exception must be thrown at any time.
 *   (1)  Link for relationship (a) from one student [1] to one professor [N]
 *   (2)  Link for relationship (a) from one professor [N] to one student [1]
 *   (3)  Link for relationship (b) from one student [1] to many professors [N]
 *   (4)  Link for relationship (b) from one professor [N] to one student [1]
 *   (5)  Link for relationship (c) from one professor [M] to many students [N]
 *   (6)  Link for relationship (c) from one student [N] to many professors [M]
 *   (7)  Link for relationship (c) from one professor [M] to many students [N]
 *   (8)  Link for relationship (d) from one professor [M] to one student [N]
 *   (9)  Link for relationship (d) from one student [N] to many professors [M]
 *   (10) Link for relationship (e) from one student [1] to many students [N]
 *   (11) Link for relationship (e) from one student [1] to one student [N]
 *   (12) Link for relationship (f) from one student [N] to many students [M]
 *   (13) Link for relationship (f) from one student [N] to one student [M]
 *
 * Then we query the data and check if the result tuples including joined tuples meet
 * our expectations.
 *   (14) Query students and related professors for relationship (a)
 *   (15) Query professors and related students for relationship (a)
 *   (16) Query students and related professors for relationship (b)
 *   (17) Query professors and related students for relationship (b)
 *   (18) Query students and related professors for relationships (a) and (b)
 *   (19) Query professors and related students for relationships (a) and (b)
 *   (20) Query students and related professors for relationship (c)
 *   (21) Query professors and related students for relationship (c)
 *   (22) Query students and related professors for relationship (d)
 *   (23) Query professors and related students for relationship (d)
 *   (24) Query students and related professors for relationships (c) and (d)
 *   (25) Query professors and related students for relationships (c) and (d)
 *   (26) Query students and related professors for relationships (a), (b), (c) and (d)
 *   (27) Query professors and related students for relationships (a), (b), (c) and (d)
 *   (28) Query students and related students for relationship (e)
 *   (29) Query students and related students for relationship (f)
 *   (30) Query students and related professors and students for all relationships
 */
TEST(RelationshipTest, specialRelationships)
{
  SchemaConfigurator configurator;
  configurator.configureTable(TableIds::Students, "students")
    .column(StudentsCols::Id, "id", DataType::Integer).autoIncrement().notNull()
    .column(StudentsCols::Name, "name", DataType::Varchar, 128)
    .primaryKeys({ StudentsCols::Id });

  configurator.configureTable(TableIds::Professors, "professors")
    .column(ProfessorsCols::Id, "id", DataType::Integer).autoIncrement().notNull()
    .column(ProfessorsCols::Name, "name", DataType::Varchar, 128)
    .primaryKeys({ ProfessorsCols::Id });

  configurator.configureRelationship(Relationships::Special1, TableIds::Students, TableIds::Professors, ISchema::RelationshipType::OneToMany);
  configurator.configureRelationship(Relationships::Special2, TableIds::Students, TableIds::Professors, ISchema::RelationshipType::OneToMany);
  configurator.configureRelationship(Relationships::Special3, TableIds::Students, TableIds::Professors, ISchema::RelationshipType::ManyToMany);
  configurator.configureRelationship(Relationships::Special4, TableIds::Students, TableIds::Professors, ISchema::RelationshipType::ManyToMany);
  configurator.configureRelationship(Relationships::Special5, TableIds::Students, TableIds::Students, ISchema::RelationshipType::OneToMany);
  configurator.configureRelationship(Relationships::Special6, TableIds::Students, TableIds::Students, ISchema::RelationshipType::ManyToMany);

  TestDatabase db;
  db.initialize(configurator, Funcs::getDefaultDatabaseFilename());

  const auto student1 = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "student1")
    .returnIds()).resultTuples[0].values;

  const auto student2 = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "student2")
    .returnIds()).resultTuples[0].values;

  const auto student3 = db.execQuery(InsertIntoExt(TableIds::Students)
    .value(StudentsCols::Name, "student3")
    .returnIds()).resultTuples[0].values;

  const auto professor1 = db.execQuery(InsertIntoExt(TableIds::Professors)
    .value(ProfessorsCols::Name, "professor1")
    .returnIds()).resultTuples[0].values;

  const auto professor2 = db.execQuery(InsertIntoExt(TableIds::Professors)
    .value(ProfessorsCols::Name, "professor2")
    .returnIds()).resultTuples[0].values;

  const auto professor3 = db.execQuery(InsertIntoExt(TableIds::Professors)
    .value(ProfessorsCols::Name, "professor3")
    .returnIds()).resultTuples[0].values;

  // (1)
  db.execQuery(LinkTuples(Relationships::Special1)
    .fromOne(student1)
    .toOne(professor1));

  // (2)
  db.execQuery(LinkTuples(Relationships::Special1)
    .fromOne(professor2)
    .toOne(student1));

  // (3)
  db.execQuery(LinkTuples(Relationships::Special2)
    .fromOne(student1)
    .toMany({ professor1, professor2 }));

  // (4)
  db.execQuery(LinkTuples(Relationships::Special2)
    .fromOne(professor3)
    .toOne(student2));

  // (5)
  db.execQuery(LinkTuples(Relationships::Special3)
    .fromOne(professor1)
    .toMany({ student2, student3 }));

  // (6)
  db.execQuery(LinkTuples(Relationships::Special3)
    .fromOne(student1)
    .toMany({ professor1, professor3 }));

  // (7)
  db.execQuery(LinkTuples(Relationships::Special3)
    .fromOne(professor2)
    .toMany({ student1, student3 }));

  // (8)
  db.execQuery(LinkTuples(Relationships::Special4)
    .fromOne(professor3)
    .toOne(student1));

  // (9)
  db.execQuery(LinkTuples(Relationships::Special4)
    .fromOne(student3)
    .toMany({ professor1, professor2, professor3 }));

  // (10)
  db.execQuery(LinkTuples(Relationships::Special5)
    .fromOne(student1)
    .toMany({ student1, student2 }));

  // (11)
  db.execQuery(LinkTuples(Relationships::Special5)
    .fromOne(student2)
    .toOne(student3));

  // (12)
  db.execQuery(LinkTuples(Relationships::Special6)
    .fromOne(student1)
    .toMany({ student2, student3 }));

  // (13)
  db.execQuery(LinkTuples(Relationships::Special6)
    .fromOne(student2)
    .toOne(student3));

  // (14)
  auto results = db.execQuery(FromTable(TableIds::Students)
    .select({ StudentsCols::Name })
    .joinColumns(Relationships::Special1, { ProfessorsCols::Name }));

  expectSpecialRelation1Students(results.resultTuples);

  // (15)
  results = db.execQuery(FromTable(TableIds::Professors)
    .selectAll()
    .joinAll(Relationships::Special1));

  expectSpecialRelation1Professors(results.resultTuples);

  // (16)
  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::Special2));

  expectSpecialRelation2Students(results.resultTuples);

  // (17)
  results = db.execQuery(FromTable(TableIds::Professors)
    .selectAll()
    .joinAll(Relationships::Special2));

  expectSpecialRelation2Professors(results.resultTuples);

  // (18)
  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::Special1)
    .joinAll(Relationships::Special2));

  expectSpecialRelation1Students(results.resultTuples);
  expectSpecialRelation2Students(results.resultTuples);

  // (19)
  results = db.execQuery(FromTable(TableIds::Professors)
    .selectAll()
    .joinAll(Relationships::Special1)
    .joinAll(Relationships::Special2));

  expectSpecialRelation1Professors(results.resultTuples);
  expectSpecialRelation2Professors(results.resultTuples);

  // (20)
  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::Special3));

  expectSpecialRelation3Students(results.resultTuples);

  // (21)
  results = db.execQuery(FromTable(TableIds::Professors)
    .selectAll()
    .joinAll(Relationships::Special3));

  expectSpecialRelation3Professors(results.resultTuples);

  // (22)
  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::Special4));

  expectSpecialRelation4Students(results.resultTuples);

  // (23)
  results = db.execQuery(FromTable(TableIds::Professors)
    .selectAll()
    .joinAll(Relationships::Special4));

  expectSpecialRelation4Professors(results.resultTuples);

  // (24)
  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::Special3)
    .joinAll(Relationships::Special4));

  expectSpecialRelation3Students(results.resultTuples);
  expectSpecialRelation4Students(results.resultTuples);

  // (25)
  results = db.execQuery(FromTable(TableIds::Professors)
    .selectAll()
    .joinAll(Relationships::Special3)
    .joinAll(Relationships::Special4));

  expectSpecialRelation3Professors(results.resultTuples);
  expectSpecialRelation4Professors(results.resultTuples);

  // (26)
  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::Special1)
    .joinAll(Relationships::Special2)
    .joinAll(Relationships::Special3)
    .joinAll(Relationships::Special4));

  expectSpecialRelation1Students(results.resultTuples);
  expectSpecialRelation2Students(results.resultTuples);
  expectSpecialRelation3Students(results.resultTuples);
  expectSpecialRelation4Students(results.resultTuples);

  // (27)
  results = db.execQuery(FromTable(TableIds::Professors)
    .selectAll()
    .joinAll(Relationships::Special1)
    .joinAll(Relationships::Special2)
    .joinAll(Relationships::Special3)
    .joinAll(Relationships::Special4));

  expectSpecialRelation1Professors(results.resultTuples);
  expectSpecialRelation2Professors(results.resultTuples);
  expectSpecialRelation3Professors(results.resultTuples);
  expectSpecialRelation4Professors(results.resultTuples);

  // (28)
  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::Special5));

  expectSpecialRelation5Students(results.resultTuples);

  // (29)
  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::Special6));

  expectSpecialRelation6Students(results.resultTuples);

  // (30)
  results = db.execQuery(FromTable(TableIds::Students)
    .selectAll()
    .joinAll(Relationships::Special1)
    .joinAll(Relationships::Special2)
    .joinAll(Relationships::Special3)
    .joinAll(Relationships::Special4)
    .joinAll(Relationships::Special5)
    .joinAll(Relationships::Special6));

  expectSpecialRelation1Students(results.resultTuples);
  expectSpecialRelation2Students(results.resultTuples);
  expectSpecialRelation3Students(results.resultTuples);
  expectSpecialRelation4Students(results.resultTuples);
  expectSpecialRelation5Students(results.resultTuples);
  expectSpecialRelation6Students(results.resultTuples);
}


}
