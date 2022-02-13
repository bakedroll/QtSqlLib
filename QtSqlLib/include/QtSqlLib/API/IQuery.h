#pragma once

#include <QtSqlLib/Schema.h>

#include <QSqlQuery>

namespace QtSqlLib::API
{

class IQuery
{
public:
  using QueryResults = std::vector<Schema::TableColumnValuesMap>;

  enum class QueryMode
  {
    Single,
    Batch
  };

  struct SqlQuery
  {
    QSqlQuery qtQuery;
    QueryMode mode = QueryMode::Single;
  };

  IQuery() = default;
  virtual ~IQuery() = default;

  IQuery(const IQuery& other) = delete;
  IQuery& operator= (const IQuery& other) = delete;

  virtual SqlQuery getSqlQuery(Schema& schema) = 0;
  virtual QueryResults getQueryResults(Schema& schema, QSqlQuery& query) const { return {}; }

};

}
