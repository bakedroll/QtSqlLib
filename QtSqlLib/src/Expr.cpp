#include "QtSqlLib/Expr.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/API/ISchema.h"
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
  const API::IQueryIdentifiers& queryIdentifiers,
  std::vector<QVariant>& boundValuesOut) const
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

    result.append(term->toQueryString(schema, queryIdentifiers, boundValuesOut));
  }

  return result;
}

Expr& Expr::addComparison(EComparisonOperator op, const QVariant& lhs, const QVariant& rhs, bool noCase)
{
  if (m_nextExpectation != NextTermExpectation::ComparisonOrNestedExpr)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "Comparison not expected");
  }

  m_termElements.emplace_back(std::make_unique<Comparison>(op, lhs, rhs, noCase));

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

QVariant Expr::makeVariant(ColumnHelper::ColumnData&& data)
{
  return QVariant::fromValue<ColumnHelper::ColumnData>(std::forward<ColumnHelper::ColumnData>(data));
}

QVariant Expr::makeVariant(QVariant&& value)
{
  return std::forward<QVariant>(value);
}

QVariant Expr::makeVariant(const ColumnHelper::ColumnData& data)
{
  return QVariant::fromValue<ColumnHelper::ColumnData>(data);
}

QVariant Expr::makeVariant(const QVariant& value)
{
  return value;
}

}
