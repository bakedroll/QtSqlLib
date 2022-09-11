#include <gtest/gtest.h>

#include <Common.h>

namespace QtSqlLibTest
{

static void expectStudentsConfidantStudents(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(QtSqlLib::ID(Relationships::StudentsConfidant)),
    QtSqlLib::ID(QtSqlLib::ID(TableIds::Students)), QtSqlLib::ID(QtSqlLib::ID(StudentsCols::Name)), QtSqlLib::ID(QtSqlLib::ID(TableIds::Professors)), QtSqlLib::ID(QtSqlLib::ID(ProfessorsCols::Name)),
    "John", QVariantList() << "Prof. Smith");

  Funcs::expectRelations(tuples, QtSqlLib::ID(QtSqlLib::ID(Relationships::StudentsConfidant)),
    QtSqlLib::ID(QtSqlLib::ID(TableIds::Students)), QtSqlLib::ID(QtSqlLib::ID(StudentsCols::Name)), QtSqlLib::ID(QtSqlLib::ID(TableIds::Professors)), QtSqlLib::ID(QtSqlLib::ID(ProfessorsCols::Name)),
    "Mary", QVariantList() << "Prof. Smith");

  Funcs::expectRelations(tuples, QtSqlLib::ID(QtSqlLib::ID(Relationships::StudentsConfidant)),
    QtSqlLib::ID(QtSqlLib::ID(TableIds::Students)), QtSqlLib::ID(QtSqlLib::ID(StudentsCols::Name)), QtSqlLib::ID(QtSqlLib::ID(TableIds::Professors)), QtSqlLib::ID(QtSqlLib::ID(ProfessorsCols::Name)),
    "Paul", QVariantList() << "Dr. Evans");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::StudentsConfidant),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "Sarah", QVariantList() << "Prof. Adams");
}

static void expectStudentsConfidantProfessors(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::StudentsConfidant),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "Prof. Smith", QVariantList() << "John" << "Mary");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::StudentsConfidant),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "Dr. Evans", QVariantList() << "Paul");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::StudentsConfidant),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "Prof. Adams", QVariantList() << "Sarah");
}

static void expectLecturerProfessors(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Lecturer),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic),
    "Prof. Smith", QVariantList() << "Programming");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Lecturer),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic),
    "Dr. Evans", QVariantList() << "Math");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Lecturer),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic),
    "Prof. Adams", QVariantList() << "Operating systems" << "Database systems");
}

static void expectLecturerLectures(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Lecturer),
    QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "Programming", QVariantList() << "Prof. Smith");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Lecturer),
    QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "Math", QVariantList() << "Dr. Evans");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Lecturer),
    QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "Operating systems", QVariantList() << "Prof. Adams");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Lecturer),
    QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "Database systems", QVariantList() << "Prof. Adams");
}

static void expectLectureParticipantLectures(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::LectureParticipant),
    QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "Operating systems", QVariantList() << "Sarah");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::LectureParticipant),
    QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "Database systems", QVariantList() << "Paul" << "Sarah");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::LectureParticipant),
    QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "Math", QVariantList() << "John" << "Mary" << "Paul");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::LectureParticipant),
    QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "Programming", QVariantList() << "John" << "Mary" << "Sarah");
}

static void expectLectureParticipantStudents(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::LectureParticipant),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic),
    "John", QVariantList() << "Math" << "Programming");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::LectureParticipant),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic),
    "Mary", QVariantList() << "Math" << "Programming");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::LectureParticipant),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic),
    "Paul", QVariantList() << "Database systems" << "Math");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::LectureParticipant),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Lectures), QtSqlLib::ID(LecturesCols::Topic),
    "Sarah", QVariantList() << "Operating systems" << "Database systems" << "Programming");
}

static void expectSpecialRelation1Students(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special1),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "student1", QVariantList() << "professor1" << "professor2");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special1),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "student2", QVariantList());

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special1),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "student3", QVariantList());
}

static void expectSpecialRelation1Professors(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special1),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "professor1", QVariantList() << "student1");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special1),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "professor2", QVariantList() << "student1");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special1),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "professor3", QVariantList());
}

