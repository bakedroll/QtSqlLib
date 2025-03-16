#pragma once

#include <QtSqlLib/API/IID.h>

#include <type_traits>
#include <vector>

namespace QtSqlLib
{

class ColumnList
{
public:
  ColumnList();
  explicit ColumnList(const std::initializer_list<API::IID::Type>& init);
  explicit ColumnList(const std::vector<API::IID::Type>& init);
  virtual ~ColumnList();

  const std::vector<API::IID::Type>& cdata() const;
  std::vector<API::IID::Type>& data();

  template <typename T, typename... Args>
  static ColumnList make(const T& value, Args... args)
  {
    const auto size = sizeof...(Args) + 1;
    ColumnList list(size);
    makeInternal(list.m_data, value, args...);
    return list;
  }

  template <typename T>
  static ColumnList make(const T& value)
  {
    ColumnList list(1);
    makeInternal(list.m_data, value);
    return list;
  }

private:
  explicit ColumnList(size_t size);

  std::vector<API::IID::Type> m_data;

  template <typename T, typename... Args>
  static void makeInternal(std::vector<API::IID::Type>& list, const T& value, Args... args)
  {
    const auto index = list.size() - (sizeof...(Args) + 1);
    list[index] = castId(value);
    makeInternal(list, args...);
  }

  template <typename T>
  static void makeInternal(std::vector<API::IID::Type>& list, const T& value)
  {
    list[list.size() - 1] = castId(value);
  }

  template <typename T>
  static API::IID::Type castId(const T& value)
  {
    return static_cast<API::IID::Type>(static_cast<typename std::underlying_type<T>::type>(value));
  }

};

}
