#include <gtest/gtest.h>

#include <QtSqlLib/Database.h>
#include <QtSqlLib/InsertInto.h>
#include <QtSqlLib/BatchInsertInto.h>

#include <utilsLib/Utils.h>

#include <QFile>

using DataType = QtSqlLib::TableConfigurator::DataType;

static const QString s_dbFilename = "test.db";

class TestDb : public QtSqlLib::Database
{
public:
  enum class TableIds
  {
    Table1
  };

  enum class Table1Cols
  {
    Id,
    Text,
    Mandatory
  };

protected:
  void configureSchema(QtSqlLib::SchemaConfigurator& configurator) override
  {
    configurator.configureTable(utilsLib::underlying(TableIds::Table1), "table1")
      .column(utilsLib::underlying(Table1Cols::Id), "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(utilsLib::underlying(Table1Cols::Text), "text", DataType::Varchar, 128)
      .column(utilsLib::underlying(Table1Cols::Mandatory), "mandatory", DataType::Real).notNull();
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

  m_db.execQuery(QtSqlLib::InsertInto(utilsLib::underlying(TestDb::TableIds::Table1))
    .value(utilsLib::underlying(TestDb::Table1Cols::Text), "test")
    .value(utilsLib::underlying(TestDb::Table1Cols::Mandatory), 0.5f));

  m_db.execQuery(QtSqlLib::BatchInsertInto(utilsLib::underlying(TestDb::TableIds::Table1))
    .values(utilsLib::underlying(TestDb::Table1Cols::Text), QVariantList() << "test1" << "test2" << "test3")
    .values(utilsLib::underlying(TestDb::Table1Cols::Mandatory), QVariantList() << 0.6f << 0.7f << 0.8f));
}
