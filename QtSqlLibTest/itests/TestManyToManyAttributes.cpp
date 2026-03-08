#include <gtest/gtest.h>

#include <Common.h>

#include <QFile>

namespace QtSqlLibTest
{

class TestManyToManyAttributes : public testing::Test
{
public:
  TestManyToManyAttributes()
  {
    QFile::remove(Funcs::getDefaultDatabaseFilename());
  }

  ~TestManyToManyAttributes() override
  {
    m_db.close();
  }

  QtSqlLib::Database m_db;

};

static void setupReplationshipTestsDatabase(QtSqlLib::API::IDatabase& db)
{
  SchemaConfigurator configurator;
  configurator.CONFIGURE_TABLE(TableIds::Students, "students")
    .COLUMN(StudentsCols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN(StudentsCols::Name, "name", DataType::Text);

  configurator.CONFIGURE_TABLE(TableIds::Lectures, "lectures")
    .COLUMN(LecturesCols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN(LecturesCols::Topic, "topic", DataType::Text);

  configurator.CONFIGURE_RELATIONSHIP(Relationships::LectureParticipant, TableIds::Students, TableIds::Lectures,
    QtSqlLib::API::RelationshipType::ManyToMany)
    .ON_DELETE(QtSqlLib::API::ForeignKeyAction::Cascade)
    .ATTRIBUTE(Attributes::Grade, "grade", DataType::Real).NOT_NULL
    .ATTRIBUTE(Attributes::Notes, "notes", DataType::Text);

  db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
}

/**
 * @test: Tests the linking of tuples with attributes.
 * @expected: All attributes are inserted correctly.
 */
TEST_F(TestManyToManyAttributes, linkTuplesWithAttributes)
{
  setupReplationshipTestsDatabase(m_db);

  const auto paul = m_db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "Paul")
    .RETURN_IDS).nextTuple().primaryKey();

  const auto math = m_db.execQuery(INSERT_INTO_EXT(TableIds::Lectures)
    .VALUE(LecturesCols::Topic, "math")
    .RETURN_IDS).nextTuple().primaryKey();

  const auto sports = m_db.execQuery(INSERT_INTO_EXT(TableIds::Lectures)
    .VALUE(LecturesCols::Topic, "sports")
    .RETURN_IDS).nextTuple().primaryKey();

  const auto mary = m_db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "Mary")
    .RETURN_IDS).nextTuple().primaryKey();

  m_db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(paul)
    .TO_ONE(math)
    .ATTRIBUTE_VALUE(Attributes::Grade, 2.3)
    .ATTRIBUTE_VALUE(Attributes::Notes, "notes 1"));

  m_db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(paul)
    .TO_ONE(sports)
    .ATTRIBUTE_VALUE(Attributes::Grade, 4.0)
    .ATTRIBUTE_VALUE(Attributes::Notes, "notes 2"));

  m_db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(mary)
    .TO_ONE(math)
    .ATTRIBUTE_VALUE(Attributes::Grade, 3.0)
    .ATTRIBUTE_VALUE(Attributes::Notes, "notes 3"));

  auto results = m_db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT(StudentsCols::Name)
    .JOIN(Relationships::LectureParticipant, LecturesCols::Topic)
    .SELECT_ATTRIBUTES(Attributes::Grade, Attributes::Notes));

  Funcs::expectRelationsWithAttributes(results, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Paul", QVariantList() << "math" << "sports",
    Attributes::Grade, QVariantList() << 2.3 << 4.0);

  Funcs::expectRelationsWithAttributes(results, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Mary", QVariantList() << "math",
    Attributes::Grade, QVariantList() << 3.0);
}

/**
 * @test: Tests the usage of attributes in WHERE clauses.
 * @expected: Results are correctly filtered.
 */
