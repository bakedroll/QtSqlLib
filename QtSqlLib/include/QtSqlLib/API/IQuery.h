#pragma once

#include <QtSqlLib/Schema.h>

#include <QSqlDatabase>
#include <QSqlQuery>

namespace QtSqlLib::API
{

class IQuery
{
public:
  using TupleValuesList = std::vector<Schema::TupleValues>;

  struct ResultTuple
  {
    Schema::TupleValues values;
    std::map<Schema::Id, TupleValuesList> joinedTuples;
  };

  struct QueryResults
  {
    using ResultTuples = std::vector<ResultTuple>;

    enum class Validity
    {
      Valid,
      Invalid
    };

    Validity validity = Validity::Invalid;
    ResultTuples resultTuples;
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

  virtual SqlQuery getSqlQuery(const QSqlDatabase& db, Schema& schema, QueryResults& previousQueryResults) = 0;
  virtual QueryResults getQueryResults(Schema& schema, QSqlQuery& query) const { return {}; }

};

}
