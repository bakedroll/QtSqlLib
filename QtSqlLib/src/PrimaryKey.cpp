#include "QtSqlLib/PrimaryKey.h"

#include "QtSqlLib/DatabaseException.h"

#include <algorithm>

namespace QtSqlLib
{

static bool qVariantsLess(const QVariant& lhs, const QVariant& rhs)
{
  if (lhs.userType() != rhs.userType())
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Cannot compare tuple values of different types.");
  }

  switch (lhs.userType())
  {
  case QMetaType::Int:
    return lhs.toInt() < rhs.toInt();
  case QMetaType::UInt:
    return lhs.toUInt() < rhs.toUInt();
  case QMetaType::LongLong:
    return lhs.toLongLong() < rhs.toLongLong();
  case QMetaType::ULongLong:
    return lhs.toULongLong() < rhs.toULongLong();
  case QMetaType::QString:
    return lhs.toString() < rhs.toString();
  case QMetaType::QByteArray:
    return lhs.toByteArray() < rhs.toByteArray();
  case QMetaType::Float:
    return lhs.toFloat() < rhs.toFloat();
  case QMetaType::Double:
    return lhs.toDouble() < rhs.toDouble();
  default:
    break;
  }

  throw DatabaseException(DatabaseException::Type::UnexpectedError, "Tuple types not comparable.");
}

PrimaryKey::PrimaryKey(
    API::IID::Type tableId,
    std::vector<ColumnValue>&& values) :
  m_tableId(tableId),
  m_values(std::move(values))
{
}

PrimaryKey::PrimaryKey() :
  m_tableId(-1)
{
}

PrimaryKey::~PrimaryKey() = default;

API::IID::Type PrimaryKey::tableId() const
{
  return m_tableId;
}

const std::vector<PrimaryKey::ColumnValue>& PrimaryKey::values() const
{
  return m_values;
}

bool PrimaryKey::hasValueIntern(const API::IID& columnId) const
{
  return std::any_of(m_values.cbegin(), m_values.cend(), [&columnId](const ColumnValue& value){
    return value.columnId == columnId.get();
  });
}

QVariant PrimaryKey::valueIntern(const API::IID& columnId) const
{
  const auto it = std::find_if(m_values.cbegin(), m_values.cend(), [&columnId](const ColumnValue& value){
    return value.columnId == columnId.get();
  });
  return it != m_values.cend() ? it->value : QVariant();
}

bool PrimaryKey::isNull() const
{
  return std::none_of(m_values.cbegin(), m_values.cend(), [](const ColumnValue& value){
    return !value.value.isNull();
  });
}

bool PrimaryKey::operator<(const PrimaryKey& rhs) const
{
  if (m_values.size() != rhs.m_values.size())
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError,
      "Cannot compare primary keys due to incompatibility.");
  }

  if (m_tableId < rhs.m_tableId)
  {
    return true;
  }
  return std::lexicographical_compare(
    m_values.cbegin(), m_values.cend(), rhs.m_values.cbegin(), rhs.m_values.cend(), [](const ColumnValue& lhs, const ColumnValue& rhs)
  {
    if (lhs.columnId != rhs.columnId)
    {
      throw DatabaseException(DatabaseException::Type::UnexpectedError,
        "Cannot compare primary keys due to incompatibility.");
    }
    return qVariantsLess(lhs.value, rhs.value);
  });
}

bool PrimaryKey::operator>(const PrimaryKey& rhs) const
{
  return rhs < *this;
}

bool PrimaryKey::operator<=(const PrimaryKey& rhs) const
{
  return (*this < rhs) || (*this == rhs);
}

bool PrimaryKey::operator>=(const PrimaryKey& rhs) const
{
  return (*this > rhs) || (*this == rhs);
}

bool PrimaryKey::operator==(const PrimaryKey& rhs) const
{
  return !(*this != rhs);
}

bool PrimaryKey::operator!=(const PrimaryKey& rhs) const
{
  return (*this < rhs) || (rhs < *this);
}

}