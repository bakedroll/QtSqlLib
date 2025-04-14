#pragma once

#include <QtSqlLib/API/IID.h>

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
  // TODO: template
  QVariant value(const API::IID& columnId) const;
  bool isNull() const;

  bool operator<(const PrimaryKey& rhs) const;
  bool operator>(const PrimaryKey& rhs) const;
  bool operator<=(const PrimaryKey& rhs) const;
  bool operator>=(const PrimaryKey& rhs) const;
  bool operator==(const PrimaryKey& rhs) const;
  bool operator!=(const PrimaryKey& rhs) const;

private:
  // TODO: tableId needed?
  API::IID::Type m_tableId;
  std::vector<ColumnValue> m_values;

};

}