TEST_F(TestManyToManyAttributes, attributeExpression)
{
  setupReplationshipTestsDatabase(m_db);

  const auto paul = m_db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "Paul")
    .RETURN_IDS).nextTuple().primaryKey();

  const auto math = m_db.execQuery(INSERT_INTO_EXT(TableIds::Lectures)
    .VALUE(LecturesCols::Topic, "math")
    .RETURN_IDS).nextTuple().primaryKey();

  const auto sports = m_db.execQuery(INSERT_INTO_EXT(TableIds::Lectures)
    .VALUE(LecturesCols::Topic, "sports")
    .RETURN_IDS).nextTuple().primaryKey();

  const auto mary = m_db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "Mary")
    .RETURN_IDS).nextTuple().primaryKey();

  m_db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(paul)
    .TO_ONE(math)
    .ATTRIBUTE_VALUE(Attributes::Grade, 2.3)
    .ATTRIBUTE_VALUE(Attributes::Notes, "notes 1"));

  m_db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(paul)
    .TO_ONE(sports)
    .ATTRIBUTE_VALUE(Attributes::Grade, 4.0)
    .ATTRIBUTE_VALUE(Attributes::Notes, "notes 2"));

  m_db.execQuery(LINK_TUPLES(Relationships::LectureParticipant)
    .FROM_ONE(mary)
    .TO_ONE(math)
    .ATTRIBUTE_VALUE(Attributes::Grade, 3.0)
    .ATTRIBUTE_VALUE(Attributes::Notes, "notes 3"));

  auto results = m_db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT(StudentsCols::Name)
    .JOIN(Relationships::LectureParticipant, LecturesCols::Topic)
    .SELECT_ATTRIBUTES(Attributes::Grade, Attributes::Notes)
    .WHERE(LESSEQUAL(ATTR(Relationships::LectureParticipant, Attributes::Grade), 3.0)));

  Funcs::expectRelationsWithAttributes(results, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Paul", QVariantList() << "math",
    Attributes::Grade, QVariantList() << 2.3);

  Funcs::expectRelationsWithAttributes(results, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Mary", QVariantList() << "math",
    Attributes::Grade, QVariantList() << 3.0);
}

/**
 * @test: Tests the insertion of tuples and linking directly with attributes.
 * @expected: All attributes are inserted correctly.
 */
TEST_F(TestManyToManyAttributes, insertTuplesWithAttributeLinking)
{
  setupReplationshipTestsDatabase(m_db);

  const auto paul = m_db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "Paul")
    .RETURN_IDS).nextTuple().primaryKey();

  const auto math = m_db.execQuery(INSERT_INTO_EXT(TableIds::Lectures)
    .VALUE(LecturesCols::Topic, "math")
    .LINK_TO_ONE_TUPLE(Relationships::LectureParticipant, paul)
    .ATTRIBUTE_VALUE(Attributes::Grade, 2.3)
    .ATTRIBUTE_VALUE(Attributes::Notes, "notes 1")
    .RETURN_IDS).nextTuple().primaryKey();

  const auto sports = m_db.execQuery(INSERT_INTO_EXT(TableIds::Lectures)
    .VALUE(LecturesCols::Topic, "sports")
    .LINK_TO_ONE_TUPLE(Relationships::LectureParticipant, paul)
    .ATTRIBUTE_VALUE(Attributes::Grade, 4.0)
    .ATTRIBUTE_VALUE(Attributes::Notes, "notes 2")
    .RETURN_IDS).nextTuple().primaryKey();

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Students)
    .VALUE(StudentsCols::Name, "Mary")
    .LINK_TO_ONE_TUPLE(Relationships::LectureParticipant, math)
    .ATTRIBUTE_VALUE(Attributes::Grade, 3.0)
    .ATTRIBUTE_VALUE(Attributes::Notes, "notes 3"));

  auto results = m_db.execQuery(FROM_TABLE(TableIds::Students)
    .SELECT(StudentsCols::Name)
    .JOIN(Relationships::LectureParticipant, LecturesCols::Topic)
    .SELECT_ATTRIBUTES(Attributes::Grade, Attributes::Notes));

  Funcs::expectRelationsWithAttributes(results, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Paul", QVariantList() << "math" << "sports",
    Attributes::Grade, QVariantList() << 2.3 << 4.0);

  Funcs::expectRelationsWithAttributes(results, Relationships::LectureParticipant,
    TableIds::Students, StudentsCols::Name, TableIds::Lectures, LecturesCols::Topic,
    "Mary", QVariantList() << "math",
    Attributes::Grade, QVariantList() << 3.0);
}

}
