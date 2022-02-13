#pragma once

#include <QtSqlLib/Schema.h>
#include <QtSqlLib/Query/QueryDefines.h>

#include <QSqlQuery>

namespace QtSqlLib::API
{

class IQuery
{
public:
  IQuery() = default;
  virtual ~IQuery() = default;

  IQuery(const IQuery& other) = delete;
  IQuery& operator= (const IQuery& other) = delete;

  virtual Query::QueryDefines::SqlQuery getSqlQuery(Schema& schema) = 0;
  virtual Query::QueryDefines::QueryResults getQueryResults(Schema& schema, QSqlQuery& query) const { return {}; }

};

}
