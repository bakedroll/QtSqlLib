#pragma once

#include <QtSqlLib/API/ISchemaConfigurator.h>
#include <QtSqlLib/API/IQuery.h>

namespace QtSqlLib::API
{

class IQueryElement;

class IDatabase
{
public:
  IDatabase() = default;
  virtual ~IDatabase() = default;

  virtual void initialize(ISchemaConfigurator& schemaConfigurator, const QString& fileName,
                          const QString& databaseName = QSqlDatabase::defaultConnection) = 0;
  virtual void close() = 0;

  virtual IQuery::QueryResults execQuery(IQueryElement& query) = 0;

};

}
