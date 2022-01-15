#pragma once

#include <QSqlQuery>

#include <QtSqlLib/SchemaConfigurator.h>

namespace QtSqlLib
{

class IQuery
{
public:
  IQuery() = default;
  virtual ~IQuery() = default;

  virtual QSqlQuery getQueryString(const SchemaConfigurator::Schema& schema) const = 0;

};

}
