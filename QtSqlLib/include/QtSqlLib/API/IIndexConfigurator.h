#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/ColumnHelper.h>

#include <vector>

namespace QtSqlLib::API
{

class IIndexConfigurator
{
public:
  virtual ~IIndexConfigurator() = default;

  virtual IIndexConfigurator& columns(const ColumnHelper::ColumnList& columns) = 0;
  virtual IIndexConfigurator& unique() = 0;

};

}
