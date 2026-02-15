#include <gtest/gtest.h>

#include <Common.h>

#include <mocks/MockSanityChecker.h>
#include <mocks/MockSchema.h>

namespace QtSqlLibTest
{

class TestColumnSelection : public testing::Test
{
public:
  TestColumnSelection() :
    m_db(std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase("QSQLITE" , QSqlDatabase::defaultConnection)))
  {
    m_db->setDatabaseName(":memory:");
    EXPECT_TRUE(m_db->open());

    const auto table1Id = static_cast<QtSqlLib::API::IID::Type>(TableIds::Table1);
    const auto table2Id = static_cast<QtSqlLib::API::IID::Type>(TableIds::Table2);
    const auto relationshipId = static_cast<QtSqlLib::API::IID::Type>(Relationships::Special1);
    const auto foreignKeyColumnId = 999;

    QtSqlLib::API::ForeignKeyReference foreignKeyReference {
      table2Id,
      QtSqlLib::API::ForeignKeyAction::NoAction,
      QtSqlLib::API::ForeignKeyAction::NoAction,
      {}
    };
    foreignKeyReference.primaryForeignKeyColIdMap[static_cast<QtSqlLib::API::IID::Type>(Table1Cols::Id)] = foreignKeyColumnId;

    auto& table1 = m_tables[table1Id];
    table1.name = "table1";
    table1.columns[static_cast<QtSqlLib::API::IID::Type>(Table1Cols::Id)].name = "id";
    table1.columns[static_cast<QtSqlLib::API::IID::Type>(Table1Cols::Text)].name = "text";
    table1.primaryKeys.emplace_back(static_cast<QtSqlLib::API::IID::Type>(Table1Cols::Id));

    auto& table2 = m_tables[table2Id];
    table2.name = "table2";
    table2.columns[static_cast<QtSqlLib::API::IID::Type>(Table2Cols::Id)].name = "id";
    table2.columns[static_cast<QtSqlLib::API::IID::Type>(Table2Cols::Text)].name = "text";
    table2.columns[foreignKeyColumnId].name = "foreign_key";
    table2.primaryKeys.emplace_back(static_cast<QtSqlLib::API::IID::Type>(Table2Cols::Id));
    table2.relationshipToForeignKeyReferencesMap[{ relationshipId, table1Id }].emplace_back(foreignKeyReference);

    auto& relationship = m_relationships[relationshipId];
    relationship.tableFromId = table1Id;
    relationship.tableToId = table2Id;
    relationship.type = QtSqlLib::API::RelationshipType::OneToMany;

    ON_CALL(m_mockSchema, getSanityChecker()).WillByDefault(ReturnRef(m_mockSanityChecker));
    ON_CALL(m_mockSchema, getTables()).WillByDefault(ReturnRef(m_tables));
    ON_CALL(m_mockSchema, getRelationships()).WillByDefault(ReturnRef(m_relationships));
  }

  ~TestColumnSelection()
  {
    m_db->close();
    m_db.reset();
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
  }

  NiceMock<MockSchema> m_mockSchema;
  NiceMock<MockSanityChecker> m_mockSanityChecker;

  std::unique_ptr<QSqlDatabase> m_db;
  QtSqlLib::ResultSet m_resultSet;

  std::map<QtSqlLib::API::IID::Type, QtSqlLib::API::Table> m_tables;
  std::map<QtSqlLib::API::IID::Type, QtSqlLib::API::Relationship> m_relationships;

};


/**
 * @test: Create a simple query with concatenated columns and check the resulting query string.
 * @expected: The query string contains the expected SQL code.
 */
TEST_F(TestColumnSelection, concatenatedColumns)
{
  const QString expectedQueryStr("SELECT CONCAT('table1'.'id', \"_\", 'table1'.'text'), 'table1'.'id' " \
    "FROM 'table1';");

  QtSqlLib::Query::FromTable fromTable(QtSqlLib::ID(TableIds::Table1));
  fromTable.SELECT(CONCAT(Table1Cols::Id, "_", Table1Cols::Text));

  const auto query = fromTable.getSqlQuery(*m_db, m_mockSchema, m_resultSet);

  EXPECT_EQ(expectedQueryStr, query.qtQuery.lastQuery());
}

/**
 * @test: Create a query with concatenated columns of two joined tables.
 * @expected: The query string contains the expected SQL code.
 */
TEST_F(TestColumnSelection, concatenatedColumnsOfJoinedTables)
{
  const QString expectedQueryStr("SELECT CONCAT('table1'.'id', \"_\", 'table2'.'text'), 'table1'.'id', 'table2'.'id', 'table2'.'foreign_key' " \
    "FROM 'table1' " \
    "LEFT JOIN 'table2' " \
    "ON 'table1'.'id' = 'table2'.'foreign_key';");

  QtSqlLib::Query::FromTable fromTable(QtSqlLib::ID(TableIds::Table1));
  fromTable.SELECT(CONCAT(Table1Cols::Id, "_", COL(Relationships::Special1, Table2Cols::Text))).JOIN(Relationships::Special1, Table2Cols::Id);

  const auto query = fromTable.getSqlQuery(*m_db, m_mockSchema, m_resultSet);

  EXPECT_EQ(expectedQueryStr, query.qtQuery.lastQuery());
}

/**
 * @test: Create a simple query with aliased columns.
 * @expected: The query string contains the expected SQL code.
 */
TEST_F(TestColumnSelection, columnAlias)
{
  const QString expectedQueryStr("SELECT 'table1'.'text' AS [t], 'table1'.'id' " \
    "FROM 'table1';");

  QtSqlLib::Query::FromTable fromTable(QtSqlLib::ID(TableIds::Table1));
  fromTable.SELECT(AS_ALIAS(Table1Cols::Text, "t"));

  const auto query = fromTable.getSqlQuery(*m_db, m_mockSchema, m_resultSet);

  EXPECT_EQ(expectedQueryStr, query.qtQuery.lastQuery());
}

/**
 * @test: Create a query that concatenates columns with an alias.
 * @expected: The query string contains the expected SQL code.
 */
TEST_F(TestColumnSelection, concatenatedColumnsAlias)
{
  const QString expectedQueryStr("SELECT CONCAT('table1'.'id', \"_\", 'table1'.'text') AS [t], 'table1'.'id' " \
    "FROM 'table1';");

  QtSqlLib::Query::FromTable fromTable(QtSqlLib::ID(TableIds::Table1));
  fromTable.SELECT(AS_ALIAS(CONCAT(Table1Cols::Id, "_", Table1Cols::Text), "t"));

  const auto query = fromTable.getSqlQuery(*m_db, m_mockSchema, m_resultSet);

  EXPECT_EQ(expectedQueryStr, query.qtQuery.lastQuery());
}

}
