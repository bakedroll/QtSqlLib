#pragma once

#include <QtSqlLib/API/IQuery.h>

namespace QtSqlLib::API
{

class IQueryElement;

class IDatabase
{
public:
  IDatabase() = default;
  virtual ~IDatabase() = default;

  virtual void initialize(const QString& fileName, const QString& databaseName) = 0;
  virtual void close() = 0;

  virtual IQuery::QueryResults execQuery(IQueryElement& query) = 0;

};

}
