#pragma once

#include <QtSqlLib/SchemaConfigurator.h>

#include <QSqlQuery>
#include <QVariant>

#include <vector>
#include <map>

namespace QtSqlLib
{

class IQuery
{
public:
  using ColumnResultMap = std::map<std::pair<unsigned int, unsigned int>, QVariant>;
  using QueryResults = std::vector<ColumnResultMap>;

  IQuery() = default;
  virtual ~IQuery() = default;

  IQuery(const IQuery& other) = delete;
  IQuery& operator= (const IQuery& other) = delete;

  virtual QSqlQuery getSqlQuery(const SchemaConfigurator::Schema& schema) const = 0;
  virtual QueryResults getQueryResults(const SchemaConfigurator::Schema& schema, QSqlQuery& query) const { return {}; }

  virtual bool isBatchExecution() const { return false; }

};

}