static void expectSpecialRelation2Students(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special2),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "student1", QVariantList() << "professor1" << "professor2");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special2),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "student2", QVariantList() << "professor3");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special2),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "student3", QVariantList());
}

static void expectSpecialRelation2Professors(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special2),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "professor1", QVariantList() << "student1");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special2),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "professor2", QVariantList() << "student1");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special2),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "professor3", QVariantList() << "student2");
}

static void expectSpecialRelation3Students(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special3),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "student1", QVariantList() << "professor1" << "professor2" << "professor3");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special3),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "student2", QVariantList() << "professor1");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special3),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "student3", QVariantList() << "professor1" << "professor2");
}

static void expectSpecialRelation3Professors(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special3),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "professor1", QVariantList() << "student1" << "student2" << "student3");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special3),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "professor2", QVariantList() << "student1" << "student3");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special3),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "professor3", QVariantList() << "student1");
}

static void expectSpecialRelation4Students(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special4),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "student1", QVariantList() << "professor3");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special4),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "student2", QVariantList());

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special4),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name),
    "student3", QVariantList() << "professor1" << "professor2" << "professor3");
}

static void expectSpecialRelation4Professors(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special4),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "professor1", QVariantList() << "student3");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special4),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "professor2", QVariantList() << "student3");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special4),
    QtSqlLib::ID(TableIds::Professors), QtSqlLib::ID(ProfessorsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "professor3", QVariantList() << "student1" << "student3");
}

static void expectSpecialRelation5Students(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special5),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "student1", QVariantList() << "student1" << "student2");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special5),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "student2", QVariantList() << "student3");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special5),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "student3", QVariantList());
}

static void expectSpecialRelation6Students(IQuery::QueryResults::ResultTuples& tuples)
{
  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special6),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "student1", QVariantList() << "student2" << "student3");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special6),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
    "student2", QVariantList() << "student3");

  Funcs::expectRelations(tuples, QtSqlLib::ID(Relationships::Special6),
    QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name), QtSqlLib::ID(TableIds::Students), QtSqlLib::ID(StudentsCols::Name),
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
  configurator.CONFIGURE_TABLE(TableIds::Students, "students")
    .COLUMN(StudentsCols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(StudentsCols::Name, "name", 128);

  configurator.CONFIGURE_TABLE(TableIds::Professors, "professors")
    .COLUMN(ProfessorsCols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(ProfessorsCols::Name, "name", 128);

  configurator.CONFIGURE_TABLE(TableIds::Lectures, "lectures")
    .COLUMN(LecturesCols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(LecturesCols::Topic, "topic", 128);

  configurator.CONFIGURE_RELATIONSHIP(Relationships::StudentsConfidant, TableIds::Students, TableIds::Professors,
    ISchema::RelationshipType::ManyToOne).ON_DELETE(ISchema::ForeignKeyAction::Cascade);

  configurator.CONFIGURE_RELATIONSHIP(Relationships::Lecturer, TableIds::Professors, TableIds::Lectures,
    ISchema::RelationshipType::OneToMany).ON_DELETE(ISchema::ForeignKeyAction::Cascade);

  configurator.CONFIGURE_RELATIONSHIP(Relationships::LectureParticipant, TableIds::Students, TableIds::Lectures,
    ISchema::RelationshipType::ManyToMany);

  db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
}

/**
 * @description:
 * Checks the correctness of the relations between tuples by querying the data and
 * compare the result tuples (including joined tuples) against our expectations.
 *   (1) Query students and related professors (a)
 *   (2) Query professors and related students (a)
 *   (3) Query professors and related lectures (b)
 *   (4) Query lectures and related professors (b)
 *   (5) Query lectures and related students (c)
 *   (6) Query students and related lectures (c)
 *   (7) Query students, related professors (a) and related lectures (c)
 *   (8) Query professors, related students (a) and related lectures (b)
 *   (9) Query lectures, related professors (b) and related students (a)
 */
static void expectCorrectRelations(TestDatabase& db)
{
  // (1)
  auto results = db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT_ALL
    .JOIN_ALL(Relationships::StudentsConfidant));

  expectStudentsConfidantStudents(results.resultTuples);

  // (2)
  results = db.execQuery(FROM_TABLE(TableIds::Professors)
    .SELECT_ALL
    .JOIN_ALL(Relationships::StudentsConfidant));

  expectStudentsConfidantProfessors(results.resultTuples);

  // (3)
  results = db.execQuery(FROM_TABLE(TableIds::Professors)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Lecturer));

  expectLecturerProfessors(results.resultTuples);

  // (4)
  results = db.execQuery(FROM_TABLE(TableIds::Lectures)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Lecturer));

  expectLecturerLectures(results.resultTuples);

  // (5)
  results = db.execQuery(FROM_TABLE(TableIds::Lectures)
    .SELECT_ALL
    .JOIN_ALL(Relationships::LectureParticipant));

  expectLectureParticipantLectures(results.resultTuples);

  // (6)
  results = db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT_ALL
    .JOIN_ALL(Relationships::LectureParticipant));

  expectLectureParticipantStudents(results.resultTuples);

  // (7)
  results = db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT_ALL
    .JOIN_ALL(Relationships::StudentsConfidant)
    .JOIN_ALL(Relationships::LectureParticipant));;

  expectStudentsConfidantStudents(results.resultTuples);
  expectLectureParticipantStudents(results.resultTuples);

  // (8)
  results = db.execQuery(FROM_TABLE(TableIds::Professors)
    .SELECT_ALL
    .JOIN_ALL(Relationships::StudentsConfidant)
    .JOIN_ALL(Relationships::Lecturer));;

  expectStudentsConfidantProfessors(results.resultTuples);
  expectLecturerProfessors(results.resultTuples);

  // (9)
  results = db.execQuery(FROM_TABLE(TableIds::Lectures)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Lecturer)
    .JOIN_ALL(Relationships::LectureParticipant));;

  expectLecturerLectures(results.resultTuples);
  expectLectureParticipantLectures(results.resultTuples);
}

