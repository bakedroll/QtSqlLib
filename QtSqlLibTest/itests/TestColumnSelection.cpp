#include <gtest/gtest.h>

#include <Common.h>

#include <mocks/MockSanityChecker.h>
#include <mocks/MockSchema.h>

namespace QtSqlLibTest
{

class TestColumnSelection : public testing::Test
{
public:
  TestColumnSelection()
  {
    m_db = QSqlDatabase::addDatabase("QSQLITE" , QSqlDatabase::defaultConnection);
    m_db.setDatabaseName(":memory:");
    EXPECT_TRUE(m_db.open());

    auto& table = m_tables[static_cast<QtSqlLib::API::IID::Type>(TableIds::Table1)];
    table.name = "table1";
    table.columns[static_cast<QtSqlLib::API::IID::Type>(Table1Cols::Id)].name = "id";
    table.columns[static_cast<QtSqlLib::API::IID::Type>(Table1Cols::Text)].name = "text";

    ON_CALL(m_mockSchema, getSanityChecker()).WillByDefault(ReturnRef(m_mockSanityChecker));
    ON_CALL(m_mockSchema, getTables()).WillByDefault(ReturnRef(m_tables));
  }

  ~TestColumnSelection()
  {
    m_db.close();
  }

  NiceMock<MockSchema> m_mockSchema;
  NiceMock<MockSanityChecker> m_mockSanityChecker;

  QSqlDatabase m_db;
  QtSqlLib::ResultSet m_resultSet;

  std::map<QtSqlLib::API::IID::Type, QtSqlLib::API::Table> m_tables;

};

/**
 * @test: Create a simple query with concatenated columns and check the resulting query string.
 * @expected: The query string contains the expected SQL code.
 */
TEST_F(TestColumnSelection, concatenatedColumns)
{
    QtSqlLib::Query::FromTable  fromTable(QtSqlLib::ID(TableIds::Table1));
    fromTable.SELECT(Table1Cols::Id, Table1Cols::Text);

    const auto query = fromTable.getSqlQuery(m_db, m_mockSchema, m_resultSet);

    const auto queryStr = query.qtQuery.lastQuery();

    printf("################\n\n%s\n\n#############", queryStr.toStdString().c_str());
}

}
