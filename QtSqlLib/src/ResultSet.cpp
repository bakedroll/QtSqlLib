#include "QtSqlLib/ResultSet.h"

#include <stdexcept>

namespace QtSqlLib
{

ResultSet::ResultSet(
    QSqlQuery&& query,
    API::QueryMetaInfo&& queryMetaInfo,
    std::vector<API::QueryMetaInfo>&& joinMetaInfo) :
  m_sqlQuery(std::move(query)),
  m_queryMetaInfo(std::move(queryMetaInfo)),
  m_joinMetaInfo(std::move(joinMetaInfo))
{
}

ResultSet::ResultSet(ResultSet&& rhs) :
  m_sqlQuery(std::move(rhs.m_sqlQuery)),
  m_queryMetaInfo(std::move(rhs.m_queryMetaInfo)),
  m_joinMetaInfo(std::move(rhs.m_joinMetaInfo))
{
}

ResultSet::~ResultSet() = default;

bool ResultSet::hasNextTuple()
{
  return false;
}

bool ResultSet::hasNextJoinedTuple()
{
  return false;
}

TupleView ResultSet::nextTuple()
{
  throw std::runtime_error("not yet implemented");
}

TupleView ResultSet::nextJoinedTuple()
{
  throw std::runtime_error("not yet implemented");
}

}
