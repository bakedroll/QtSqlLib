#pragma once

#include <QtSqlLib/IQuery.h>
#include <QtSqlLib/IQuerySequence.h>

#include <vector>

namespace QtSqlLib
{

class IDatabase
{
public:
  IDatabase() = default;
  virtual ~IDatabase() = default;

  virtual void initialize(const QString& filename) = 0;
  virtual void close() = 0;

  virtual QueryDefines::QueryResults execQuery(IQuery& query) = 0;
  virtual QueryDefines::QueryResults execQuery(IQuerySequence& query) = 0;

};

}
