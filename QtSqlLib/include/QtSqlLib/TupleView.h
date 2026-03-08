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

  template <typename T>
  bool hasAttributeValue(const T& attributeId) const
  {
    return hasAttributeValueIntern(QtSqlLib::ID<T>(attributeId));
  }

  template <typename T>
  QVariant attributeValue(const T& attributeId) const
  {
    return attributeValueIntern(QtSqlLib::ID<T>(attributeId));
  }

  QVariant columnValueAtIndex(size_t index) const;

private:
  int m_queryPos;
  const QSqlQuery& m_sqlQuery;
  const API::QueryMetaInfo& m_queryMetaInfo;

  bool hasColumnValueIntern(const API::IID& columnId) const;
  QVariant columnValueIntern(const API::IID& columnId) const;

  bool hasAttributeValueIntern(const API::IID& attributeId) const;
  QVariant attributeValueIntern(const API::IID& attributeId) const;

  void throwIfInvalidated() const;

};

}
