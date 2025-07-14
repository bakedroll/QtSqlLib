#include <QtSqlLib/ColumnStatistics.h>

#include <QtSqlLib/DatabaseException.h>

#include <bitset>
#include <cstddef>
#include <type_traits>

namespace QtSqlLib
{

static constexpr size_t s_bitsIdType = sizeof(API::IID::Type) * 8;

ColumnStatistics ColumnStatistics::min(API::IID::Type columnId)
{
  return ColumnStatistics(EType::Min, EMethod::All, true, columnId);
}

ColumnStatistics ColumnStatistics::max(API::IID::Type columnId)
{
  return ColumnStatistics(EType::Max, EMethod::All, true, columnId);
}

ColumnStatistics ColumnStatistics::sum(API::IID::Type columnId, EMethod method)
{
  return ColumnStatistics(EType::Sum, method, true, columnId);
}

ColumnStatistics ColumnStatistics::count(API::IID::Type columnId, EMethod method)
{
  return ColumnStatistics(EType::Count, method, true, columnId);
}

ColumnStatistics ColumnStatistics::count(EMethod method)
{
  return ColumnStatistics(EType::Count, method, false, 0);
}

ColumnStatistics ColumnStatistics::avg(API::IID::Type columnId, EMethod method)
{
  return ColumnStatistics(EType::Avg, method, true, columnId);
}

bool ColumnStatistics::isColumnStatistics(API::IID::Type id)
{
  return (static_cast<std::make_unsigned_t<API::IID::Type>>(id) >> (sizeof(API::IID::Type) * 8 - 5)) > 0;
}

ColumnStatistics ColumnStatistics::fromId(API::IID::Type id)
{
  constexpr auto bitsColumnId = s_bitsIdType - 5;
  std::bitset<bitsColumnId> columnIdBitmask;
  columnIdBitmask.set();


  return ColumnStatistics(
    static_cast<EType>((id & (0x1C << bitsColumnId)) >> (s_bitsIdType - 3)),
    static_cast<EMethod>((id & (0x02 << bitsColumnId)) >> (s_bitsIdType - 4)),
    static_cast<bool>((id & (0x01 << bitsColumnId)) >> (s_bitsIdType - 5)),
    id & columnIdBitmask.to_ulong());
}

QString ColumnStatistics::toString(EType type, EMethod method, const std::optional<QString>& column)
{
  QString expression = column.has_value()
    ? QString("%1 %2").arg(method == EMethod::All ? "ALL" : "DISTINCT").arg(column.value())
    : "*";

  switch (type)
  {
  case EType::Min:
    return QString("MIN(%1)").arg(expression);
  case EType::Max:
    return QString("MAX(%1)").arg(expression);
  case EType::Sum:
    return QString("SUM(%1)").arg(expression);
  case EType::Count:
    return QString("COUNT(%1)").arg(expression);
  case EType::Avg:
    return QString("AVG(%1)").arg(expression);
  default:
    break;
  }

  throw DatabaseException(DatabaseException::Type::UnexpectedError, "Unknown column statistics type");
}

API::IID::Type ColumnStatistics::id() const
{
  return m_columnId
    | static_cast<unsigned char>(m_type) << (s_bitsIdType - 3)
    | static_cast<unsigned char>(m_method) << (s_bitsIdType - 4)
    | m_hasColumn << (s_bitsIdType - 5);
}

ColumnStatistics::EType ColumnStatistics::type() const
{
  return m_type;
}

ColumnStatistics::EMethod ColumnStatistics::method() const
{
  return m_method;
}

bool ColumnStatistics::hasColumn() const
{
  return m_hasColumn;
}

API::IID::Type ColumnStatistics::columnId() const
{
  return m_columnId;
}

ColumnStatistics::ColumnStatistics(EType type, EMethod method, bool hasColumn, API::IID::Type columnId) :
  m_type(type),
  m_method(method),
  m_hasColumn(hasColumn),
  m_columnId(columnId)
{
}

}
