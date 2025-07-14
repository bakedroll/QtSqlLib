#include <QtSqlLib/ColumnStatistics.h>

namespace QtSqlLib
{

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
  return (id >> (sizeof(API::IID::Type) * 8 - 5)) > 0;
}

ColumnStatistics ColumnStatistics::fromId(API::IID::Type id)
{
  return ColumnStatistics(
    static_cast<EType>((id & 0xE0000000) >> (sizeof(API::IID::Type) * 8 - 3)),
    static_cast<EMethod>((id & 0x10000000) >> (sizeof(API::IID::Type) * 8 - 4)),
    static_cast<bool>((id & 0x08000000) >> (sizeof(API::IID::Type) * 8 - 5)),
    id & 0x07FFFFFF);
}

API::IID::Type ColumnStatistics::id() const
{
  return m_columnId
    | static_cast<unsigned char>(m_type) << (sizeof(API::IID::Type) * 8 - 3)
    | static_cast<unsigned char>(m_method) << (sizeof(API::IID::Type) * 8 - 4)
    | m_hasColumn << (sizeof(API::IID::Type) * 8 - 5);
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
