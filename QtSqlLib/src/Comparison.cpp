#include "Comparison.h"

#include "QtSqlLib/API/IQueryIdentifiers.h"
#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/API/ISchema.h"

namespace QtSqlLib
{

Comparison::Comparison(EComparisonOperator op, const QVariant& lhs, const QVariant& rhs, bool noCase) :
  ITermElement(),
  m_noCase(noCase),
  m_operator(op),
  m_lhs(lhs),
  m_rhs(rhs)
{
}

Comparison::~Comparison() = default;

QString Comparison::toQueryString(
  API::ISchema& schema,
  const API::IQueryIdentifiers& queryIdentifiers,
  std::vector<QVariant>& boundValuesOut) const
{
  const auto getOperandString = [&schema, &queryIdentifiers, &boundValuesOut](const QVariant& operand) -> QString
  {
    if (operand.canConvert<ColumnHelper::ColumnData>())
    {
      const auto columnData = operand.value<ColumnHelper::ColumnData>();
      return queryIdentifiers.resolveColumnIdentifier(schema, columnData);
    }
    if (operand.isNull())
    {
      return "NULL";
    }

    boundValuesOut.emplace_back(operand);
    return "?";
  };

  QString operatorStr;
  switch (m_operator)
  {
  case EComparisonOperator::Equal:
    operatorStr = "=";
    break;
  case EComparisonOperator::Unequal:
    operatorStr = "!=";
    break;
  case EComparisonOperator::LessEqual:
    operatorStr = "<=";
    break;
  case EComparisonOperator::Less:
    operatorStr = "<";
    break;
  case EComparisonOperator::GreaterEqual:
    operatorStr = ">=";
    break;
  case EComparisonOperator::Greater:
    operatorStr = ">";
    break;
  case EComparisonOperator::IsNull:
    operatorStr = "IS";
    break;
  default:
    assert(false);
    break;
  }

  return QString("%1 %2 %3%4").arg(getOperandString(m_lhs)).arg(operatorStr).arg(getOperandString(m_rhs))
    .arg(m_noCase ? " COLLATE NOCASE" : "");
}

}
