#pragma once

#include "Schema.h"

#include <QVariant>
#include <QSqlQuery>

#include <vector>

namespace QtSqlLib
{

class QueryDefines
{
public:
  QueryDefines() = delete;

  using ColumnResultMap = std::map<std::pair<Schema::Id, Schema::Id>, QVariant>;
  using QueryResults = std::vector<ColumnResultMap>;

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
};

}
