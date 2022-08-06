#include "QtSqlLib/Expr.h"

#include "QtSqlLib/DatabaseException.h"

#include <utilsLib/Utils.h>

namespace QtSqlLib
{

Expr::ColumnId::ColumnId()
  : ColumnId("", {0U, 0U}, false)
{
}

Expr::ColumnId::ColumnId(const Schema::TableColumnId& tableColumnId)
  : ColumnId("", tableColumnId, true)
{
}

Expr::ColumnId::ColumnId(Schema::Id columnId)
  : ColumnId("", {0U, columnId}, false)
{
}

Expr::ColumnId::ColumnId(const QString& tableAlias, const Schema::TableColumnId& tableColumnId)
  : ColumnId(tableAlias, tableColumnId, true)
{
}

Expr::ColumnId::~ColumnId() = default;

const Schema::TableColumnId& Expr::ColumnId::get() const
{
  return m_tableColumnId;
}

bool Expr::ColumnId::isTableIdValid() const
{
  return m_bIsTableIdValid;
}

QString Expr::ColumnId::getTableAlias() const
{
  return m_tableAlias;
}

Expr::ColumnId::ColumnId(const QString& tableAlias, const Schema::TableColumnId& tableColumnId, bool bIsTableIdValid) :
  m_tableAlias(tableAlias),
  m_tableColumnId(tableColumnId),
  m_bIsTableIdValid(bIsTableIdValid)
{
}

Expr::Expr()
  : m_nextExpectation(NextTermExpectation::ComparisonOrNestedExpr)
{
}

Expr::Expr(Expr&& other) noexcept = default;

Expr::~Expr() = default;

Expr& Expr::equal(const ColumnId& columnId, const QVariant& value)
{
  return addComparison(ComparisonOperator::Equal, columnId, value);
}

Expr& Expr::equal(const ColumnId& colIdLhs, const ColumnId& colIdRhs)
{
  return addComparison(ComparisonOperator::Equal, colIdLhs, colIdRhs);
}

Expr& Expr::unequal(const ColumnId& columnId, const QVariant& value)
{
  return addComparison(ComparisonOperator::Unequal, columnId, value);
}

Expr& Expr::unequal(const ColumnId& colIdLhs, const ColumnId& colIdRhs)
{
  return addComparison(ComparisonOperator::Unequal, colIdLhs, colIdRhs);
}

Expr& Expr::lessEqual(const ColumnId& columnId, const QVariant& value)
{
  return addComparison(ComparisonOperator::LessEqual, columnId, value);
}

Expr& Expr::lessEqual(const ColumnId& colIdLhs, const ColumnId& colIdRhs)
{
  return addComparison(ComparisonOperator::LessEqual, colIdLhs, colIdRhs);
}

Expr& Expr::less(const ColumnId& columnId, const QVariant& value)
{
  return addComparison(ComparisonOperator::Less, columnId, value);
}

Expr& Expr::less(const ColumnId& colIdLhs, const ColumnId& colIdRhs)
{
  return addComparison(ComparisonOperator::Less, colIdLhs, colIdRhs);
}

Expr& Expr::greaterEqual(const ColumnId& columnId, const QVariant& value)
{
  return addComparison(ComparisonOperator::GreaterEqual, columnId, value);
}

Expr& Expr::greaterEqual(const ColumnId& colIdLhs, const ColumnId& colIdRhs)
{
  return addComparison(ComparisonOperator::GreaterEqual, colIdLhs, colIdRhs);
}

Expr& Expr::greater(const ColumnId& columnId, const QVariant& value)
{
  return addComparison(ComparisonOperator::Greater, columnId, value);
}

Expr& Expr::greater(const ColumnId& colIdLhs, const ColumnId& colIdRhs)
{
  return addComparison(ComparisonOperator::Greater, colIdLhs, colIdRhs);
}

Expr& Expr::isNull(const ColumnId& columnId)
{
  return addComparison(ComparisonOperator::IsNull, columnId, ColumnId());
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
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "Nested Expression not expected");
  }

  m_termElements.emplace_back(std::make_unique<NestedExpression>(nestedExpr));

  m_nextExpectation = NextTermExpectation::LogicalOperator;
  return *this;
}

QString Expr::toQString(Schema& schema, const std::optional<Schema::Id>& defaultTableId) const
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

QString Expr::Comparison::toQString(Schema& schema, const std::optional<Schema::Id>& defaultTableId = std::nullopt) const
{
  const auto getOperandString = [&schema, defaultTableId](const Operand& operand) -> QString
  {
    switch (operand.type)
    {
    case OperandType::ColumnId:
    {
      const auto columnId = operand.value.value<ColumnId>();
      if (!defaultTableId.has_value() && !columnId.isTableIdValid())
      {
        return "NULL";
      }
      else if (defaultTableId && !columnId.isTableIdValid())
      {
        schema.throwIfTableIdNotExisting(*defaultTableId);
      }

      const auto colId = columnId.get().columnId;
      const auto& table = schema.getTables().at(columnId.isTableIdValid()
        ? columnId.get().tableId
        : *defaultTableId);

      schema.throwIfColumnIdNotExisting(table, colId);

      const auto tableAlias = columnId.getTableAlias();

      return QString("'%1'.'%2'").arg(tableAlias.isEmpty() ? table.name : tableAlias).arg(table.columns.at(colId).name);
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
  case ComparisonOperator::IsNull:
    operatorStr = "is";
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

QString Expr::NestedExpression::toQString(Schema& schema, const std::optional<Schema::Id>& defaultTableId) const
{
  return QString("(%1)").arg(m_nestedExpr->toQString(schema, defaultTableId));
}

Expr::Logic::Logic(LogicalOperator op)
  : m_operator(op)
{
}

Expr::Logic::~Logic() = default;

QString Expr::Logic::toQString(Schema& schema, const std::optional<Schema::Id>& defaultTableId) const
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

Expr& Expr::addComparison(ComparisonOperator op, const ColumnId& colIdLhs, const ColumnId& colIdRhs)
{
  return addComparison(std::make_unique<Comparison>(op,
    Operand { OperandType::ColumnId, QVariant::fromValue(colIdLhs) },
    Operand { OperandType::ColumnId, QVariant::fromValue(colIdRhs) }));
}

Expr& Expr::addComparison(ComparisonOperator op, const ColumnId& colIdLhs, const QVariant& value)
{
  return addComparison(std::make_unique<Comparison>(op,
    Operand { OperandType::ColumnId, QVariant::fromValue(colIdLhs) },
    Operand { OperandType::Value, value }));
}

Expr& Expr::addComparison(std::unique_ptr<Comparison>&& comparison)
{
  if (m_nextExpectation != NextTermExpectation::ComparisonOrNestedExpr)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "Comparison not expected");
  }

  m_termElements.emplace_back(std::move(comparison));

  m_nextExpectation = NextTermExpectation::LogicalOperator;
  return *this;
}

Expr& Expr::addLogic(std::unique_ptr<Logic>&& logic)
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
