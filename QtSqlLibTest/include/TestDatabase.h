#pragma once

#include <QtSqlLib/Database.h>

namespace QtSqlLibTest
{

class TestDatabase : public QtSqlLib::Database
{
public:
  using ConfigFunc = std::function<void(QtSqlLib::SchemaConfigurator&)>;

  TestDatabase();
  ~TestDatabase() override;

};

}
