#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/PrimaryKey.h>

#include <QSqlQuery>

#include "API/SchemaTypes.h"

namespace QtSqlLib
{

class TupleView
{
public:
  explicit TupleView(
    const QSqlQuery& sqlQuery,
    const API::QueryMetaInfo& queryMetaInfo,
    API::IID::Type relationshipId = -1);

  virtual ~TupleView();

  API::IID::Type tableId() const;
  API::IID::Type relationshipId() const;

  PrimaryKey primaryKey() const;
  // TOOD: implementation needed?
  //const std::vector<QVariant>& values() const;

  // TODO: use template for IDs
  QVariant columnValue(const API::IID& columnId) const;

private:
  const QSqlQuery& m_sqlQuery;
  const API::QueryMetaInfo& m_queryMetaInfo;
  API::IID::Type m_relationshipId;

};

}
