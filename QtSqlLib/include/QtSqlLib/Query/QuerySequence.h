#pragma once

#include <QtSqlLib/API/IQuery.h>
#include <QtSqlLib/API/IQuerySequence.h>

#include <memory>
#include <vector>

namespace QtSqlLib::Query
{

class QuerySequence : public API::IQuerySequence
{
public:
  int getNumQueries() const final;
  void prepare(Schema& schema) override;

  QueryDefines::SqlQuery getSqlQuery(int num, Schema& schema) override;
  QueryDefines::QueryResults getQueryResults(int num, Schema& schema, QSqlQuery& query) const  override;

  void addQuery(std::unique_ptr<API::IQuery> query);
  void addQuery(std::unique_ptr<IQuerySequence> sequence);

  API::IQuery& getQuery(int index);

private:
  enum class QueryType
  {
    SingleQuery,
    Sequence
  };

  std::vector<QueryType> m_queries;
  std::map<int, int> m_queryNumSingleQueryMap;
  std::map<int, std::pair<int, int>> m_queryNumQuerySequenceMap;

  std::map<int, std::unique_ptr<API::IQuery>> m_singleQueriesMap;
  std::map<int, std::unique_ptr<IQuerySequence>> m_querySequencesMap;

};

}
