#pragma once

#include <QSqlQuery>

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/SchemaTypes.h>
#include <QtSqlLib/PrimaryKey.h>
#include <QtSqlLib/TupleView.h>

#include <optional>
#include <set>
#include <vector>

namespace QtSqlLib
{

class ResultSet
{
public:
  ResultSet(
    QSqlQuery&& query,
    API::QueryMetaInfo&& queryMetaInfo,
    std::vector<API::QueryMetaInfo>&& joinMetaInfo);

  ResultSet();

  ResultSet(const ResultSet& rhs) = delete;
  ResultSet& operator=(const ResultSet& rhs) = delete;

  ResultSet(ResultSet&& rhs);
  ResultSet& operator=(ResultSet&& rhs);

  virtual ~ResultSet();

  bool isValid() const;
  void resetIteration();

  bool hasNextTuple();
  bool hasNextJoinedTuple();

  TupleView nextTuple();
  TupleView nextJoinedTuple();

private:
  struct NextTupleResult
  {
    bool hasNext = false;
    bool hasNextJoin = false;
    std::vector<bool> nextJoinsMask;
  };

  enum class SearchMode
  {
    MAIN_TUPLE,
    JOIN_TUPLE
  };

  QSqlQuery m_sqlQuery;
  API::QueryMetaInfo m_queryMetaInfo;
  std::vector<API::QueryMetaInfo> m_joinMetaInfo;

  bool m_isValid;
  NextTupleResult m_nextTupleResult;
  std::set<PrimaryKey> m_retrievedResultKeys;
  std::map<std::pair<API::IID::Type, PrimaryKey>, std::set<PrimaryKey>> m_retrievedJoinResultKeys;

  void searchNextTuple(SearchMode searchMode);
  void findNextJoinTuple(const PrimaryKey& tupleKey);

  void resetNextTupleResult();
  void clearNextJoinsMask();

};

}
