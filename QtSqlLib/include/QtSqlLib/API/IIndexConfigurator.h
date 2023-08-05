#pragma once

#include <QtSqlLib/API/IID.h>

#include <vector>

namespace QtSqlLib::API
{

class IIndexConfigurator
{
public:
  virtual ~IIndexConfigurator() = default;

  virtual IIndexConfigurator& columns(const std::vector<API::IID::Type>& columnIds) = 0;
  virtual IIndexConfigurator& unique() = 0;

};

}
