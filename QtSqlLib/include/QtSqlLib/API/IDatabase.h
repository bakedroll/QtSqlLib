#pragma once

#include <QtSqlLib/ColumnList.h>
#include <QtSqlLib/ResultSet.h>

#include <QSqlDatabase>
#include <QString>

namespace QtSqlLib::API
{

class IQueryElement;
class ISchemaConfigurator;

class IDatabase
{
public:
  IDatabase() = default;
  virtual ~IDatabase() = default;

  virtual void initialize(
    ISchemaConfigurator& schemaConfigurator, const QString& fileName,
    const QString& databaseName = QSqlDatabase::defaultConnection) = 0;
  virtual void close() = 0;

  virtual ResultSet execQuery(IQueryElement& query) = 0;

};

}
