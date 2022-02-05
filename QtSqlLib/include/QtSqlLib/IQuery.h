#pragma once

#include <QtSqlLib/Schema.h>

#include <QSqlQuery>
#include <QVariant>

#include <vector>
#include <map>

namespace QtSqlLib
{

class IQuery
{
public:
  using ColumnResultMap = std::map<std::pair<Schema::Id, Schema::Id>, QVariant>;
  using QueryResults = std::vector<ColumnResultMap>;

  IQuery() = default;
  virtual ~IQuery() = default;

  IQuery(const IQuery& other) = delete;
  IQuery& operator= (const IQuery& other) = delete;

  virtual QSqlQuery getSqlQuery(Schema& schema) = 0;
  virtual QueryResults getQueryResults(Schema& schema, QSqlQuery& query) const { return {}; }

  virtual bool isBatchExecution() const { return false; }

};

}
