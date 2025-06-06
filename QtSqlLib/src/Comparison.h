#pragma once

#include "ITermElement.h"

#include "QtSqlLib/ComparisonOperator.h"

#include <QVariant>

namespace QtSqlLib
{

class Comparison : public ITermElement
{
public:
  enum class OperandType
  {
    ColumnId,
    Value
  };

  struct Operand
  {
    OperandType type;
    QVariant value;
  };

  Comparison(ComparisonOperator op, const Operand& lhs, const Operand& rhs, bool noCase = false);
  ~Comparison() override;

  QString toQueryString(
    API::ISchema& schema,
    std::vector<QVariant>& boundValuesOut,
    const OptionalIID& defaultTableId) const override;

private:
  bool m_noCase;
  ComparisonOperator m_operator;
  Operand m_lhs;
  Operand m_rhs;

};

}