/**
 * @description:
 * This test adds links between tuples of related tables directly on data insertion
 * whithin the #INSERT_INTO_EXT query.
 * For more information about the created tables and relationships, see #setupReplationshipTestsDatabase().
 *
 * Create tuples and links between them as follows:
 *   (1) Insert student John
 *       Insert lecture Operating systems
 *       Insert lecture Database systems
 *   (2) Insert professor Prof. Smith
 *         Link TO_ONE [ John ] (a)
 *   (3) Insert lecture Math
 *         Link TO_ONE [ John ] (c)
 *   (4) Insert student Mary
 *         Link TO_ONE [ Prof. Smith ] (a)
 *         Link TO_ONE [ Math ] (c)
 *   (5) Insert professor Dr. Evans
 *         Link TO_ONE [ Math ] (b)
 *   (6) Insert lecture Programming
 *         Link TO_MANY [ John, Mary ] (c)
 *         Link TO_ONE  [ Prof. Smith ] (b)
 *   (7) Insert student Paul
 *         Link TO_ONE  [ Dr. Evans ] (a)
 *         Link TO_MANY [ Database systems, Math ] (c)
 *   (8) Insert student Sarah
 *         Link TO_MANY [ Operating systems, Database systems, Programming ] (c)
 *   (9) Insert professor Prof. Adams
 *         Link TO_ONE  [ Sarah ] (a)
 *         Link TO_MANY [ Operating systems, Database systems ] (b)
 *
 *   (10) Finally check the correctness of the inserted relations
 */
