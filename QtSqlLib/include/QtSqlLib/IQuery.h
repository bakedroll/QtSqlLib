#pragma once

#include <QtSqlLib/Schema.h>
#include <QtSqlLib/QueryDefines.h>

#include <QSqlQuery>

namespace QtSqlLib
{

class IQuery
{
public:
  IQuery() = default;
  virtual ~IQuery() = default;

  IQuery(const IQuery& other) = delete;
  IQuery& operator= (const IQuery& other) = delete;

  virtual QueryDefines::SqlQuery getSqlQuery(Schema& schema) = 0;
  virtual QueryDefines::QueryResults getQueryResults(Schema& schema, QSqlQuery& query) const { return {}; }

};

}
