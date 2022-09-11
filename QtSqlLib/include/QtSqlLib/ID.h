#pragma once

#include <type_traits>

#include <QtSqlLib/API/IID.h>

namespace QtSqlLib
{

template <typename TEnum>
class ID : public API::IID
{
public:
  static_assert(std::is_enum<TEnum>::value,
    "TEnum must be an enum type");

  ID(TEnum id)
    : m_id(static_cast<int>(static_cast<typename std::underlying_type<TEnum>::type>(id)))
  {
  }

  int get() const override
  {
    return m_id;
  }

  bool operator<(const IID& rhs) const override
  {
    return m_id < rhs.get();
  }

private:
  int m_id;

};

template <>
class ID<int> : public API::IID
{
public:
  ID(int id)
    : m_id(id)
  {
  }

  int get() const override
  {
    return m_id;
  }

  bool operator<(const IID& rhs) const override
  {
    return m_id < rhs.get();
  }

private:
  int m_id;

};

}
