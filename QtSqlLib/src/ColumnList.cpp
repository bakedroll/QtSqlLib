#include "QtSqlLib/ColumnList.h"

namespace QtSqlLib
{

ColumnList::ColumnList() = default;

ColumnList::ColumnList(const std::initializer_list<API::IID::Type>& init) :
  m_data(init)
{
}

ColumnList::~ColumnList() = default;

const std::vector<API::IID::Type>& ColumnList::cdata() const
{
  return m_data;
}

std::vector<API::IID::Type>& ColumnList::data()
{
  return m_data;
}

ColumnList::ColumnList(size_t size) :
  m_data(size)
{
}

}
