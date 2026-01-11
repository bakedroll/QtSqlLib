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
  return ColumnStatistics(EType::Min, EMethod::All, true, columnId, std::nullopt);
}

ColumnStatistics ColumnStatistics::max(API::IID::Type columnId)
{
  return ColumnStatistics(EType::Max, EMethod::All, true, columnId, std::nullopt);
}

ColumnStatistics ColumnStatistics::sum(API::IID::Type columnId, EMethod method)
{
  return ColumnStatistics(EType::Sum, method, true, columnId, std::nullopt);
}

ColumnStatistics ColumnStatistics::count(API::IID::Type columnId, EMethod method)
{
  return ColumnStatistics(EType::Count, method, true, columnId, std::nullopt);
}

ColumnStatistics ColumnStatistics::count(EMethod method)
{
  return ColumnStatistics(EType::Count, method, false, 0, std::nullopt);
}

ColumnStatistics ColumnStatistics::avg(API::IID::Type columnId, EMethod method)
{
  return ColumnStatistics(EType::Avg, method, true, columnId, std::nullopt);
}

ColumnStatistics ColumnStatistics::groupConcat(API::IID::Type columnId, const std::optional<char>& separator)
{
  return ColumnStatistics(EType::GroupConcat, EMethod::All, true, columnId, separator);
}

bool ColumnStatistics::isColumnStatistics(API::IID::Type id)
{
  return (static_cast<std::make_unsigned_t<API::IID::Type>>(id) >> (sizeof(API::IID::Type) * 8 - 11)) > 0;
}

ColumnStatistics ColumnStatistics::fromId(API::IID::Type id)
{
  constexpr auto bitsColumnId = s_bitsIdType - 11;
  std::bitset<bitsColumnId> columnIdBitmask;
  columnIdBitmask.set();

  const auto unsignedId = static_cast<std::make_unsigned_t<API::IID::Type>>(id);
  const auto type = static_cast<EType>((unsignedId & (0x700 << bitsColumnId)) >> (s_bitsIdType - 3));
  if (type == EType::GroupConcat)
  {
    char separator = static_cast<char>((unsignedId & (0x0FF << bitsColumnId)) >> (s_bitsIdType - 11));
    return ColumnStatistics(
      type,
      EMethod::All,
      true,
      unsignedId & columnIdBitmask.to_ulong(),
      separator > 0 ? std::make_optional<char>(separator) : std::nullopt);
  }

  return ColumnStatistics(
    type,
    static_cast<EMethod>((unsignedId & (0x080 << bitsColumnId)) >> (s_bitsIdType - 4)),
    static_cast<bool>((unsignedId & (0x040 << bitsColumnId)) >> (s_bitsIdType - 5)),
    unsignedId & columnIdBitmask.to_ulong(),
    std::nullopt);
}

QString ColumnStatistics::toString(
  EType type, EMethod method, const std::optional<char>& separator, const std::optional<QString>& column)
{
  if (type == EType::GroupConcat)
  {
    if (!column.has_value())
    {
      throw DatabaseException(DatabaseException::Type::UnexpectedError, "GROUP_CONCAT() must specify a valid column.");
    }

    return QString("GROUP_CONCAT(%1%2)")
      .arg(column.value())
      .arg(separator.has_value() ? std::string(", '").append(1, separator.value()).append("'").c_str() : "");
  }

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
  if (m_type == EType::GroupConcat)
  {
    char separator = m_groupConcatSeparator.has_value() ? m_groupConcatSeparator.value() : 0;
    return m_columnId
      | static_cast<unsigned char>(m_type) << (s_bitsIdType - 3)
      | static_cast<unsigned char>(separator) << (s_bitsIdType - 11);
  }

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

std::optional<char> ColumnStatistics::separator() const
{
  return m_groupConcatSeparator;
}

ColumnStatistics::ColumnStatistics(EType type, EMethod method, bool hasColumn, API::IID::Type columnId, const std::optional<char>& separator) :
  m_type(type),
  m_method(method),
  m_hasColumn(hasColumn),
  m_columnId(columnId),
  m_groupConcatSeparator(separator)
{
}

}
