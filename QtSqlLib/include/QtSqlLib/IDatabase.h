#pragma once

#include <QString>

namespace QtSqlLib
{

class IDatabase
{
public:
  IDatabase() = default;
  virtual ~IDatabase() = default;

  virtual void initialize(const QString& filename) = 0;
  virtual void close() = 0;

};

}