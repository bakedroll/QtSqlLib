#pragma once

#include "QtSqlLib/API/IIndexConfigurator.h"

#include "QtSqlLib/API/SchemaTypes.h"

namespace QtSqlLib
{

class IndexConfigurator : public API::IIndexConfigurator
{
public:
  IndexConfigurator(API::Index& index);
  ~IndexConfigurator() override;

  IIndexConfigurator& columns(const std::vector<API::IID::Type>& columnIds) override;
  IIndexConfigurator& unique() override;

private:
  API::Index& m_index;

  bool m_columnsConfigured;
  bool m_uniqueConfigured;

};

}