TEST(RelationshipTest, linkTuplesOnInsertTest)
{
  TestDatabase db;
  setupReplationshipTestsDatabase(db);

  // (1)
  const auto studentJohn = db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "John")
    .RETURN_IDS).resultTuples[0].values;

  const auto lectureOs = db.execQuery(INSERT_INTO_EXT(TableIds::Lectures)
    .VALUE(LecturesCols::Topic, "Operating systems")
    .RETURN_IDS).resultTuples[0].values;

  const auto lectureDbs = db.execQuery(INSERT_INTO_EXT(TableIds::Lectures)
    .VALUE(LecturesCols::Topic, "Database systems")
    .RETURN_IDS).resultTuples[0].values;

  // (2)
  const auto profSmith = db.execQuery(INSERT_INTO_EXT(TableIds::Professors)
    .VALUE(ProfessorsCols::Name, "Prof. Smith")
    .LINK_TO_ONE_TUPLE(Relationships::StudentsConfidant, studentJohn)
    .RETURN_IDS).resultTuples[0].values;

  // (3)
  const auto lectureMath = db.execQuery(INSERT_INTO_EXT(TableIds::Lectures)
    .VALUE(LecturesCols::Topic, "Math")
    .LINK_TO_ONE_TUPLE(Relationships::LectureParticipant, studentJohn)
    .RETURN_IDS).resultTuples[0].values;

  // (4)
  const auto studentMary = db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "Mary")
    .LINK_TO_ONE_TUPLE(Relationships::StudentsConfidant, profSmith)
    .LINK_TO_ONE_TUPLE(Relationships::LectureParticipant, lectureMath)
    .RETURN_IDS).resultTuples[0].values;

  // (5)
  const auto profEvans = db.execQuery(INSERT_INTO_EXT(TableIds::Professors)
    .VALUE(ProfessorsCols::Name, "Dr. Evans")
    .LINK_TO_ONE_TUPLE(Relationships::Lecturer, lectureMath)
    .RETURN_IDS).resultTuples[0].values;

  // (6)
  const auto lectureProgramming = db.execQuery(INSERT_INTO_EXT(TableIds::Lectures)
    .VALUE(LecturesCols::Topic, "Programming")
    .LINK_TO_MANY_TUPLES(Relationships::LectureParticipant, { studentJohn, studentMary })
    .LINK_TO_ONE_TUPLE(Relationships::Lecturer, profSmith)
    .RETURN_IDS).resultTuples[0].values;

  // (7)
  const auto studentPaul = db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "Paul")
    .LINK_TO_ONE_TUPLE(Relationships::StudentsConfidant, profEvans)
    .LINK_TO_MANY_TUPLES(Relationships::LectureParticipant, { lectureDbs, lectureMath })
    .RETURN_IDS).resultTuples[0].values;

  // (8)
  const auto studentSarah = db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "Sarah")
    .LINK_TO_MANY_TUPLES(Relationships::LectureParticipant, { lectureOs, lectureDbs, lectureProgramming })
    .RETURN_IDS).resultTuples[0].values;

  // (9)
  const auto profAdams = db.execQuery(INSERT_INTO_EXT(TableIds::Professors)
    .VALUE(ProfessorsCols::Name, "Prof. Adams")
    .LINK_TO_ONE_TUPLE(Relationships::StudentsConfidant, studentSarah)
    .LINK_TO_MANY_TUPLES(Relationships::Lecturer, { lectureOs, lectureDbs })
    .RETURN_IDS).resultTuples[0].values;

  // (10)
  expectCorrectRelations(db);
}

