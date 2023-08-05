#pragma once

#include <QtSqlLib/API/ISchema.h>
#include <QtSqlLib/ResultSet.h>

#include <QSqlQuery>

namespace QtSqlLib::API
{

class IQuery
{
public:
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

  virtual SqlQuery getSqlQuery(const QSqlDatabase& db, ISchema& schema, const ResultSet& previousQueryResults) = 0;
  virtual ResultSet getQueryResults(ISchema& schema, QSqlQuery& query) const { return ResultSet::invalid(); }

};

}
