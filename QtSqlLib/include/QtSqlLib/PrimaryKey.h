#pragma once

#include <QtSqlLib/ID.h>

#include <QVariant>

#include <vector>

namespace QtSqlLib
{

class PrimaryKey
{
public:
  struct ColumnValue
  {
    API::IID::Type columnId = -1;
    QVariant value;
  };

  explicit PrimaryKey(
    API::IID::Type tableId,
    std::vector<ColumnValue>&& values);

  PrimaryKey();

  virtual ~PrimaryKey();

  API::IID::Type tableId() const;
  const std::vector<ColumnValue>& values() const;

  template <typename T>
  bool hasValue(const T& columnId) const
  {
    return hasValueIntern(QtSqlLib::ID<T>(columnId));
  }

  template <typename T>
  QVariant value(const T& columnId) const
  {
    return valueIntern(QtSqlLib::ID<T>(columnId));
  }

  bool isNull() const;

  bool operator<(const PrimaryKey& rhs) const;
  bool operator>(const PrimaryKey& rhs) const;
  bool operator<=(const PrimaryKey& rhs) const;
  bool operator>=(const PrimaryKey& rhs) const;
  bool operator==(const PrimaryKey& rhs) const;
  bool operator!=(const PrimaryKey& rhs) const;

private:
  API::IID::Type m_tableId;
  std::vector<ColumnValue> m_values;

  bool hasValueIntern(const API::IID& columnId) const;
  QVariant valueIntern(const API::IID& columnId) const;

};

}
