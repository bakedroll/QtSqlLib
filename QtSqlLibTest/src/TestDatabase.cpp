#include "TestDatabase.h"

#include "Common.h"

#include <QFile>

namespace QtSqlLibTest
{

TestDatabase::TestDatabase() = default;

TestDatabase::~TestDatabase()
{
  Database::close();
  QFile::remove(Funcs::getDefaultDatabaseFilename());
}

}
