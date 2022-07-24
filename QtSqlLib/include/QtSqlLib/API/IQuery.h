#pragma once

#include <QtSqlLib/Schema.h>

#include <QSqlQuery>

namespace QtSqlLib::API
{

class IQuery
{
public:
  struct QueryResults
  {
    using Values = std::vector<Schema::TupleValues>;

    enum class Validity
    {
      Valid,
      Invalid
    };

    Validity validity = Validity::Invalid;
    Values values;
  };

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

  virtual SqlQuery getSqlQuery(Schema& schema, QueryResults& previousQueryResults) = 0;
  virtual QueryResults getQueryResults(Schema& schema, QSqlQuery& query) const { return {}; }

};

}
