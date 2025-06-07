#include "QtSqlLib/Expr.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/API/ISchema.h"
#include "QtSqlLib/ColumnID.h"
#include "QtSqlLib/DatabaseException.h"

#include "Comparison.h"
#include "Logic.h"
#include "NestedExpression.h"

namespace QtSqlLib
{

Expr::Expr()
  : m_nextExpectation(NextTermExpectation::ComparisonOrNestedExpr)
{
}

Expr::Expr(Expr&& other) noexcept = default;

Expr::~Expr() = default;

Expr& Expr::equal(const ColumnID& columnId, const QVariant& value, bool noCase)
{
  return addComparison(ComparisonOperator::Equal, columnId, value, noCase);
}

Expr& Expr::unequal(const ColumnID& columnId, const QVariant& value, bool noCase)
{
  return addComparison(ComparisonOperator::Unequal, columnId, value, noCase);
}

Expr& Expr::lessEqual(const ColumnID& columnId, const QVariant& value, bool noCase)
{
  return addComparison(ComparisonOperator::LessEqual, columnId, value, noCase);
}

Expr& Expr::less(const ColumnID& columnId, const QVariant& value, bool noCase)
{
  return addComparison(ComparisonOperator::Less, columnId, value, noCase);
}

Expr& Expr::greaterEqual(const ColumnID& columnId, const QVariant& value, bool noCase)
{
  return addComparison(ComparisonOperator::GreaterEqual, columnId, value, noCase);
}

Expr& Expr::greater(const ColumnID& columnId, const QVariant& value, bool noCase)
{
  return addComparison(ComparisonOperator::Greater, columnId, value, noCase);
}

Expr& Expr::isNull(const ColumnID& columnId)
{
  return addComparison(ComparisonOperator::IsNull, columnId, QVariant::fromValue(ColumnID()), false);
}

Expr& Expr::opOr()
{
  return addLogic(std::make_unique<Logic>(LogicalOperator::Or));
}

Expr& Expr::opAnd()
{
  return addLogic(std::make_unique<Logic>(LogicalOperator::And));
}

Expr& Expr::braces(Expr& nestedExpr)
{
  if (m_nextExpectation != NextTermExpectation::ComparisonOrNestedExpr)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "Nested Expression not expected");
  }

  m_termElements.emplace_back(std::make_unique<NestedExpression>(nestedExpr));

  m_nextExpectation = NextTermExpectation::LogicalOperator;
  return *this;
}

QString Expr::toQueryString(
  API::ISchema& schema,
  std::vector<QVariant>& boundValuesOut,
  const OptionalIID& defaultTableId) const
{
  if (m_termElements.size() == 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "Expression must not be empty");
  }

  if (m_nextExpectation == NextTermExpectation::ComparisonOrNestedExpr)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "Expression must not end with a logical operator");
  }

  QString result;
  for (const auto& term : m_termElements)
  {
    if (!result.isEmpty())
    {
      result.append(" ");
    }

    result.append(term->toQueryString(schema, boundValuesOut, defaultTableId));
  }

  return result;
}

Expr& Expr::addComparison(ComparisonOperator op, const ColumnID& colIdLhs, const QVariant& value, bool noCase)
{
  return addComparison(std::make_unique<Comparison>(op,
    Comparison::Operand { Comparison::OperandType::ColumnId, QVariant::fromValue(colIdLhs) },
    Comparison::Operand { value.canConvert<ColumnID>() ? Comparison::OperandType::ColumnId : Comparison::OperandType::Value, value },
    noCase));
}

Expr& Expr::addComparison(std::unique_ptr<ITermElement>&& comparison)
{
  if (m_nextExpectation != NextTermExpectation::ComparisonOrNestedExpr)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "Comparison not expected");
  }

  m_termElements.emplace_back(std::move(comparison));

  m_nextExpectation = NextTermExpectation::LogicalOperator;
  return *this;
}

Expr& Expr::addLogic(std::unique_ptr<ITermElement>&& logic)
{
  if (m_nextExpectation != NextTermExpectation::LogicalOperator)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "Logical operator not expected");
  }

  m_termElements.emplace_back(std::move(logic));

  m_nextExpectation = NextTermExpectation::ComparisonOrNestedExpr;
  return *this;
}

}
