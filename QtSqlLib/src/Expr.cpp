#include "QtSqlLib/Expr.h"

#include "QtSqlLib/DatabaseException.h"

#include <utilsLib/Utils.h>

namespace QtSqlLib
{

Expr::Expr()
  : m_nextExpectation(NextTermExpectation::ComparisonOrNestedExpr)
{
}

Expr::Expr(Expr&& other) noexcept = default;

Expr::~Expr() = default;

Expr& Expr::equal(unsigned int columnId, const QVariant& value)
{
  return addComparison(ComparisonOperator::Equal, columnId, value);
}

Expr& Expr::equal(unsigned int colIdLhs, unsigned int colIdRhs)
{
  return addComparison(ComparisonOperator::Equal, colIdLhs, colIdRhs);
}

Expr& Expr::unequal(unsigned int columnId, const QVariant& value)
{
  return addComparison(ComparisonOperator::Unequal, columnId, value);
}

Expr& Expr::unequal(unsigned int colIdLhs, unsigned int colIdRhs)
{
  return addComparison(ComparisonOperator::Unequal, colIdLhs, colIdRhs);
}

Expr& Expr::lessEqual(unsigned int columnId, const QVariant& value)
{
  return addComparison(ComparisonOperator::LessEqual, columnId, value);
}

Expr& Expr::lessEqual(unsigned int colIdLhs, unsigned int colIdRhs)
{
  return addComparison(ComparisonOperator::LessEqual, colIdLhs, colIdRhs);
}

Expr& Expr::less(unsigned int columnId, const QVariant& value)
{
  return addComparison(ComparisonOperator::Less, columnId, value);
}

Expr& Expr::less(unsigned int colIdLhs, unsigned int colIdRhs)
{
  return addComparison(ComparisonOperator::Less, colIdLhs, colIdRhs);
}

Expr& Expr::greaterEqual(unsigned int columnId, const QVariant& value)
{
  return addComparison(ComparisonOperator::GreaterEqual, columnId, value);
}

Expr& Expr::greaterEqual(unsigned int colIdLhs, unsigned int colIdRhs)
{
  return addComparison(ComparisonOperator::GreaterEqual, colIdLhs, colIdRhs);
}

Expr& Expr::greater(unsigned int columnId, const QVariant& value)
{
  return addComparison(ComparisonOperator::Greater, columnId, value);
}

Expr& Expr::greater(unsigned int colIdLhs, unsigned int colIdRhs)
{
  return addComparison(ComparisonOperator::Greater, colIdLhs, colIdRhs);
}

Expr& Expr::or()
{
  return addLogic(std::make_unique<Logic>(LogicalOperator::Or));
}

Expr& Expr::and()
{
  return addLogic(std::make_unique<Logic>(LogicalOperator::And));
}

Expr& Expr::braces(Expr& nestedExpr)
{
  if (m_nextExpectation != NextTermExpectation::ComparisonOrNestedExpr)
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery, "Nested Expression not expected");
  }

  m_termElements.emplace_back(std::make_unique<NestedExpression>(nestedExpr));

  m_nextExpectation = NextTermExpectation::LogicalOperator;
  return *this;
}

QString Expr::toQString(const SchemaConfigurator::Schema& schema, unsigned int defaultTableId) const
{
  if (m_termElements.size() == 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery, "Invalid Expression: Expression must not be empty");
  }

  if (m_nextExpectation == NextTermExpectation::ComparisonOrNestedExpr)
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery, "Invalid Expression: Expression must not end with a logical operator");
  }

  QString result;
  for (const auto& term : m_termElements)
  {
    if (!result.isEmpty())
    {
      result.append(" ");
    }

    result.append(term->toQString(schema, defaultTableId));
  }

  return result;
}

Expr::Comparison::Comparison(ComparisonOperator op, const Operand& lhs, const Operand& rhs)
  : ITermElement()
  , m_operator(op)
  , m_lhs(lhs)
  , m_rhs(rhs)
{
}

Expr::Comparison::~Comparison() = default;

