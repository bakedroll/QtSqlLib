#include "DatabaseDummy.h"

#include "Common.h"

#include <QFile>

#include <gtest/gtest.h>

namespace QtSqlLibTest
{

DatabaseDummy::DatabaseDummy() = default;

DatabaseDummy::~DatabaseDummy()
{
  Database::close();

  QFile::remove(Funcs::getDefaultDatabaseFilename());
}

void DatabaseDummy::setConfigureSchemaFunc(const ConfigFunc& func)
{
  m_configureSchemaFunc = func;
}

void DatabaseDummy::configureSchema(QtSqlLib::SchemaConfigurator& configurator)
{
  ASSERT_TRUE(m_configureSchemaFunc);
  m_configureSchemaFunc(configurator);
}

}
