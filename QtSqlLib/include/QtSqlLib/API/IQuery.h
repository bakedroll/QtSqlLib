#pragma once

#include <QtSqlLib/API/ISchema.h>

#include <QSqlQuery>

namespace QtSqlLib::API
{

class IQuery
{
public:
  using TupleValuesList = std::vector<ISchema::TupleValues>;

  struct ResultTuple
  {
    ISchema::TupleValues values;
    std::map<ISchema::Id, TupleValuesList> joinedTuples;
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

  virtual SqlQuery getSqlQuery(const QSqlDatabase& db, ISchema& schema, QueryResults& previousQueryResults) = 0;
  virtual QueryResults getQueryResults(ISchema& schema, QSqlQuery& query) const { return {}; }

};

}
