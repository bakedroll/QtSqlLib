#pragma once

#include <QtSqlLib/API/IID.h>

#include <vector>

#define IDS(...) QtSqlLib::IDList::make(__VA_ARGS__)

namespace QtSqlLib
{

class IDList
{
public:
  IDList() = delete;

  template <typename... Args>
  static std::vector<API::IID::Type> make(const API::IID& value, Args... args)
  {
    std::vector<API::IID::Type> list;
    return makeInternal(list, value, args...);
  }

  static std::vector<API::IID::Type> make(const API::IID& value);

private:
  template <typename... Args>
  static std::vector<API::IID::Type> makeInternal(std::vector<API::IID::Type>& list, const API::IID& value, Args... args)
  {
    list.emplace_back(value.get());
    return makeInternal(list, args...);
  }

  static std::vector<API::IID::Type> makeInternal(std::vector<API::IID::Type>& list, const API::IID& value);

};

}