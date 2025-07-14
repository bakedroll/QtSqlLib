#include <gtest/gtest.h>

#include <Common.h>

#include <QFile>

namespace QtSqlLibTest
{

class TestAggregationAndSorting : public testing::Test
{
public:
  TestAggregationAndSorting()
  {
    QFile::remove(Funcs::getDefaultDatabaseFilename());
  }

  ~TestAggregationAndSorting() override
  {
    m_db.close();
  }

  QtSqlLib::Database m_db;

};

/**
 * @test:
 * @expected:
 */
TEST(TestAggregationAndSorting, test)
{

}

}
