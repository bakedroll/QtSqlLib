#pragma once

#include "Schema.h"

#include <QSqlQuery>

#include <vector>

namespace QtSqlLib
{

class QueryDefines
{
public:
  QueryDefines() = delete;

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
};

}
