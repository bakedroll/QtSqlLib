#include <gtest/gtest.h>

#include <QtSqlLib/Database.h>

#include <utilsLib/Utils.h>

#include <QFile>

using DataType = QtSqlLib::TableConfigurator::DataType;

static const QString s_dbFilename = "test.db";

class TestDb : public QtSqlLib::Database
{
  enum class TableIds
  {
    Table1
  };

  enum class Table1Cols
  {
    Id,
    Name
  };

  void configureSchema(QtSqlLib::SchemaConfigurator& configurator) override
  {
    configurator.configureTable(utilsLib::underlying(TableIds::Table1), "table1")
      .column(utilsLib::underlying(Table1Cols::Id), "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(utilsLib::underlying(Table1Cols::Name), "name", DataType::Varchar, 128);
  }
};

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

TEST_F(DatabaseTest, initializeDatabase)
{
  m_db.initialize(s_dbFilename);
}
