#pragma once

#include <QtSqlLib/API/IQuery.h>
#include <QtSqlLib/API/IQuerySequence.h>

#include <vector>

namespace QtSqlLib::API
{

class IDatabase
{
public:
  IDatabase() = default;
  virtual ~IDatabase() = default;

  virtual void initialize(const QString& filename) = 0;
  virtual void close() = 0;

  virtual Query::QueryDefines::QueryResults execQuery(IQuery& query) = 0;
  virtual Query::QueryDefines::QueryResults execQuery(IQuerySequence& query) = 0;

};

}
