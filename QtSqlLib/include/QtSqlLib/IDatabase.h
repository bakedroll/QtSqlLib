#pragma once

#include <QString>

namespace QtSqlLib
{
class IQuery;

class IDatabase
{
public:
  IDatabase() = default;
  virtual ~IDatabase() = default;

  virtual void initialize(const QString& filename) = 0;
  virtual void close() = 0;

  virtual void execQuery(const IQuery& query) const = 0;

};

}