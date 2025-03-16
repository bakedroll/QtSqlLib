#include <gtest/gtest.h>

#include <Common.h>

#include <QFile>

namespace QtSqlLibTest
{

class TestTableIndex : public testing::Test
{
public:
  TestTableIndex()
  {
    QFile::remove(Funcs::getDefaultDatabaseFilename());
  }

  ~TestTableIndex() override
  {
    m_db.close();
  }

  QtSqlLib::Database m_db;

};

/**
 * @test: Checks, if multiple indices can be created on two different tables.
 * @expected: No exceptions will be thrown.
 */
TEST_F(TestTableIndex, createMultipleIndicesWithoutException)
{
  SchemaConfigurator configurator;
  configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
    .COLUMN(Table1Cols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(Table1Cols::Text, "text", 128);

  configurator.CONFIGURE_TABLE(TableIds::Table2, "table2")
    .COLUMN(Table2Cols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(Table2Cols::Text, "text", 128);

  configurator.CONFIGURE_INDEX(TableIds::Table1)
    .COLUMNS(Table1Cols::Id, Table1Cols::Text);

  configurator.CONFIGURE_INDEX(TableIds::Table1)
    .UNIQUE.COLUMNS(Table1Cols::Text);

  configurator.CONFIGURE_INDEX(TableIds::Table2)
    .COLUMNS(Table2Cols::Id, Table2Cols::Text);

  configurator.CONFIGURE_INDEX(TableIds::Table2)
    .UNIQUE.COLUMNS(Table2Cols::Text, Table2Cols::Id);

  m_db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
}

/**
 * @test: Checks, if indices can be created on foreign keys of relationships.
 * @expected: No exceptions will be thrown.
 */
TEST_F(TestTableIndex, createIndicesOnRelationshipForeignKeysWithoutException)
{
  SchemaConfigurator configurator;
  configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
    .COLUMN(Table1Cols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(Table1Cols::Text, "text", 128);

  configurator.CONFIGURE_TABLE(TableIds::Table2, "table2")
    .COLUMN(Table2Cols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(Table2Cols::Text, "text", 128);

  configurator.CONFIGURE_RELATIONSHIP(Relationships::Special1, TableIds::Table1, TableIds::Table2,
    QtSqlLib::API::RelationshipType::OneToMany).ENABLE_FOREIGN_KEY_INDEXING;

  configurator.CONFIGURE_RELATIONSHIP(Relationships::Special2, TableIds::Table2, TableIds::Table1,
    QtSqlLib::API::RelationshipType::ManyToMany).ENABLE_FOREIGN_KEY_INDEXING;

  m_db.initialize(configurator, Funcs::getDefaultDatabaseFilename());
}

/**
* @test: Checks, if exceptions will be thrown on invalid behavior.
*          (1) Pass empty columns list
*          (2) Call columns() multiple times
*          (3) Call unique() multiple times
*          (4) Invalid table id
*          (5) Invalid column id
* @expected: Exceptions will be thrown as expected.
*/
TEST_F(TestTableIndex, exceptionsMustThrowOnInvalidBehavior)
{
  SchemaConfigurator configurator;
  configurator.CONFIGURE_TABLE(TableIds::Table1, "table1")
    .COLUMN(Table1Cols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(Table1Cols::Text, "text", 128);

  // (1)
  EXPECT_THROW(configurator.CONFIGURE_INDEX(TableIds::Table1).columns(QtSqlLib::ColumnList{}), DatabaseException);

  // (2)
  EXPECT_THROW(configurator.CONFIGURE_INDEX(TableIds::Table1)
    .COLUMNS(Table1Cols::Text)
    .COLUMNS(Table1Cols::Id), DatabaseException);

  // (3)
  EXPECT_THROW(configurator.CONFIGURE_INDEX(TableIds::Table1)
    .COLUMNS(Table1Cols::Text)
    .UNIQUE
    .UNIQUE, DatabaseException);

  // (4)
  QtSqlLib::Database db1;
  SchemaConfigurator configurator1;
  configurator1.CONFIGURE_TABLE(TableIds::Table1, "table1")
    .COLUMN(Table1Cols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(Table1Cols::Text, "text", 128);

  configurator1.CONFIGURE_INDEX(TableIds::Table2)
    .COLUMNS(Table1Cols::Id, Table1Cols::Text);

  EXPECT_THROW(db1.initialize(configurator1, Funcs::getDefaultDatabaseFilename()), DatabaseException);

  // (5)
  QtSqlLib::Database db2;
  SchemaConfigurator configurator2;
  configurator2.CONFIGURE_TABLE(TableIds::Table1, "table1")
    .COLUMN(Table1Cols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(Table1Cols::Text, "text", 128);

  configurator2.CONFIGURE_INDEX(TableIds::Table1)
    .COLUMNS(Table1Cols::Id, Table1Cols::Number);

  EXPECT_THROW(db2.initialize(configurator2, Funcs::getDefaultDatabaseFilename()), DatabaseException);
}

}