/**
 * @description:
 * This test inserts all necessary tuples first and then links them
 * according to the description in #setupReplationshipTestsDatabase().
 *
 * Create tuples:
 *   (1) Insert students
 *   (2) Insert professors
 *   (3) Insert lectures
 *
 * Link the tuples:
 *   (4)  Link [ Prof. Smith ]      TO_MANY [ John, Mary ] (a)
 *   (5)  Link [ Dr. Evans ]        TO_ONE  [ Paul ] (a)
 *   (6)  Link [ Sarah ]            TO_ONE  [ Prof. Adams ] (a)
 *   (7)  Link [ Prof. Smith ]      TO_ONE  [ Programming ] (b)
 *   (8)  Link [ Math ]             TO_ONE  [ Dr. Evans ] (b)
 *   (9)  Link [ Prof. Adams ]      TO_MANY [ Operating systems, Database systems ] (b)
 *   (10) Link [ John ]             TO_MANY [ Math, Programming ] (c)
 *   (11) Link [ Programming ]      TO_MANY [ Mary, Sarah ] (c)
 *   (12) Link [ Mary ]             TO_ONE  [ Math ] (c)
 *   (13) Link [ Paul ]             TO_MANY [  Database systems, Math ] (c)
 *   (14) Link [ Sarah ]            TO_ONE  [ Operating systems ] (c)
 *   (15) Link [ Database systems ] TO_ONE  [ Sarah ] (c)
 *
 *   (16) Check the correctness of the inserted relations
 *
 * Exception handling tests:
 *   (17) Link student to MANY professors
 *   (18) Use invalid relationship ID
 *   (19) Use INSERT_INTO_EXT::LINK_TO_ONE_TUPLE() with invalid tuple Key
 *   (20) Use INSERT_INTO_EXT::linkToManyTuple() with invalid tuple Key
 *   (21) Use LINK_TUPLES::FROM_ONE() with invalid tuple key
 *   (22) Use LINK_TUPLES::TO_ONE() with invalid tuple key
 *   (23) Use LINK_TUPLES::TO_MANY() with an invalid tuple key
 */
TEST(RelationshipTest, linkTuplesQueryTest)
{
  TestDatabase db;
  setupReplationshipTestsDatabase(db);

  // (1)
  const auto studentJohn = db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "John")
    .RETURN_IDS).resultTuples[0].values;

  const auto studentMary = db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "Mary")
    .RETURN_IDS).resultTuples[0].values;

  const auto studentPaul = db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "Paul")
    .RETURN_IDS).resultTuples[0].values;

  const auto studentSarah = db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "Sarah")
    .RETURN_IDS).resultTuples[0].values;

  // (2)
  const auto profSmith = db.execQuery(INSERT_INTO_EXT(TableIds::Professors)
    .VALUE(ProfessorsCols::Name, "Prof. Smith")
    .RETURN_IDS).resultTuples[0].values;

  const auto profEvans = db.execQuery(INSERT_INTO_EXT(TableIds::Professors)
    .VALUE(ProfessorsCols::Name, "Dr. Evans")
    .RETURN_IDS).resultTuples[0].values;

  const auto profAdams = db.execQuery(INSERT_INTO_EXT(TableIds::Professors)
    .VALUE(ProfessorsCols::Name, "Prof. Adams")
    .RETURN_IDS).resultTuples[0].values;

  // (3)
  const auto lectureOs = db.execQuery(INSERT_INTO_EXT(TableIds::Lectures)
    .VALUE(LecturesCols::Topic, "Operating systems")
    .RETURN_IDS).resultTuples[0].values;

  const auto lectureDbs = db.execQuery(INSERT_INTO_EXT(TableIds::Lectures)
    .VALUE(LecturesCols::Topic, "Database systems")
    .RETURN_IDS).resultTuples[0].values;

  const auto lectureMath = db.execQuery(INSERT_INTO_EXT(TableIds::Lectures)
    .VALUE(LecturesCols::Topic, "Math")
    .RETURN_IDS).resultTuples[0].values;

  const auto lectureProgramming = db.execQuery(INSERT_INTO_EXT(TableIds::Lectures)
    .VALUE(LecturesCols::Topic, "Programming")
    .RETURN_IDS).resultTuples[0].values;

  // (4)
  db.execQuery(LINK_TUPLES(Relationships::StudentsConfidant)
    .FROM_ONE(profSmith)
    .TO_MANY({ studentJohn, studentMary }));

  // (5)
  db.execQuery(LINK_TUPLES(Relationships::StudentsConfidant)
    .FROM_ONE(profEvans)
    .TO_ONE(studentPaul));

  // (6)
  db.execQuery(LINK_TUPLES(Relationships::StudentsConfidant)
    .FROM_ONE(studentSarah)
    .TO_ONE(profAdams));

  // (7)
  db.execQuery(LINK_TUPLES(Relationships::Lecturer)
    .FROM_ONE(profSmith)
    .TO_ONE(lectureProgramming));

  // (8)
  db.execQuery(LINK_TUPLES(Relationships::Lecturer)
    .FROM_ONE(lectureMath)
    .TO_ONE(profEvans));

  // (9)
  db.execQuery(LINK_TUPLES(Relationships::Lecturer)
    .FROM_ONE(profAdams)
    .TO_MANY({ lectureOs, lectureDbs }));

  // (10)
  db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(studentJohn)
    .TO_MANY({ lectureMath, lectureProgramming }));

  // (11)
  db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(lectureProgramming)
    .TO_MANY({ studentMary, studentSarah }));

  // (12)
  db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(studentMary)
    .TO_ONE(lectureMath));

  // (13)
  db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(studentPaul)
    .TO_MANY({ lectureDbs, lectureMath }));

  // (14)
  db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(studentSarah)
    .TO_ONE(lectureOs));

  // (15)
  db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(lectureDbs)
    .TO_ONE(studentSarah));

  // (16)
  expectCorrectRelations(db);

  // (17)
  EXPECT_THROW(db.execQuery(LINK_TUPLES(Relationships::StudentsConfidant)
    .FROM_ONE(studentSarah)
    .TO_MANY({ profSmith, profEvans }))
    , DatabaseException);

  EXPECT_THROW(db.execQuery(INSERT_INTO_EXT(TableIds::Professors)
    .VALUE(ProfessorsCols::Name, "Dummy")
    .LINK_TO_MANY_TUPLES(Relationships::StudentsConfidant, { studentJohn, studentMary }))
    , DatabaseException);

  // (18)
  EXPECT_THROW(db.execQuery(INSERT_INTO_EXT(TableIds::Professors)
    .VALUE(ProfessorsCols::Name, "Dummy")
    .LINK_TO_ONE_TUPLE(Relationships::LectureParticipant, studentJohn))
    , DatabaseException);

  // (19)
  EXPECT_THROW(db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "Dummy")
    .LINK_TO_ONE_TUPLE(Relationships::LectureParticipant, profSmith))
    , DatabaseException);

  // (20)
  EXPECT_THROW(db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "Dummy")
    .LINK_TO_MANY_TUPLES(Relationships::LectureParticipant, { profSmith, profEvans }))
    , DatabaseException);

  // (21)
  EXPECT_THROW(db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(profSmith)
    .TO_ONE(lectureProgramming))
    , DatabaseException);

  // (22)
  EXPECT_THROW(db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(studentMary)
    .TO_ONE(profSmith))
    , DatabaseException);

  // (23)
  EXPECT_THROW(db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(studentMary)
    .TO_MANY({ lectureMath, profSmith }))
    , DatabaseException);
}

