#pragma once

#include <QSqlQuery>

#include <QtSqlLib/API/SchemaTypes.h>
#include <QtSqlLib/TupleView.h>

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

  virtual ~ResultSet();

  bool hasNextTuple();
  bool hasNextJoinedTuple();

  TupleView nextTuple();
  TupleView nextJoinedTuple();

private:
  QSqlQuery m_sqlQuery;
  API::QueryMetaInfo m_queryMetaInfo;
  std::vector<API::QueryMetaInfo> m_joinMetaInfo;

};

}
