#pragma once

#include <QtSqlLib/IQuery.h>
#include <QtSqlLib/IQuerySequence.hpp>

#include <memory>
#include <vector>

namespace QtSqlLib
{

class QuerySequence : public IQuerySequence
{
public:
  int getNumQueries() const override;

  QueryDefines::SqlQuery getSqlQuery(int num, Schema& schema) override;
  QueryDefines::QueryResults getQueryResults(int num, Schema& schema, QSqlQuery& query) const  override;

  void addQuery(std::unique_ptr<IQuery> query);

private:
  std::vector<std::unique_ptr<IQuery>> m_queries;

};

}
