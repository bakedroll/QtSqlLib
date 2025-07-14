#pragma once

#include "ITermElement.h"

#include "QtSqlLib/EComparisonOperator.h"

#include <QVariant>

namespace QtSqlLib
{

class Comparison : public ITermElement
{
public:
  Comparison(EComparisonOperator op, const QVariant& lhs, const QVariant& rhs, bool noCase = false);
  ~Comparison() override;

  QString toQueryString(
    API::ISchema& schema,
    const API::IQueryIdentifiers& queryIdentifiers,
    std::vector<QVariant>& boundValuesOut) const override;

private:
  bool m_noCase;
  EComparisonOperator m_operator;
  QVariant m_lhs;
  QVariant m_rhs;

};

}
