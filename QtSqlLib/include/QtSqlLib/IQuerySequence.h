#pragma once

#include <QtSqlLib/Schema.h>
#include <QtSqlLib/QueryDefines.h>

#include <QSqlQuery>

namespace QtSqlLib
{

class IQuerySequence
{
public:
  IQuerySequence() = default;
  virtual ~IQuerySequence() = default;

  IQuerySequence(const IQuerySequence& other) = delete;
  IQuerySequence& operator= (const IQuerySequence& other) = delete;

  virtual int getNumQueries() const = 0;

  virtual void prepare(Schema& schema) {}
  virtual QueryDefines::SqlQuery getSqlQuery(int num, Schema& schema) = 0;
  virtual QueryDefines::QueryResults getQueryResults(int num, Schema& schema, QSqlQuery& query) const { return {}; }

};

}
