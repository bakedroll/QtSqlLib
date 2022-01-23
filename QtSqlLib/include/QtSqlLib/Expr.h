#pragma once

#include "QtSqlLib/SchemaConfigurator.h"

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

  Expr& equal(unsigned int columnId, const QVariant& value);
  Expr& equal(unsigned int colIdLhs, unsigned int colIdRhs);

  Expr& unequal(unsigned int columnId, const QVariant& value);
  Expr& unequal(unsigned int colIdLhs, unsigned int colIdRhs);

  Expr& lessEqual(unsigned int columnId, const QVariant& value);
  Expr& lessEqual(unsigned int colIdLhs, unsigned int colIdRhs);

  Expr& less(unsigned int columnId, const QVariant& value);
  Expr& less(unsigned int colIdLhs, unsigned int colIdRhs);

  Expr& greaterEqual(unsigned int columnId, const QVariant& value);
  Expr& greaterEqual(unsigned int colIdLhs, unsigned int colIdRhs);

  Expr& greater(unsigned int columnId, const QVariant& value);
  Expr& greater(unsigned int colIdLhs, unsigned int colIdRhs);

  Expr& or();
  Expr& and();

  Expr& braces(Expr& nestedExpr);

  QString toQString(const SchemaConfigurator::Schema& schema, unsigned int defaultTableId) const;

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
    virtual QString toQString(const SchemaConfigurator::Schema& schema, unsigned int defaultTableId) const = 0;

  };

  class Comparison : public ITermElement
  {
  public:
    Comparison(ComparisonOperator op, const Operand& lhs, const Operand& rhs);
    ~Comparison() override;

    QString toQString(const SchemaConfigurator::Schema& schema, unsigned int defaultTableId) const override;

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

    QString toQString(const SchemaConfigurator::Schema& schema, unsigned int defaultTableId) const override;

  private:
    std::unique_ptr<Expr> m_nestedExpr;

  };

  class Logic : public ITermElement
  {
  public:
    explicit Logic(LogicalOperator op);
    ~Logic() override;

    QString toQString(const SchemaConfigurator::Schema& schema, unsigned int defaultTableId) const override;

  private:
    LogicalOperator m_operator;

  };

  enum class NextTermExpectation
  {
    ComparisonOrNestedExpr,
    LogicalOperator
  };

  Expr& addComparison(ComparisonOperator op, unsigned int colIdLhs, unsigned int colIdRhs);
  Expr& addComparison(ComparisonOperator op, unsigned int colIdLhs, const QVariant& value);

  Expr& addComparison(std::unique_ptr<Comparison>&& comparison);

  Expr& addLogic(std::unique_ptr<Logic>&& logic);

  NextTermExpectation m_nextExpectation;
  std::vector<std::unique_ptr<ITermElement>> m_termElements;

};

}
