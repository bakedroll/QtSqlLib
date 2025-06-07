#pragma once

#include <QtSqlLib/ID.h>
#include <QtSqlLib/PrimaryKey.h>

#include <QSqlQuery>

#include "API/SchemaTypes.h"

#include <optional>

namespace QtSqlLib
{

class TupleView
{
public:
  explicit TupleView(
    const QSqlQuery& sqlQuery,
    const API::QueryMetaInfo& queryMetaInfo);

  virtual ~TupleView();

  API::IID::Type tableId() const;
  std::optional<API::IID::Type> relationshipId() const;

  PrimaryKey primaryKey() const;

  template <typename T>
  bool hasColumnValue(const T& columnId) const
  {
    return hasColumnValueIntern(QtSqlLib::ID<T>(columnId));
  }

  template <typename T>
  QVariant columnValue(const T& columnId) const
  {
    return columnValueIntern(QtSqlLib::ID<T>(columnId));
  }

private:
  int m_queryPos;
  const QSqlQuery& m_sqlQuery;
  const API::QueryMetaInfo& m_queryMetaInfo;

  bool hasColumnValueIntern(const API::IID& columnId) const;
  QVariant columnValueIntern(const API::IID& columnId) const;

  void throwIfInvalidated() const;

};

}
