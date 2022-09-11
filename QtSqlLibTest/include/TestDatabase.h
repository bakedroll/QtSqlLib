#pragma once

#include <QtSqlLib/Database.h>

namespace QtSqlLibTest
{

class TestDatabase : public QtSqlLib::Database
{
public:
  TestDatabase();
  ~TestDatabase() override;

};

}
