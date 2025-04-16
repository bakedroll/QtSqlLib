#include "QtSqlLib/ResultSet.h"

namespace QtSqlLib
{

ResultSet::ResultSet(
    API::IID::Type tableId,
    QSqlQuery&& query,
    API::QueryMetaInfo&& queryMetaInfo,
    std::vector<API::JoinMetaInfo>&& joinMetaInfo) :
  m_tableId(tableId),
  m_sqlQuery(query),
  m_queryMetaInfo(std::move(queryMetaInfo)),
  m_joinMetaInfo(std::move(joinMetaInfo))
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

}

TupleView ResultSet::nextJoinedTuple()
{

}

}
