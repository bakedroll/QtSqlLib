#pragma once

#include <type_traits>

#include <QtSqlLib/API/IID.h>

#include <QtSqlLib/ColumnStatistics.h>

namespace QtSqlLib
{

template <typename TEnum>
class ID : public API::IID
{
public:
  static_assert(std::is_enum<TEnum>::value,
    "TEnum must be an enum type or underlying ID type");

  ID(TEnum id)
    : m_id(static_cast<Type>(static_cast<typename std::underlying_type<TEnum>::type>(id)))
  {
  }

  Type get() const override
  {
    return m_id;
  }

  bool operator<(const IID& rhs) const override
  {
    return m_id < rhs.get();
  }

private:
  Type m_id;

};

template <>
class ID<API::IID::Type> : public API::IID
{
public:
  ID(Type id)
    : m_id(id)
  {
  }

  Type get() const override
  {
    return m_id;
  }

  bool operator<(const IID& rhs) const override
  {
    return m_id < rhs.get();
  }

private:
  Type m_id;

};

// TODO: remove?
template <>
class ID<ColumnStatistics> : public API::IID
{
public:
  ID(const ColumnStatistics& statistics)
    : m_statistics(statistics)
  {
  }

  Type get() const override
  {
    return m_statistics.id();
  }

  bool operator<(const IID& rhs) const override
  {
    return m_statistics.id() < rhs.get();
  }

private:
  ColumnStatistics m_statistics;

};

}
