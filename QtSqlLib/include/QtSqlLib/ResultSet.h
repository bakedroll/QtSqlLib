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
    API::IID::Type tableId,
    QSqlQuery&& query,
    API::QueryMetaInfo&& queryMetaInfo,
    std::vector<API::JoinMetaInfo>&& joinMetaInfo);

  virtual ~ResultSet();

  bool hasNextTuple();
  bool hasNextJoinedTuple();

  TupleView nextTuple();
  TupleView nextJoinedTuple();

private:
  API::IID::Type m_tableId;
  QSqlQuery m_sqlQuery;
  API::QueryMetaInfo m_queryMetaInfo;
  std::vector<API::JoinMetaInfo> m_joinMetaInfo;

};

}
