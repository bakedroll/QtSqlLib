#pragma once

#include "QtSqlLib/Schema.h"

#include <QVariant>

namespace QtSqlLib
{

class Expr
{
public:
  Expr();
  Expr(Expr&& other) noexcept;

  Expr(const Expr& other) = delete;
  Expr& operator=(const Expr& other) = delete;

  virtual ~Expr();

  Expr& equal(Schema::Id columnId, const QVariant& value);
  Expr& equal(Schema::Id colIdLhs, Schema::Id colIdRhs);

  Expr& unequal(Schema::Id columnId, const QVariant& value);
  Expr& unequal(Schema::Id colIdLhs, Schema::Id colIdRhs);

  Expr& lessEqual(Schema::Id columnId, const QVariant& value);
  Expr& lessEqual(Schema::Id colIdLhs, Schema::Id colIdRhs);

  Expr& less(Schema::Id columnId, const QVariant& value);
  Expr& less(Schema::Id colIdLhs, Schema::Id colIdRhs);

  Expr& greaterEqual(Schema::Id columnId, const QVariant& value);
  Expr& greaterEqual(Schema::Id colIdLhs, Schema::Id colIdRhs);

  Expr& greater(Schema::Id columnId, const QVariant& value);
  Expr& greater(Schema::Id colIdLhs, Schema::Id colIdRhs);

  Expr& or();
  Expr& and();

  Expr& braces(Expr& nestedExpr);

  QString toQString(Schema& schema, Schema::Id defaultTableId) const;

private:
  enum class ComparisonOperator
  {
    Equal,
    Unequal,
    LessEqual,
    Less,
    GreaterEqual,
    Greater
  };

  enum class OperandType
  {
    Attribute,
    Value
  };

  enum class LogicalOperator
  {
    And,
    Or
  };

  struct Operand
  {
    OperandType type;
    QVariant value;
  };

  class ITermElement
  {
  public:
    ITermElement() = default;
    virtual ~ITermElement() = default;
    virtual QString toQString(Schema& schema, Schema::Id defaultTableId) const = 0;

  };

  class Comparison : public ITermElement
  {
  public:
    Comparison(ComparisonOperator op, const Operand& lhs, const Operand& rhs);
    ~Comparison() override;

    QString toQString(Schema& schema, Schema::Id defaultTableId) const override;

  private:
    ComparisonOperator m_operator;
    Operand m_lhs;
    Operand m_rhs;

  };

  class NestedExpression : public ITermElement
  {
  public:
    explicit NestedExpression(Expr& expr);
    ~NestedExpression() override;

    QString toQString(Schema& schema, Schema::Id defaultTableId) const override;

  private:
    std::unique_ptr<Expr> m_nestedExpr;

  };

  class Logic : public ITermElement
  {
  public:
    explicit Logic(LogicalOperator op);
    ~Logic() override;

    QString toQString(Schema& schema, Schema::Id defaultTableId) const override;

  private:
    LogicalOperator m_operator;

  };

  enum class NextTermExpectation
  {
    ComparisonOrNestedExpr,
    LogicalOperator
  };

  Expr& addComparison(ComparisonOperator op, Schema::Id colIdLhs, Schema::Id colIdRhs);
  Expr& addComparison(ComparisonOperator op, Schema::Id colIdLhs, const QVariant& value);

  Expr& addComparison(std::unique_ptr<Comparison>&& comparison);

  Expr& addLogic(std::unique_ptr<Logic>&& logic);

  NextTermExpectation m_nextExpectation;
  std::vector<std::unique_ptr<ITermElement>> m_termElements;

};

}
