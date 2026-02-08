#include "Comparison.h"

#include "QtSqlLib/API/IQueryIdentifiers.h"
#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/API/ISchema.h"

namespace QtSqlLib
{

Comparison::Comparison(EComparisonOperator op, const QVariant& lhs, const QVariant& rhs) :
  ITermElement(),
  m_noCase(false),
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
    if (operand.canConvert<ColumnHelper::ColumnAlias>())
    {
      return QString("[%1]").arg(operand.value<ColumnHelper::ColumnAlias>().alias);
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
  case EComparisonOperator::Is:
    operatorStr = "IS";
    break;
  case EComparisonOperator::Not:
    operatorStr = "NOT";
    break;
  case EComparisonOperator::Like:
    operatorStr = "LIKE";
    break;
  case EComparisonOperator::In:
    operatorStr = "IN";
    break;
  default:
    assert(false);
    break;
  }

  return QString("%1 %2 %3%4").arg(getOperandString(m_lhs)).arg(operatorStr).arg(getOperandString(m_rhs))
    .arg(m_noCase ? " COLLATE NOCASE" : "");
}

void Comparison::setNoCase(bool noCase)
{
  m_noCase = noCase;
}

}
