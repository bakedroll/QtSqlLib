#pragma once

#include <QtSqlLib/Database.h>

namespace QtSqlLibTest
{

class DatabaseDummy : public QtSqlLib::Database
{
public:
  using ConfigFunc = std::function<void(QtSqlLib::SchemaConfigurator&)>;

  DatabaseDummy();
  ~DatabaseDummy() override;

  void setConfigureSchemaFunc(const ConfigFunc& func);

protected:
  void configureSchema(QtSqlLib::SchemaConfigurator& configurator) override;

private:
  ConfigFunc m_configureSchemaFunc;

};

}
