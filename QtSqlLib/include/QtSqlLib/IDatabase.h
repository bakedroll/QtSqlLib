#pragma once

#include "QtSqlLib/IQuery.h"

#include <QString>
#include <QVariant>

#include <vector>
#include <map>

namespace QtSqlLib
{

class IDatabase
{
public:
  IDatabase() = default;
  virtual ~IDatabase() = default;

  virtual void initialize(const QString& filename) = 0;
  virtual void close() = 0;

  virtual IQuery::QueryResults execQuery(const IQuery& query) const = 0;

};

}