/**
 * @description:
 * Create two tables with three tuples respectively:
 *   - Table students, containing tuples: [ student1, student2, student3 ]
 *   - Table professors, containing tuples: [ professor1, professor2, professor3 ]
 *   - Each table has two primary keys
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
  configurator.CONFIGURE_TABLE(TableIds::Students, "students")
    .COLUMN(StudentsCols::Id, "id", DataType::Integer).NOT_NULL
    .COLUMN_VARCHAR(StudentsCols::Name, "name", 128)
    .PRIMARY_KEYS(IDS(QtSqlLib::ID(StudentsCols::Id), QtSqlLib::ID(StudentsCols::Name)));

  configurator.CONFIGURE_TABLE(TableIds::Professors, "professors")
    .COLUMN(ProfessorsCols::Id, "id", DataType::Integer).NOT_NULL
    .COLUMN_VARCHAR(ProfessorsCols::Name, "name", 128)
    .PRIMARY_KEYS(IDS(QtSqlLib::ID(ProfessorsCols::Id), QtSqlLib::ID(ProfessorsCols::Name)));

  configurator.CONFIGURE_RELATIONSHIP(Relationships::Special1, TableIds::Students, TableIds::Professors, ISchema::RelationshipType::OneToMany);
  configurator.CONFIGURE_RELATIONSHIP(Relationships::Special2, TableIds::Students, TableIds::Professors, ISchema::RelationshipType::OneToMany);
  configurator.CONFIGURE_RELATIONSHIP(Relationships::Special3, TableIds::Students, TableIds::Professors, ISchema::RelationshipType::ManyToMany);
  configurator.CONFIGURE_RELATIONSHIP(Relationships::Special4, TableIds::Students, TableIds::Professors, ISchema::RelationshipType::ManyToMany);
  configurator.CONFIGURE_RELATIONSHIP(Relationships::Special5, TableIds::Students, TableIds::Students, ISchema::RelationshipType::OneToMany);
  configurator.CONFIGURE_RELATIONSHIP(Relationships::Special6, TableIds::Students, TableIds::Students, ISchema::RelationshipType::ManyToMany);

  TestDatabase db;
  db.initialize(configurator, Funcs::getDefaultDatabaseFilename());

  const auto student1 = db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Id, 0)
    .VALUE(StudentsCols::Name, "student1")
    .RETURN_IDS).resultTuples[0].values;

  const auto student2 = db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Id, 1)
    .VALUE(StudentsCols::Name, "student2")
    .RETURN_IDS).resultTuples[0].values;

  const auto student3 = db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Id, 2)
    .VALUE(StudentsCols::Name, "student3")
    .RETURN_IDS).resultTuples[0].values;

  const auto professor1 = db.execQuery(INSERT_INTO_EXT(TableIds::Professors)
    .VALUE(ProfessorsCols::Id, 0)
    .VALUE(ProfessorsCols::Name, "professor1")
    .RETURN_IDS).resultTuples[0].values;

  const auto professor2 = db.execQuery(INSERT_INTO_EXT(TableIds::Professors)
    .VALUE(ProfessorsCols::Id, 1)
    .VALUE(ProfessorsCols::Name, "professor2")
    .RETURN_IDS).resultTuples[0].values;

  const auto professor3 = db.execQuery(INSERT_INTO_EXT(TableIds::Professors)
    .VALUE(ProfessorsCols::Id, 2)
    .VALUE(ProfessorsCols::Name, "professor3")
    .RETURN_IDS).resultTuples[0].values;

  // (1)
  db.execQuery(LINK_TUPLES(Relationships::Special1)
    .FROM_ONE(student1)
    .TO_ONE(professor1));

  // (2)
  db.execQuery(LINK_TUPLES(Relationships::Special1)
    .FROM_ONE(professor2)
    .TO_ONE(student1));

  // (3)
  db.execQuery(LINK_TUPLES(Relationships::Special2)
    .FROM_ONE(student1)
    .TO_MANY({ professor1, professor2 }));

  // (4)
  db.execQuery(LINK_TUPLES(Relationships::Special2)
    .FROM_ONE(professor3)
    .TO_ONE(student2));

  // (5)
  db.execQuery(LINK_TUPLES(Relationships::Special3)
    .FROM_ONE(professor1)
    .TO_MANY({ student2, student3 }));

  // (6)
  db.execQuery(LINK_TUPLES(Relationships::Special3)
    .FROM_ONE(student1)
    .TO_MANY({ professor1, professor3 }));

  // (7)
  db.execQuery(LINK_TUPLES(Relationships::Special3)
    .FROM_ONE(professor2)
    .TO_MANY({ student1, student3 }));

  // (8)
  db.execQuery(LINK_TUPLES(Relationships::Special4)
    .FROM_ONE(professor3)
    .TO_ONE(student1));

  // (9)
  db.execQuery(LINK_TUPLES(Relationships::Special4)
    .FROM_ONE(student3)
    .TO_MANY({ professor1, professor2, professor3 }));

  // (10)
  db.execQuery(LINK_TUPLES(Relationships::Special5)
    .FROM_ONE(student1)
    .TO_MANY({ student1, student2 }));

  // (11)
  db.execQuery(LINK_TUPLES(Relationships::Special5)
    .FROM_ONE(student2)
    .TO_ONE(student3));

  // (12)
  db.execQuery(LINK_TUPLES(Relationships::Special6)
    .FROM_ONE(student1)
    .TO_MANY({ student2, student3 }));

  // (13)
  db.execQuery(LINK_TUPLES(Relationships::Special6)
    .FROM_ONE(student2)
    .TO_ONE(student3));

  // (14)
  auto results = db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT(IDS(QtSqlLib::ID(StudentsCols::Name)))
    .JOIN(Relationships::Special1, IDS(QtSqlLib::ID(ProfessorsCols::Name))));

  expectSpecialRelation1Students(results.resultTuples);

  // (15)
  results = db.execQuery(FROM_TABLE(TableIds::Professors)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special1));

  expectSpecialRelation1Professors(results.resultTuples);

  // (16)
  results = db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special2));

  expectSpecialRelation2Students(results.resultTuples);

  // (17)
  results = db.execQuery(FROM_TABLE(TableIds::Professors)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special2));

  expectSpecialRelation2Professors(results.resultTuples);

  // (18)
  results = db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special1)
    .JOIN_ALL(Relationships::Special2));

  expectSpecialRelation1Students(results.resultTuples);
  expectSpecialRelation2Students(results.resultTuples);

  // (19)
  results = db.execQuery(FROM_TABLE(TableIds::Professors)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special1)
    .JOIN_ALL(Relationships::Special2));

  expectSpecialRelation1Professors(results.resultTuples);
  expectSpecialRelation2Professors(results.resultTuples);

  // (20)
  results = db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special3));

  expectSpecialRelation3Students(results.resultTuples);

  // (21)
  results = db.execQuery(FROM_TABLE(TableIds::Professors)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special3));

  expectSpecialRelation3Professors(results.resultTuples);

  // (22)
  results = db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special4));

  expectSpecialRelation4Students(results.resultTuples);

  // (23)
  results = db.execQuery(FROM_TABLE(TableIds::Professors)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special4));

  expectSpecialRelation4Professors(results.resultTuples);

  // (24)
  results = db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special3)
    .JOIN_ALL(Relationships::Special4));

  expectSpecialRelation3Students(results.resultTuples);
  expectSpecialRelation4Students(results.resultTuples);

  // (25)
  results = db.execQuery(FROM_TABLE(TableIds::Professors)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special3)
    .JOIN_ALL(Relationships::Special4));

  expectSpecialRelation3Professors(results.resultTuples);
  expectSpecialRelation4Professors(results.resultTuples);

  // (26)
  results = db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special1)
    .JOIN_ALL(Relationships::Special2)
    .JOIN_ALL(Relationships::Special3)
    .JOIN_ALL(Relationships::Special4));

  expectSpecialRelation1Students(results.resultTuples);
  expectSpecialRelation2Students(results.resultTuples);
  expectSpecialRelation3Students(results.resultTuples);
  expectSpecialRelation4Students(results.resultTuples);

  // (27)
  results = db.execQuery(FROM_TABLE(TableIds::Professors)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special1)
    .JOIN_ALL(Relationships::Special2)
    .JOIN_ALL(Relationships::Special3)
    .JOIN_ALL(Relationships::Special4));

  expectSpecialRelation1Professors(results.resultTuples);
  expectSpecialRelation2Professors(results.resultTuples);
  expectSpecialRelation3Professors(results.resultTuples);
  expectSpecialRelation4Professors(results.resultTuples);

  // (28)
  results = db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special5));

  expectSpecialRelation5Students(results.resultTuples);

  // (29)
  results = db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special6));

  expectSpecialRelation6Students(results.resultTuples);

  // (30)
  results = db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT_ALL
    .JOIN_ALL(Relationships::Special1)
    .JOIN_ALL(Relationships::Special2)
    .JOIN_ALL(Relationships::Special3)
    .JOIN_ALL(Relationships::Special4)
    .JOIN_ALL(Relationships::Special5)
    .JOIN_ALL(Relationships::Special6));

  expectSpecialRelation1Students(results.resultTuples);
  expectSpecialRelation2Students(results.resultTuples);
  expectSpecialRelation3Students(results.resultTuples);
  expectSpecialRelation4Students(results.resultTuples);
  expectSpecialRelation5Students(results.resultTuples);
  expectSpecialRelation6Students(results.resultTuples);
}

}
