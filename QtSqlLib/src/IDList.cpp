#include "QtSqlLib/IDList.h"

namespace QtSqlLib
{

std::vector<API::IID::Type> IDList::make(const API::IID& value)
{
  std::vector<API::IID::Type> list;
  return makeInternal(list, value);
}

std::vector<API::IID::Type> IDList::makeInternal(std::vector<API::IID::Type>& list, const API::IID& value)
{
  list.emplace_back(value.get());
  return list;
}

}
