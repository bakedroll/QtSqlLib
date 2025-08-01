#pragma once

#include "ITermElement.h"

#include "QtSqlLib/LogicalOperator.h"

namespace QtSqlLib
{

class Logic : public ITermElement
{
public:
  explicit Logic(LogicalOperator op);
  ~Logic() override;

  QString toQueryString(
    API::ISchema& schema,
    const API::IQueryIdentifiers& queryIdentifiers,
    std::vector<QVariant>& boundValuesOut) const override;

private:
  LogicalOperator m_operator;

};

}