QString Expr::Comparison::toQString(const SchemaConfigurator::Schema& schema, unsigned int defaultTableId) const
{
  const auto getOperandString = [&schema, defaultTableId](const Operand& operand) -> QString
  {
    switch (operand.type)
    {
    case OperandType::Attribute:
    {
      if (schema.tables.count(defaultTableId) == 0)
      {
        throw DatabaseException(DatabaseException::Type::InvalidQuery, QString("Invalid Expression: Unknown table id %1").arg(defaultTableId));
      }

      const auto colId = operand.value.toUInt();
      const auto& table = schema.tables.at(defaultTableId);
      if (table.columns.count(colId) == 0)
      {
        throw DatabaseException(DatabaseException::Type::InvalidQuery, QString("Invalid Expression: Unknown column id %1 for table id %2").arg(colId).arg(defaultTableId));
      }

      return QString("'%1'.%2").arg(table.name).arg(table.columns.at(colId).name);
    }
    case OperandType::Value:
    {
      if (operand.value.userType() == QMetaType::QString)
      {
        return QString("'%1'").arg(operand.value.toString());
      }

      return operand.value.toString();
    }
    default:
      assert(false);
      break;
    }

    return "";
  };

  QString operatorStr;
  switch (m_operator)
  {
  case ComparisonOperator::Equal:
    operatorStr = "==";
    break;
  case ComparisonOperator::Unequal:
    operatorStr = "!=";
    break;
  case ComparisonOperator::LessEqual:
    operatorStr = "<=";
    break;
  case ComparisonOperator::Less:
    operatorStr = "<";
    break;
  case ComparisonOperator::GreaterEqual:
    operatorStr = ">=";
    break;
  case ComparisonOperator::Greater:
    operatorStr = ">";
    break;
  default:
    assert(false);
    break;
  }

  return QString("%1 %2 %3").arg(getOperandString(m_lhs)).arg(operatorStr).arg(getOperandString(m_rhs));
}

Expr::NestedExpression::NestedExpression(Expr& expr)
  : m_nestedExpr(std::make_unique<Expr>(std::move(expr)))
{
}

Expr::NestedExpression::~NestedExpression() = default;

QString Expr::NestedExpression::toQString(const SchemaConfigurator::Schema& schema, unsigned int defaultTableId) const
{
  return QString("(%1)").arg(m_nestedExpr->toQString(schema, defaultTableId));
}

Expr::Logic::Logic(LogicalOperator op)
  : m_operator(op)
{
}

Expr::Logic::~Logic() = default;

QString Expr::Logic::toQString(const SchemaConfigurator::Schema& schema, unsigned int defaultTableId) const
{
  switch (m_operator)
  {
  case LogicalOperator::And:
    return "AND";
  case LogicalOperator::Or:
    return "OR";
  default:
    break;
  }

  assert(false);
  return "";
}

Expr& Expr::addComparison(ComparisonOperator op, unsigned int colIdLhs, unsigned int colIdRhs)
{
  return addComparison(std::make_unique<Comparison>(op,
    Operand { OperandType::Attribute, colIdLhs },
    Operand { OperandType::Attribute, colIdRhs }));
}

Expr& Expr::addComparison(ComparisonOperator op, unsigned int colIdLhs, const QVariant& value)
{
  return addComparison(std::make_unique<Comparison>(op,
    Operand { OperandType::Attribute, colIdLhs },
    Operand { OperandType::Value, value }));
}

Expr& Expr::addComparison(std::unique_ptr<Comparison>&& comparison)
{
  if (m_nextExpectation != NextTermExpectation::ComparisonOrNestedExpr)
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery, "Comparison not expected");
  }

  m_termElements.emplace_back(std::move(comparison));

  m_nextExpectation = NextTermExpectation::LogicalOperator;
  return *this;
}

Expr& Expr::addLogic(std::unique_ptr<Logic>&& logic)
{
  if (m_nextExpectation != NextTermExpectation::LogicalOperator)
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery, "Logical operator not expected");
  }

  m_termElements.emplace_back(std::move(logic));

  m_nextExpectation = NextTermExpectation::ComparisonOrNestedExpr;
  return *this;
}

}
